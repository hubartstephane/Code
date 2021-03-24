#include <chaos/Chaos.h>

namespace chaos
{
	GameApplication::GameApplication(SubClassOf<Game> in_game_class, SubClassOf<Window> in_main_window_class, WindowParams const& in_window_params, WindowHints const & in_window_hints) :
		game_class(in_game_class),
		WindowApplication(in_main_window_class, in_window_params, in_window_hints)
	{
		assert(game_class.IsValid());
	}

	void GameApplication::OnWindowCreated(Window* window)
	{
		// super
		WindowApplication::OnWindowCreated(window);
		// set the game
		GameWindow* game_window = auto_cast(window);
		if (game_window != nullptr)
			game_window->SetGame(game.get());
	}

	bool GameApplication::PreMessageLoop()
	{
		assert(glfwGetCurrentContext() == shared_context);

		// create the game
		game = game_class.CreateInstance();
		if (game == nullptr)
			return false;
		// initialize the game from configuration
		nlohmann::json const* game_config = JSONTools::GetStructure(configuration, "game");
		if (game_config != nullptr)
			if (!game->InitializeFromConfiguration(*game_config, configuration_path))
				return false;
		// update all windows and give them the game
		for (auto& window : windows)
		{
			GameWindow* game_window = auto_cast(window.get());
			if (game_window != nullptr)
				game_window->SetGame(game.get());
		}
		// super method : need to be done AFTER !
		if (!WindowApplication::PreMessageLoop())
			return false;
		return true;
	}

	void GameApplication::Tick(float delta_time)
	{
		assert(glfwGetCurrentContext() == shared_context);
		// super
		WindowApplication::Tick(delta_time);
		// update the game
		if (game != nullptr)
			game->Tick(delta_time);
	}

}; // namespace chaos

