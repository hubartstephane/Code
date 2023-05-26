#include "chaos/ChaosPCH.h"
#include "chaos/ChaosInternals.h"

namespace chaos
{
	//
	// WindowApplication
	//

	WindowApplication::WindowApplication(SubClassOf<Window> in_main_window_class, WindowCreateParams const& in_window_create_params):
		main_window_class(in_main_window_class),
		window_create_params(in_window_create_params)
	{
		assert(in_main_window_class.IsValid());
	}

	void WindowApplication::FreezeNextFrameTickDuration()
	{
		forced_zero_tick_duration = true;
	}

	bool WindowApplication::IsWindowHandledByApplication(Window const* window) const
	{
		auto it = std::ranges::find_if(windows, [window](shared_ptr<Window> const& w)
		{
			return (w == window);
		});
		return (it != windows.end());
	}

	void WindowApplication::DestroyWindow(Window* window)
	{
		assert(window != nullptr);

		auto it = std::ranges::find_if(windows, [window](shared_ptr<Window> const& w)
		{
			return (w == window);
		});

		if (it != windows.end())
		{
			shared_ptr<Window> prevent_destruction = window;
			windows.erase(it);
			if (window->GetWindowDestructionGuard() == 0) // can destroy immediatly the window or must wait until no current operation ?
				OnWindowDestroyed(window);
		}
	}

	void WindowApplication::RunMessageLoop(std::function<bool()> const & loop_condition_func)
	{
		double t1 = glfwGetTime();

		while (!loop_condition_func || loop_condition_func())
		{
			glfwPollEvents();

			double t2 = glfwGetTime();
			float delta_time = (float)(t2 - t1);
			float real_delta_time = delta_time;

			if (forced_zero_tick_duration) // a single frame with a zero delta_time (used whenever a long operation is beeing done during one frame and we don't want to have big dt)
			{
				delta_time = 0.0f;
				forced_zero_tick_duration = false;
			}
			else
			{
				if (forced_tick_duration > 0.0f)
					delta_time = forced_tick_duration;
				else if (max_tick_duration > 0.0f)
					delta_time = std::min(delta_time, max_tick_duration);
			}
			// internal tick
			bool tick_result = WithGLFWContext(shared_context, [this, delta_time]()
			{
				return Tick(delta_time);
			});
			if (!tick_result) // quit the loop if the current tick method requires so
			{
				return;
			}

			// destroy windows that mean to be
			for (weak_ptr<Window>& window : GetWeakWindowArray())
				if (window != nullptr)
					if (window->ShouldClose())
						window->Destroy();

			// tick the windows
			for (weak_ptr<Window> & window : GetWeakWindowArray())
			{
				if (window != nullptr)
				{
					window->WithWindowContext([&window, delta_time, real_delta_time]()
					{
						window->TickRenderer(real_delta_time);
						window->Tick(delta_time);
						window->DrawWindow();
					});
					}
			}
			// update time
			t1 = t2;
		}
	}

	void WindowApplication::DestroyAllWindows()
	{
		for (weak_ptr<Window> window : GetWeakWindowArray())
			if (window != nullptr)
				window->Destroy();
	}

