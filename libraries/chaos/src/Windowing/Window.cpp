#include "chaos/ChaosPCH.h"
#include "chaos/ChaosInternals.h"

namespace chaos
{
	/**
	* Internal structure and method to have the surface around the client area
	*/

	static WindowFrameSizeInfo GetWindowFrameSizeInfo(GLFWwindow* window)
	{
		WindowFrameSizeInfo result;
		if (window != nullptr)
		{
			glfwGetWindowFrameSize(window, &result.left, &result.top, &result.right, &result.bottom);

			// XXX: the left and right values are surestimated
			//      without the following correction we would have the following bug
			//      1/double click on the title bar so that the window fills the right monitors (but still with a title bar)
			//      2/press F10 so that the window really becomes fullscreen (no more decorators)
			//      3/observe that the window is now on the left monitor
			//        -> because the effective top-left corner (that is to say not the client position) is a bit on the left monitor making
			//           left monitor preferred
			result.left = 0;
			result.right = 0;
		}
		return result;
	}

	/**
	* WindowCreateParams
	*/

	bool DoSaveIntoJSON(nlohmann::json * json, WindowCreateParams const& src)
	{
		if (!PrepareSaveObjectIntoJSON(json))
			return false;
		JSONTools::SetAttribute(json, "resizable", src.resizable);
		JSONTools::SetAttribute(json, "start_visible", src.start_visible);
		JSONTools::SetAttribute(json, "decorated", src.decorated);
		JSONTools::SetAttribute(json, "toplevel", src.toplevel);
		JSONTools::SetAttribute(json, "focused", src.focused);
		return true;
	}

	bool DoLoadFromJSON(JSONReadConfiguration config, WindowCreateParams& dst)
	{
		JSONTools::GetAttribute(config, "resizable", dst.resizable);
		JSONTools::GetAttribute(config, "start_visible", dst.start_visible);
		JSONTools::GetAttribute(config, "decorated", dst.decorated);
		JSONTools::GetAttribute(config, "toplevel", dst.toplevel);
		JSONTools::GetAttribute(config, "focused", dst.focused);
		return true;
	}

	bool DoSaveIntoJSON(nlohmann::json* json, WindowPlacementInfo const& src)
	{
		if (!PrepareSaveObjectIntoJSON(json))
			return false;
		JSONTools::SetAttribute(json, "monitor_index", src.monitor_index);
		JSONTools::SetAttribute(json, "position", src.position);
		JSONTools::SetAttribute(json, "size", src.size);
		JSONTools::SetAttribute(json, "fullscreen", src.fullscreen);
		return true;
	}

	bool DoLoadFromJSON(JSONReadConfiguration config, WindowPlacementInfo& dst)
	{
		JSONTools::GetAttribute(config, "monitor_index", dst.monitor_index);
		JSONTools::GetAttribute(config, "position", dst.position);
		JSONTools::GetAttribute(config, "size", dst.size);
		JSONTools::GetAttribute(config, "fullscreen", dst.fullscreen);
		return true;
	}

	/**
	* Window
	*/

	Window::Window()
	{
		// create the renderer
		renderer = new GPURenderer(this);
	}

	Window::~Window()
	{
		assert(glfw_window == nullptr);
		assert(imgui_context == nullptr);
	}

	void Window::Destroy()
	{
		if (WindowApplication* WindowApplication = Application::GetInstance())
			WindowApplication->DestroyWindow(this);
	}

	void Window::DestroyImGuiContext()
	{
		// destroy ImGui context (must happen before the windows destruction because some GLFW callbacks rely on the existence of the ImGui context)
		if (imgui_context != nullptr)
		{
#if _WIN32
			SetImGuiWindowProc(false); // remove our WndProc layer before ImGui may remove its layer
#endif // #if _WIN32
			ImGuiContext* previous_imgui_context = ImGui::GetCurrentContext();
			ImGui::SetCurrentContext(imgui_context);

			ImGui_ImplGlfw_Shutdown();
			ImGui_ImplOpenGL3_Shutdown();
			ImGui::DestroyContext();
			ImGui::SetCurrentContext((previous_imgui_context != imgui_context) ? previous_imgui_context : nullptr); // if there was another context, restore it

			imgui_context = nullptr;
		}
	}

	void Window::DestroyGLFWWindow()
	{
		// destroy GLFW window
		if (glfw_window != nullptr)
		{
			GLFWwindow * previous_context = glfwGetCurrentContext();
			glfwMakeContextCurrent(nullptr);
			glfwDestroyWindow(glfw_window);
			glfwMakeContextCurrent((previous_context != glfw_window)? previous_context : nullptr); // if there was another context, restore it

			glfw_window = nullptr;
		}
	}

	namespace GlobalVariables
	{
		CHAOS_GLOBAL_VARIABLE(bool, UnlimitedFPS, false);
	};

	void Window::ShowWindow(bool visible)
	{
		if (glfw_window == nullptr)
			return;
		if (visible)
			glfwShowWindow(glfw_window);
		else
			glfwHideWindow(glfw_window);
	}

	bool Window::IsWindowVisible() const
	{
		if (glfw_window == nullptr)
			return false;
		return glfwGetWindowAttrib(glfw_window, GLFW_VISIBLE) != 0;
	}

