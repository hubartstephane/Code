#include "chaos/ChaosPCH.h"
#include "chaos/ChaosInternals.h"

namespace chaos
{

	CHAOS_IMPLEMENT_GAMEPLAY_GETTERS(Player);

	// =================================================
	// Player
	// =================================================

	void Player::OnLevelChanged(Level * new_level, Level * old_level, LevelInstance * new_level_instance)
	{
        health = max_health;
		invulnerability_timer = std::max(invulnerability_duration, 0.0f);
	}

	bool Player::Initialize(GameInstance * in_game_instance)
	{
		// ensure valid arguments and not already initialized
		assert(in_game_instance != nullptr);
		assert(game_instance == nullptr);

		game_instance = in_game_instance;

		// read the configuration
		return ReadConfigurableProperties(ReadConfigurablePropertiesContext::INITIALIZATION);
	}

	bool Player::CapturePhysicalGamepad(PhysicalGamepad * in_physical_gamepad)
	{
		// if we already have a device, ignore
		if (gamepad != nullptr)
			return false;
		// if no entry or the device is already owned by someone, exit
		if (in_physical_gamepad == nullptr || in_physical_gamepad->IsAllocated())
			return false;
		// try capture the device
		shared_ptr<PlayerGamepadCallbacks> gamepad_callback = new PlayerGamepadCallbacks(this);
		gamepad = in_physical_gamepad->CaptureDevice(gamepad_callback.get());
		if (gamepad == nullptr)
			return false;
		return true;
	}

	void Player::OnGamepadDisconnected()
	{
		Game * game = GetGame();
		if (game != nullptr)
			game->RequirePauseGame();
	}

	void Player::HandleSpecialGamepadCommands(float delta_time)
	{
		if (gamepad == nullptr)
			return;
		// get the gamepad data
		GamepadState const* gamepad_state = gamepad->GetGamepadState();
		if (gamepad_state == nullptr)
			return;
		// maybe a game/pause resume
		if ((gamepad_state->GetButtonStateChange(GamepadButton::SPECIAL_LEFT) == ButtonStateChange::BECOME_PRESSED) ||
			(gamepad_state->GetButtonStateChange(GamepadButton::SPECIAL_RIGHT) == ButtonStateChange::BECOME_PRESSED))
		{
			Game* game = GetGame();
			if (game != nullptr)
				game->RequireTogglePause();
		}
	}

	bool Player::DoTick(float delta_time)
	{	
		// remove previous frame cached input
		ResetCachedInputs();
		// cache values for stick displacement
		CacheKeyboardPlayerDisplacementInputs();
		// cache values for stick displacement
		CacheGamepadPlayerDisplacementInputs();

		bool mute_force_feedback   = true; // no forcefeedback except during main loop
		bool paused_force_feedback = true;

		// tick player displacement
		Game* game = GetGame();
		if (game != nullptr && !game->IsFreeCameraMode() && game->IsPlaying())
		{
			// tick the inputs
			GamepadState const* gamepad_state = (gamepad != nullptr)? gamepad->GetGamepadState() : nullptr;
			HandleInputs(delta_time, gamepad_state);
			// tick other player objects
			TickInternal(delta_time);

			mute_force_feedback = false;
			paused_force_feedback = false;
		}

		// some gamepad commands that should be handled even when not in Game
		HandleSpecialGamepadCommands(delta_time);
		// update the forcefeedback mute state
		if (gamepad != nullptr)
		{
			gamepad->SetForceFeedbackMuted(mute_force_feedback || (GetInputMode() != InputMode::GAMEPAD));
			gamepad->SetForceFeedbackPaused(paused_force_feedback);
		}

		return true;
	}

	void Player::TickInternal(float delta_time)
	{
		// tick the invulnerability
		TickInvulnerability(delta_time);
		// tick the player displacement
		TickPlayerDisplacement(delta_time);
	}

	void Player::TickInvulnerability(float delta_time)
	{
		invulnerability_timer -= delta_time;
		if (invulnerability_timer < 0.0f)
			invulnerability_timer = 0.0f;
	}

	void Player::TickPlayerDisplacement(float delta_time)
	{
		if (displacement_component != nullptr)
			displacement_component->Tick(delta_time);
	}

