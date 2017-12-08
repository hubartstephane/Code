#include <chaos/MyGLFWWindow.h>
#include <chaos/LogTools.h>
#include <chaos/GLTools.h>
#include <chaos/Application.h>
#include <chaos/FileTools.h>
#include <chaos/Buffer.h>
#include <chaos/JSONTools.h>

namespace chaos
{
	namespace MyGLFW
	{
    /**
    * Tools
    */

    std::vector<GLFWmonitor *> Tools::GetSortedMonitors()
    {
      std::vector<GLFWmonitor *> result;

      int monitor_count = 0;
      GLFWmonitor ** monitors = glfwGetMonitors(&monitor_count);

      if (monitor_count > 0 && monitors != nullptr)
      {
        result.reserve((size_t)monitor_count);

        // fill that array with pairs : (monitors, X position)
        for (int i = 0; i < monitor_count; ++i)
          result.push_back(monitors[i]);

        // sort the array by X position of the monitor    
        auto b = result.begin();
        auto e = result.end();

        std::sort(b, e, [](GLFWmonitor * src1, GLFWmonitor * src2) {
          int x1 = 0;
          int y1 = 0;
          int x2 = 0;
          int y2 = 0;
          glfwGetMonitorPos(src1, &x1, &y1);
          glfwGetMonitorPos(src2, &x2, &y2);
          return (x1 < x2) ? true : false;
        });
      }
      return result;
    }

    GLFWmonitor * Tools::GetMonitorByIndex(int monitor_index)
    {
      GLFWmonitor * result = glfwGetPrimaryMonitor();
      if (monitor_index != 0)
      {
        std::vector<GLFWmonitor *> monitors = GetSortedMonitors();

        // search the primary monitor inside
        auto b = monitors.begin();
        auto e = monitors.end();
        auto it = std::find(b, e, result);

        // primary index
        int primary_index = (int)(it - b); // we want to offset it with positive or negative values (cast it into signed)

                                           // compute the index of the monitor we are interested in (and clamp)
        int result_index = primary_index + monitor_index;
        if (result_index < 0)
          result_index = 0;
        else if (result_index >= (int)monitors.size())
          result_index = (int)monitors.size() - 1;

        result = monitors[(size_t)result_index];
      }
      return result;
    }

    /**
    * WindowHints
    */

