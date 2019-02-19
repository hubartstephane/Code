#pragma once

#include <chaos/StandardHeaders.h> 
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
	float const PLAYER_Y = -385.0f;
	/** the height of the player on screen */
	float const PLAYER_HEIGHT = 35.0f;

public:

	DEATH_GAMEFRAMEWORK_DECLARE_PLAYER(Ludum);

	/** constructor */
	LudumPlayer(death::GameInstance * in_game_instance);

	/** change the length of the player */
	void SetPlayerLength(float in_length, bool increment);
	/** get current player length */
	float GetPlayerLength() const { return player_length; }

protected:

	/** override */
	virtual void TickPlayerDisplacement(double delta_time) override;
	/** override */
	virtual bool OnMouseMove(double x, double y) override;
	/** override */
	virtual bool OnCharEvent(unsigned int c) override;
	/** override */
	virtual void InternalHandleGamepadInputs(double delta_time, chaos::MyGLFW::GamepadData const * gpd) override;

protected:

	/** current player length */
	float player_length = 70.0f;
};
