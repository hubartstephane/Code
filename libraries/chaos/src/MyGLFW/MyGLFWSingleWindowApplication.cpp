#include <chaos/Chaos.h>

namespace chaos
{
	namespace MyGLFW
	{
		// 
		// SingleWindowApplication
		//

		SingleWindowApplication::SingleWindowApplication(SingleWindowApplicationParams const & in_window_params) :
			window_params(in_window_params)
		{
		}

		void SingleWindowApplication::TweakHintsFromConfiguration(SingleWindowApplicationParams & params, nlohmann::json const & in_config)
		{
			JSONTools::GetAttribute(in_config, "monitor_index", params.monitor_index);
			JSONTools::GetAttribute(in_config, "width", params.width);
			JSONTools::GetAttribute(in_config, "height", params.height);

			WindowHints & hints = params.hints;

			JSONTools::GetAttribute(in_config, "debug_context", hints.debug_context);
			JSONTools::GetAttribute(in_config, "major_version", hints.major_version);
			JSONTools::GetAttribute(in_config, "minor_version", hints.minor_version);
			JSONTools::GetAttribute(in_config, "refresh_rate", hints.refresh_rate);
			JSONTools::GetAttribute(in_config, "opengl_profile", hints.opengl_profile);
			JSONTools::GetAttribute(in_config, "unlimited_fps", hints.unlimited_fps);
#if 0 // probably no reason why this should be in config file
			JSONTools::GetAttribute(in_config, "resizable", hints.resizable);
			JSONTools::GetAttribute(in_config, "start_visible", hints.start_visible);
			JSONTools::GetAttribute(in_config, "decorated", hints.decorated);
			JSONTools::GetAttribute(in_config, "toplevel", hints.toplevel);
			JSONTools::GetAttribute(in_config, "samples", hints.samples);
			JSONTools::GetAttribute(in_config, "double_buffer", hints.double_buffer);
			JSONTools::GetAttribute(in_config, "depth_bits", hints.depth_bits);
			JSONTools::GetAttribute(in_config, "stencil_bits", hints.stencil_bits);
			JSONTools::GetAttribute(in_config, "red_bits", hints.red_bits);
			JSONTools::GetAttribute(in_config, "green_bits", hints.green_bits);
			JSONTools::GetAttribute(in_config, "blue_bits", hints.blue_bits);
			JSONTools::GetAttribute(in_config, "alpha_bits", hints.alpha_bits);
			JSONTools::GetAttribute(in_config, "focused", hints.focused);
#endif
		}
		
		void SingleWindowApplication::FreezeNextFrameTickDuration()
		{
			forced_zero_tick_duration = true;
		}

		bool SingleWindowApplication::MessageLoop()
		{
			GLFWwindow * glfw_window = window->GetGLFWHandler();

			double t1 = glfwGetTime();

			while (!window->ShouldClose())
			{
				glfwPollEvents();

				double t2 = glfwGetTime();
				float delta_time = (float)(t2 - t1);
				float real_delta_time = delta_time;

				if (forced_zero_tick_duration)
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

				// tick the renderer
				if (renderer != nullptr)
					renderer->Tick(real_delta_time); // renderer has metrics that rely on the real frame_rate (not modified one due to some of our tweaks)
				// tick the manager
				TickManagers(delta_time);
				// tick the window
				window->MainTick(delta_time);
				// update time
				t1 = t2;
			}
			return true;
		}

		Window * SingleWindowApplication::GenerateWindow()
		{
			return new Window;
		}