		void WindowHints::ApplyHints()
		{
			glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, debug_context);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major_version);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor_version);
			glfwWindowHint(GLFW_REFRESH_RATE, refresh_rate); // only usefull in fullscreen mode
			glfwWindowHint(GLFW_OPENGL_PROFILE, opengl_profile);
			glfwWindowHint(GLFW_RESIZABLE, resizable);
			glfwWindowHint(GLFW_VISIBLE, false);          // XXX : we don't use start_visible yet because we may do some small window displacement (probably never visible but ...)
			glfwWindowHint(GLFW_DECORATED, decorated);
			glfwWindowHint(GLFW_FLOATING, toplevel);
			glfwWindowHint(GLFW_SAMPLES, samples);
			glfwWindowHint(GLFW_DOUBLEBUFFER, double_buffer);
			glfwWindowHint(GLFW_DEPTH_BITS, depth_bits);
			glfwWindowHint(GLFW_STENCIL_BITS, stencil_bits);
			glfwWindowHint(GLFW_RED_BITS, red_bits);
			glfwWindowHint(GLFW_GREEN_BITS, green_bits);
			glfwWindowHint(GLFW_BLUE_BITS, blue_bits);
			glfwWindowHint(GLFW_ALPHA_BITS, alpha_bits);
			glfwWindowHint(GLFW_FOCUSED, focused);
		}

    /**
    * Window
    */

    GLFWwindow * Window::GetGLFWHandler()
    {
      return glfw_window;
    }
    
    bool Window::ShouldClose()
    {
      return (glfwWindowShouldClose(glfw_window) != 0);
    }

    void Window::MainTick(double delta_time)
    {
      if (Tick(delta_time))
        RequireWindowRefresh();

      if (refresh_required)
      {
        DoOnDraw(glfw_window);
        refresh_required = false;
      }
    }

		void Window::BindGLFWWindow(GLFWwindow * in_glfw_window)
		{
			assert(glfw_window == nullptr); // ensure not already bound
			assert(in_glfw_window != nullptr);

			glfw_window = in_glfw_window;

			glfwSetWindowUserPointer(in_glfw_window, this);

			glfwSetCursorPosCallback(in_glfw_window, DoOnMouseMove);
			glfwSetMouseButtonCallback(in_glfw_window, DoOnMouseButton);
			glfwSetScrollCallback(in_glfw_window, DoOnMouseWheel);
			glfwSetWindowSizeCallback(in_glfw_window, DoOnWindowResize);
			glfwSetKeyCallback(in_glfw_window, DoOnKeyEvent);
			glfwSetWindowCloseCallback(in_glfw_window, DoOnWindowClosed);
			glfwSetWindowRefreshCallback(in_glfw_window, DoOnDraw);
			glfwSetDropCallback(in_glfw_window, DoOnDropFile);
			glfwSetWindowFocusCallback(in_glfw_window, DoOnFocusStateChange);
			glfwSetWindowIconifyCallback(in_glfw_window, DoOnIconifiedStateChange);
		}

		void Window::DoOnIconifiedStateChange(GLFWwindow * in_glfw_window, int value)
		{
			Window * my_window = (Window*)glfwGetWindowUserPointer(in_glfw_window);
			if (my_window != nullptr)
				my_window->OnIconifiedStateChange(value == GL_TRUE);
		}

		void Window::DoOnFocusStateChange(GLFWwindow * in_glfw_window, int value)
		{
			Window * my_window = (Window*)glfwGetWindowUserPointer(in_glfw_window);
			if (my_window != nullptr)
				my_window->OnFocusStateChange(value == GL_TRUE);
		}

		void Window::DoOnWindowClosed(GLFWwindow * in_glfw_window)
		{
			Window * my_window = (Window*)glfwGetWindowUserPointer(in_glfw_window);
			if (my_window != nullptr)
				if (my_window->OnWindowClosed())
					my_window->RequireWindowClosure();
		}

		void Window::DoOnWindowResize(GLFWwindow * in_glfw_window, int width, int height)
		{
			Window * my_window = (Window*)glfwGetWindowUserPointer(in_glfw_window);
			if (my_window != nullptr)
				my_window->OnWindowResize(glm::ivec2(width, height));
		}

		void Window::DoOnDraw(GLFWwindow * in_glfw_window)
		{
			Window * my_window = (Window*)glfwGetWindowUserPointer(in_glfw_window);
			if (my_window != nullptr)
			{
				int width = 0;
				int height = 0;

				glfwMakeContextCurrent(in_glfw_window);
				glfwGetFramebufferSize(in_glfw_window, &width, &height); // framebuffer size is in pixel ! (not glfwGetWindowSize)

				if (width <= 0 || height <= 0) // some crash to expect in drawing elsewhere
					return;

				if (my_window->OnDraw(glm::ivec2(width, height)))
					if (my_window->double_buffer)
						glfwSwapBuffers(in_glfw_window);
			}
		}

		void Window::DoOnMouseMove(GLFWwindow * in_glfw_window, double x, double y)
		{
			Window * my_window = (Window*)glfwGetWindowUserPointer(in_glfw_window);
			if (my_window != nullptr)
				my_window->OnMouseMove(x, y);
		}

		void Window::DoOnMouseButton(GLFWwindow * in_glfw_window, int button, int action, int modifier)
		{
			Window * my_window = (Window*)glfwGetWindowUserPointer(in_glfw_window);
			if (my_window != nullptr)
				my_window->OnMouseButton(button, action, modifier);
		}

		void Window::DoOnMouseWheel(GLFWwindow * in_glfw_window, double scroll_x, double scroll_y)
		{
			Window * my_window = (Window*)glfwGetWindowUserPointer(in_glfw_window);
			if (my_window != nullptr)
				my_window->OnMouseWheel(scroll_x, scroll_y);
		}

		void Window::DoOnKeyEvent(GLFWwindow * in_glfw_window, int key, int scan_code, int action, int modifier)
		{
			Window * my_window = (Window*)glfwGetWindowUserPointer(in_glfw_window);
			if (my_window != nullptr)
				my_window->OnKeyEvent(key, scan_code, action, modifier);
		}

		void Window::DoOnDropFile(GLFWwindow * in_glfw_window, int count, char const ** paths)
		{
			Window * my_window = (Window*)glfwGetWindowUserPointer(in_glfw_window);
			if (my_window != nullptr)
				my_window->OnDropFile(count, paths);
		}

		void Window::RequireWindowClosure()
		{
			glfwSetWindowShouldClose(glfw_window, 1);
		}

		void Window::RequireWindowRefresh()
		{
			//  refresh_required = true;
			HWND hWnd = glfwGetWin32Window(glfw_window);
			if (hWnd != NULL)
				InvalidateRect(hWnd, NULL, false); // this cause flickering
		}

		void Window::TweakHints(WindowHints & hints, GLFWmonitor * monitor, bool pseudo_fullscreen) const
		{
			// retrieve the mode of the monitor to deduce pixel format
			GLFWvidmode const * mode = glfwGetVideoMode(monitor);

			// the pixel format
			hints.red_bits = mode->redBits;
			hints.green_bits = mode->greenBits;
			hints.blue_bits = mode->blueBits;
			hints.alpha_bits = (hints.red_bits == 8 && hints.green_bits == 8 && hints.blue_bits == 8) ? 8 : 0; // alpha only if RGBA 32bits

			hints.refresh_rate = mode->refreshRate;

			if (pseudo_fullscreen) // full-screen, the window use the full-size
			{
				hints.decorated = 0;
				hints.resizable = 0;
				hints.toplevel = 1;
				hints.focused = 1;
			}
		}

    bool Window::PrepareWindow(GLFWwindow * in_glfw_window, bool in_double_buffer, nlohmann::json const & in_configuration)
    {
      BindGLFWWindow(in_glfw_window);
      double_buffer = in_double_buffer;     
      return Initialize(in_configuration);
    }

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

    bool SingleWindowApplication::MessageLoop()
    {
      GLFWwindow * glfw_window = window->GetGLFWHandler();

      double t1 = glfwGetTime();

      while (!window->ShouldClose())
      {
        glfwPollEvents();

        double t2 = glfwGetTime();
        double delta_time = t2 - t1;
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

      bool pseudo_fullscreen = (params.width <= 0 && params.height <= 0);

      // compute the monitor upon which the window will be : use it for pixel format
      if (params.monitor == nullptr)
        params.monitor = Tools::GetMonitorByIndex(params.monitor_index);

      // retrieve the position of the monitor
      int monitor_x = 0;
      int monitor_y = 0;
      glfwGetMonitorPos(params.monitor, &monitor_x, &monitor_y);

      // retrieve the mode of the monitor to deduce pixel format
      GLFWvidmode const * mode = glfwGetVideoMode(params.monitor);

      // compute the position and size of the window 
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
          params.width = min(mode->width, params.width);

        if (params.height <= 0)
          params.height = mode->height;
        else
          params.height = min(mode->height, params.height);

        x = monitor_x + (mode->width - params.width) / 2;
        y = monitor_y + (mode->height - params.height) / 2;
      }

      // prepare window creation
      window->TweakHints(params.hints, params.monitor, pseudo_fullscreen);
      TweakHintsFromConfiguration(params, configuration["window"]);
      params.hints.ApplyHints();

      // create window
      if (params.title == nullptr) // title cannot be null
        params.title = "";

      // we are doing a pseudo fullscreen => monitor parameters of glfwCreateWindow must be null or it will "capture" the screen
      GLFWwindow * glfw_window = glfwCreateWindow(params.width, params.height, params.title, nullptr /* monitor */, nullptr /* share list */);

      // main loop
      if (glfw_window != nullptr)
      {
        glfwMakeContextCurrent(glfw_window);

        // XXX : seems to be mandatory for some functions like : glGenVertexArrays(...)
        //       see https://www.opengl.org/wiki/OpenGL_Loading_Library
        glewExperimental = GL_TRUE;
        // create the context
        GLenum err = glewInit();
        if (err != GLEW_OK)
        {
          LogTools::Log("glewInit(...) failure : %s", glewGetErrorString(err));
        }
        else
        {
          // set the debug log hander
          GLTools::SetDebugMessageHandler();
          // some generic information
          GLTools::DisplayGenericInformation();

          // the loop
          bool double_buffer = params.hints.double_buffer ? true : false;
          result = window->PrepareWindow(glfw_window, double_buffer, configuration);
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
          window->Finalize();
        }
        glfwDestroyWindow(glfw_window);
      }

      return result;
    }

    void SingleWindowApplication::TickManagers(double delta_time)
    {
      if (main_clock != nullptr)
        main_clock->TickClock(delta_time);
      if (sound_manager != nullptr)
        sound_manager->Tick((float)delta_time);
    }

    void SingleWindowApplication::OnGLFWError(int code, const char* msg)
    {
      LogTools::Log("Window(...) [%d] failure : %s", code, msg);
    }

    bool SingleWindowApplication::Initialize()
    {
      if (!Application::Initialize())
        return false;
      // initialize the clock
      main_clock = new Clock();
      if (main_clock != nullptr)
       main_clock->InitializeFromConfiguration(configuration["ClockManager"]);
      // initialize the sound manager
      sound_manager = new SoundManager();
      if (sound_manager != nullptr)
        sound_manager->InitializeFromConfiguration(configuration["SoundManager"]);
      // create the window
      window = GenerateWindow();
      if (window == nullptr)
        return false;
      // success
      return true;
    }

    bool SingleWindowApplication::Finalize()
    {
      if (window != nullptr)
      {
        delete(window);
        window = nullptr;
      }
      main_clock = nullptr;
      sound_manager = nullptr;
      Application::Finalize();
      return true;
    }

    void SingleWindowApplication::FreeImageOutputMessageFunc(FREE_IMAGE_FORMAT fif, const char *msg)
    {

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


	}; // namespace MyGLFW

}; // namespace chaos
