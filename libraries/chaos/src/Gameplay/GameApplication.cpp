#include <chaos/Chaos.h>

namespace chaos
{
	GameApplication::GameApplication(SubClassOf<Game> in_game_class, SubClassOf<MyGLFW::Window> in_main_window_class, MyGLFW::WindowParams const& in_window_params, MyGLFW::WindowHints const in_window_hints) :
		game_class(in_game_class),
		WindowApplication(in_main_window_class, in_window_params, in_window_hints)
	{
		assert(game_class.IsValid());
	}

	bool GameApplication::PreMainLoop()
	{
		if (!MyGLFW::WindowApplication::PreMainLoop())
			return false;
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
		return true;
	}

	void GameApplication::Tick(float delta_time)
	{
		// super
		MyGLFW::WindowApplication::Tick(delta_time);
		// update the game
		if (game != nullptr)
			game->Tick(delta_time);
	}

}; // namespace chaos