	void Player::CacheKeyboardPlayerDisplacementInputs()
	{
		// test whether the stick position can be overriden
		glm::vec2 simulated_stick = glm::vec2(0.0f, 0.0f);

		if (CheckKeyPressed(KeyboardButton::LEFT))
			simulated_stick.x -= 1.0f;
		if (CheckKeyPressed(KeyboardButton::RIGHT))
			simulated_stick.x += 1.0f;

		if (CheckKeyPressed(KeyboardButton::DOWN))
			simulated_stick.y -= 1.0f;
		if (CheckKeyPressed(KeyboardButton::UP))
			simulated_stick.y += 1.0f;

		if (glm::length2(simulated_stick) > 0)
			left_stick_position = simulated_stick;
	}

	void Player::CacheGamepadPlayerDisplacementInputs()
	{
		// early exit
		if (gamepad == nullptr)
			return;
		// get the gamepad data
		GamepadState const * gamepad_state = gamepad->GetGamepadState();
		if (gamepad_state == nullptr)
			return;
		// change the application mode
		if (gamepad_state->IsAnyAction())			
			SetInputMode(InputMode::GAMEPAD);

		// cache the LEFT stick position (it is aliases with the DPAD)
		glm::vec2 lsp = gamepad_state->GetStickValue(GamepadStick::LEFT_STICK);
		if (glm::length2(lsp) > 0.0f)
			left_stick_position = lsp;
		else
		{
			if (gamepad_state->IsButtonPressed(GamepadButton::DPAD_LEFT, false))
				left_stick_position.x = -1.0f;
			else if (gamepad_state->IsButtonPressed(GamepadButton::DPAD_RIGHT, false))
				left_stick_position.x = 1.0f;

			if (gamepad_state->IsButtonPressed(GamepadButton::DPAD_UP, false))
				left_stick_position.y = +1.0f;
			else if (gamepad_state->IsButtonPressed(GamepadButton::DPAD_DOWN, false))
				left_stick_position.y = -1.0f;
		}

		// cache the RIGHT stick position
		glm::vec2 rsp = gamepad_state->GetStickValue(GamepadStick::RIGHT_STICK);
		if (glm::length2(rsp) > 0.0f)
			right_stick_position = rsp;

		// cache the TRIGGERS
		left_trigger  = gamepad_state->GetAxisValue(GamepadAxis::LEFT_TRIGGER);
		right_trigger = gamepad_state->GetAxisValue(GamepadAxis::RIGHT_TRIGGER);
	}

	void Player::HandleInputs(float delta_time, GamepadState const* gamepad_state)
	{

	}

	void Player::ResetCachedInputs()
	{
		previous_left_stick_position = left_stick_position;
		previous_right_stick_position = right_stick_position;
		previous_left_trigger = left_trigger;
		previous_right_trigger = right_trigger;

		left_stick_position  = glm::vec2(0.0f, 0.0f);
		right_stick_position = glm::vec2(0.0f, 0.0f);
		left_trigger  = 0.0f;
		right_trigger = 0.0f;
	}

	void Player::SetScore(int in_score, bool increment)
	{
		if (increment)
			score += in_score;
		else
			score = in_score;	
	}

	void Player::SetLifeCount(int in_life, bool increment)
	{
		// update life
		if (increment)
			life_count += in_life;
		else
			life_count = in_life;
		// clamp life
		life_count = std::max(life_count, 0);
	}

	bool Player::SerializeIntoJSON(nlohmann::json& json) const
	{
		if (!JSONSerializableInterface::SerializeIntoJSON(json))
			return false;

		JSONTools::SetAttribute(json, "LIFE_COUNT", life_count);
		JSONTools::SetAttribute(json, "HEALTH", health);
		JSONTools::SetAttribute(json, "MAX_HEALTH", max_health);
		JSONTools::SetAttribute(json, "INVULNERABILITY_TIMER", invulnerability_timer);
		JSONTools::SetAttribute(json, "INVULNERABILITY_DURATION", invulnerability_duration);
		JSONTools::SetAttribute(json, "SCORE", score);

		if (pawn != nullptr)
			JSONTools::SetAttribute(json, "PAWN", *pawn);

		return true;
	}

