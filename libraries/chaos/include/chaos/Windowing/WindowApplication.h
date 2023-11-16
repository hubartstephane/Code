namespace chaos
{
#ifdef CHAOS_FORWARD_DECLARATION

	class WindowApplication;

#elif !defined CHAOS_TEMPLATE_IMPLEMENTATION

	/**
	* WindowApplication
	*/

	class CHAOS_API WindowApplication : public Application, public TickableInterface, public GPUProgramProviderInterface
	{
		friend class Window;

	public:

		/** constructor */
		WindowApplication(SubClassOf<Window> in_main_window_class, WindowCreateParams const& in_window_create_params = {});

		/** gets the number of windows */
		size_t GetWindowCount() const;
		/** get the window per index */
		AutoCastable<Window> GetWindow(size_t index);
		/** get the window per index */
		AutoConstCastable<Window> GetWindow(size_t index) const;
		/** get the window per name */
		AutoCastable<Window> FindWindow(ObjectRequest request);
		/** get the window per name */
		AutoConstCastable<Window> FindWindow(ObjectRequest request) const;

		/** require application end */
		virtual void Quit();

		/** destroy a window */
		void DestroyWindow(Window* window);

		/** getter of the main clock */
		static Clock* GetMainClockInstance();
		/** getter of the main clock */
		static Clock const* GetMainClockConstInstance();

		/** getter of the sound manager */
		static SoundManager* GetSoundManagerInstance();
		/** getter of the sound manager */
		static SoundManager const* GetSoundManagerConstInstance();

		/** getter of the GPU resource manager */
		static GPUResourceManager* GetGPUResourceManagerInstance();
		/** getter of the GPU resource manager */
		static GPUResourceManager const* GetGPUResourceManagerConstInstance();

		/** gets the main clock */
		Clock* GetMainClock() { return main_clock.get(); }
		/** gets the main clock */
		Clock const* GetMainClock() const { return main_clock.get(); }

		/** gets the sound manager */
		SoundManager* GetSoundManager() { return sound_manager.get(); }
		/** gets the sound manager */
		SoundManager const* GetSoundManager() const { return sound_manager.get(); }

		/** gets the graphic resource manager */
		GPUResourceManager* GetGPUResourceManager() { return gpu_resource_manager.get(); }
		/** gets the graphic resource manager */
		GPUResourceManager const* GetGPUResourceManager() const { return gpu_resource_manager.get(); }

		/** getter on the texture atlas */
		BitmapAtlas::TextureArrayAtlas* GetTextureAtlas() { return texture_atlas.get(); }
		/** getter on the texture atlas */
		BitmapAtlas::TextureArrayAtlas const* GetTextureAtlas() const { return texture_atlas.get(); }

		/** getter on the text generator */
		ParticleTextGenerator::Generator* GetTextGenerator() { return particle_text_generator.get(); }
		/** getter on the text generator */
		ParticleTextGenerator::Generator const* GetTextGenerator() const { return particle_text_generator.get(); }

		/** get the button map */
		std::map<GamepadButton, std::pair<std::string, std::string>>& GetGamepadButtonMap() { return gamepad_button_map; }
		/** get the button map */
		std::map<GamepadButton, std::pair<std::string, std::string>> const& GetGamepadButtonMap() const { return gamepad_button_map; }

		/** used to force for one frame the duration of tick function to 0 : usefull for function that are long and would block the game for some time */
		void FreezeNextFrameTickDuration();

		/** reload all GPU resources */
		virtual bool ReloadGPUResources();
		/** override */
		virtual bool OnKeyEventImpl(KeyEvent const& event) override;

		/** get the OpenGL context, call the function, restore previous context after */
		template<typename FUNC>
		static auto WithGLFWContext(GLFWwindow* glfw_context, FUNC const& func)
		{
			GLFWwindow* previous_glfw_context = glfwGetCurrentContext();
			glfwMakeContextCurrent(glfw_context);

			if constexpr (std::is_same_v<void, decltype(func())>)
			{
				func();
				glfwMakeContextCurrent(previous_glfw_context);
			}
			else
			{
				auto result = func();
				glfwMakeContextCurrent(previous_glfw_context);
				return result;
			}
		}

		/** get the OpenGL main context */
		static GLFWwindow* GetSharedGLContext();

		/** Run the message loop while the condition is true */
		virtual void RunMessageLoop(std::function<bool()> const& loop_condition_func = {});

		/** create a window */
		Window* CreateTypedWindow(SubClassOf<Window> window_class, WindowCreateParams create_params = {}, ObjectRequest = {});

		/** enable per window special mode */
		void SetImGuiMenuMode(bool mode);
		/** gets the window special mode */
		bool GetImGuiMenuMode() const;

		/** search whether a given window exists */
		bool IsKnownWindowVisible(char const* name) const;
		/** create a window from its name */
		bool SetKnownWindowVisibility(char const* name, bool visible);

		/** destroy all windows */
		void DestroyAllWindows();

	protected:

		/** Main method */
		virtual int Main() override;
		/** create the main window */
		virtual Window* CreateMainWindow();
		/** the possibility to have final initialization before the main loop is run */
		virtual bool PostOpenGLContextCreation();

		/** called whenever a monitor is connected or disconnected */
		virtual void OnMonitorEvent(GLFWmonitor* monitor, int monitor_state);
		/** called whenever a monitor is connected or disconnected */
		static void DoOnMonitorEvent(GLFWmonitor* monitor, int monitor_state);

		/** enumerate windows that the application can open */
		virtual bool EnumerateKnownWindows(std::function<bool(char const * name, SubClassOf<Window> window_class)> const& func) const;

		/** override */
		virtual bool DoProcessAction(GPUProgramProviderExecutionData const& execution_data) const override;

		/** an error callback */
		static void OnGLFWError(int code, const char* msg);
		/** a debugging function to output some message from FreeImage */
		static void FreeImageOutputMessageFunc(FREE_IMAGE_FORMAT fif, const char* msg);
		/** initializing standard libraries */
		virtual bool InitializeStandardLibraries() override;
		/** Finalizalizing standard libraries */
		virtual void FinalizeStandardLibraries() override;
		/** initialize the application */
		virtual bool Initialize() override;
		/** finalize the application */
		virtual void Finalize() override;
		/** initialize the managers */
		virtual bool InitializeManagers() override;
		/** finalize the managers */
		virtual void FinalizeManagers() override;
		/** the GPU manager must be initialized after the OpenGL context is OK. */
		virtual bool CreateGPUResourceManager();
		/** finalize the GPU manager */
		virtual void FinalizeGPUResourceManager();

		/** initialize a mapping with button names / text generator joker */
		virtual bool InitializeGamepadButtonMap();
		/** create the texture atlas */
		virtual bool CreateTextureAtlas();
		/** create the text generator */
		virtual bool CreateTextGenerator();

		/** main method to generate atlas entries */
		virtual bool FillAtlasGeneratorInput(BitmapAtlas::AtlasInput& input);
		/** generate atlas entries relative to sprite directory */
		virtual bool FillAtlasGeneratorInputSprites(BitmapAtlas::AtlasInput& input);
		/** generate atlas entries relative to fonts */
		virtual bool FillAtlasGeneratorInputFonts(BitmapAtlas::AtlasInput& input);

		/** override */
		virtual bool DoTick(float delta_time) override;

		/** override */
		virtual bool OnReadConfigurableProperties(JSONReadConfiguration config, ReadConfigurablePropertiesContext context) override;

		/** the user callback called when current input mode changes */
		virtual void OnInputModeChanged(InputMode new_mode, InputMode old_mode) override;

		/** called at window creation */
		virtual void OnWindowCreated(Window* window);
		/** called at window destruction */
		virtual void OnWindowDestroyed(Window* window);

		/** gets the main window of the application */
		AutoCastable<Window> GetMainWindow() { return FindWindow("main"); }
		/** gets the main window of the application */
		AutoConstCastable<Window> GetMainWindow() const { return FindWindow("main"); }

		/** add some items to a windows menu */
		virtual void OnDrawWindowImGuiMenu(Window * window);

		/** internal method to show or hide a window */
		void SetWindowInternalVisibility(bool visible, char const* name, SubClassOf<Window> window_class);

		/** checks whether the window is inside the application windows array */
		bool IsWindowHandledByApplication(Window const* window) const;

		/** some window may be removed or created during a loop on 'windows' array itself. this intermediate array helps to detect such issues */
		std::vector<weak_ptr<Window>> GetWeakWindowArray() const;

#ifdef _WIN32
		/** called whenever the application language is being changed */
		virtual void OnInputLanguageChanged();
#endif

		/** enumerate all windows */
		template<typename FUNC>
		auto ForAllWindows(FUNC func) -> boost::mpl::if_c<std::is_convertible_v<decltype(func((Window*)(nullptr))), bool>, decltype(func((Window*)(nullptr))), void>::type
		{
			using result_type = decltype(func((Window *)(nullptr)));
			constexpr bool convertible_to_bool = std::is_convertible_v<result_type, bool>;

			// enumerate all windows
			for (weak_ptr<Window> & window : GetWeakWindowArray()) // use a 'weak' copy of all existing windows because some windows may be erased during the loop
			{
				if (window != nullptr) // check whether the window has be erased
				{
					if constexpr (convertible_to_bool)
					{
						if (auto result = func(window.get()))
							return result;
					}
					else
					{
						func(window.get());
					}
				}
			}

			// default result
			if constexpr (convertible_to_bool)
				return result_type();
		}

	protected:

		/** the main clock of the manager */
		shared_ptr<Clock> main_clock;
		/** the sound manager */
		shared_ptr<SoundManager> sound_manager;
		/** the graphic resource manager */
		shared_ptr<GPUResourceManager> gpu_resource_manager;

		/** the texture atlas */
		shared_ptr<BitmapAtlas::TextureArrayAtlas> texture_atlas;
		/** the text generator */
		shared_ptr<ParticleTextGenerator::Generator> particle_text_generator;

		/** a mapping between the button index and its resource name + text generator alias */
		std::map<GamepadButton, std::pair<std::string, std::string>> gamepad_button_map;

		/** the initial_window param */
		WindowCreateParams window_create_params;
		/** the class of the main window */
		SubClassOf<Window> main_window_class;
		/** the window list */
		std::vector<shared_ptr<Window>> windows;

		/** forced time slice for tick */
		float forced_tick_duration = 0.0f;
		/** maximum time slice for tick */
		float max_tick_duration = 0.0f;
		/** whether the delta time is forced to 0 for one frame (usefull for long operations like screen capture or GPU resource reloading) */
		bool forced_zero_tick_duration = false;

		/** the imgui menu mode */
		bool imgui_menu_mode = false;

		/** the hints for all windows */
		GLFWHints glfw_hints;

		/** an invisible window that is used as a OpenGL context for all others */
		GLFWwindow* shared_context = nullptr;
	};

	/**
	* RunWindowApplication : utility template function to run an application only from a class
	*/

	template<typename WINDOW_TYPE, typename ...PARAMS>
	/*CHAOS_API*/ bool RunWindowApplication(int argc, char** argv, char** env, PARAMS && ...params)
	{
		return RunApplication<WindowApplication>(argc, argv, env, WINDOW_TYPE::GetStaticClass(), std::forward<PARAMS>(params)...);
	}

#endif

}; // namespace chaos