	static void EnsureWindowToolbarVisibleInAnyMonitor(glm::ivec2& window_position, glm::ivec2& window_size)
	{
		//        monitor
		// 
		//  +---+---------+---+
		//  |   |         |   |  ensure that the window toolbar touchs the center portion of the monitor
		//  |   |         |   |  (the toolbar should never be cropped along the Y axis)
		//  |   +---------+   |
		//  |                 |
		//  +-----------------+

		int MIN_HOVER_X = 300;
		int MIN_HOVER_Y = 150;

		int monitor_count = 0;
		GLFWmonitor** monitors = glfwGetMonitors(&monitor_count);

		auto length2 = [](glm::ivec2 const& src)
		{
			return (src.x * src.x + src.y * src.y);
		};

		std::optional<glm::ivec2> smaller_displacement;
		for (int i = 0; i < monitor_count; ++i)
		{
			if (GLFWmonitor* other_monitor = monitors[i])
			{
				if (GLFWvidmode const* other_mode = glfwGetVideoMode(other_monitor))
				{
					// get size and position of the monitor
					glm::ivec2 other_monitor_position = { 0 , 0 };
					glfwGetMonitorPos(other_monitor, &other_monitor_position.x, &other_monitor_position.y);

					glm::ivec2 other_monitor_size = { other_mode->width , other_mode->height };

					// shrink it with the BORDER
					other_monitor_position.x += MIN_HOVER_X;  // do not shrink the monitor area at the top. we don't want fullscreen window to be pushed down
					other_monitor_size.x -= 2 * MIN_HOVER_X;
					other_monitor_size.y -= MIN_HOVER_Y;

					// compute the displacement required so that the window is touching this monitor
					glm::ivec2 required_displacement = { 0, 0 };

					if (window_position.x > other_monitor_position.x + other_monitor_size.x)
						required_displacement.x = (other_monitor_position.x + other_monitor_size.x) - window_position.x;
					else if (window_position.x + window_size.x < other_monitor_position.x)
						required_displacement.x = (other_monitor_position.x - window_size.x) - window_position.x;

					if (window_position.y > other_monitor_position.y + other_monitor_size.y)
						required_displacement.y = (other_monitor_position.y + other_monitor_size.y) - window_position.y;
					else if (window_position.y < other_monitor_position.y)
						required_displacement.y = other_monitor_position.y - window_position.y;

					// compare displacement with the best case
					if ((!smaller_displacement.has_value()) ||
						(length2(smaller_displacement.value()) > length2(required_displacement)))
					{
						smaller_displacement = required_displacement;
					}

					// early exit if no displacement is required
					if (smaller_displacement.value().x == 0 && smaller_displacement.value().y == 0)
					{
						smaller_displacement.reset(); // nothing to update
						break;
					}
				}
			}
		}

		// update
		if (smaller_displacement.has_value())
			window_position += smaller_displacement.value();
	}

	void Window::SetWindowPlacement(WindowPlacementInfo placement_info)
	{
		// compute the monitor over which the window will be
		GLFWmonitor* monitor = placement_info.monitor;
		if (monitor == nullptr)
		{
			// explicitly given monitor
			if (placement_info.monitor_index.has_value())
				monitor = GLFWTools::GetMonitorByIndex(placement_info.monitor_index.value());
			// position is an absolute position. get the corresponding monitor and transform position into monitor local
			else if (placement_info.position.has_value()) 
			{
				monitor = GLFWTools::GetNearestMonitor(placement_info.position.value());
				placement_info.position = GLFWTools::AbsolutePositionToMonitor(placement_info.position.value(), monitor);
			}
			// fallback on primary monitor
			else
				monitor = glfwGetPrimaryMonitor();

			if (monitor == nullptr) // all monitors are off
				return;
		}

		// XXX: at this point, if placement_info.position is defined, it is relative to the monitor

		// retrieve the position and size of the monitor
		GLFWvidmode const* mode = glfwGetVideoMode(monitor);
		if (mode == nullptr)
			return;
		glm::ivec2 monitor_size = { mode->width , mode->height };

		// compute the position and size of the window
		bool pseudo_fullscreen = true;
		if (!placement_info.fullscreen)
			if (placement_info.size.has_value())
				if (placement_info.size.value().x > 0 && placement_info.size.value().y > 0)
					pseudo_fullscreen = false;

		// compute window size and position
		glm::ivec2 window_position = { 0, 0 };
		glm::ivec2 window_size = { 0, 0 };

		if (pseudo_fullscreen) // full-screen, the window use the full-size
		{
			// update internals
			if (placement_info.fullscreen) // no decorator
			{
				fullscreen_monitor = monitor;

				glfwSetWindowAttrib(glfw_window, GLFW_DECORATED, 0);
#if !_DEBUG
				glfwSetWindowAttrib(glfw_window, GLFW_FLOATING, 1);
#endif
			}
			else // use initial settings to know whether decorators are necessary
			{
				fullscreen_monitor = nullptr;

				glfwSetWindowAttrib(glfw_window, GLFW_DECORATED, initial_decorated ? 1 : 0);
#if !_DEBUG
				glfwSetWindowAttrib(glfw_window, GLFW_FLOATING, initial_toplevel ? 1 : 0);
#endif
			}
			// compute size and position
			window_size = monitor_size;
			window_position = { 0, 0 };
			// set position in absolute coordinates
			window_position = GLFWTools::MonitorPositionToAbsolute(window_position, monitor);
		}
		else
		{
			assert(placement_info.size.has_value());

			// update internals
			fullscreen_monitor = nullptr;
			glfwSetWindowAttrib(glfw_window, GLFW_DECORATED, initial_decorated? 1 : 0);
#if !_DEBUG
			glfwSetWindowAttrib(glfw_window, GLFW_FLOATING, initial_toplevel? 1 : 0);
#endif

			// compute size
			window_size = placement_info.size.value();
			// compute position (local to concerned monitor)
			if (placement_info.position.has_value())
				window_position = placement_info.position.value();
			else
				window_position = (monitor_size - window_size) / 2; // use center of screen by default

			// set position in absolute coordinates
			window_position = GLFWTools::MonitorPositionToAbsolute(window_position, monitor);

			// ensure a large enough of the window is visible in any monitor (so that it can be displaced by the user)
			EnsureWindowToolbarVisibleInAnyMonitor(window_position, window_size);
		}
		// update placement
		SetWindowSize(window_size, true); // include decorators
		SetWindowPosition(window_position, true); // include decorators
	}

