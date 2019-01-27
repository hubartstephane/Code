#include <death/GameLevel.h>
#include <death/Game.h>
#include <death/GameInstance.h>
#include <death/Player.h>

namespace death
{
	// =====================================
	// GameLevelInstance implementation
	// =====================================

	GameLevelInstance * GameLevel::CreateLevelInstance(Game * in_game)
	{
		GameLevelInstance * result = DoCreateLevelInstance(in_game); // create the instance
		if (result == nullptr)
			return nullptr;
		result->level = this;
		result->game  = in_game;
		if (!result->Initialize(in_game)) // additional initialization
		{
			delete(result);
			return nullptr;
		}
		return result;
	}

	GameLevelInstance * GameLevel::DoCreateLevelInstance(Game * in_game)
	{
		return new GameLevelInstance;
	}

	// =====================================
	// GameLevel implementation
	// =====================================


	GameLevel * GameLevelInstance::GetLevel() 
	{ 
		return level; 
	}

	GameLevel const * GameLevelInstance::GetLevel() const 
	{ 
		return level; 
	}

	Game * GameLevelInstance::GetGame() 
	{ 
		return game; 
	}
	
	Game const * GameLevelInstance::GetGame() const 
	{ 
		return game; 
	}

	GameInstance * GameLevelInstance::GetGameInstance() 
	{ 
		return game->GetGameInstance(); 
	}

	GameInstance const * GameLevelInstance::GetGameInstance() const 
	{ 
		return game->GetGameInstance(); 
	}

	void GameLevelInstance::OnPlayerLevelStarted(Player * player)
	{

	}

	void GameLevelInstance::OnPlayerLevelEnded(Player * player)
	{
	
	}

	void GameLevelInstance::OnLevelEnded()
	{
		GameInstance * game_instance = GetGameInstance();
		if (game_instance == nullptr)
			return;
		size_t count = game_instance->GetPlayerCount();
		for (size_t i = 0; i < count; ++i)
			OnPlayerLevelEnded(game_instance->GetPlayer(i));
	}
	
	void GameLevelInstance::OnLevelStarted()
	{
		GameInstance * game_instance = GetGameInstance();
		if (game_instance == nullptr)
			return;
		size_t count = game_instance->GetPlayerCount();
		for (size_t i = 0; i < count; ++i)
			OnPlayerLevelStarted(game_instance->GetPlayer(i));
	}

	bool GameLevelInstance::Initialize(Game * in_game)
	{
		return true;
	}

	bool GameLevelInstance::IsLevelCompleted() const
	{
		return false;
	}

	bool GameLevelInstance::CanCompleteLevel() const
	{
		return true; // no delay to effectively complete the level
	}

	

}; // namespace death

