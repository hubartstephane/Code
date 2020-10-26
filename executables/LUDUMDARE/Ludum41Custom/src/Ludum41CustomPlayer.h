#pragma once

#include <chaos/Chaos.h>

#include <death/Game.h>
#include <death/GameInstance.h>
#include <death/Player.h>
#include <death/GameFramework.h>

// =========================================================
// LudumPlayer
// =========================================================

class LudumPlayer : public death::Player
{
	/** the Y coordinate of the player on screen */
	static constexpr float PLAYER_Y = 100.0f;
	/** the height of the player on screen */
	static constexpr float PLAYER_HEIGHT = 35.0f;

public:

	DEATH_GAMEFRAMEWORK_DECLARE_FRIENDSHIPS(Ludum);

	CHAOS_OBJECT_DECLARE_CLASS2(LudumPlayer, death::Player);

	/** change the length of the player */
	void SetPlayerLength(float in_length, bool increment);
	/** get current player length */
	float GetPlayerLength() const { return player_length; }

	/** get the maximum life count */
	int GetMaxLifeCount() const { return max_life_count; }

	virtual bool IsDead() const override;

	virtual void OnLifeLost() override;

protected:

	/** override */
	virtual void TickPlayerDisplacement(float delta_time) override;
	/** override */
	virtual bool OnMouseMoveImpl(double x, double y) override;
	/** override */
	virtual bool OnCharEventImpl(unsigned int c) override;
	/** override */
	virtual void InternalHandleGamepadInputs(float delta_time, chaos::MyGLFW::GamepadData const * gpd) override;
	/** override */
	virtual bool InitializeGameValues(nlohmann::json const& config, boost::filesystem::path const& config_path, bool hot_reload) override;
	/** override */
	virtual void OnInputModeChanged(chaos::InputMode new_mode, chaos::InputMode old_mode) override;
	/** override */
	virtual bool Initialize(death::GameInstance* in_game_instance) override;

	/** displace the racket from given quantity */
	void DisplacePlayerRacket(float delta_x);

protected:

	/** current player length */
	float player_length = 70.0f;
	/** the maximum life for the player */
	int max_life_count = 3;
};
