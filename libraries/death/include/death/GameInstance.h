#pragma once

#include <chaos/StandardHeaders.h>
#include <chaos/ReferencedObject.h>
#include <chaos/Tickable.h>
#include <death/Player.h>
#include <death/Game.h>

namespace death
{

	// =============================================
	// GameInstance
	// =============================================

	class GameInstance : public chaos::Tickable
	{
		friend class Game;
		friend class Player;

	public:

		/** constructor */
		GameInstance(Game * in_game);

		/** get the player by its index */
		Player * GetPlayer(int player_index);
		/** get the player by its index */
		Player const * GetPlayer(int player_index) const;
		/** get the number of players */
		size_t GetPlayerCount() const { return players.size(); }

		/** returns the game */
		Game * GetGame() { return game; }
		/** returns the game */
		Game const * GetGame() const { return game; }

		/** create one player and give it the gamepad provided if any */
		Player * CreatePlayer(chaos::MyGLFW::PhysicalGamepad * in_physical_gamepad);

		/** try to give a physical to any player (returns the player) */
		Player * GivePhysicalGamepadToPlayer(chaos::MyGLFW::PhysicalGamepad * in_physical_gamepad);

		/** get the best score among players */
		int GetBestPlayerScore() const;


		/** returns main clock */
		chaos::Clock * GetMainClock() { return main_clock.get(); }
		/** returns main clock */
		chaos::Clock const * GetMainClock() const { return main_clock.get(); }

		/** returns game clock */
		chaos::Clock * GetGameClock() { return game_clock.get(); }
		/** returns game clock */
		chaos::Clock const * GetGameClock() const { return game_clock.get(); }

		/** returns pause clock */
		chaos::Clock * GetPauseClock() { return pause_clock.get(); }
		/** returns pause clock */
		chaos::Clock const * GetPauseClock() const { return pause_clock.get(); }

		/** returns the main time */
		double GetMainClockTime() const;
		/** returns the game time */
		double GetGameClockTime() const;
		/** returns the pause time */
		double GetPauseClockTime() const;

	protected:

		/** initialize the game instance */
		virtual bool Initialize(death::Game * in_game);

		/** override */
		virtual bool DoTick(double delta_time) override;

		/** handle keyboard input */
		virtual bool OnKeyEvent(int key, int action);
		/** handle keyboard input */
		virtual bool OnCharEvent(unsigned int c);
		/** handle mouse input */
		virtual bool OnMouseButton(int button, int action, int modifier);
		/** handle mouse movement */
		virtual bool OnMouseMove(double x, double y);
		/** handle gamepad inputs that are not catched by the players */
		virtual void HandlePlayerGamepadInput(double delta_time, chaos::MyGLFW::GamepadData & gpd);
		/** handle the input for our own purpose (consuming any data) */
		virtual void InternalHandleGamepadInputs(double delta_time, chaos::MyGLFW::GamepadData & gpd);


		/** returns the maximum number of player */
		size_t GetMaxPlayerCount() const;

		/** return a new player */
		virtual Player * DoCreatePlayer();

		/** fill the rendering params before rendering */
		virtual void FillUniformProvider(chaos::GPUProgramProvider & main_uniform_provider);

		/** state changes */
		virtual void OnEnterPause();
		/** state changes */
		virtual void OnLeavePause();
		/** state changes */
		virtual void OnGameOver();

		/** pause/resume pause/game clocks */
		void OnPauseStateUpdateClocks(bool enter_pause);

	protected:

		/** the game */
		Game * game = nullptr;

		/** all the players present in the game */
		std::vector<chaos::shared_ptr<Player>> players;

		/** the clocks */
		chaos::shared_ptr<chaos::Clock> main_clock;
		chaos::shared_ptr<chaos::Clock> game_clock;
		chaos::shared_ptr<chaos::Clock> pause_clock;

	};

}; // namespace death