		bool SingleWindowApplication::Main()
		{
			bool result = false;

			SingleWindowApplicationParams params = window_params; // work on a copy of the params

			// set an error callback
			glfwSetErrorCallback(OnGLFWError);

			// compute the monitor upon which the window will be : use it for pixel format
			if (params.monitor == nullptr)
				params.monitor = GetMonitorByIndex(params.monitor_index);

			// retrieve the position of the monitor
			int monitor_x = 0;
			int monitor_y = 0;
			glfwGetMonitorPos(params.monitor, &monitor_x, &monitor_y);

			// retrieve the mode of the monitor to deduce pixel format
			GLFWvidmode const * mode = glfwGetVideoMode(params.monitor);

			nlohmann::json const * window_configuration = JSONTools::GetStructure(configuration, "window");
			if (window_configuration != nullptr)
				TweakHintsFromConfiguration(params, *window_configuration);

			// compute the position and size of the window
			bool pseudo_fullscreen = (params.width <= 0 && params.height <= 0);

			// prepare window creation
			window->TweakHints(params.hints, params.monitor, pseudo_fullscreen);
			params.hints.ApplyHints();
			
			// compute window size and position
			int x = 0;
			int y = 0;
			if (pseudo_fullscreen) // full-screen, the window use the full-size
			{
				params.width = mode->width;
				params.height = mode->height;

				x = monitor_x;
				y = monitor_y;
			}
			else
			{
				if (params.width <= 0)
					params.width = mode->width;
				else
					params.width = std::min(mode->width, params.width);

				if (params.height <= 0)
					params.height = mode->height;
				else
					params.height = std::min(mode->height, params.height);

				x = monitor_x + (mode->width - params.width) / 2;
				y = monitor_y + (mode->height - params.height) / 2;
			}

			// create window
			if (params.title == nullptr) // title cannot be null
				params.title = "";

			// we are doing a pseudo fullscreen => monitor parameters of glfwCreateWindow must be null or it will "capture" the screen
			GLFWwindow * glfw_window = glfwCreateWindow(params.width, params.height, params.title, nullptr /* monitor */, nullptr /* share list */);
			if (glfw_window == nullptr)
				return false;
			glfwMakeContextCurrent(glfw_window);

			// vsync ?
			if (params.hints.unlimited_fps)
				glfwSwapInterval(0); 

			// XXX : seems to be mandatory for some functions like : glGenVertexArrays(...)
			//       see https://www.opengl.org/wiki/OpenGL_Loading_Library
			glewExperimental = GL_TRUE;
			// create the context
			GLenum err = glewInit();
			if (err != GLEW_OK)
			{
				LogTools::Log("glewInit(...) failure : %s", glewGetErrorString(err));
				return false;
			}

			// create the renderer
			renderer = new GPURenderer;
			if (renderer == nullptr)
				return false;
			if (!renderer->Initialize())
				return false;

			// set the debug log hander
			GLTools::SetDebugMessageHandler();
			// some generic information
			GLTools::DisplayGenericInformation();

			// initialize the GPU resource Manager
			if (!InitializeGPUManager())
				return false;

			// bind the window
			window->BindGLFWWindow(glfw_window, params.hints.double_buffer ? true : false);

			// prepare the window
			result = window->InitializeFromConfiguration(configuration, configuration_path);
			if (result)
			{		
				// x and y are the coordinates of the client area : when there is a decoration, we want to tweak the window size / position with that
				int left, top, right, bottom;
				glfwGetWindowFrameSize(glfw_window, &left, &top, &right, &bottom);
				if (left != 0 || top != 0 || right != 0 || bottom != 0)
				{
					x += left;
					y += top;
					params.width = params.width - left - right;
					params.height = params.height - top - bottom;
					glfwSetWindowSize(glfw_window, params.width, params.height);
				}

				glfwSetWindowPos(glfw_window, x, y);
				glfwSetInputMode(glfw_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				// glfwSetInputMode(glfw_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				//  glfwSetInputMode(glfw_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

				glfwSetInputMode(glfw_window, GLFW_STICKY_KEYS, 1);


				// now that the window is fully placed ... we can show it
				if (params.hints.start_visible)
					glfwShowWindow(glfw_window);

				// the main loop
				MessageLoop();
			}
			return true;
		}

		
		void SingleWindowApplication::TickManagers(float delta_time)
		{

			if (main_clock != nullptr)
				main_clock->TickClock(delta_time);
			if (sound_manager != nullptr)
				sound_manager->Tick(delta_time);
		}

		void SingleWindowApplication::OnGLFWError(int code, const char* msg)
		{
			LogTools::Log("Window(...) [%d] failure : %s", code, msg);
		}

		bool SingleWindowApplication::InitializeGPUManager()
		{
			// initialize the GPU manager
			gpu_manager = new GPUResourceManager;
			if (gpu_manager == nullptr)
				return false;
			gpu_manager->StartManager();
			// get the structure of interrest
			nlohmann::json const * gpu_config = JSONTools::GetStructure(configuration, "gpu");
			if (gpu_config != nullptr)
				if (!gpu_manager->InitializeFromConfiguration(*gpu_config, configuration_path))
					return false;
			return true;
		}

		bool SingleWindowApplication::ReloadGPUResources()
		{
			// this call may block for too much time
			FreezeNextFrameTickDuration();

			// reload the configuration file
			nlohmann::json config;
			if (!ReloadConfigurationFile(config))
				return false;
			// get the structure of interrest
			nlohmann::json const * gpu_config = JSONTools::GetStructure(config, "gpu");
			if (gpu_config == nullptr)
				return false;
			// create a temporary manager
			shared_ptr<GPUResourceManager> other_gpu_manager = new GPUResourceManager; // destroyed at the end of the function
			if (other_gpu_manager == nullptr)
				return false;
			if (!other_gpu_manager->StartManager())
				return false;
			// reload all resources ... (even unchanged)
			if (other_gpu_manager->InitializeFromConfiguration(*gpu_config, configuration_path))
				gpu_manager->RefreshGPUResources(other_gpu_manager.get());
			other_gpu_manager->StopManager();					
			return true;
		}

		bool SingleWindowApplication::FinalizeGPUManager()
		{
			// stop the resource manager
			if (gpu_manager != nullptr)
			{
				gpu_manager->StopManager();
				gpu_manager = nullptr;
			}
			return true;
		}

		bool SingleWindowApplication::InitializeManagers()
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
			nlohmann::json const * clock_config = JSONTools::GetStructure(configuration, "clocks");
			if (clock_config != nullptr)
				main_clock->InitializeFromConfiguration(*clock_config, configuration_path);

			// initialize the sound manager
			sound_manager = new SoundManager();
			if (sound_manager == nullptr)
				return false;
			sound_manager->StartManager();
			nlohmann::json const * sound_config = JSONTools::GetStructure(configuration, "sounds");
			if (sound_config != nullptr)
				sound_manager->InitializeFromConfiguration(*sound_config, configuration_path);

			return true;
		}

