#pragma once

#include <chaos/StandardHeaders.h> 
#include <chaos/Object.h>
#include <chaos/TiledMap.h>

#include <death/Level.h>
#include <death/TM.h>
#include <death/Game.h>
#include <death/Player.h>
#include <death/GameFramework.h>

#include "Ludum46Game.h"

// =================================================
// LudumLevelInstance
// =================================================

class LudumLevelInstance : public death::TMLevelInstance
{
public:

	DEATH_GAMEFRAMEWORK_DECLARE_FRIENDSHIPS(Ludum);

	CHAOS_OBJECT_DECLARE_CLASS2(LudumLevelInstance, death::TMLevelInstance);

	LudumLevelInstance();

	void SpawnBloodParticles(chaos::box2 const& box, int particles_count);

	void SpawnBurnedSoulParticles(chaos::box2 const& box, int particles_count);

protected:

	/** override */
	virtual bool DoTick(float delta_time) override;
	/** override */
	virtual bool Initialize(death::Game * in_game, death::Level * in_level) override;
	/** override */
	virtual void CreateCameras() override;

	/** override */
	virtual bool IsPlayerDead(death::Player* player) override;

	virtual bool CheckLevelCompletion() const override;

	virtual bool CanCompleteLevel() const override;

	int GetCurrentSoulCount() const;

	int GetPotentialSoulCount() const;






protected:

	/** pointer on game */
	class LudumGame * game = nullptr;


	mutable float completion_timer = -1.0f; // shu46 : due to Checkcomplete constness !!
	float completion_delay = 3.0f;

	float flame_health = 100.0f;
	float flame_initial_health = 100.0f;
	float flame_health_lost_per_second = 1.0f;
};
