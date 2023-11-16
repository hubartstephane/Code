namespace chaos
{
#ifdef CHAOS_FORWARD_DECLARATION

	class GameInstance;

#elif !defined CHAOS_TEMPLATE_IMPLEMENTATION

	// =============================================
	// GameInstance
	// =============================================

	class CHAOS_API GameInstance : public Tickable, public InputEventReceiverInterface, public GPUProgramProviderInterface, public CheckpointObject<GameCheckpoint>, public ConfigurableInterface
	{
		CHAOS_GAMEPLAY_ALLFRIENDS;

		CHAOS_DECLARE_OBJECT_CLASS(GameInstance, Tickable);

	public:

		CHAOS_DECLARE_GAMEPLAY_GETTERS();

		/** create one player and give it the gamepad provided if any */
		Player* CreatePlayer(PhysicalGamepad* in_physical_gamepad);

		/** try to give a physical to any player (returns the player) */
		Player* GivePhysicalGamepadToPlayer(PhysicalGamepad* in_physical_gamepad);

		/** get the best score among players */
		int GetBestPlayerScore() const;

		// The clocks:
		//   - root  clock : the top level clock. never reseted, never paused
		//   - main  clock : reseted whenever a new game starts/ends. never paused
		//   - game  clock : reseted whenever a new game starts/ends. paused in MainMenu and Pause
		//   - level clock : reseted whenever a new level starts/ends. paused in MainMenu and Pause
		//   - pause clock : reseted whenever we enter/leave pause. only running during pause

		/** returns main clock */
		Clock* GetMainClock() { return main_clock.get(); }
		/** returns main clock */
		Clock const* GetMainClock() const { return main_clock.get(); }

		/** returns game clock */
		Clock* GetGameClock() { return game_clock.get(); }
		/** returns game clock */
		Clock const* GetGameClock() const { return game_clock.get(); }

		/** returns pause clock */
		Clock* GetPauseClock() { return pause_clock.get(); }
		/** returns pause clock */
		Clock const* GetPauseClock() const { return pause_clock.get(); }

		/** returns the main time */
		double GetMainClockTime() const;
		/** returns the game time */
		double GetGameClockTime() const;
		/** returns the pause time */
		double GetPauseClockTime() const;

		/** create a respawn checkpoint */
		bool CreateRespawnCheckpoint();
		/** restart from the respawn checkpoint */
		bool RestartFromRespawnCheckpoint();

		/** returns the sound category */
		SoundCategory* GetSoundCategory();
		/** returns the sound category */
		SoundCategory const* GetSoundCategory() const;


	protected:

		/** override */
		virtual bool DoProcessAction(GPUProgramProviderExecutionData const& execution_data) const override;

		/** initialize the game instance */
		virtual bool Initialize(Game* in_game);

		/** override */
		virtual bool DoTick(float delta_time) override;

		/** handle an uncatched gamepad input incomming */
		virtual bool OnGamepadInput(PhysicalGamepad* in_physical_gamepad);
		/** handle keyboard input */
		virtual bool OnKeyEventImpl(KeyEvent const& event) override;
		/** handle keyboard input */
		virtual bool OnCharEventImpl(unsigned int c) override;
		/** handle mouse input */
		virtual bool OnMouseButtonImpl(int button, int action, int modifier) override;
		/** handle mouse movement */
		virtual bool OnMouseMoveImpl(glm::vec2 const & delta) override;

		/** returns the maximum number of player */
		virtual size_t GetMaxPlayerCount() const;

		/** return a new player */
		virtual Player* DoCreatePlayer();

		/** state changes */
		virtual void OnEnterPause();
		/** state changes */
		virtual void OnLeavePause();
		/** state changes */
		virtual void OnGameOver();

		/* returns true whether we can change level */
		virtual bool CanCompleteLevel() const;

		/** pause/resume pause/game clocks */
		void OnPauseStateUpdateClocks(bool enter_pause);

		/** called whenever the level is being changed */
		virtual void OnLevelChanged(Level* new_level, Level* old_level, LevelInstance* new_level_instance);

		/** check whether there is a game over */
		virtual bool DoCheckGameOverCondition();

		/** restart player (from checkpoint or immediatly) */
		virtual bool RespawnPlayer(Player* player);

		/** called for each player whenever a level is started */
		virtual void OnPlayerEntered(Player* player);
		/** called for each player whenever a level is ended */
		virtual void OnPlayerLeaved(Player* player);

		/** called whenever the game is started */
		virtual void OnEnterGame();
		/** called whenever the game is finished */
		virtual void OnLeaveGame();

		/** override */
		virtual bool DoSaveIntoCheckpoint(GameCheckpoint* checkpoint) const override;
		/** override */
		virtual bool DoLoadFromCheckpoint(GameCheckpoint const* checkpoint) override;

	protected:

		/** the game */
		Game* game = nullptr;

		/** the player class */
		SubClassOf<Player> player_class;

		/** all the players present in the game */
		std::vector<shared_ptr<Player>> players;

		/** the clocks */
		shared_ptr<Clock> main_clock;
		shared_ptr<Clock> game_clock;
		shared_ptr<Clock> pause_clock;

		/** a sound category for the game instance */
		shared_ptr<SoundCategory> sound_category;

		/** respawn checkpoint */
		shared_ptr<GameCheckpoint> respawn_checkpoint;
	};

#endif

}; // namespace chaos