	bool Window::CreateGLFWWindow(WindowPlacementInfo const & placement_info, WindowCreateParams const &create_params, GLFWwindow* share_context_window, GLFWHints glfw_hints)
	{
		// resource already existing
		if (glfw_window != nullptr)
			return false;

		// prepare window creation
		glfwWindowHint(GLFW_RESIZABLE, create_params.resizable);
		glfwWindowHint(GLFW_VISIBLE, create_params.start_visible);
		glfwWindowHint(GLFW_DECORATED, create_params.decorated);
		glfwWindowHint(GLFW_FLOATING, create_params.toplevel);
		glfwWindowHint(GLFW_FOCUSED, create_params.focused);
		glfwWindowHint(GLFW_VISIBLE, 0); // override the initial visibility

		initial_toplevel = create_params.toplevel;
		initial_decorated = create_params.decorated;
		double_buffer = (glfw_hints.double_buffer ? true : false);
	
		// we are doing a pseudo fullscreen => monitor parameters of glfwCreateWindow must be null or it will "capture" the screen
		// use a random size that will be corrected later in SetWindowPlacement call
		glfw_window = glfwCreateWindow(500, 500, create_params.title.c_str(), nullptr, share_context_window);
		if (glfw_window == nullptr)
			return false;
		glfwMakeContextCurrent(glfw_window);

		// set vsync
		if (GlobalVariables::UnlimitedFPS.Get())
			glfw_hints.unlimited_fps = true;

		if (glfw_hints.unlimited_fps)
		{
			WithWindowContext([]()
			{
				glfwSwapInterval(0);
			});
		}

		// set the callbacks
		SetGLFWCallbacks();

		// some input mode
		glfwSetInputMode(glfw_window, GLFW_STICKY_KEYS, 1);

		// prepare cursor mode
		cursor_mode = (CursorMode)glfwGetInputMode(glfw_window, GLFW_CURSOR);
		SetCursorMode(CursorMode::Disabled);

		// update placement
		SetWindowPlacement(placement_info);

		// now that the window is fully placed ... we can show it
		if (create_params.start_visible)
			glfwShowWindow(glfw_window);

		return true;
	}

	void Window::IncrementWindowDestructionGuard()
	{
		++window_destruction_guard;
	}

	void Window::DecrementWindowDestructionGuard()
	{
		assert(window_destruction_guard > 0);
		if (--window_destruction_guard == 0)
			if (WindowApplication* window_application = Application::GetInstance())
				if (!window_application->IsWindowHandledByApplication(this)) // window already removed from the application's list. just destroy resources
					window_application->OnWindowDestroyed(this);
	}

	void Window::CreateImGuiContext()
	{
		// save imgui context
		ImGuiContext* previous_imgui_context = ImGui::GetCurrentContext();

		// create a new context for this window
		imgui_context = ImGui::CreateContext();
		ImGui::SetCurrentContext(imgui_context);
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavNoCaptureKeyboard;  // Do not capture keyboard during navigation
		ImGui::StyleColorsDark();

		// initialize the context
		//
		// note: we don't install callbacks. we will manually call the functions ourselves
		//       under normal circumstances, we have the following call chain
		//
		//       Window Message Handler (or NOT) -> GLFW call -> IMGUI callbacks -> our callbacks
		//                ^
		//                +-- settings the IMGUI context here is not enought
		//
		//       due to our multiple-window system, inside the IMGUI callbacks, the current IMGUI context may not be set causing a crash
		//       -> for example, push both LEFT and RIGHT shift and release one of theses keys
		//
		//       we can change the Window Message Function to ensure the IMG context is properly set, but some time the GLFW call is not made from inside the Window Message Handler
		//
		//       By not installing IMGUI callbacks we have the following chain
		//
		//       Window Message Handler or (NOT) -> GLFW call -> our callbacks -> set IMGUI context -> manually call IMGUI callbacks
		//
		//       see in ImGui_ImplGlfw_InstallCallbacks (IMGUI source code) which callback are concerned

		ImGui_ImplGlfw_InitForOpenGL(GetGLFWHandler(), false); // do not install callbacks !!
		ImGui_ImplOpenGL3_Init("#version 130");

		// substitute our own window proc (now that ImGui has inserted its own WindowProc */
#if _WIN32
		SetImGuiWindowProc(true);
#endif // #if _WIN32
		// the context is ready for rendering
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// restore previous imgui context
		ImGui::SetCurrentContext(previous_imgui_context);
	}

	bool Window::CreateRootWidget()
	{
		// create the root widget
		root_widget = new WindowRootWidget;
		if (root_widget != nullptr)
		{
			root_widget->window = this;
			return true;
		}
		return false;
	}

	void Window::SetCursorMode(CursorMode mode)
	{
		if (cursor_mode != mode)
		{
			cursor_mode = mode;
			if (!GetImGuiMenuMode())
				glfwSetInputMode(glfw_window, GLFW_CURSOR, (int)mode); // do not effectively change cursor mode, during imgui_menu_mode
		}
	}

	CursorMode Window::GetCursorMode() const
	{
		return cursor_mode;
	}

	bool Window::GetImGuiMenuMode() const
	{
		if (WindowApplication* window_application = Application::GetInstance())
			return window_application->GetImGuiMenuMode();
		return false;
	}

