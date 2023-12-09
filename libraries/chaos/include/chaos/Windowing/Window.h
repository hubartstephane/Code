namespace chaos
{

#ifdef CHAOS_FORWARD_DECLARATION

	class WindowCreateParams;
	class WindowPlacementInfo;
	class WindowFrameSizeInfo;
	class NonFullScreenWindowData;
	class Window;

	enum class CursorMode;

#elif !defined CHAOS_TEMPLATE_IMPLEMENTATION

	/**
	* CursorMode
	*/

	enum class CHAOS_API CursorMode : int
	{
		Normal = GLFW_CURSOR_NORMAL,
		Disabled = GLFW_CURSOR_DISABLED,
		Hidden = GLFW_CURSOR_HIDDEN
	};

	/**
	* WindowPlacementInfo: describes how the window should be placed
	*/

	class CHAOS_API WindowPlacementInfo
	{
	public:

		/** the wanted monitor */
		GLFWmonitor* monitor = nullptr;
		/** the monitor index (used only if monitor is not explicitly defined above) */
		std::optional<int> monitor_index;

		/** the wanted position (if a monitor is explicitly given, the position is relative to this monitor, elsewhere it is absolute) */
		std::optional<glm::ivec2> position;
		/** the wanted size (fit the whole monitor if not defined or with negative values or fullscreen is set to true) */
		std::optional<glm::ivec2> size;
		/** a fullscreen window fits the whole monitor and displays no decorator */
		bool fullscreen = false;
	};

	CHAOS_API bool DoSaveIntoJSON(nlohmann::json* json, WindowPlacementInfo const& src);

	CHAOS_API bool DoLoadFromJSON(JSONReadConfiguration config, WindowPlacementInfo& dst);

	/**
	* WindowFrameSizeInfo: describes the decoration surface surrounding a window
	*/

	class CHAOS_API WindowFrameSizeInfo
	{
	public:

		int left = 0;
		int top = 0;
		int right = 0;
		int bottom = 0;
	};

	CHAOS_API WindowFrameSizeInfo GetWindowFrameSizeInfo(GLFWwindow* window);

	/**
	* WindowCreateParams : parameters for playing single window application
	*/

	class CHAOS_API WindowCreateParams
	{
	public:

		/** the title */
		std::string title;
		/** true if the window can be resized */
		int resizable = 1;
		/** true if the window starts visible */
		int start_visible = 1;
		/** true if the window has some decoration */
		int decorated = 1;
		/** true if the window is toplevel */
		int toplevel = 0;
		/** self description */
		int focused = 0;
	};

	CHAOS_API bool DoSaveIntoJSON(nlohmann::json * json, WindowCreateParams const& src);

	CHAOS_API bool DoLoadFromJSON(JSONReadConfiguration config, WindowCreateParams& dst);

	/**
	* NonFullScreenWindowData : a binding class between chaos and GLFW to handle window (beware the prefix "My")
	*/

	class NonFullScreenWindowData
	{
	public:

		/** the position of window */
		glm::ivec2 position = { 0, 0 };
		/** the size of window */
		glm::ivec2 size = { 0, 0 };
	};

	/**
	* Window : a binding class between chaos and GLFW to handle window (beware the prefix "My")
	*/

	class CHAOS_API Window : public Object, public WindowInterface, public ImGuiDrawableOwnerInterface, public ConfigurableInterface, public KeyEventActionEnumerableInterface
	{
		friend class WindowApplication;

		CHAOS_DECLARE_OBJECT_CLASS(Window, Object);

	public:

		/** constructor */
		Window();
		/** destructor */
		virtual ~Window();

		/** require the destruction of the window (at least its GLFW resource). this will remove it from the WindowApplication */
		void Destroy();

		/** called to require the window to close */
		void RequireWindowClosure();
		/** getter on the handler */
		GLFWwindow* GetGLFWHandler();
		/** returns whether the window has a pending GLFW close message */
		bool ShouldClose();

		/** toggle to fullscreen mode */
		void ToggleFullscreen();
		/** returns whether the screen is fullscreen */
		bool IsFullscreen() const { return fullscreen_monitor != nullptr; }

		/** returns the monitor where the window is centered */
		GLFWmonitor* GetPreferredMonitor() const;
		/** set window to fullscreen or remove */
		void SetFullscreen(GLFWmonitor* monitor);

		/** returns the position of the window */
		glm::ivec2 GetWindowPosition(bool include_decorators) const;
		/** returns the size of the window */
		glm::ivec2 GetWindowSize(bool include_decorators) const;

		/** change the window position */
		void SetWindowPosition(glm::ivec2 position, bool include_decorators);
		/** change the window size */
		void SetWindowSize(glm::ivec2 size, bool include_decorators);
		/** change the placement of the window */
		void SetWindowPlacement(WindowPlacementInfo placement_info);

		/** show or hide the window */
		void ShowWindow(bool visible);
		/** get visibility status */
		bool IsWindowVisible() const;

		/** require a screen capture */
		bool ScreenCapture();

		/** getting the required viewport for given window */
		virtual aabox2 GetRequiredViewport(glm::ivec2 const& size) const;

		/** getting the renderer */
		GPURenderer* GetRenderer() { return renderer.get(); }

		/** get the root widget */
		WindowRootWidget* GetRootWidget() { return root_widget.get(); }
		/** get the root widget */
		WindowRootWidget const* GetRootWidget() const { return root_widget.get(); }

		/** update the whole widget hierarchy placement */
		virtual void UpdateWidgetPlacementHierarchy();

		/** using window context, call functor, then restore previous */
		template<typename FUNC>
		auto WithWindowContext(FUNC const& func)
		{
			// prevent the window destruction
			shared_ptr<Window> prevent_destruction = this;
			IncrementWindowDestructionGuard();
			// save GLFW context
			GLFWwindow* previous_glfw_window = glfwGetCurrentContext();
			glfwMakeContextCurrent(glfw_window);
			// save ImGui context
			ImGuiContext* previous_imgui_context = ImGui::GetCurrentContext();
			ImGuiContext* imgui_context_to_set = imgui_context;
			ImGui::SetCurrentContext(imgui_context_to_set);

			if constexpr (std::is_same_v<void, decltype(func())>)
			{
				// call delegate
				func();
				// restore GLFW and ImGui contexts
				glfwMakeContextCurrent(previous_glfw_window);
				// restore ImGui context (if different, because maybe the context has been destroy inside the func() call
				if (previous_imgui_context != imgui_context_to_set)
					ImGui::SetCurrentContext(previous_imgui_context);
				// enable window destruction back
				DecrementWindowDestructionGuard();
			}
			else
			{
				// call delegate
				auto result = func();
				// restore GLFW and ImGui contexts
				glfwMakeContextCurrent(previous_glfw_window);
				// restore ImGui context (if different, because maybe the context has been destroy inside the func() call
				if (previous_imgui_context != imgui_context_to_set)
					ImGui::SetCurrentContext(previous_imgui_context);
				// enable window destruction back
				DecrementWindowDestructionGuard();
				return result;
			}
		}

		/** gets the ImGui context */
		ImGuiContext* GetImGuiContext() const { return imgui_context; }

		/** change the cursor mode */
		void SetCursorMode(CursorMode mode);
		/** get the cursor mode */
		CursorMode GetCursorMode() const;

		/** draw ImGui */
		virtual void DrawWindowImGui();

	protected:

		/** override */
		virtual bool DoProcessAction(GPUProgramProviderExecutionData const& execution_data) const override;
		/** override */
		virtual bool OnKeyEventImpl(KeyEvent const& event) override;
		/** override */
		virtual bool OnDraw(GPURenderer* renderer, GPUProgramProviderInterface const* uniform_provider, WindowDrawParams const& draw_params) override;
		/** override */
		virtual bool DoTick(float delta_time) override;

		/** override */
		virtual bool EnumerateKeyEventActions(KeyEventActionProcessor & processor) override;

		/** create the root widget */
		virtual bool CreateRootWidget();

		/** display both the window content and the widget overlay */
		virtual bool DrawInternal(GPUProgramProviderInterface const* uniform_provider);

		/** bind Window with GLFW */
		virtual void SetGLFWCallbacks();

		/** create an ImGui context */
		void CreateImGuiContext();
		/** destroying the ImGui context */
		void DestroyImGuiContext();

		/** create the internal window */
		bool CreateGLFWWindow(WindowPlacementInfo const & placement_info, WindowCreateParams const &create_params, GLFWwindow* share_context, GLFWHints glfw_hints);
		/** destroying the window */
		void DestroyGLFWWindow();

		/** called whenever the mode is being changed */
		void OnImGuiMenuModeChanged(bool mode);
		/** gets the window special mode */
		bool GetImGuiMenuMode() const;

		/** called whenever the window is redrawn (entry point) */
		virtual void DrawWindow();

		/** called at window creation */
		virtual bool Initialize();
		/** called at window destruction */
		virtual void Finalize();
		/** called at window creation (returns false if the window must be killed) */
		virtual bool InitializeFromConfiguration(nlohmann::json const* config);

		/** get the mouse position */
		glm::vec2 GetMousePosition() const;
		/** returns true if the mouse position is valid (very first frame) */
		bool IsMousePositionValid() const;

		/** tick the renderer of the window with the real framerate (with no time scale) */
		void TickRenderer(float real_delta_time);

		/** called whenever the window is resized */
		virtual void OnWindowResize(glm::ivec2 size);
		/** called whenever the user try to close window */
		virtual bool OnWindowClosed();
		/** called whenever a file is dropped */
		virtual void OnDropFile(int count, char const** paths);
		/** called whenever the window becomes iconified or is restored */
		virtual void OnIconifiedStateChange(bool iconified);
		/** called whenever the window gain or loose focus */
		virtual void OnFocusStateChange(bool gain_focus);
		/** called whenever a monitor is connected or disconnected */
		virtual void OnMonitorEvent(GLFWmonitor* monitor, int monitor_state);

		/** draw the main menu */
		virtual void OnDrawWindowImGuiMenu();
		/** draw the content */
		virtual void OnDrawWindowImGuiContent();

#if _WIN32

		/** set or restore WndProc for ImGui usage */
		void SetImGuiWindowProc(bool set_proc);
		/** a dedicated WndProc for ImGui */
		static LRESULT CALLBACK ImGuiWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
		/** special handlers */
		virtual bool HookedWindowProc(UINT msg, WPARAM wParam, LPARAM lParam);
		/** called whenever the language is being changed */
		virtual void OnInputLanguageChanged();

#endif // #if _WIN32

		/** increment the counter that prevent window destruction */
		void IncrementWindowDestructionGuard();
		/** decrement the counter that prevent window destruction */
		void DecrementWindowDestructionGuard();
		/** get the destruction guard count */
		int GetWindowDestructionGuard() const { return window_destruction_guard; }

		/** register the known drawables */
		virtual void RegisterKnownDrawables();

		/** override */
		virtual bool OnConfigurationChanged(JSONReadConfiguration config) override;
		/** override */
		virtual bool OnReadConfigurableProperties(JSONReadConfiguration config, ReadConfigurablePropertiesContext context) override;
		/** override */
		virtual bool OnStorePersistentProperties(JSONWriteConfiguration config) const override;

	private:

		/** binding function with GLFW library */
		static void DoOnWindowClosed(GLFWwindow* in_glfw_window);
		/** binding function with GLFW library */
		static void DoOnWindowResize(GLFWwindow* in_glfw_window, int width, int height);
		/** binding function with GLFW library */
		static void DoOnMouseMove(GLFWwindow* in_glfw_window, double x, double y);
		/** binding function with GLFW library */
		static void DoOnMouseButton(GLFWwindow* in_glfw_window, int button, int action, int modifier);
		/** binding function with GLFW library */
		static void DoOnMouseWheel(GLFWwindow* in_glfw_window, double scroll_x, double scroll_y);
		/** binding function with GLFW library */
		static void DoOnKeyEvent(GLFWwindow* in_glfw_window, int keycode, int scancode, int action, int modifier);
		/** binding function with GLFW library */
		static void DoOnCharEvent(GLFWwindow* in_glfw_window, unsigned int c);
		/** binding function with GLFW library */
		static void DoOnDropFile(GLFWwindow* in_glfw_window, int count, char const** paths);
		/** binding function with GLFW library */
		static void DoOnIconifiedStateChange(GLFWwindow* in_glfw_window, int iconified);
		/** binding function with GLFW library */
		static void DoOnFocusStateChange(GLFWwindow* in_glfw_window, int gain_focus);

	protected:

		/** the context for ImGui */
		ImGuiContext* imgui_context = nullptr;
		/** the window in GLFW library */
		GLFWwindow* glfw_window = nullptr;
		/** is the window with double buffer */
		bool double_buffer = true;
		/** whether the window should be toplevel when non-fullscreen */
		bool initial_toplevel = false;
		/** whether the window should be decorated when non-fullscreen */
		bool initial_decorated = false;
		/** the root widget for the window */
		shared_ptr<WindowRootWidget> root_widget;
		/** the renderer */
		shared_ptr<GPURenderer> renderer;
		/** previous mouse position */
		std::optional<glm::vec2> mouse_position;
		/** used to store data when toggling fullscreen */
		std::optional<NonFullScreenWindowData> non_fullscreen_data;
		/** if the window is fullscreen, this points to the concerned monitor */
		GLFWmonitor* fullscreen_monitor = nullptr;
		/** the current cursor mode */
		CursorMode cursor_mode = CursorMode::Normal;
		/** a counter that prevent destruction of the window resources */
		int window_destruction_guard = 0;
	};

#endif

}; // namespace chaos