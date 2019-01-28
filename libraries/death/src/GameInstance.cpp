#include <death/GameInstance.h>

namespace death
{
	GameInstance::GameInstance(Game * in_game) :
		game(in_game)
	{
		assert(in_game != nullptr);
	}

	Player * GameInstance::GetPlayer(int player_index)
	{
		size_t count = players.size();
		for (size_t i = 0; i < count; ++i)
			if (players[i]->GetPlayerIndex() == player_index)
				return players[i].get();
		return nullptr;
	}

	Player const * GameInstance::GetPlayer(int player_index) const
	{
		size_t count = players.size();
		for (size_t i = 0; i < count; ++i)
			if (players[i]->GetPlayerIndex() == player_index)
				return players[i].get();
		return nullptr;
	}

	int GameInstance::GetBestPlayerScore() const
	{
		int result = 0;

		size_t count = players.size();
		for (size_t i = 0; i < count; ++i)
			if (players[i]->GetScore() > result)
				result = players[i]->GetScore();
		return result;
	}

	bool GameInstance::OnKeyEvent(int key, int action)
	{
		size_t count = players.size();
		for (size_t i = 0; i < count; ++i)
			if (players[i]->OnKeyEvent(key, action))
				return true;
		return false;
	}

	bool GameInstance::OnCharEvent(unsigned int c)
	{
		size_t count = players.size();
		for (size_t i = 0; i < count; ++i)
			if (players[i]->OnCharEvent(c))
				return true;
		return false;
	}

	bool GameInstance::OnMouseButton(int button, int action, int modifier)
	{
		size_t count = players.size();
		for (size_t i = 0; i < count; ++i)
			if (players[i]->OnMouseButton(button, action, modifier))
				return true;
		return false;
	}

	bool GameInstance::OnMouseMove(double x, double y)
	{
		size_t count = players.size();
		for (size_t i = 0; i < count; ++i)
			if (players[i]->OnMouseMove(x, y))
				return true;
		return false;
	}

	Player * GameInstance::CreatePlayer(chaos::MyGLFW::PhysicalGamepad * in_physical_gamepad)
	{
		// ensure we can cerate a new player
		size_t count = players.size();
		if (count >= GetMaxPlayerCount())
			return nullptr;
		// create the new player
		Player * result = DoCreatePlayer();
		if (result == nullptr)
			return nullptr;
		// insert the player in our list
		players.push_back(result);
		// give the physical device to the player
		result->CapturePhysicalGamepad(in_physical_gamepad);


		return result;
	}

	Player * GameInstance::GivePhysicalGamepadToPlayer(chaos::MyGLFW::PhysicalGamepad * in_physical_gamepad)
	{
		size_t count = players.size();
		for (size_t i = 0; i < count; ++i)
			if (players[i]->CapturePhysicalGamepad(in_physical_gamepad))
				return players[i].get();
		return nullptr;

	}

	bool GameInstance::DoTick(double delta_time)
	{
		size_t count = players.size();
		for (size_t i = 0; i < count; ++i)
			players[i]->Tick(delta_time);
		return true;
	}

	void GameInstance::HandlePlayerGamepadInput(double delta_time, chaos::MyGLFW::GamepadData & gpd)
	{
		// Handle the inputs as we want, modifying the object by consuming data inside
		InternalHandleGamepadInputs(delta_time, gpd);

		// give remaining input back to game instance
		if (game != nullptr && (gpd.IsAnyAction() || gpd.IsAnyAxisAction()))
			game->HandlePlayerGamepadInput(delta_time, gpd);
	}

	void GameInstance::InternalHandleGamepadInputs(double delta_time, chaos::MyGLFW::GamepadData & gpd)
	{

	}

	size_t GameInstance::GetMaxPlayerCount() const
	{
		return 1;
	}

	Player * GameInstance::DoCreatePlayer()
	{
		return new Player(this);
	}

	double GameInstance::GetMainClockTime() const
	{
		if (main_clock == nullptr)
			return 0.0;
		return main_clock->GetClockTime();
	}

	double GameInstance::GetGameClockTime() const
	{
		if (game_clock == nullptr)
			return 0.0;
		return game_clock->GetClockTime();
	}

	double GameInstance::GetPauseClockTime() const
	{
		if (pause_clock == nullptr)
			return 0.0;
		return pause_clock->GetClockTime();
	}

	void GameInstance::FillUniformProvider(chaos::GPUProgramProvider & main_uniform_provider)
	{
		double main_time = GetMainClockTime();
		main_uniform_provider.AddVariableValue("main_time", main_time);
		double game_time = GetGameClockTime();
		main_uniform_provider.AddVariableValue("game_time", game_time);
		double pause_time = GetPauseClockTime();
		main_uniform_provider.AddVariableValue("pause_time", pause_time);
	}

	bool GameInstance::Initialize(death::Game * in_game)
	{
		// create the game instance clocks
		chaos::Clock * root_clock = in_game->GetRootClock();
		if (root_clock == nullptr)
			return false;

		if (main_clock == nullptr)
		{
			main_clock = root_clock->CreateChildClock("main_clock");
			if (main_clock == nullptr)
				return false;
		}

		if (game_clock == nullptr)
		{
			game_clock = root_clock->CreateChildClock("game_clock");
			if (game_clock == nullptr)
				return false;
		}

		if (pause_clock == nullptr)
		{
			chaos::ClockCreateParams pause_clock_params;
			pause_clock_params.paused = true;
			pause_clock = root_clock->CreateChildClock("pause_clock", pause_clock_params); // start paused ...
			if (pause_clock == nullptr)
				return false;
		}

		return true;
	}

	void GameInstance::OnEnterPause()
	{
		OnPauseStateUpdateClocks(true);

	}

	void GameInstance::OnLeavePause()
	{
		OnPauseStateUpdateClocks(false);

	}

	void GameInstance::OnGameOver()
	{
		if (game_clock != nullptr)
			game_clock->SetPause(true);
	}

	void GameInstance::OnPauseStateUpdateClocks(bool enter_pause)
	{
		if (pause_clock != nullptr)
		{
			pause_clock->SetPause(!enter_pause);
			pause_clock->Reset();
		}
		if (game_clock != nullptr)
			game_clock->SetPause(enter_pause);
	}

}; // namespace death
