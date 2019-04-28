#pragma once

#include <chaos/StandardHeaders.h> 
#include <chaos/ReferencedObject.h>
#include <chaos/TiledMap.h>
#include <chaos/TiledMapTools.h>

#include <death/GameLevel.h>
#include <death/TiledMapLevel.h>
#include <death/Game.h>
#include <death/GameFramework.h>

#include "Ludum44Game.h"

// =================================================
// PowerUpTriggerSurfaceObject
// =================================================

class PowerUpTriggerSurfaceObject : public death::TiledMap::TriggerSurfaceObject
{

public:

	/** constructor */
	using death::TiledMap::TriggerSurfaceObject::TriggerSurfaceObject;

protected:

	/** called whenever a collision with player is detected (returns false, if loop is to be broken) */
	virtual bool OnPlayerCollisionEvent(double delta_time, death::Player * player, chaos::ParticleDefault::Particle * player_particle, int event_type) override;
};

// =================================================
// FinishingTriggerSurfaceObject
// =================================================

class FinishingTriggerSurfaceObject : public death::TiledMap::TriggerSurfaceObject
{

public:

	/** constructor */
	using death::TiledMap::TriggerSurfaceObject::TriggerSurfaceObject;

protected:

	/** called whenever a collision with player is detected (returns false, if loop is to be broken) */
	virtual bool OnPlayerCollisionEvent(double delta_time, death::Player * player, chaos::ParticleDefault::Particle * player_particle, int event_type) override;
};

// =================================================
// CheckpointTriggerSurfaceObject
// =================================================

class CheckpointTriggerSurfaceObject : public death::TiledMap::TriggerSurfaceObject
{

public:

	/** constructor */
	using death::TiledMap::TriggerSurfaceObject::TriggerSurfaceObject;

protected:

	/** called whenever a collision with player is detected (returns false, if loop is to be broken) */
	virtual bool OnPlayerCollisionEvent(double delta_time, death::Player * player, chaos::ParticleDefault::Particle * player_particle, int event_type) override;
};

// =================================================
// SpeedUpTriggerSurfaceObject
// =================================================

class SpeedUpTriggerSurfaceObject : public death::TiledMap::TriggerSurfaceObject
{

public:

	/** constructor */
	using death::TiledMap::TriggerSurfaceObject::TriggerSurfaceObject;

protected:

	/** called whenever a collision with player is detected (returns false, if loop is to be broken) */
	virtual bool OnPlayerCollisionEvent(double delta_time, death::Player * player, chaos::ParticleDefault::Particle * player_particle, int event_type) override;
};


// =================================================
// SpawnerTriggerSurfaceObject
// =================================================

class SpawnerTriggerSurfaceObject : public death::TiledMap::TriggerSurfaceObject
{

public:

	/** constructor */
	using death::TiledMap::TriggerSurfaceObject::TriggerSurfaceObject;

protected:

	/** called whenever a collision with player is detected (returns false, if loop is to be broken) */
	virtual bool OnPlayerCollisionEvent(double delta_time, death::Player * player, chaos::ParticleDefault::Particle * player_particle, int event_type) override;
};



// =================================================
// Levels
// =================================================

class LudumLevel : public death::TiledMap::Level
{
	friend class LudumLevelInstance;

public:

	/** constructor */
	LudumLevel();

protected:

	/** override */
	virtual death::GameLevelInstance * DoCreateLevelInstance(death::Game * in_game) override;

	virtual bool FinalizeLayerParticles(death::TiledMap::LayerInstance * layer_instance, chaos::ParticleAllocationBase * allocation) override;

	virtual chaos::ParticleLayerBase * CreateParticleLayer(death::TiledMap::LayerInstance * layer_instance) override;

	virtual death::TiledMap::TriggerSurfaceObject * DoCreateTriggerSurface(death::TiledMap::LayerInstance * in_layer_instance, chaos::TiledMap::GeometricObject * in_geometric_object) override;

	virtual bool OnPlayerTileCollision(double delta_time, class death::Player * player, chaos::ParticleDefault::Particle * player_particle, death::TiledMap::TileParticle * particle) override;
};