	void Window::OnImGuiMenuModeChanged(bool mode)
	{
		if (mode)
			glfwSetInputMode(glfw_window, GLFW_CURSOR, (int)CursorMode::Normal);
		else
			glfwSetInputMode(glfw_window, GLFW_CURSOR, (int)cursor_mode);
	}

	glm::ivec2 Window::GetWindowPosition(bool include_decorators) const
	{
		glm::ivec2 result = { 0, 0 };
		if (glfw_window != nullptr)
		{
			// get client top-left coner
			glfwGetWindowPos(glfw_window, &result.x, &result.y);

			// include decorators
			if (include_decorators)
			{
				WindowFrameSizeInfo framesize_info = GetWindowFrameSizeInfo(glfw_window);
				result.x -= framesize_info.left;
				result.y -= framesize_info.top;
			}
		}	
		return result;
	}

	glm::ivec2 Window::GetWindowSize(bool include_decorators) const
	{
		glm::ivec2 result = { 0, 0 };
		if (glfw_window != nullptr)
		{
			// get the client size
			glfwGetWindowSize(glfw_window, &result.x, &result.y);

			// include decorators
			if (include_decorators)
			{
				WindowFrameSizeInfo framesize_info = GetWindowFrameSizeInfo(glfw_window);
				result.x += framesize_info.left + framesize_info.right;
				result.y += framesize_info.top  + framesize_info.bottom;
			}
		}
		return result;
	}

	void Window::SetWindowPosition(glm::ivec2 position, bool include_decorators)
	{
		if (glfw_window == nullptr)
			return;

		if (include_decorators) // client area is displaced to the right and the bottom
		{
			WindowFrameSizeInfo framesize_info = GetWindowFrameSizeInfo(glfw_window);
			position.x += framesize_info.left;
			position.y += framesize_info.top;
		}
		glfwSetWindowPos(glfw_window, position.x, position.y); // glfwSetWindowPos(...) receives client information
	}

	void Window::SetWindowSize(glm::ivec2 size, bool include_decorators)
	{
		if (glfw_window == nullptr)
			return;
		if (include_decorators) // client aera is smaller than the given size
		{
			WindowFrameSizeInfo framesize_info = GetWindowFrameSizeInfo(glfw_window);
			size.x -= (framesize_info.left + framesize_info.right);
			size.y -= (framesize_info.top  + framesize_info.bottom);
		}
		glfwSetWindowSize(glfw_window, size.x, size.y); // glfwSetWindowSize(...) receives client information
	}

	GLFWwindow* Window::GetGLFWHandler()
	{
		return glfw_window;
	}

	bool Window::ShouldClose()
	{
		return (glfw_window != nullptr && glfwWindowShouldClose(glfw_window) != 0);
	}

	void Window::TickRenderer(float real_delta_time)
	{
		// cannot tick a non existing window
		if (glfw_window == nullptr || renderer == nullptr)
			return;

		WithWindowContext([this, real_delta_time]()
		{
			// tick the renderer: it has metrics that rely on the real frame_rate (not modified one due to some of our tweaks)
			renderer->Tick(real_delta_time);
		});
	}

	bool Window::IsMousePositionValid() const
	{
		return mouse_position.has_value();
	}

	glm::vec2 Window::GetMousePosition() const
	{
		if (!IsMousePositionValid())
			return glm::vec2(0.0f, 0.0f);
		return mouse_position.value();
	}

#if _WIN32

	LRESULT CALLBACK Window::ImGuiWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		LRESULT result = 0;

		if (Window* window = (Window*)GetPropW(hWnd, L"CHAOS_WINDOW"))
		{
			if (WNDPROC previous_wndproc = (WNDPROC)GetPropW(hWnd, L"CHAOS_PREVIOUS_WNDPROC"))
			{
				// prevent the destruction of the window during windows message function
				shared_ptr<Window> prevent_destruction = window;
				window->IncrementWindowDestructionGuard();
				// set the ImGui context as current
				ImGuiContext* previous_imgui_context = ImGui::GetCurrentContext();
				ImGuiContext* toset_imgui_context = window->imgui_context;
				ImGui::SetCurrentContext(toset_imgui_context);
				// call special treatments
				window->HookedWindowProc(msg, wParam, lParam);
				// call "super" window proc
				result = ::CallWindowProc(previous_wndproc, hWnd, msg, wParam, lParam); // call "super" WndProc
				// restore the previous ImGui context
				if (toset_imgui_context != previous_imgui_context) // maybe previous context was same then window's context and window's context has been deleted
					ImGui::SetCurrentContext(previous_imgui_context);
				// enable window resource destruction back
				window->DecrementWindowDestructionGuard();
			}
		}
		return result;
	}

	bool Window::HookedWindowProc(UINT msg, WPARAM wParam, LPARAM lParam)
	{
		if (msg == WM_INPUTLANGCHANGE) // note: WM_INPUTLANGCHANGE is sent only to the topmost window of the EXE. That's exactly what we want
		{
			if (WindowApplication* application = Application::GetInstance())
				application->OnInputLanguageChanged(); // the application will be responsible to propage the change to all windows
			return true;
		}
		return false;
	}

	void Window::OnInputLanguageChanged()
	{
	}

	// Note on ImGui and WndProc
	//
	//   first GLFW install its own WndProc to make a bind between WIN32API and GLFW
	//
	//   then ImGui install its own WndProc on top of that (see ImGui_ImplGlfw_WndProc)
	//
	//   The issue is that ImGui WndProc relies on the current ImGui context to dispatch events. The current ImGui context does not necessary correspond the the window whose message is being processed
	//
	//   That's why we install our own WndProc on top of all to get a ImGui from a HWND (and restore it to nullptr at the end)
	//
	//   Thoses layers of WndProc must be removed in reverse order
	//
	//   Our new WndProc does not use glfwMakeCurrentContext(...) to make current GLFW context active because:
	//   - this doesn't seem to be necessary
	//   - this function is much more costly than ImGui::SetCurrentContext(...)
	//   - There is a huge number of window events and we are only interrested to have a GLFW context in some glfw callback

	void Window::SetImGuiWindowProc(bool set_proc)
	{
		HWND hWnd = glfwGetWin32Window(GetGLFWHandler());
		// set the WndProc
		if (set_proc)
		{
			assert(GetPropW(hWnd, L"CHAOS_WINDOW") == nullptr);
			assert(GetPropW(hWnd, L"CHAOS_PREVIOUS_WNDPROC") == nullptr);
			SetPropW(hWnd, L"CHAOS_WINDOW", this);
			SetPropW(hWnd, L"CHAOS_PREVIOUS_WNDPROC", (WNDPROC)::GetWindowLongPtr(hWnd, GWLP_WNDPROC));
			::SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)&Window::ImGuiWindowProc);
		}
		// restore the WndProc
		else
		{
			assert(GetPropW(hWnd, L"CHAOS_WINDOW") != nullptr);
			assert(GetPropW(hWnd, L"CHAOS_PREVIOUS_WNDPROC") != nullptr);
			::SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)GetPropW(hWnd, L"CHAOS_PREVIOUS_WNDPROC"));
			RemovePropW(hWnd, L"CHAOS_WINDOW");
			RemovePropW(hWnd, L"CHAOS_PREVIOUS_WNDPROC");
		}
	}

