#pragma once

#include <death/Game.h>
#include <death/GameHUD.h>
#include <death/GameHUDComponent.h>
#include <death/GameFramework.h>

namespace death
{
	namespace GameHUDKeys
	{
		CHAOS_DECLARE_TAG(COMBO_ID);

		CHAOS_DECLARE_TAG(BACKGROUND_GAMEOBJECT_LAYER_ID);
		CHAOS_DECLARE_TAG(GAMEOBJECT_LAYER_ID);
		CHAOS_DECLARE_TAG(BRICK_LAYER_ID);
		CHAOS_DECLARE_TAG(BALL_LAYER_ID);
		CHAOS_DECLARE_TAG(CHALLENGE_LAYER_ID);
	};
};

// ====================================================================
// GameHUDComboComponent
// ====================================================================

class GameHUDComboComponent : public death::GameHUDCacheValueComponent<int>
{
public:

	GameHUDComboComponent() : death::GameHUDCacheValueComponent<int>("Combo : %d x", -1) {}

protected:

	/** override */
	virtual bool UpdateCachedValue(bool & destroy_allocation) override;
	/** override */
	virtual void TweakTextGeneratorParams(chaos::ParticleTextGenerator::GeneratorParams & params, chaos::box2 const & view_box) override;
};

// ====================================================================
// GameHUDLifeComponent
// ====================================================================

class GameHUDLifeComponent : public death::GameHUDSingleAllocationComponent
{
protected:

	/** override */
	virtual bool DoTick(double delta_time) override;
	/** update all particles (count, alpha) */
	void UpdateLifeParticles(double delta_time);
	/** tick heart */
	void TickHeartBeat(double delta_time);

protected:

	/** caching the current life count */
	int cached_value = -1;
	/** the current heart warning timer value */
	float heart_warning = 0.0f;
	/** the heart warning period */
	float heart_beat_speed = 1.0f;
};

// ====================================================================
// LudumPlayingHUD
// ====================================================================


class LudumPlayingHUD : public death::PlayingHUD
{

public:

	DEATH_GAMEFRAMEWORK_DECLARE_HUD(Ludum);

	/** constructor */
	LudumPlayingHUD(death::Game * in_game) :
		death::PlayingHUD(in_game) {}

protected:

	/** override */
	virtual bool FillHUDContent() override;
	/** override */
	virtual bool CreateHUDLayers() override;

protected:

	/** caching the current life count */
	int cached_value = -1;
};