		bool SingleWindowApplication::FinalizeManagers()
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
			if (gpu_manager != nullptr)
			{
				gpu_manager->StopManager();
				gpu_manager = nullptr;
			}
			// super method
			Application::FinalizeManagers();
			return true;
		}

		bool SingleWindowApplication::Initialize()
		{
			if (!Application::Initialize())
				return false;
			// create the window
			window = GenerateWindow();
			if (window == nullptr)
				return false;
			// success
			return true;
		}

		bool SingleWindowApplication::Finalize()
		{
			FinalizeGPUManager();

			// stop the window
			if (window != nullptr)
			{
				window->Finalize();

				GLFWwindow * glfw_window = window->GetGLFWHandler();
				if (glfw_window != nullptr)
					glfwDestroyWindow(glfw_window);

				delete(window);
				window = nullptr;
			}
			Application::Finalize();
			return true;
		}

		void SingleWindowApplication::FreeImageOutputMessageFunc(FREE_IMAGE_FORMAT fif, const char *msg)
		{
			LogTools::Log("FreeImage warning message [%d][%s]", fif, msg);
		}

		bool SingleWindowApplication::InitializeStandardLibraries()
		{
			if (!Application::InitializeStandardLibraries())
				return false;
			FreeImage_Initialise(); // glew will be initialized 
			FreeImage_SetOutputMessage(&FreeImageOutputMessageFunc);
			glfwInit();
			return true;
		}

		bool SingleWindowApplication::FinalizeStandardLibraries()
		{
			glfwTerminate();
			FreeImage_DeInitialise();
			Application::FinalizeStandardLibraries();
			return true;
		}

		void SingleWindowApplication::OnInputModeChanged(InputMode new_mode, InputMode old_mode)
		{
			if (window != nullptr)
				window->OnInputModeChanged(new_mode, old_mode);			
		}

		Clock * SingleWindowApplication::GetMainClockInstance()
		{
			SingleWindowApplication * application = GetInstance();
			if (application == nullptr)
				return nullptr;
			return application->GetMainClock();
		}

		Clock const * SingleWindowApplication::GetMainClockConstInstance()
		{
			SingleWindowApplication const * application = GetConstInstance();
			if (application == nullptr)
				return nullptr;
			return application->GetMainClock();
		}

		SoundManager * SingleWindowApplication::GetSoundManagerInstance()
		{
			SingleWindowApplication * application = GetInstance();
			if (application == nullptr)
				return nullptr;
			return application->GetSoundManager();
		}

		SoundManager const * SingleWindowApplication::GetSoundManagerConstInstance()
		{
			SingleWindowApplication const * application = GetConstInstance();
			if (application == nullptr)
				return nullptr;
			return application->GetSoundManager();
		}

		GPUResourceManager * SingleWindowApplication::GetGPUResourceManagerInstance()
		{
			SingleWindowApplication * application = GetInstance();
			if (application == nullptr)
				return nullptr;
			return application->GetGPUResourceManager();
		}

		GPUResourceManager const * SingleWindowApplication::GetGPUResourceManagerConstInstance()
		{
			SingleWindowApplication const * application = GetConstInstance();
			if (application == nullptr)
				return nullptr;
			return application->GetGPUResourceManager();
		}

#if _DEBUG
		CHAOS_HELP_TEXT(SHORTCUTS, "F8  : ReloadGPUResources");
#endif

		bool SingleWindowApplication::OnKeyEventImpl(KeyEvent const& event)
		{	
			// reloading GPU resources		
#if _DEBUG
			if (event.IsKeyPressed(GLFW_KEY_F8))
			{
				// CMD GLFW_KEY_F8 : ReloadGPUResources(...)
				ReloadGPUResources();
				return true;
			}
#endif
			return Application::OnKeyEventImpl(event);
		}

		GLFWwindow* SingleWindowApplication::GetGLFWWindow() const
		{
			return (window == nullptr) ? nullptr : window->GetGLFWWindow();
		}

	}; // namespace MyGLFW

}; // namespace chaos