#endif // #if _WIN32

	void Window::SetGLFWCallbacks()
	{
		glfwSetWindowUserPointer(glfw_window, this);

		glfwSetCursorPosCallback(glfw_window, DoOnMouseMove);
		glfwSetMouseButtonCallback(glfw_window, DoOnMouseButton);
		glfwSetScrollCallback(glfw_window, DoOnMouseWheel);
		glfwSetWindowSizeCallback(glfw_window, DoOnWindowResize);
		glfwSetKeyCallback(glfw_window, DoOnKeyEvent);
		glfwSetCharCallback(glfw_window, DoOnCharEvent);
		glfwSetWindowCloseCallback(glfw_window, DoOnWindowClosed);
		glfwSetDropCallback(glfw_window, DoOnDropFile);
		glfwSetWindowFocusCallback(glfw_window, DoOnFocusStateChange);
		glfwSetWindowIconifyCallback(glfw_window, DoOnIconifiedStateChange);
	}

	template<typename FUNC>
	static void GetWindowAndProcess(GLFWwindow* in_glfw_window, FUNC const& func)
	{
		if (Window* my_window = (Window*)glfwGetWindowUserPointer(in_glfw_window))
		{
			my_window->WithWindowContext([my_window, &func]()
			{
				func(my_window);
			});
		}
	}

	void Window::DoOnIconifiedStateChange(GLFWwindow* in_glfw_window, int value)
	{
		GetWindowAndProcess(in_glfw_window, [value](Window * my_window)
		{
			my_window->OnIconifiedStateChange(value == GL_TRUE);
		});
	}

	void Window::DoOnFocusStateChange(GLFWwindow* in_glfw_window, int value)
	{
		GetWindowAndProcess(in_glfw_window, [=](Window * my_window)
		{
			// XXX: manually call IMGUI callbacks. Check for context because this could be called even before IMGUI is fully bound to the window
			if (ImGui::GetCurrentContext() != nullptr)
				ImGui_ImplGlfw_WindowFocusCallback(in_glfw_window, value);

			my_window->OnFocusStateChange(value == GL_TRUE);
		});
	}

	void Window::DoOnWindowClosed(GLFWwindow* in_glfw_window)
	{
		GetWindowAndProcess(in_glfw_window, [](Window* my_window)
		{
			if (my_window->OnWindowClosed())
				my_window->RequireWindowClosure();
		});
	}

	void Window::DoOnWindowResize(GLFWwindow* in_glfw_window, int width, int height)
	{
		GetWindowAndProcess(in_glfw_window, [width, height](Window* my_window)
		{
			my_window->OnWindowResize(glm::ivec2(width, height));
		});
	}

	void Window::DoOnMouseMove(GLFWwindow* in_glfw_window, double x, double y)
	{
		GetWindowAndProcess(in_glfw_window, [=](Window* my_window)
		{
			// XXX: manually call IMGUI callbacks. Check for context because this could be called even before IMGUI is fully bound to the window
			if (ImGui::GetCurrentContext() != nullptr)
			{
				ImGui_ImplGlfw_CursorPosCallback(in_glfw_window, x, y);

				ImGuiIO& io = ImGui::GetIO();
				if (io.WantCaptureMouse)
					return;
			}

			Application::SetApplicationInputMode(InputMode::MOUSE);

			// try process ourselves
			shuxxx





			glm::vec2 position = { float(x), float(y) };
			if (!my_window->IsMousePositionValid())
				my_window->OnMouseMove({ 0.0f, 0.0f });
			else
				my_window->OnMouseMove(position - my_window->mouse_position.value());
			my_window->mouse_position = position;

			// give opportunity to application
			if (Application* application = Application::GetInstance())
				application->OnMouseMove(delta);
		});
	}

	void Window::DoOnMouseButton(GLFWwindow* in_glfw_window, int button, int action, int modifier)
	{
		GetWindowAndProcess(in_glfw_window, [=](Window* my_window)
		{
			// XXX: manually call IMGUI callbacks. Check for context because this could be called even before IMGUI is fully bound to the window
			if (ImGui::GetCurrentContext() != nullptr)
			{
				ImGui_ImplGlfw_MouseButtonCallback(in_glfw_window, button, action, modifier);

				ImGuiIO& io = ImGui::GetIO();
				if (io.WantCaptureMouse)
					return;
			}

			MouseButton mouse_button = (MouseButton)button;
			KeyboardState::SetMouseButtonState(mouse_button, action);

			// notify the application of the keyboard button state
			if (WindowApplication* application = Application::GetInstance())
				application->SetInputMode(InputMode::MOUSE);

			// try process ourselves
			if (my_window->OnMouseButton(button, action, modifier))
				return;
			// give opportunity to application
			if (Application* application = Application::GetInstance())
				application->OnMouseButton(button, action, modifier);
		});
	}

	void Window::DoOnMouseWheel(GLFWwindow* in_glfw_window, double scroll_x, double scroll_y)
	{
		GetWindowAndProcess(in_glfw_window, [=](Window* my_window)
		{
			// XXX: manually call IMGUI callbacks. Check for context because this could be called even before IMGUI is fully bound to the window
			if (ImGui::GetCurrentContext() != nullptr)
			{
				ImGui_ImplGlfw_ScrollCallback(in_glfw_window, scroll_x, scroll_y);

				ImGuiIO& io = ImGui::GetIO();
				if (io.WantCaptureMouse)
					return;
			}

			Application::SetApplicationInputMode(InputMode::MOUSE);

			// try process ourselves
			if (my_window->OnMouseWheel(scroll_x, scroll_y))
				return;
			// give opportunity to application
			if (Application* application = Application::GetInstance())
				application->OnMouseWheel(scroll_x, scroll_y);
		});
	}

	void Window::DoOnKeyEvent(GLFWwindow* in_glfw_window, int keycode, int scancode, int action, int modifier)
	{
		GetWindowAndProcess(in_glfw_window, [=](Window* my_window)
		{
			// XXX: manually call IMGUI callbacks. Check for context because this could be called even before IMGUI is fully bound to the window
			if (ImGui::GetCurrentContext() != nullptr)
			{
				ImGui_ImplGlfw_KeyCallback(in_glfw_window, keycode, scancode, action, modifier);

				ImGuiIO& io = ImGui::GetIO();
				if (io.WantCaptureKeyboard)
					return;
			}

			// GLFW keycode corresponds to the character that would be produced on a QWERTY layout
			// we have to make a conversion to know the character is to be produced on CURRENT layout

			KeyboardButton keyboard_button = KeyboardButton(KeyboardLayoutConversion::ConvertGLFWKeycode(keycode, KeyboardLayoutType::QWERTY, KeyboardLayoutType::CURRENT));

			KeyboardState::SetKeyboardButtonState(keyboard_button, action);

			// notify the application of the keyboard button state
			if (WindowApplication* application = Application::GetInstance())
				application->SetInputMode(InputMode::KEYBOARD);

			// handle the message
			KeyEvent event;
			event.button = keyboard_button;
			event.scancode = scancode;
			event.action = action;
			event.modifier = modifier;

			// try process ourselves
			if (my_window->OnKeyEvent(event))
				return;
			// give opportunity to application
			if (Application* application = Application::GetInstance())
				application->OnKeyEvent(event);
		});
	}

	void Window::DoOnCharEvent(GLFWwindow* in_glfw_window, unsigned int c)
	{
		GetWindowAndProcess(in_glfw_window, [=](Window* my_window)
		{
			// XXX: manually call IMGUI callbacks. Check for context because this could be called even before IMGUI is fully bound to the window
			if (ImGui::GetCurrentContext() != nullptr)
			{
				ImGui_ImplGlfw_CharCallback(in_glfw_window, c);

				ImGuiIO& io = ImGui::GetIO();
				if (io.WantCaptureKeyboard)
					return;
			}

			Application::SetApplicationInputMode(InputMode::KEYBOARD);

			// try process ourselves
			if (my_window->OnCharEvent(c))
				return;
			// give opportunity to application
			if (Application* application = Application::GetInstance())
				application->OnCharEvent(c);
		});
	}

	void Window::DoOnDropFile(GLFWwindow* in_glfw_window, int count, char const** paths)
	{
		GetWindowAndProcess(in_glfw_window, [count, paths](Window* my_window)
		{
			my_window->OnDropFile(count, paths);
		});
	}

	void Window::DrawWindow()
	{
		if (glfw_window != nullptr && renderer != nullptr)
		{
			assert(glfw_window == glfwGetCurrentContext());

			renderer->BeginRenderingFrame();

			// data provider
			GPUProgramProviderCommonTransforms common_transforms; // some common deduction rules

			GPUProgramProviderInterface const* application_provider_interface = Application::GetInstance(); // cast the application into a provider_interface

			GPUProgramProviderChain provider(this, application_provider_interface, common_transforms); // order: window, application, common deduction rules

			// render
			if (DrawInternal(&provider))
			{
				if (double_buffer)
					glfwSwapBuffers(glfw_window);
				else
					glFlush();
			}
			renderer->EndRenderingFrame();
		}
	}

	void Window::RequireWindowClosure()
	{
		glfwSetWindowShouldClose(glfw_window, 1);
	}

	GLFWmonitor* Window::GetPreferredMonitor() const
	{
		glm::ivec2 top_left_corner = GetWindowPosition(true); // include decorators
		return GLFWTools::GetNearestMonitor(top_left_corner);
	}

	void Window::SetFullscreen(GLFWmonitor* monitor)
	{
		if (glfw_window == nullptr)
			return;

		WindowPlacementInfo placement_info;

		if (monitor == nullptr) // want to "unfull screen"
		{
			if (fullscreen_monitor != nullptr) // is currently fullscreen
			{
				if (non_fullscreen_data.has_value())
				{
					placement_info.position   = non_fullscreen_data->position; // absolute placement of the window (monitor not defined)
					placement_info.size       = non_fullscreen_data->size;
					placement_info.fullscreen = false;
					SetWindowPlacement(placement_info);
				}
				else
				{
					if (GLFWmonitor* preferred_monitor = GetPreferredMonitor())
					{
						if (GLFWvidmode const* mode = glfwGetVideoMode(preferred_monitor))
						{
							glm::ivec2 monitor_size = { mode->width, mode->height}; // half monitor size

							placement_info.monitor    = preferred_monitor;
							placement_info.size       = monitor_size / 2;
							placement_info.position   = (monitor_size - placement_info.size.value()) / 2; // relative placement to given monitor
							placement_info.fullscreen = false;
							SetWindowPlacement(placement_info);
						}
					}
				}
			}
		}
		else // monitor not null
		{
			if (fullscreen_monitor != monitor) // is not currently fullscreen or for another monitor
			{
				// store current information (if not already fullscreen)
				if (fullscreen_monitor == nullptr)
				{
					NonFullScreenWindowData data;
					data.position = GetWindowPosition(true); // include decorators
					data.size     = GetWindowSize(true);
					non_fullscreen_data = data;
				}
				placement_info.monitor = monitor; // fullscreen placement
				placement_info.fullscreen = true;
				SetWindowPlacement(placement_info);
			}
		}
	}

	void Window::ToggleFullscreen()
	{
		if (fullscreen_monitor != nullptr)
			SetFullscreen(nullptr);
		else if (GLFWmonitor* preferred_monitor = GetPreferredMonitor())
			SetFullscreen(preferred_monitor);
	}

	bool Window::ScreenCapture()
	{
		// get renderer
		if (glfw_window == nullptr || renderer == nullptr)
			return false;

		return WithWindowContext([this]()
		{
			// this call may take a while causing a jump in 'delta_time'
			WindowApplication* application = Application::GetInstance();
			if (application == nullptr)
				return false;
			application->FreezeNextFrameTickDuration();

			// compute rendering size
			glm::ivec2 window_size = GetWindowSize(false); // only interrested in client area

			// generate a framebuffer
			GPUFramebufferGenerator framebuffer_generator;
			framebuffer_generator.AddColorAttachment(0, PixelFormat::BGRA, window_size, "scene");
			framebuffer_generator.AddDepthStencilAttachment(window_size, "depth");

			shared_ptr<GPUFramebuffer> framebuffer = framebuffer_generator.GenerateFramebuffer(window_size);
			if (framebuffer == nullptr)
				return false;
			if (!framebuffer->CheckCompletionStatus())
				return false;

			// render in the frame buffer
			GPUProgramProviderCommonTransforms common_transforms;
			GPUProgramProviderChain provider(this, (WindowApplication*)Application::GetInstance(), common_transforms);

			renderer->BeginRenderingFrame();
			renderer->PushFramebufferRenderContext(framebuffer.get(), false);

			DrawInternal(&provider);

			renderer->PopFramebufferRenderContext();
			renderer->EndRenderingFrame();

			// extract the texture
			GPUFramebufferAttachmentInfo const* attachment = framebuffer->GetColorAttachment(0);
			if (attachment == nullptr || attachment->texture == nullptr)
				return false;

			bitmap_ptr img = bitmap_ptr(ImageTools::GenFreeImage(attachment->texture->GetResourceID(), 0));
			if (img == nullptr)
				return false;

			// create the directory
			boost::filesystem::path capture_directory_path = application->GetUserLocalTempPath() / "Captures";
			if (!boost::filesystem::is_directory(capture_directory_path))
				if (!boost::filesystem::create_directories(capture_directory_path))
					return false;

			// save the image
			std::string format = StringTools::Printf(
				"capture_%s_%%d.png",
				StringTools::TimeToString(std::chrono::system_clock::now(), TimeToStringFormatType::FILENAME).c_str());

			boost::filesystem::path file_path = FileTools::GetUniquePath(capture_directory_path, format.c_str(), true);
			if (file_path.empty())
				return false;

			return (FreeImage_Save(FIF_PNG, img.get(), file_path.string().c_str(), 0) != 0);
		});
	}

	aabox2 Window::GetRequiredViewport(glm::ivec2 const& size) const
	{
		aabox2 result;
		result.position = { 0, 0 };
		result.size = size;
		return SetBoxAspect(result, 16.0f / 9.0f, SetBoxAspectMethod::SHRINK_BOX);
	}

	CHAOS_HELP_TEXT(SHORTCUTS, "F9  : ScreenCapture");
	CHAOS_HELP_TEXT(SHORTCUTS, "F10 : ToggleFullscreen");

	bool Window::EnumerateKeyEventActions(KeyEventActionProcessor & processor)
	{
		bool result = false;

		result = processor(KeyboardButton::F9, 0, "Screen Capture", [this]()
		{
			ScreenCapture();
		});
		if (result)
			return true;

		result = processor(KeyboardButton::F10, 0, "Toggle Fullscreen", [this]()
		{
			ToggleFullscreen();
		});
		if (result)
			return true;

		return false;
	}

	bool Window::OnKeyEventImpl(KeyEvent const& event)
	{
		// kill the window (use ALT+F4 for the moment)
#if 0
		if (event.IsKeyPressed(KeyboardButton::ESCAPE, GLFW_MOD_SHIFT))
		{
			RequireWindowClosure();
			return true;
		}
#endif
		// screen capture
		// CMD F9 : ScreenCapture(...)
		if (event.IsKeyPressed(KeyboardButton::F9))
		{
			ScreenCapture();
			return true;
		}
		// try to go fullscreen
		// CMD F10 : ToggleFullscreen(...)
		if (event.IsKeyPressed(KeyboardButton::F10))
		{
			ToggleFullscreen();
			return true;
		}
		// super method
		return WindowInterface::OnKeyEventImpl(event);
	}

	void Window::DrawWindowImGui()
	{
		OnDrawWindowImGuiMenu();
		OnDrawWindowImGuiContent();
	}

	void Window::OnDrawWindowImGuiMenu()
	{
		if (WindowApplication* window_application = Application::GetInstance())
		{
			window_application->OnDrawWindowImGuiMenu(this);
		}

		DrawImGui(ImGuiDrawMenuMode::FullWindow); // the owned window
	}

	void Window::OnDrawWindowImGuiContent()
	{
	}

	bool Window::DrawInternal(GPUProgramProviderInterface const* uniform_provider)
	{
		bool result = false;

		glm::ivec2 window_size = GetWindowSize(false); // only interrested in client area
		if (window_size.x <= 0 || window_size.y <= 0) // some crash to expect in drawing elsewhere
			return result;

		// some parameters
		WindowDrawParams draw_params;
		draw_params.viewport = GetRequiredViewport(window_size);

		// draw the viewport
		if (!IsGeometryEmpty(draw_params.viewport))
		{
			GLTools::SetViewport(draw_params.viewport);
			result |= OnDraw(renderer.get(), uniform_provider, draw_params);
		}
		// draw the root widget
		if (root_widget != nullptr)
		{
			if (root_widget->IsUpdatePlacementHierarchyRequired())
				UpdateWidgetPlacementHierarchy();
			result |= root_widget->OnDraw(renderer.get(), uniform_provider, draw_params);
		}
		// draw ImGui
		DrawWindowImGui();

		// finalize the rendering
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// prepare rendering until we come back into this function
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		return result;
	}

	void Window::UpdateWidgetPlacementHierarchy()
	{
		if (root_widget != nullptr)
		{
			aabox2 placement;
			placement.position = { 0, 0 };
			placement.size = GetWindowSize(false); // only interrested in client area
			root_widget->SetPlacement(placement);
		}
	}

	bool Window::OnDraw(GPURenderer* renderer, GPUProgramProviderInterface const* uniform_provider, WindowDrawParams const& draw_params)
	{
		glm::vec4 clear_color(0.0f, 0.0f, 0.0f, 0.0f);
		glClearBufferfv(GL_COLOR, 0, (GLfloat*)&clear_color);
		return true;
	}

	bool Window::DoTick(float delta_time)
	{
		bool result = TickableInterface::DoTick(delta_time);
		if (root_widget != nullptr)
		{
			if (root_widget->IsUpdatePlacementHierarchyRequired())
				UpdateWidgetPlacementHierarchy();
			result |= root_widget->Tick(delta_time);
		}
		return result;
	}

	bool Window::Initialize()
	{
		if (!ReadConfigurableProperties(ReadConfigurablePropertiesContext::INITIALIZATION, false))
			return false;
		return InitializeFromConfiguration(GetJSONReadConfiguration().default_config);
	}

	bool Window::InitializeFromConfiguration(nlohmann::json const * config)
	{
		RegisterKnownDrawables();
		return true;
	}

	void Window::RegisterKnownDrawables()
	{
		RegisterDrawable("System Information", [this]()
		{
			return new ImGuiSystemInformationDrawable;
		});

		RegisterDrawable("Window Information", [this]()
		{
			ImGuiWindowInformationDrawable* result = new ImGuiWindowInformationDrawable;
			if (result != nullptr)
				result->SetWindow(this);
			return result;
		});

		RegisterDrawable("Help", [this]()
		{
			return new ImGuiHelpDrawable;
		});
	}

	bool Window::OnConfigurationChanged(JSONReadConfiguration config)
	{
		return ConfigurableInterface::OnConfigurationChanged(config);
	}

	bool Window::OnReadConfigurableProperties(JSONReadConfiguration config, ReadConfigurablePropertiesContext context)
	{
		return true;
	}

	bool Window::OnStorePersistentProperties(JSONWriteConfiguration config) const
	{
		if (glfw_window != nullptr)
		{
			if (GLFWmonitor* preferred_monitor = GetPreferredMonitor())
			{
				// gather information
				glm::ivec2 window_position = GetWindowPosition(true); // include decorators
				window_position = GLFWTools::AbsolutePositionToMonitor(window_position, preferred_monitor); // relative to preferred monitor

				glm::ivec2 window_size = GetWindowSize(true); // include decorators

				int monitor_index = GLFWTools::GetMonitorIndex(preferred_monitor);
				// store information
				JSONTools::SetAttribute(config, "position", window_position);
				JSONTools::SetAttribute(config, "size", window_size);
				JSONTools::SetAttribute(config, "monitor_index", monitor_index);
				JSONTools::SetAttribute(config, "fullscreen", IsFullscreen());
			}
		}
		return true;
	}

	bool Window::DoProcessAction(GPUProgramProviderExecutionData const& execution_data) const
	{
		return false;
	}

	void Window::OnWindowResize(glm::ivec2 size)
	{
		UpdateWidgetPlacementHierarchy();
	}

	bool Window::OnWindowClosed()
	{
		return true;
	}

	void Window::OnDropFile(int count, char const** paths)
	{
	}

	void Window::OnIconifiedStateChange(bool iconified)
	{
	}

	void Window::OnFocusStateChange(bool gain_focus)
	{
	}

	void Window::OnMonitorEvent(GLFWmonitor* monitor, int monitor_state)
	{
		if (monitor == fullscreen_monitor && monitor_state == GLFW_DISCONNECTED)
			fullscreen_monitor = nullptr;
	}

	void Window::Finalize()
	{
	}

}; // namespace chaos