	Window* WindowApplication::CreateTypedWindow(SubClassOf<Window> window_class, WindowCreateParams create_params, ObjectRequest request)
	{
		if (FindWindow(request) != nullptr)
		{
			Log::Error("WindowApplication::CreateTypedWindow(...) name already used");
			return nullptr;
		}

		return WithGLFWContext(nullptr, [this, window_class, &create_params, &request]() -> Window*
		{
			// create the window class
			shared_ptr<Window> result = window_class.CreateInstance();
			if (result == nullptr)
			{
				return nullptr;
			}
			windows.push_back(result.get());
			// set the name
			result->SetObjectNaming(request);
			// override the create params with the JSON configuration (if any)
			nlohmann::json const* window_configuration = nullptr;
			if (nlohmann::json const* windows_configuration = JSONTools::GetStructure(configuration, "windows"))
			{
				window_configuration = JSONTools::GetStructure(*windows_configuration, result->GetName());
				if (window_configuration != nullptr)
				{
					LoadFromJSON(*window_configuration, create_params);
				}
			}
			// create the GLFW resource
			if (!result->CreateGLFWWindow(create_params, shared_context, glfw_hints))
			{
				result->Destroy();
				return nullptr; // the shared_ptr destruction will handle the object lifetime
			}
			result->CreateImGuiContext();
			// post initialization method
			glfwMakeContextCurrent(result->GetGLFWHandler());
			// finalize the creation
			nlohmann::json default_window_config;
			if (window_configuration == nullptr)
				window_configuration = &default_window_config;
			result->InitializeFromConfiguration(*window_configuration);
			// create the root widget
			result->CreateRootWidget();
			// notify the application
			OnWindowCreated(result.get());
			return result.get();
		});
	}

	void WindowApplication::OnWindowDestroyed(Window* window)
	{
		if (window == main_window)
			main_window = nullptr;
		window->Finalize();
		window->DestroyImGuiContext();
		window->DestroyGLFWWindow();
	}

	void WindowApplication::OnWindowCreated(Window* window)
	{
	}

	bool WindowApplication::Initialize()
	{
		// super method
		if (!Application::Initialize())
			return false;

		// set error callback
		glfwSetErrorCallback(OnGLFWError);

		// the glfw configuration (valid for all windows)
		if (nlohmann::json const* glfw_configuration = JSONTools::GetStructure(configuration, "glfw"))
			LoadFromJSON(*glfw_configuration, glfw_hints);
		glfw_hints.ApplyHints();

		// create a hidden window whose purpose is to provide a sharable context for all others
		glfwWindowHint(GLFW_VISIBLE, 0);
		shared_context = glfwCreateWindow(100, 100, "", nullptr, nullptr);
		if (shared_context == nullptr)
			return false;

		if (!WithGLFWContext(shared_context, [this]()
		{
			// XXX : seems to be mandatory for some functions like : glGenVertexArrays(...)
			//       see https://www.opengl.org/wiki/OpenGL_Loading_Library
			glewExperimental = GL_TRUE;
			// create the context
			GLenum err = glewInit();
			if (err != GLEW_OK)
			{
				Log::Message("glewInit(...) failure : %s", glewGetErrorString(err));
				return false;
			}
			// callback for monitor events
			glfwSetMonitorCallback(DoOnMonitorEvent);
			// set the debug log hander
			GLTools::SetDebugMessageHandler();
			// some generic information
			GLTools::DisplayGenericInformation();
			// initialize the GPU resource ResourceManager (first window/OpenGL context must have been created)
			if (!CreateGPUResourceManager())
				return false;
			return true;
		}))
		{
			return false;
		}

		// a final initialization (after main window is constructed ... and OpenGL context)
		if (!WithGLFWContext(shared_context, [this]()
		{
			return PostOpenGLContextCreation();
		}))
		{
			return false;
		}

		return true;
	}

	int WindowApplication::Main()
	{
		// create the main window
		main_window = CreateMainWindow();
		if (main_window == nullptr)
			return -1;
		// run the main loop as long as there are windows
		RunMessageLoop([this]() { return (windows.size() > 0); });
		return 0;
	}

	Window* WindowApplication::CreateMainWindow()
	{
		WindowCreateParams create_params = window_create_params;

		// gives a title to the window (if necessary)
		std::string title_storage;

		if (StringTools::IsEmpty(create_params.title))
		{
			if (GetArguments().size() > 0)
			{
				title_storage = PathTools::PathToName(GetArguments()[0]);
				create_params.title = title_storage.c_str();
			}
		}
		// create the window
		return CreateTypedWindow(main_window_class, window_create_params, "main_window");
	}

