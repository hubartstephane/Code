#pragma once

#include <chaos/StandardHeaders.h> 

#include <death/Game.h>
#include <death/GameInstance.h>
#include <death/Player.h>
#include <death/GameFramework.h>

#include "Ludum43Particles.h"

// =========================================================
// LudumPlayer
// =========================================================

class LudumPlayer : public death::Player
{

public:

	DEATH_GAMEFRAMEWORK_DECLARE_PLAYER(Ludum);

	/** constructor */
	LudumPlayer(death::GameInstance * in_game_instance);

	/** get the player particle */
	ParticlePlayer * GetPlayerParticle();
	ParticlePlayer const * GetPlayerParticle() const;

	/** called whenever the level is being changed */
	void OnLevelChanged();

	/** override */
	virtual void SetPlayerAllocation(chaos::ParticleAllocation * in_allocation) override;

protected:

	/** override */
	virtual void TickPlayerDisplacement(double delta_time) override;
	/** override */
	virtual void InternalHandleGamepadInputs(double delta_time, chaos::MyGLFW::GamepadData const * gpd) override;
	/** override */
	virtual void HandleKeyboardInputs(double delta_time) override;

	/** update player displacement with the stick values that are stored */
	virtual void UpdatePlayerAcceleration(double delta_time);
	/** cooldown the weapon */
	void TickCooldown(double delta_time);
	void TickDashValues(double delta_time);

	/** change dash mode */
	void SetDashMode(bool dash);
	/** change reverse mode */
	void SetReverseMode(bool reversed_mode);
	
protected:

	float current_cooldown = 0.1f;
	float current_dash_duration = 0.0f;
	float current_dash_cooldown = 0.0f;

};
