#pragma once

#include <chaos/StandardHeaders.h>
#include <chaos/Tickable.h>
#include <death/GameFramework.h>

namespace death
{

	// =============================================
	// GameInstanceEntity
	// =============================================

	class GameInstanceEntity
	{
		DEATH_GAMEFRAMEWORK_ALLFRIENDS()

	public:

		/** constructor */
		GameInstanceEntity(GameInstance * in_game_instance);
		/** destructor */
		//virtual ~GameInstanceEntity() = default;


		virtual ~GameInstanceEntity();





		/** returns the game */
		Game * GetGame();
		/** returns the game */
		Game const * GetGame() const;

		/** returns the game instance */
		GameInstance * GetGameInstance() { return game_instance; }
		/** returns the game instance */
		GameInstance const * GetGameInstance() const { return game_instance; }

		/** returns the level */
		GameLevel * GetLevel();
		/** returns the level */
		GameLevel const * GetLevel() const;

		/** returns the level instance */
		GameLevelInstance * GetLevelInstance();
		/** returns the level */
		GameLevelInstance const * GetLevelInstance() const;

	protected:

		/** initialize the game instance */
		virtual bool Initialize(death::GameInstance * in_game_instance);

	protected:

		/** the game instance owning the player */
		GameInstance * game_instance = nullptr;
	};

}; // namespace death
