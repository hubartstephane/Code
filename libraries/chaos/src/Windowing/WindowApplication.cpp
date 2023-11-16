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
			ForAllWindows([](Window* window)
			{
				if (window->ShouldClose())
					window->Destroy();
				});

			// tick the windows
			ForAllWindows([delta_time, real_delta_time](Window* window)
			{
				window->WithWindowContext([&window, delta_time, real_delta_time]()
				{
					window->TickRenderer(real_delta_time);
					window->Tick(delta_time);
					window->DrawWindow();
				});
			});
			// update time
			t1 = t2;
		}
	}

	void WindowApplication::DestroyAllWindows()
	{
		ForAllWindows([](Window* window)
		{
			window->Destroy();
		});
	}

	Window* WindowApplication::CreateTypedWindow(SubClassOf<Window> window_class, WindowCreateParams create_params, ObjectRequest request)
	{
		if (FindWindow(request) != nullptr)
		{
			Log::Error("WindowApplication::CreateTypedWindow(...) name already used");
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
			if (nlohmann::json const* windows_configuration = JSONTools::GetStructureNode(configuration, "windows"))
			{
				window_configuration = JSONTools::GetStructureNode(*windows_configuration, result->GetName());
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





			// SHU_PERSISTENT
#if 0 
			// read information from session file
			result->ReadPersistentData();
			
#endif			
			
			
			
			// create the root widget
			result->CreateRootWidget();
			// notify the application
			OnWindowCreated(result.get());
			return result.get();
		});
	}

	void WindowApplication::OnWindowDestroyed(Window* window)
	{


		// SHU_PERSISTENT
#if 0
		window->WritePersistentData();
#endif
		window->StorePersistentProperties();
		
		
		
		
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
		if (nlohmann::json const* glfw_configuration = JSONTools::GetStructureNode(configuration, "glfw"))
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
		Window * main_window = CreateMainWindow();
		if (main_window == nullptr)
			return -1;
		// run the main loop as long as there are windows
		RunMessageLoop([this]() { return (windows.size() > 0); });
		return 0;
	}

	void WindowApplication::Quit()
	{
		// write the name of the windows that are opened
		JSONWriteConfiguration config = GetJSONWriteConfiguration();

		std::vector<char const*> names;
		for (shared_ptr<Window> const& window : windows)
			names.push_back(window->GetName());
		JSONTools::SetAttribute(config, "opened_window", names);

		// effectively quit the application
		DestroyAllWindows();
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
		if (nlohmann::json const* fonts_config = JSONTools::GetStructureNode(configuration, "fonts"))
		{
			// read the default font parameters
			BitmapAtlas::FontInfoInputParams font_params;

			nlohmann::json const* default_font_param_json = JSONTools::GetStructureNode(*fonts_config, "default_font_param");
			if (default_font_param_json != nullptr)
				LoadFromJSON(*default_font_param_json, font_params);

			// Add the fonts
			if (nlohmann::json const* fonts_json = JSONTools::GetObjectNode(*fonts_config, "fonts"))
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
		CHAOS_GLOBAL_VARIABLE(bool, UseCachedAtlas, false);
#if !_DEBUG
		CHAOS_GLOBAL_VARIABLE(bool, DumpCachedAtlas, false);
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

		nlohmann::json const* atlas_json = JSONTools::GetStructureNode(configuration, "atlas");
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
		nlohmann::json const* fonts_config = JSONTools::GetStructureNode(configuration, "fonts");

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
				if (nlohmann::json const* font_bitmaps_json = JSONTools::GetObjectNode(*fonts_config, "bitmaps"))
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
			if (nlohmann::json const* font_colors_json = JSONTools::GetObjectNode(*fonts_config, "colors"))
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
		// update keyboard and mouse state
		KeyboardState::UpdateKeyStates(delta_time);
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
		GiveChildConfiguration(gpu_resource_manager, "gpu");
		gpu_resource_manager->StartManager();
		// create internal resource
		if (!gpu_resource_manager->InitializeInternalResources())
			return false;
		// get JSON section and load all requested resources
		nlohmann::json const* gpu_config = JSONTools::GetStructureNode(configuration, "gpu");
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

			if (gpu_resource_manager != nullptr)
				gpu_resource_manager->ReloadObjectConfiguration(true); // send notification






#if 0

			// reload the configuration file
			nlohmann::json config;
			if (!ReloadConfigurationFile(config))
				return false;
			// get the structure of interrest
			nlohmann::json const* gpu_config = JSONTools::GetStructureNode(config, "gpu");
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
#endif




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
		nlohmann::json const* clock_config = JSONTools::GetStructureNode(configuration, "clocks");
		if (clock_config != nullptr)
			main_clock->InitializeFromConfiguration(*clock_config);

		// initialize the sound manager
		sound_manager = new SoundManager();
		if (sound_manager == nullptr)
			return false;
		sound_manager->StartManager();

		GiveChildConfiguration(sound_manager, "sounds");

		sound_manager->InitializeFromConfiguration();

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
		ForAllWindows([new_mode, old_mode](Window * window)
		{
			window->WithWindowContext([&window, new_mode, old_mode]()
			{
				window->OnInputModeChanged(new_mode, old_mode);
			});
		});
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
		if (event.IsKeyPressed(KeyboardButton::F7))
		{
			SetImGuiMenuMode(!GetImGuiMenuMode());
			return true;
		}

#if _DEBUG
		// reloading GPU resources
		if (event.IsKeyPressed(KeyboardButton::F8))
		{
			// CMD F8 : ReloadGPUResources(...)
			ReloadGPUResources();
			return true;
		}
#endif
		return Application::OnKeyEventImpl(event);
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
		ForAllWindows([monitor, monitor_state](Window* window)
		{
			window->WithWindowContext([&window, monitor, monitor_state]()
			{
				if (window->GetImGuiContext() != nullptr)
					ImGui_ImplGlfw_MonitorCallback(monitor, monitor_state); // manually call ImGui delegate (see comment in WindowApplication::OnWindowCreated(...)
				window->OnMonitorEvent(monitor, monitor_state);
			});
		});
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
			ForAllWindows([mode](Window* window)
			{
				window->WithWindowContext([&window, mode]()
				{
					window->OnImGuiMenuModeChanged(mode);
				});
			});
		}
	}

	void WindowApplication::SetWindowInternalVisibility(bool visible, char const * name, SubClassOf<Window> window_class)
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

	bool WindowApplication::IsKnownWindowVisible(char const * name) const
	{
		return (FindWindow(name) != nullptr);
	}

	bool WindowApplication::SetKnownWindowVisibility(char const* name, bool visible)
	{
		return EnumerateKnownWindows([this, name, visible](char const* window_name, SubClassOf<Window> window_class)
		{
			// is it the window we are searching?
			if (StringTools::Stricmp(name, window_name) != 0)
				return false;
			SetWindowInternalVisibility(visible, window_name, window_class);
			return true; // stop the search
		});
	}

	bool WindowApplication::EnumerateKnownWindows(std::function<bool(char const * name, SubClassOf<Window> window_class)> const& func) const
	{
		return
			func("Console", SubClassOf<ConsoleWindow>())
#if _DEBUG
			|| func("ImGuiDemo", SubClassOf<ImGuiDemoWindow>())
#endif
			;
	}

	void WindowApplication::OnDrawWindowImGuiMenu(Window* window)
	{
		ImGuiDrawableInterface::MenuBar(ImGuiDrawMenuMode::FullWindow, [this]()
		{
			if (ImGui::BeginMenu("Actions"))
			{
				if (ImGui::MenuItem("Open Temp Dir.", nullptr, false, true))
				{
					WinTools::ShowFile(GetUserLocalTempPath());
				}
				if (ImGui::MenuItem("Open Resources Dir.", nullptr, false, true))
				{
					WinTools::ShowFile(GetResourcesPath());
				}
				if (ImGui::MenuItem("Open Config File", nullptr, false, true))
				{
					WinTools::ShowFile(JSONTools::DumpConfigFile(configuration));
				}
				if (ImGui::MenuItem("Open Persistent File", nullptr, false, true))
				{
					SavePersistentConfiguration();
					WinTools::ShowFile(GetPersistentDataPath());
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Quit", nullptr, false, true))
				{
					Quit();
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Windows"))
			{
				EnumerateKnownWindows([this](char const * name, SubClassOf<Window> window_class)
				{
					bool window_exists = (FindWindow(name) != nullptr);
					if (ImGui::MenuItem(name, nullptr, window_exists, true))
						SetWindowInternalVisibility(!window_exists, name, window_class);
					return false; // don't stop the search
				});
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

	bool WindowApplication::OnReadConfigurableProperties(JSONReadConfiguration config, ReadConfigurablePropertiesContext context)
	{
		if (!Application::OnReadConfigurableProperties(config, context))
			return false;

		// open windows that were there during previous session
		std::vector<std::string> opened_window;
		JSONTools::GetAttribute(config, "opened_window", opened_window);

		for (std::string const& name : opened_window)
			SetKnownWindowVisibility(name.c_str(), true);

		return true;
	}











#ifdef _WIN32

	void WindowApplication::OnInputLanguageChanged()
	{
		// update the layout
		KeyboardLayout::InvalidateCachedLayout();
		// propage the information to all windows (WM_INPUTLANGCHANGE is only sent to the topmost one)
		ForAllWindows([](Window * window)
		{
			window->OnInputLanguageChanged();
		});
	}

#endif // #if _WIN32


}; // namespace chaos