	bool Player::SerializeFromJSON(nlohmann::json const& json)
	{
		if (!JSONSerializableInterface::SerializeFromJSON(json))
			return false;

		JSONTools::GetAttribute(json, "LIFE_COUNT", life_count);
		JSONTools::GetAttribute(json, "HEALTH", health);
		JSONTools::GetAttribute(json, "MAX_HEALTH", max_health);
		JSONTools::GetAttribute(json, "INVULNERABILITY_TIMER", invulnerability_timer);
		JSONTools::GetAttribute(json, "INVULNERABILITY_DURATION", invulnerability_duration);
		JSONTools::GetAttribute(json, "SCORE", score);

		// XXX : the indirection is important to avoid a reallocation of the pawn
		if (pawn != nullptr)
			JSONTools::GetAttribute(json, "PAWN", *pawn);

		return true;
	}

	void Player::OnLifeLost()
	{
        health = max_health;
		invulnerability_timer = std::max(invulnerability_duration, 0.0f);
	}

    void Player::SetHealth(float in_value, bool in_increment)
    {
        // compute new life 
        float old_health = health;
        float new_health = health;

        if (in_increment)
            new_health += in_value;
        else
            new_health = in_value;

        if (new_health < 0.0f)
            new_health = 0.0f;
        else if (new_health > max_health)
            new_health = max_health;

		bool invulnerable = false;
#if _DEBUG
		if (GetGame()->GetCheatMode())
			invulnerable = true;
#endif
		// invulnerability ?
		if (new_health < old_health && !invulnerable)
		{
			if (invulnerability_timer > 0)
				invulnerable = true;
			else
				invulnerability_timer = std::max(invulnerability_duration, 0.0f);
		}

        // commit life lost
		if (!invulnerable || new_health > old_health)
			health = new_health;
		OnHealthChanged(old_health, new_health, invulnerable);
    }

    void Player::OnHealthChanged(float old_health, float new_health, bool invulnerable)
    {
        // special FX
        if (old_health > new_health && !invulnerable)
        {
			PlayHealthChangedEffects(old_health > new_health);
        }
    }

	void Player::PlayHealthChangedEffects(bool health_lost)
	{
		// force feedback effect
		if (gamepad != nullptr)
			gamepad->AddForceFeedbackEffect(new DefaultForceFeedbackEffect(0.09f, 1.0f, 1.0f));
		// camera effect
		Camera* camera = GetLevelInstance()->GetCamera(0);
		if (camera != nullptr)
		{
			ShakeCameraComponent* shake_component = camera->FindComponentByClass<ShakeCameraComponent>();
			if (shake_component != nullptr)
				shake_component->RestartModifier();
		}
	}

	bool Player::IsDead() const
	{
		// no more health
		if (health <= 0.0f)
			return true;
		return false;
	}

    bool Player::DoCheckKeyPressed(Key button, bool previous_frame)
    {
		// gamepad input
		if (button.IsGamepadKey())
		{
			if (gamepad != nullptr)
				return gamepad->IsButtonPressed(button.GetGamepadButton(), previous_frame);
			return false;
		}
		// super call
		return InputEventReceiverInterface::DoCheckKeyPressed(button, previous_frame);
    }

	bool Player::OnReadConfigurableProperties(JSONReadConfiguration config, ReadConfigurablePropertiesContext context)
	{
		CHAOS_JSON_ATTRIBUTE(config, life_count);
		CHAOS_JSON_ATTRIBUTE(config, max_health);
		CHAOS_JSON_ATTRIBUTE(config, invulnerability_duration);
		health = max_health;
		return true;
	}

	// =================================================
	// PlayerGamepadCallbacks
	// =================================================

	PlayerGamepadCallbacks::PlayerGamepadCallbacks(Player * in_player) :
		player(in_player)
	{
		assert(in_player != nullptr);
	}

	bool PlayerGamepadCallbacks::OnGamepadDisconnected(Gamepad * in_gamepad)
	{
		player->OnGamepadDisconnected();
		return true;
	}

}; // namespace chaos