	bool WindowApplication::InitializeGamepadButtonMap()
	{
		// the map [button ID] => [bitmap name + text generator alias]
		gamepad_button_map[GamepadButton::A] = { "XboxOne_A", "ButtonA" };
		gamepad_button_map[GamepadButton::B] = { "XboxOne_B", "ButtonB" };
		gamepad_button_map[GamepadButton::X] = { "XboxOne_X", "ButtonX" };
		gamepad_button_map[GamepadButton::Y] = { "XboxOne_Y", "ButtonY" };

		gamepad_button_map[GamepadButton::DPAD_DOWN] = { "XboxOne_DPad_Down", "DPAD_Down" };
		gamepad_button_map[GamepadButton::DPAD_UP] = { "XboxOne_DPad_Up", "DPAD_Up" };
		gamepad_button_map[GamepadButton::DPAD_LEFT] = { "XboxOne_DPad_Left", "DPAD_Left" };
		gamepad_button_map[GamepadButton::DPAD_RIGHT] = { "XboxOne_DPad_Right", "DPAD_Right" };

		gamepad_button_map[GamepadButton::LEFT_BUMPER] = { "XboxOne_LB", "LeftShoulder" };
		gamepad_button_map[GamepadButton::RIGHT_BUMPER] = { "XboxOne_RB", "RightShoulder" };
		gamepad_button_map[GamepadButton::LEFT_TRIGGER] = { "XboxOne_LT", "LeftTrigger" };
		gamepad_button_map[GamepadButton::RIGHT_TRIGGER] = { "XboxOne_RT", "RightTrigger" };

		gamepad_button_map[GamepadButton::SPECIAL_LEFT] = { "XboxOne_Windows", "SpecialLeft" };
		gamepad_button_map[GamepadButton::SPECIAL_RIGHT] = { "XboxOne_Menu", "SpecialRight" };

		return true;
	}

	bool WindowApplication::FillAtlasGeneratorInput(BitmapAtlas::AtlasInput& input)
	{
		if (!FillAtlasGeneratorInputSprites(input))
			return false;
		if (!FillAtlasGeneratorInputFonts(input))
			return false;
		return true;
	}

	bool WindowApplication::FillAtlasGeneratorInputSprites(BitmapAtlas::AtlasInput& input)
	{
		// get the directory where the sprites are
		std::string sprite_directory;
		if (!JSONTools::GetAttribute(configuration, "sprite_directory", sprite_directory))
			return true;
		// find or create folder
		BitmapAtlas::FolderInfoInput* folder_info = input.AddFolder("sprites", 0);
		if (folder_info == nullptr)
			return false;
		// Add sprites
		folder_info->AddBitmapFilesFromDirectory(sprite_directory, true);

		return true;
	}

	bool WindowApplication::FillAtlasGeneratorInputFonts(BitmapAtlas::AtlasInput& input)
	{
		nlohmann::json const* fonts_config = JSONTools::GetStructure(configuration, "fonts");
		if (fonts_config != nullptr)
		{
			// read the default font parameters
			BitmapAtlas::FontInfoInputParams font_params;

			nlohmann::json const* default_font_param_json = JSONTools::GetStructure(*fonts_config, "default_font_param");
			if (default_font_param_json != nullptr)
				LoadFromJSON(*default_font_param_json, font_params);

			// Add the fonts
			nlohmann::json const* fonts_json = JSONTools::GetStructure(*fonts_config, "fonts");
			if (fonts_json != nullptr && fonts_json->is_object())
			{
				for (nlohmann::json::const_iterator it = fonts_json->begin(); it != fonts_json->end(); ++it)
				{
					if (!it->is_string())
						continue;
					// read information
					std::string const& font_name = it.key();
					std::string font_path = it->get<std::string>();
					if (input.AddFont(font_path.c_str(), nullptr, true, font_name.c_str(), 0, font_params) == nullptr)
						return false;
				}
			}
		}

		return true;
	}

	namespace GlobalVariables
	{
		CHAOS_GLOBAL_VARIABLE(bool, UseCachedAtlas);
#if !_DEBUG
		CHAOS_GLOBAL_VARIABLE(bool, DumpCachedAtlas);
#endif
	};

