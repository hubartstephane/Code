namespace chaos
{
#ifdef CHAOS_FORWARD_DECLARATION

	class GameApplication;

#elif !defined CHAOS_TEMPLATE_IMPLEMENTATION

	class CHAOS_API GameApplication : public WindowApplication
	{
	public:

		/** constructor */
		GameApplication(SubClassOf<Game> in_game_class, SubClassOf<GameWindow> in_main_window_class, SubClassOf<GameViewportWidget> in_game_viewport_widget_class, WindowCreateParams const& in_window_create_params = {});

		/** gets the game */
		Game* GetGame() { return game.get(); }
		/** gets the game */
		Game const* GetGame() const { return game.get(); }

		/** gets the viewport class */
		SubClassOf<GameViewportWidget> GetGameViewportWidgetClass() const { return game_viewport_widget_class; }

		/** whether the game should "ignored" */
		virtual bool IsGameSuspended() const;

	protected:

		/** override */
		virtual void OnWindowDestroyed(Window* window) override;
		/** override */
		virtual bool PostOpenGLContextCreation() override;
		/** override */
		virtual bool DoTick(float delta_time) override;
		/** override */
		virtual bool FillAtlasGeneratorInput(BitmapAtlas::AtlasInput& input) override;

		/** override */
		virtual bool OnMouseMoveImpl(glm::vec2 const& delta) override;
		/** override */
		virtual bool OnMouseButtonImpl(int button, int action, int modifier) override;
		/** override */
		virtual bool OnMouseWheelImpl(double scroll_x, double scroll_y) override;
		/** override */
		virtual bool OnKeyEventImpl(KeyEvent const& event) override;
		/** override */
		virtual bool OnCharEventImpl(unsigned int c) override;

	protected:

		/** the class for the game */
		SubClassOf<Game> game_class;
		/** the class for the viewport */
		SubClassOf<GameViewportWidget> game_viewport_widget_class;
		/** pointer on the game */
		shared_ptr<Game> game;
	};

	template<typename GAME_TYPE, typename GAME_APPLICATION_TYPE = GameApplication, typename MAIN_WINDOW_CLASS = GameWindow, typename GAME_VIEWPORT_WIDGET_CLASS = GameViewportWidget, typename ...PARAMS>
	/*CHAOS_API*/ int RunGame(int argc, char** argv, char** env, PARAMS && ...params)
	{
		return RunApplication<GAME_APPLICATION_TYPE>(
			argc, argv, env,
			GAME_TYPE::GetStaticClass(),
			MAIN_WINDOW_CLASS::GetStaticClass(),
			GAME_VIEWPORT_WIDGET_CLASS::GetStaticClass(),
			std::forward<PARAMS>(params)...);
	}

#endif

}; // namespace chaos