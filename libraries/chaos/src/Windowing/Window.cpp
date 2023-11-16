#include "chaos/ChaosPCH.h"
#include "chaos/ChaosInternals.h"

namespace chaos
{
	/**
	* WindowCreateParams
	*/

	bool SaveIntoJSON(nlohmann::json& json, WindowCreateParams const& src)
	{
		if (!json.is_object())
			json = nlohmann::json::object();
		JSONTools::SetAttribute(json, "monitor_index", src.monitor_index);
		JSONTools::SetAttribute(json, "width", src.width);
		JSONTools::SetAttribute(json, "height", src.height);
		JSONTools::SetAttribute(json, "resizable", src.resizable);
		JSONTools::SetAttribute(json, "start_visible", src.start_visible);
		JSONTools::SetAttribute(json, "decorated", src.decorated);
		JSONTools::SetAttribute(json, "toplevel", src.toplevel);
		JSONTools::SetAttribute(json, "focused", src.focused);
		return true;
	}

	bool LoadFromJSON(nlohmann::json const& json, WindowCreateParams& dst)
	{
		if (!json.is_object())
			return false;
		JSONTools::GetAttribute(json, "monitor_index", dst.monitor_index);
		JSONTools::GetAttribute(json, "width", dst.width);
		JSONTools::GetAttribute(json, "height", dst.height);
		JSONTools::GetAttribute(json, "resizable", dst.resizable);
		JSONTools::GetAttribute(json, "start_visible", dst.start_visible);
		JSONTools::GetAttribute(json, "decorated", dst.decorated);
		JSONTools::GetAttribute(json, "toplevel", dst.toplevel);
		JSONTools::GetAttribute(json, "focused", dst.focused);
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

	bool Window::CreateGLFWWindow(WindowCreateParams create_params, GLFWwindow* share_context_window, GLFWHints glfw_hints)
	{
		// resource already existing
		if (glfw_window != nullptr)
			return false;

		// compute the monitor upon which the window will be : use it for pixel format
		if (create_params.monitor == nullptr)
			create_params.monitor = GLFWTools::GetMonitorByIndex(create_params.monitor_index);

		// retrieve the position of the monitor
		int monitor_x = 0;
		int monitor_y = 0;
		glfwGetMonitorPos(create_params.monitor, &monitor_x, &monitor_y);

		// compute the position and size of the window
		bool pseudo_fullscreen = (create_params.width <= 0 && create_params.height <= 0);

		// prepare window creation
		glfwWindowHint(GLFW_RESIZABLE, create_params.resizable);
		glfwWindowHint(GLFW_VISIBLE, create_params.start_visible);
		glfwWindowHint(GLFW_DECORATED, create_params.decorated);
		glfwWindowHint(GLFW_FLOATING, create_params.toplevel);
		glfwWindowHint(GLFW_FOCUSED, create_params.focused);
		glfwWindowHint(GLFW_VISIBLE, 0); // override the initial visibility

		// compute window size and position
		GLFWvidmode const* mode = glfwGetVideoMode(create_params.monitor);

		int x = 0;
		int y = 0;
		if (pseudo_fullscreen) // full-screen, the window use the full-size
		{
			create_params.width = mode->width;
			create_params.height = mode->height;

			x = monitor_x;
			y = monitor_y;
		}
		else
		{
			if (create_params.width <= 0)
				create_params.width = mode->width;
			else
				create_params.width = std::min(mode->width, create_params.width);

			if (create_params.height <= 0)
				create_params.height = mode->height;
			else
				create_params.height = std::min(mode->height, create_params.height);

			x = monitor_x + (mode->width - create_params.width) / 2;
			y = monitor_y + (mode->height - create_params.height) / 2;
		}

		// title cannot be null
		if (create_params.title == nullptr)
			create_params.title = "";

		// we are doing a pseudo fullscreen => monitor parameters of glfwCreateWindow must be null or it will "capture" the screen
		glfw_window = glfwCreateWindow(create_params.width, create_params.height, create_params.title, nullptr, share_context_window);
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
		SetGLFWCallbacks(glfw_hints.double_buffer ? true : false);

		// x and y are the coordinates of the client area : when there is a decoration, we want to tweak the window size / position with that
		int left, top, right, bottom;
		glfwGetWindowFrameSize(glfw_window, &left, &top, &right, &bottom);
		if (left != 0 || top != 0 || right != 0 || bottom != 0)
		{
			x += left;
			y += top;
			create_params.width = create_params.width - left - right;
			create_params.height = create_params.height - top - bottom;
			glfwSetWindowSize(glfw_window, create_params.width, create_params.height);
		}

		glfwSetWindowPos(glfw_window, x, y);
		glfwSetInputMode(glfw_window, GLFW_STICKY_KEYS, 1);

		// prepare cursor mode
		cursor_mode = (CursorMode)glfwGetInputMode(glfw_window, GLFW_CURSOR);
		SetCursorMode(CursorMode::Disabled);

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

		ImGui_ImplGlfw_InitForOpenGL(GetGLFWHandler(), false); // do not install callbakcs !!
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

	glm::ivec2 Window::GetWindowPosition() const
	{
		glm::ivec2 result = { 0, 0 };
		if (glfw_window != nullptr)
			glfwGetWindowPos(glfw_window, &result.x, &result.y);
		return result;
	}

	glm::ivec2 Window::GetWindowSize() const
	{
		glm::ivec2 result = { 0, 0 };
		if (glfw_window != nullptr)
			glfwGetWindowSize(glfw_window, &result.x, &result.y);
		return result;
	}

	void Window::SetWindowPosition(glm::ivec2 const& position)
	{
		if (glfw_window == nullptr)
			return;
		glfwSetWindowPos(glfw_window, position.x, position.y);

	}

	void Window::SetWindowSize(glm::ivec2 const& size)
	{
		if (glfw_window == nullptr)
			return;
		glfwSetWindowSize(glfw_window, size.x, size.y);
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

	void Window::SetGLFWCallbacks(bool in_double_buffer)
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

		double_buffer = in_double_buffer;
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

			glm::vec2 position = { float(x), float(y) };
			if (!my_window->IsMousePositionValid())
				my_window->OnMouseMove({ 0.0f, 0.0f });
			else
				my_window->OnMouseMove(position - my_window->mouse_position.value());
			my_window->mouse_position = position;
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

			my_window->OnMouseButton(button, action, modifier);
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

			my_window->OnMouseWheel(scroll_x, scroll_y);
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

			my_window->OnKeyEvent(event);
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

			my_window->OnCharEvent(c);
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

	GLFWmonitor* Window::GetFullscreenMonitor() const
	{
		if (glfw_window == nullptr)
			return nullptr;

		// fullscreen window is not decorated
		int decorated = glfwGetWindowAttrib(glfw_window, GLFW_DECORATED);
		if (decorated)
			return nullptr;

		// get the window information
		glm::ivec2 window_position = GetWindowPosition();

		glm::ivec2 window_size = GetWindowSize();

		// search for all monitors if there is a match
		int monitor_count = 0;
		GLFWmonitor** monitors = glfwGetMonitors(&monitor_count);
		if (monitor_count > 0 && monitors != nullptr)
		{
			for (int i = 0; i < monitor_count; ++i)
			{
				// get the monitor
				GLFWmonitor* monitor = monitors[i];
				if (monitor == nullptr)
					continue;
				// get the monitor position
				glm::ivec2 monitor_position = glm::vec2(0, 0);
				glfwGetMonitorPos(monitor, &monitor_position.x, &monitor_position.y);
				// compare with the window position
				if (window_position != monitor_position)
					continue;
				// get the monitor size
				GLFWvidmode const* mode = glfwGetVideoMode(monitor);
				if (mode == nullptr)
					continue;
				glm::ivec2 monitor_size = glm::ivec2(mode->width, mode->height);
				// compare with the window size
				if (window_size != monitor_size)
					continue;

				return monitor;
			}
		}
		return nullptr;
	}

	GLFWmonitor* Window::GetPreferredMonitor() const
	{
		glm::ivec2 window_center = GetWindowPosition() + GetWindowSize() / 2;

		return GLFWTools::GetNearestMonitor(window_center);
	}

	void Window::SetFullscreen(GLFWmonitor* monitor)
	{
		if (glfw_window == nullptr)
			return;

		GLFWmonitor* fullscreen_monitor = GetFullscreenMonitor(); // is the window already fullscreen for a monitor ?

		if (monitor == nullptr) // want to "unfull screen"
		{
			if (fullscreen_monitor != nullptr) // is currently fullscreen
			{
				// data properly initialized
				if (non_fullscreen_data.has_value())
				{
					glfwSetWindowAttrib(glfw_window, GLFW_DECORATED, non_fullscreen_data->decorated != 0);
					glfwSetWindowPos(glfw_window, non_fullscreen_data->position.x, non_fullscreen_data->position.y);
					glfwSetWindowSize(glfw_window, non_fullscreen_data->size.x, non_fullscreen_data->size.y);
				}
				// window probably started fullscreen
				else
				{
					if (GLFWmonitor* preferred_monitor = GetPreferredMonitor())
					{
						if (GLFWvidmode const* mode = glfwGetVideoMode(preferred_monitor))
						{
							glfwSetWindowAttrib(glfw_window, GLFW_DECORATED, 1); // default choice

							glm::ivec2 window_size = { mode->width / 2, mode->height / 2 }; // half monitor size
							SetWindowSize(window_size);

							glm::ivec2 monitor_position = { 0, 0 };
							glfwGetMonitorPos(preferred_monitor, &monitor_position.x, &monitor_position.y);
							glm::ivec2 window_position = monitor_position + window_size / 2;
							SetWindowPosition(window_position);
						}
					}
				}
			}
		}
		else
		{
			if (fullscreen_monitor != monitor) // want to change target monitor
			{
				// store current information (if not already fullscreen)
				if (fullscreen_monitor == nullptr)
				{
					NonFullScreenWindowData data;
					data.decorated = glfwGetWindowAttrib(glfw_window, GLFW_DECORATED) != 0;
					glfwGetWindowPos(glfw_window, &data.position.x, &data.position.y);
					glfwGetWindowSize(glfw_window, &data.size.x, &data.size.y);
					non_fullscreen_data = data;
				}
				// remove decoration
				glfwSetWindowAttrib(glfw_window, GLFW_DECORATED, 0);
				// fit window to monitor size and position
				if (GLFWvidmode const* mode = glfwGetVideoMode(monitor))
				{
					// change window size and position
					glm::ivec2 monitor_position = { 0, 0 };
					glfwGetMonitorPos(monitor, &monitor_position.x, &monitor_position.y);
					SetWindowPosition(monitor_position);
					SetWindowSize({ mode->width, mode->height });
				}
			}
		}
	}

	void Window::ToggleFullscreen()
	{
		if (GLFWmonitor* fullscreen_monitor = GetFullscreenMonitor())
			SetFullscreen(nullptr);
		else if (GLFWmonitor* prefered_monitor = GetPreferredMonitor())
			SetFullscreen(prefered_monitor);
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
			glm::ivec2 window_size = GetWindowSize();

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
		// give opportunity to application
		if (Application* application = Application::GetInstance())
			if (application->OnKeyEvent(event))
				return true;
		// super method
		return WindowInterface::OnKeyEventImpl(event);
	}

	bool Window::OnMouseMoveImpl(glm::vec2 const& delta)
	{
		// give opportunity to application
		if (Application* application = Application::GetInstance())
			if (application->OnMouseMove(delta))
				return true;
		return WindowInterface::OnMouseMoveImpl(delta);
	}

	bool Window::OnMouseButtonImpl(int button, int action, int modifier)
	{
		// give opportunity to application
		if (Application* application = Application::GetInstance())
			if (application->OnMouseButton(button, action, modifier))
				return true;
		return WindowInterface::OnMouseButtonImpl(button, action, modifier);
	}

	bool Window::OnMouseWheelImpl(double scroll_x, double scroll_y)
	{
		// give opportunity to application
		if (Application* application = Application::GetInstance())
			if (application->OnMouseWheel(scroll_x, scroll_y))
				return true;
		return WindowInterface::OnMouseWheelImpl(scroll_x, scroll_y);
	}

	bool Window::OnCharEventImpl(unsigned int c)
	{
		// give opportunity to application
		if (Application* application = Application::GetInstance())
			if (application->OnCharEvent(c))
				return true;
		return WindowInterface::OnCharEventImpl(c);
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

		glm::ivec2 window_size = GetWindowSize();
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
			placement.size = GetWindowSize();
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

	bool Window::InitializeFromConfiguration(nlohmann::json const& config)
	{
		ReadConfigurableProperties();
		RegisterKnownDrawables();
		return true; 
	}

	void Window::RegisterKnownDrawables()
	{
		RegisterDrawable("Window Information", [this]()
		{
			ImGuiWindowInformationDrawable* result = new ImGuiWindowInformationDrawable;
			if (result != nullptr)
				result->SetWindow(this);
			return result;
		});
	}

	bool Window::OnReadConfigurableProperties(JSONReadConfiguration config, ReadConfigurablePropertiesContext context)
	{
		glm::ivec2 position = { 0, 0 };		
		if (JSONTools::GetAttribute(config, "position", position))
			SetWindowPosition(position);

		glm::ivec2 size = { 0, 0 };
		if (JSONTools::GetAttribute(config, "size", size))
			SetWindowSize(size);

		return true;
	}

	bool Window::OnStorePersistentProperties(JSONWriteConfiguration config)
	{
		JSONTools::SetAttribute(config, "position", GetWindowPosition());
		JSONTools::SetAttribute(config, "size", GetWindowSize());
		return true;
	}





#if 0

	// SHU_PERSISTENT


	void Window::OnReadPersistentData(nlohmann::json const& json)
	{
		glm::ivec2 position = { 0, 0 };
		glm::ivec2 size = { 0, 0 };
		JSONTools::GetAttribute(json, "position", position);
		JSONTools::GetAttribute(json, "size", size);
		SetWindowPosition(position);
		SetWindowSize(size);
	}

	void Window::OnWritePersistentData(nlohmann::json & json) const
	{
		JSONTools::SetAttribute(json, "position", GetWindowPosition());
		JSONTools::SetAttribute(json, "size", GetWindowSize());
	}
#endif













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
	}

	void Window::Finalize()
	{
	}

}; // namespace chaos