	bool WindowApplication::CreateTextureAtlas()
	{
		char const* CachedAtlasFilename = "CachedAtlas";

		// Try to load already computed data
		if (GlobalVariables::UseCachedAtlas.Get())
		{
			BitmapAtlas::TextureArrayAtlas* tmp_texture_atlas = new BitmapAtlas::TextureArrayAtlas;
			if (tmp_texture_atlas != nullptr)
			{
				if (tmp_texture_atlas->LoadAtlas(GetUserLocalTempPath() / CachedAtlasFilename))
				{
					texture_atlas = tmp_texture_atlas;
					return true;
				}
				delete(tmp_texture_atlas);
			}
		}

		// fill sub images for atlas generation
		BitmapAtlas::AtlasInput input;
		if (!FillAtlasGeneratorInput(input))
			return false;

		// atlas generation params
		int const DEFAULT_ATLAS_SIZE = 1024;
		int const DEFAULT_ATLAS_PADDING = 10;

		BitmapAtlas::AtlasGeneratorParams params = BitmapAtlas::AtlasGeneratorParams(DEFAULT_ATLAS_SIZE, DEFAULT_ATLAS_SIZE, DEFAULT_ATLAS_PADDING, PixelFormatMergeParams());

		nlohmann::json const* atlas_json = JSONTools::GetStructure(configuration, "atlas");
		if (atlas_json != nullptr)
			LoadFromJSON(*atlas_json, params);

		// atlas generation params : maybe a dump
		char const* dump_atlas_dirname = nullptr;
#if _DEBUG
		dump_atlas_dirname = CachedAtlasFilename;
#else
		if (GlobalVariables::DumpCachedAtlas.Get())
			dump_atlas_dirname = CachedAtlasFilename;
#endif

		// generate the atlas
		BitmapAtlas::TextureArrayAtlasGenerator generator;
		texture_atlas = generator.ComputeResult(input, params, dump_atlas_dirname);
		if (texture_atlas == nullptr)
			return false;

		return true;
	}

