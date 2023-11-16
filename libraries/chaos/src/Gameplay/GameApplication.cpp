#include "chaos/ChaosPCH.h"
#include "chaos/ChaosInternals.h"

namespace chaos
{
	GameApplication::GameApplication(SubClassOf<Game> in_game_class, SubClassOf<GameWindow> in_main_window_class, SubClassOf<GameViewportWidget> in_game_viewport_widget_class, WindowCreateParams const& in_window_create_params):
		WindowApplication(in_main_window_class, in_window_create_params),
		game_class(in_game_class),
		game_viewport_widget_class(in_game_viewport_widget_class)
	{
		assert(in_game_class.IsValid());
		assert(game_viewport_widget_class.IsValid());
	}

	bool GameApplication::PostOpenGLContextCreation()
	{
		assert(glfwGetCurrentContext() == shared_context);
		 
		// create the game
		game = game_class.CreateInstance();
		if (game == nullptr)
			return false;
		GiveChildConfiguration(game.get(), "game");
		if (!game->InitializeFromConfiguration())
			return false;
		// super method : need to be done game initialization ! (because atlas creation requires the game to have loaded its levels)
		if (!WindowApplication::PostOpenGLContextCreation())
			return false;
		// create now some game resources that need application resources to be initialized
		if (!game->CreateGPUResources())
			return false;
		return true;
	}

	bool GameApplication::DoTick(float delta_time)
	{
		assert(glfwGetCurrentContext() == shared_context);
		// super
		WindowApplication::DoTick(delta_time);
		// update the game
		if (game != nullptr)
			if (!IsGameSuspended())
				game->Tick(delta_time);
		return true;
	}

	bool GameApplication::FillAtlasGeneratorInput(BitmapAtlas::AtlasInput& input)
	{
		if (!WindowApplication::FillAtlasGeneratorInput(input))
			return false;
		if (game != nullptr)
			if (!game->FillAtlasGeneratorInput(input))
				return false;
		return true;
	}

	bool GameApplication::OnMouseMoveImpl(glm::vec2 const& delta)
	{
		if (game != nullptr)
			if (!IsGameSuspended())
				if (game->OnMouseMove(delta))
					return true;
		return WindowApplication::OnMouseMoveImpl(delta);
	}

	bool GameApplication::OnMouseButtonImpl(int button, int action, int modifier)
	{
		if (game != nullptr)
			if (!IsGameSuspended())
				if (game->OnMouseButton(button, action, modifier))
					return true;
		return WindowApplication::OnMouseButtonImpl(button, action, modifier);
	}

	bool GameApplication::OnMouseWheelImpl(double scroll_x, double scroll_y)
	{
		if (game != nullptr)
			if (!IsGameSuspended())
				if (game->OnMouseWheel(scroll_x, scroll_y))
					return true;
		return WindowApplication::OnMouseWheelImpl(scroll_x, scroll_y);
	}

	bool GameApplication::OnKeyEventImpl(KeyEvent const& event)
	{
		if (game != nullptr)
			if (!IsGameSuspended())
				if (game->OnKeyEvent(event))
					return true;
		return WindowApplication::OnKeyEventImpl(event);
	}

	bool GameApplication::OnCharEventImpl(unsigned int c)
	{
		if (game != nullptr)
			if (!IsGameSuspended())
				if (game->OnCharEvent(c))
					return true;
		return WindowApplication::OnCharEventImpl(c);
	}

	bool GameApplication::IsGameSuspended() const
	{
		return imgui_menu_mode;
	}

	void GameApplication::OnWindowDestroyed(Window* window)
	{
		WindowApplication::OnWindowDestroyed(window);

		// destroy all other windows as soon as there are no more game_window
		bool has_game_window = false;
		for (shared_ptr<Window> const & window : windows)
		{
			if (GameWindow const* game_window = auto_cast(window.get()))
			{
				has_game_window = true;
				break;
			}
		}
		if (!has_game_window)
			DestroyAllWindows();
	}

}; // namespace chaos