	bool WindowApplication::CreateTextGenerator()
	{
		// get the font sub objects
		nlohmann::json const* fonts_config = JSONTools::GetStructure(configuration, "fonts");

		// create the generator
		particle_text_generator = new ParticleTextGenerator::Generator(*texture_atlas);
		if (particle_text_generator == nullptr)
			return false;

		// bitmaps in generator
		BitmapAtlas::FolderInfo const* folder_info = texture_atlas->GetFolderInfo("sprites");
		if (folder_info != nullptr)
		{
			// for each bitmap, that correspond to a button, register a [NAME] in the generator
			for (auto it = gamepad_button_map.begin(); it != gamepad_button_map.end(); ++it)
			{
				std::string const& bitmap_name = it->second.first;
				BitmapAtlas::BitmapInfo const* info = folder_info->GetBitmapInfo(bitmap_name.c_str());
				if (info == nullptr)
					continue;
				std::string const& generator_alias = it->second.second;
				particle_text_generator->AddBitmap(generator_alias.c_str(), info);
			}
			// embedded sprites
			if (fonts_config != nullptr)
			{
				nlohmann::json const* font_bitmaps_json = JSONTools::GetStructure(*fonts_config, "bitmaps");
				if (font_bitmaps_json != nullptr && font_bitmaps_json->is_object())
				{
					for (nlohmann::json::const_iterator it = font_bitmaps_json->begin(); it != font_bitmaps_json->end(); ++it)
					{
						if (!it->is_string())
							continue;
						std::string const & bitmap_name = it.key();
						std::string bitmap_path = it->get<std::string>();
						BitmapAtlas::BitmapInfo const* info = folder_info->GetBitmapInfo(bitmap_path.c_str());
						if (info == nullptr)
							continue;
						particle_text_generator->AddBitmap(bitmap_name.c_str(), info);
					}
				}
			}
		}

		// the colors
		if (fonts_config != nullptr)
		{
			nlohmann::json const* font_colors_json = JSONTools::GetStructure(*fonts_config, "colors");
			if (font_colors_json != nullptr && font_colors_json->is_object())
			{
				for (nlohmann::json::const_iterator it = font_colors_json->begin(); it != font_colors_json->end(); ++it)
				{
					glm::vec4 color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);  // initialization for if input is smaller than 4
					if (!LoadFromJSON(*it, color))
						continue;
					std::string const & color_name = it.key();
					particle_text_generator->AddColor(color_name.c_str(), color);
				}
			}
		}
		return true;
	}

	bool WindowApplication::PostOpenGLContextCreation()
	{
		assert(glfwGetCurrentContext() == shared_context);

		if (!CreateTextureAtlas())
		{
			Log::Error("WindowApplication::CreateTextureAtlas(...) failure");
			return false;
		}
		if (!InitializeGamepadButtonMap())
		{
			Log::Error("WindowApplication::InitializeGamepadButtonMap(...) failure");
			return false;
		}
		if (!CreateTextGenerator())
		{
			Log::Error("WindowApplication::CreateTextGenerator(...) failure");
			return false;
		}
		return true;
	}

	bool WindowApplication::DoTick(float delta_time)
	{
		assert(glfwGetCurrentContext() == shared_context);

		// tick the managers
		if (main_clock != nullptr)
			main_clock->TickClock(delta_time);
		if (sound_manager != nullptr)
			sound_manager->Tick(delta_time);
		// update keyboard state
		UpdateKeyStates(delta_time);
		return true;
	}

	void WindowApplication::OnGLFWError(int code, const char* msg)
	{
		Log::Message("Window(...) [%d] failure : %s", code, msg);
	}

	bool WindowApplication::CreateGPUResourceManager()
	{
		assert(glfwGetCurrentContext() == shared_context);

		// create and start the GPU manager
		gpu_resource_manager = new GPUResourceManager;
		if (gpu_resource_manager == nullptr)
			return false;
		gpu_resource_manager->StartManager();
		// create internal resource
		if (!gpu_resource_manager->InitializeInternalResources())
			return false;
		// get JSON section and load all requested resources
		nlohmann::json const* gpu_config = JSONTools::GetStructure(configuration, "gpu");
		if (gpu_config != nullptr)
			if (!gpu_resource_manager->InitializeFromConfiguration(*gpu_config))
				return false;
		return true;
	}

	bool WindowApplication::ReloadGPUResources()
	{
		return WithGLFWContext(shared_context, [this]()
		{
			// this call may block for too much time
			FreezeNextFrameTickDuration();

			// reload the configuration file
			nlohmann::json config;
			if (!ReloadConfigurationFile(config))
				return false;
			// get the structure of interrest
			nlohmann::json const* gpu_config = JSONTools::GetStructure(config, "gpu");
			if (gpu_config == nullptr)
				return false;
			// create a temporary manager
			shared_ptr<GPUResourceManager> other_gpu_resource_manager = new GPUResourceManager; // destroyed at the end of the function
			if (other_gpu_resource_manager == nullptr)
				return false;
			if (!other_gpu_resource_manager->StartManager())
				return false;
			// reload all resources ... (even unchanged)
			if (other_gpu_resource_manager->InitializeFromConfiguration(*gpu_config))
				gpu_resource_manager->RefreshGPUResources(other_gpu_resource_manager.get());
			other_gpu_resource_manager->StopManager();
			return true;
		});
	}

	void WindowApplication::FinalizeGPUResourceManager()
	{
		// stop the resource manager
		if (gpu_resource_manager != nullptr)
		{
			gpu_resource_manager->StopManager();
			gpu_resource_manager = nullptr;
		}
	}

	bool WindowApplication::InitializeManagers()
	{
		if (!Application::InitializeManagers())
			return false;

		// update some internals
		JSONTools::GetAttribute(configuration, "max_tick_duration", max_tick_duration);
		JSONTools::GetAttribute(configuration, "forced_tick_duration", forced_tick_duration);

		// initialize the clock
		main_clock = new Clock("main_clock");
		if (main_clock == nullptr)
			return false;
		nlohmann::json const* clock_config = JSONTools::GetStructure(configuration, "clocks");
		if (clock_config != nullptr)
			main_clock->InitializeFromConfiguration(*clock_config);

		// initialize the sound manager
		sound_manager = new SoundManager();
		if (sound_manager == nullptr)
			return false;
		sound_manager->StartManager();
		nlohmann::json const* sound_config = JSONTools::GetStructure(configuration, "sounds");
		if (sound_config != nullptr)
			sound_manager->InitializeFromConfiguration(*sound_config);

		return true;
	}

	void WindowApplication::FinalizeManagers()
	{
		// stop the clock
		main_clock = nullptr;
		// stop the sound manager
		if (sound_manager != nullptr)
		{
			sound_manager->StopManager();
			sound_manager = nullptr;
		}
		// stop the resource manager
		if (gpu_resource_manager != nullptr)
		{
			gpu_resource_manager->StopManager();
			gpu_resource_manager = nullptr;
		}
		// super method
		Application::FinalizeManagers();
	}

	void WindowApplication::Finalize()
	{
		// destroy the resources
		FinalizeGPUResourceManager();
		// destroy all windows
		DestroyAllWindows();
		// super
		Application::Finalize();
	}

	void WindowApplication::FreeImageOutputMessageFunc(FREE_IMAGE_FORMAT fif, const char* msg)
	{
		Log::Message("FreeImage warning message [%d][%s]", fif, msg);
	}

	bool WindowApplication::InitializeStandardLibraries()
	{
		if (!Application::InitializeStandardLibraries())
			return false;

		IMGUI_CHECKVERSION();

		FreeImage_Initialise(); // glew will be initialized
		FreeImage_SetOutputMessage(&FreeImageOutputMessageFunc);
		glfwInit();
		return true;
	}

	void WindowApplication::FinalizeStandardLibraries()
	{
		glfwTerminate();
		FreeImage_DeInitialise();
		Application::FinalizeStandardLibraries();
	}

	void WindowApplication::OnInputModeChanged(InputMode new_mode, InputMode old_mode)
	{
		if (main_window != nullptr)
			main_window->OnInputModeChanged(new_mode, old_mode);
	}

	Clock* WindowApplication::GetMainClockInstance()
	{
		WindowApplication* application = GetInstance();
		if (application == nullptr)
			return nullptr;
		return application->GetMainClock();
	}

	Clock const* WindowApplication::GetMainClockConstInstance()
	{
		WindowApplication const* application = GetConstInstance();
		if (application == nullptr)
			return nullptr;
		return application->GetMainClock();
	}

	SoundManager* WindowApplication::GetSoundManagerInstance()
	{
		WindowApplication* application = GetInstance();
		if (application == nullptr)
			return nullptr;
		return application->GetSoundManager();
	}

	SoundManager const* WindowApplication::GetSoundManagerConstInstance()
	{
		WindowApplication const* application = GetConstInstance();
		if (application == nullptr)
			return nullptr;
		return application->GetSoundManager();
	}

	GPUResourceManager* WindowApplication::GetGPUResourceManagerInstance()
	{
		WindowApplication* application = GetInstance();
		if (application == nullptr)
			return nullptr;
		return application->GetGPUResourceManager();
	}

	GPUResourceManager const* WindowApplication::GetGPUResourceManagerConstInstance()
	{
		WindowApplication const* application = GetConstInstance();
		if (application == nullptr)
			return nullptr;
		return application->GetGPUResourceManager();
	}

#if _DEBUG
	CHAOS_HELP_TEXT(SHORTCUTS, "F7  : ToggleImGuiMenuMode");
	CHAOS_HELP_TEXT(SHORTCUTS, "F8  : ReloadGPUResources");
#endif

	bool WindowApplication::OnKeyEventImpl(KeyEvent const& event)
	{
		
#if _DEBUG
		if (event.IsKeyPressed(GLFW_KEY_F7))
		{
			SetImGuiMenuMode(!GetImGuiMenuMode());
			return true;
		}
		// reloading GPU resources
		if (event.IsKeyPressed(GLFW_KEY_F8))
		{
			// CMD GLFW_KEY_F8 : ReloadGPUResources(...)
			ReloadGPUResources();
			return true;
		}
#endif
		return Application::OnKeyEventImpl(event);
	}

	bool WindowApplication::GetApplicationKeyState(Key key, bool previous_frame)
	{
		WindowApplication* application = Application::GetInstance();
		if (application != nullptr)
			return application->GetKeyState(key, previous_frame);
		return false;
	}

	void WindowApplication::SetKeyboardButtonState(KeyboardButton key, int action)
	{
		int raw_value = int(key);
		if (raw_value >= 0 && raw_value < keyboard_state.size())
			keyboard_state[raw_value].SetValue(action == GLFW_PRESS || action == GLFW_REPEAT);
	}

	void WindowApplication::SetMouseButtonState(MouseButton key, int action)
	{
		int raw_value = int(key);
		if (raw_value >= 0 && raw_value < mouse_button_state.size())
			mouse_button_state[raw_value].SetValue(action == GLFW_PRESS || action == GLFW_REPEAT);
	}

	bool WindowApplication::GetKeyState(Key key, bool previous_frame) const
	{
		int raw_value = key.GetRawValue();

		if (key.GetType() == KeyType::KEYBOARD)
		{
			if (raw_value >= 0 && raw_value < keyboard_state.size())
				return keyboard_state[raw_value].GetValue(previous_frame);
		}
		else if (key.GetType() == KeyType::MOUSE)
		{
			if (raw_value >= 0 && raw_value < mouse_button_state.size())
				return mouse_button_state[raw_value].GetValue(previous_frame);
		}
		return false;
	}

	void WindowApplication::UpdateKeyStates(float delta_time)
	{
		for (ButtonState& button : keyboard_state)
			button.UpdateSameValueTimer(delta_time);
	}

	GLFWwindow* WindowApplication::GetSharedGLContext()
	{
		WindowApplication* application = Application::GetInstance();
		if (application != nullptr)
			return application->shared_context;
		return nullptr;
	}

	size_t WindowApplication::GetWindowCount() const
	{
		return windows.size();
	}

	AutoCastable<Window> WindowApplication::GetWindow(size_t index)
	{
		return windows[index].get();
	}

	AutoConstCastable<Window> WindowApplication::GetWindow(size_t index) const
	{
		return windows[index].get();
	}

	AutoCastable<Window> WindowApplication::FindWindow(ObjectRequest request)
	{
		return request.FindObject(windows);
	}

	AutoConstCastable<Window> WindowApplication::FindWindow(ObjectRequest request) const
	{
		return request.FindObject(windows);
	}

	bool WindowApplication::DoProcessAction(GPUProgramProviderExecutionData const& execution_data) const
	{
		if (execution_data.Match("material", GPUProgramProviderPassType::FALLBACK))
		{
			return execution_data.Process(GetTextureAtlas()->GetTexture());
		}
		return false;
	}

	void WindowApplication::OnMonitorEvent(GLFWmonitor* monitor, int monitor_state)
	{
		// the GLFW monitor delegate does not send event to any window
		// that's why we are catching here the event and dispatching to all application's windows
		for (shared_ptr<Window> & window : windows)
		{
			if (window != nullptr)
			{
				window->WithWindowContext([&window, monitor, monitor_state]()
				{
					if (window->GetImGuiContext() != nullptr)
						ImGui_ImplGlfw_MonitorCallback(monitor, monitor_state); // manually call ImGui delegate (see comment in WindowApplication::OnWindowCreated(...)
					window->OnMonitorEvent(monitor, monitor_state);
				});
			}
		}
	}

	void WindowApplication::DoOnMonitorEvent(GLFWmonitor* monitor, int monitor_state)
	{
		if (WindowApplication* application = Application::GetInstance())
			application->OnMonitorEvent(monitor, monitor_state);
	}

	bool WindowApplication::GetImGuiMenuMode() const
	{
		return imgui_menu_mode;
	}

	void WindowApplication::SetImGuiMenuMode(bool mode)
	{
		if (imgui_menu_mode != mode)
		{
			imgui_menu_mode = mode;
			for (shared_ptr<Window>& window : windows)
			{
				window->OnImGuiMenuModeChanged(mode);
			}
		}
	}

	void WindowApplication::ShowWindowInternal(bool visible, char const * name, SubClassOf<Window> window_class)
	{
		Window* existing_window = FindWindow(name);

		if (visible)
		{
			if (existing_window == nullptr)
			{
				WindowCreateParams create_params;
				create_params.title = name;
				create_params.width = 800;
				create_params.height = 800;
				CreateTypedWindow(window_class, create_params, name);
			}
		}
		else
		{
			if (existing_window != nullptr)
				existing_window->RequireWindowClosure();
		}
	}

	bool WindowApplication::IsConsoleWindowVisible() const
	{
		return (FindWindow("Console") != nullptr);
	}

	void WindowApplication::ShowConsoleWindow(bool visible)
	{
		ShowWindowInternal(visible, "Console", SubClassOf<ConsoleWindow>());
	}

#if _DEBUG

	bool WindowApplication::IsImGuiDemoWindowVisible() const
	{
		return (FindWindow("ImGuiDemo") != nullptr);
	}

	void WindowApplication::ShowImGuiDemoWindow(bool visible)
	{
		ShowWindowInternal(visible, "ImGuiDemo", SubClassOf<ImGuiDemoWindow>());
	}

#endif

	void WindowApplication::AddWindowMenuItem(char const* window_name, bool (WindowApplication::*IsWindowVisibleFunc)() const, void (WindowApplication::*ShowWindowFunc)(bool))
	{
		bool window_exists = (this->*IsWindowVisibleFunc)();
		if (ImGui::MenuItem(window_name, nullptr, window_exists, true))
		{
			(this->*ShowWindowFunc)(!window_exists);
		}
	}

	void WindowApplication::OnDrawWindowImGuiMenu(Window* window)
	{
		ImGuiDrawableInterface::MenuBar(ImGuiDrawMenuMode::FullWindow, [this]()
		{
			if (ImGui::BeginMenu("Actions"))
			{
				if (ImGui::MenuItem("Open Temp Dir.", nullptr, false, true))
				{
					ShowUserLocalTempDirectory();
				}
				if (ImGui::MenuItem("Open Resources Dir.", nullptr, false, true))
				{
					WinTools::ShowFile(GetResourcesPath());
				}
				if (ImGui::MenuItem("Dump Config File", nullptr, false, true))
				{
					WinTools::ShowFile(JSONTools::DumpConfigFile(configuration));
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Quit", nullptr, false, true))
				{
					DestroyAllWindows();
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Windows"))
			{
				AddWindowMenuItem("Console", &WindowApplication::IsConsoleWindowVisible, &WindowApplication::ShowConsoleWindow);
#if _DEBUG
				AddWindowMenuItem("ImGuiDemo", &WindowApplication::IsImGuiDemoWindowVisible, &WindowApplication::ShowImGuiDemoWindow);
#endif

				ImGui::EndMenu();
			}
		});
	}

	std::vector<weak_ptr<Window>> WindowApplication::GetWeakWindowArray() const
	{
		std::vector<weak_ptr<Window>> result;
		result.reserve(windows.size());
		for (shared_ptr<Window> const& window : windows)
			result.emplace_back(window.get());
		return result;
	}

}; // namespace chaos
