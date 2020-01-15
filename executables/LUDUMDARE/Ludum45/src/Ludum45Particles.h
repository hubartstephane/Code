#pragma once

#include <chaos/StandardHeaders.h> 
#include <chaos/ParticleTools.h> 
#include <chaos/ParticleManager.h> 
#include <chaos/ParticleDefault.h> 
#include <chaos/GPUVertexDeclaration.h> 
#include <chaos/BitmapAtlas.h> 

#include <death/TiledMapParticle.h> 


// ===========================================================================
// VertexBase and ParticleBase
// ===========================================================================

using VertexBase = chaos::ParticleDefault::Vertex;

//chaos::GPUVertexDeclaration * GetTypedVertexDeclaration(boost::mpl::identity<VertexBase>);

class ParticleBase : public  death::TiledMap::TileParticle
{
public:

	glm::vec2 velocity = glm::vec2(0.0f, 0.0f);

};

// ===========================================================================
// ParticleLifeTrait
// ===========================================================================

using ParticleLife = chaos::ParticleDefault::Particle;

class ParticleLifeTrait : public chaos::ParticleAllocationTrait<chaos::ParticleDefault::Particle, chaos::ParticleDefault::Vertex>
{
public:

	bool UpdateParticle(float delta_time, ParticleLife * particle) const;

	size_t ParticleToVertices(ParticleLife const * particle, VertexBase * vertices, size_t vertices_per_particle) const;

    void ParticleToVertices(ParticleLife const & particle, chaos::QuadOutput<VertexBase> & output) const;
    void ParticleToVertices(ParticleLife const& particle, chaos::TrianglePairOutput<VertexBase>& output) const;

};


// ===========================================================================
// ParticleBonus
// ===========================================================================


class ParticleBonus : public ParticleBase
{
public:

	

	chaos::TagType bonus_type;
};

class ParticleBonusTrait : public chaos::ParticleAllocationTrait<ParticleBonus, VertexBase>
{
public:

	class LayerTrait
	{
	public:

		class LudumGame * game = nullptr;
	};

	chaos::box2 BeginUpdateParticles(float delta_time, chaos::ParticleAccessor<ParticleBonus>& particle_accessor, LayerTrait const * layer_trait) const;


	bool UpdateParticle(float delta_time, ParticleBonus * particle, chaos::box2 const & player_box, LayerTrait const * layer_trait) const;

	size_t ParticleToVertices(ParticleBonus const * particle, VertexBase * vertices, size_t vertices_per_particle, LayerTrait const * layer_trait) const;

    void ParticleToVertices(ParticleBonus const& particle, chaos::QuadOutput<VertexBase>& output, LayerTrait const* layer_trait) const;
    void ParticleToVertices(ParticleBonus const& particle, chaos::TrianglePairOutput<VertexBase>& output, LayerTrait const* layer_trait) const;

};

// ===========================================================================
// ParticleEnemy
// ===========================================================================


class ParticleEnemy : public ParticleBase
{
public:


	float enemy_life = 0.0f;

	float enemy_damage = 10.0f;

	float touched_count_down = 0.0f;


	float  orientation = 0.0f;
	float  image_timer = 0.0f;
	size_t current_frame = 0;

	int enemy_particle_count = 0;

	int         enemy_index = 0;
	float       time = 0.0f;
	chaos::box2 spawner_surface;

	class EnemyPattern * pattern = nullptr;
	
};

class ParticleEnemyTrait : public chaos::ParticleAllocationTrait<ParticleEnemy, VertexBase>
{
public:

	class LayerTrait
	{
	public:

		class LudumGame * game = nullptr;
	};

	chaos::box2 BeginUpdateParticles(float delta_time, chaos::ParticleAccessor<ParticleEnemy>& particle_accessor, LayerTrait const * layer_trait) const;


	bool UpdateParticle(float delta_time, ParticleEnemy * particle, chaos::box2 const & player_box, LayerTrait const * layer_trait) const;

	size_t ParticleToVertices(ParticleEnemy const * particle, VertexBase * vertices, size_t vertices_per_particle, LayerTrait const * layer_trait) const;

    void ParticleToVertices(ParticleEnemy const& particle, chaos::QuadOutput<VertexBase>& output, LayerTrait const* layer_trait) const;
    void ParticleToVertices(ParticleEnemy const& particle, chaos::TrianglePairOutput<VertexBase>& output, LayerTrait const* layer_trait) const;
};

// ===========================================================================
// ParticlePlayer
// ===========================================================================

class ParticlePlayer : public ParticleBase
{
public:

	float  orientation = 0.0f;
	float  image_timer = 0.0f;
	size_t current_frame = 0;
	

};

class ParticlePlayerTrait : public chaos::ParticleAllocationTrait<ParticlePlayer, VertexBase>
{
public:

	class LayerTrait
	{
	public:

		class LudumGame * game = nullptr;
	};


	bool UpdateParticle(float delta_time, ParticlePlayer * particle, LayerTrait const * layer_trait) const;

	size_t ParticleToVertices(ParticlePlayer const * particle, VertexBase * vertices, size_t vertices_per_particle, LayerTrait const * layer_trait) const;

    void ParticleToVertices(ParticlePlayer const& particle, chaos::QuadOutput<VertexBase>& output, LayerTrait const* layer_trait) const;
    void ParticleToVertices(ParticlePlayer const& particle, chaos::TrianglePairOutput<VertexBase>& output, LayerTrait const* layer_trait) const;

};

// ===========================================================================
// ParticleShroudLife
// ===========================================================================

class ParticleShroudLife : public ParticleBase
{
public:

	chaos::BitmapAtlas::BitmapInfo const * bitmap_info = nullptr;

};

class ParticleShroudLifeTrait : public chaos::ParticleAllocationTrait<ParticleShroudLife, VertexBase>
{
public:

	class LayerTrait
	{
	public:

		class LudumGame * game = nullptr;
	};


	bool UpdateParticle(float delta_time, ParticleShroudLife * particle, LayerTrait const * layer_trait) const;

	size_t ParticleToVertices(ParticleShroudLife const * particle, VertexBase * vertices, size_t vertices_per_particle, LayerTrait const * layer_trait) const;

    void ParticleToVertices(ParticleShroudLife const& particle, chaos::QuadOutput<VertexBase>& output, LayerTrait const* layer_trait) const;
    void ParticleToVertices(ParticleShroudLife const& particle, chaos::TrianglePairOutput<VertexBase>& output, LayerTrait const* layer_trait) const;

};



// ===========================================================================
// ParticlePlayer
// ===========================================================================

class ParticleFire : public ParticleBase
{
public:

	bool  player_ownership = true;
	bool  trample = false;
	float damage = 1.0f;
	float rotation = 0.0f;

	float lifetime = 10.0f;
};


class ParticleFireUpdateData
{
public:

	/** the camera box */
	chaos::box2 camera_box;
	/** all the enemies */
	std::vector<ParticleEnemy*> enemies;
	/** the main player */
	class LudumPlayer * player = nullptr;
};


class ParticleFireTrait : public chaos::ParticleAllocationTrait<ParticleFire, VertexBase>
{
public:

	class LayerTrait
	{
	public:

		class LudumGame * game = nullptr;
	};

	ParticleFireUpdateData BeginUpdateParticles(float delta_time, chaos::ParticleAccessor<ParticleFire> & particle_accessor, LayerTrait const * layer_trait) const;

	bool UpdateParticle(float delta_time, ParticleFire * particle, ParticleFireUpdateData const & update_data, LayerTrait const * layer_trait) const;

	size_t ParticleToVertices(ParticleFire const * particle, VertexBase * vertices, size_t vertices_per_particle, LayerTrait const * layer_trait) const;

    void ParticleToVertices(ParticleFire const& particle, chaos::QuadOutput<VertexBase>& output, LayerTrait const* layer_trait) const;
    void ParticleToVertices(ParticleFire const& particle, chaos::TrianglePairOutput<VertexBase>& output, LayerTrait const* layer_trait) const;
};





// ===========================================================================
// ParticleExplosion
// ===========================================================================

class ParticleExplosion : public chaos::ParticleDefault::Particle
{

public:

	chaos::BitmapAtlas::BitmapInfo const * explosion_info = nullptr;

	float age = 0.0f;
};

class ParticleExplosionTrait : public chaos::ParticleAllocationTrait<ParticleExplosion, VertexBase>
{
public:

	class LayerTrait
	{
	public:

		class LudumGame * game = nullptr;
	};

	bool UpdateParticle(float delta_time, ParticleExplosion * particle, LayerTrait const * layer_trait) const;

	size_t ParticleToVertices(ParticleExplosion const * particle, VertexBase * vertices, size_t vertices_per_particle, LayerTrait const * layer_trait) const;

    void ParticleToVertices(ParticleExplosion const& particle, chaos::QuadOutput<VertexBase>& output, LayerTrait const* layer_trait) const;
    void ParticleToVertices(ParticleExplosion const& particle, chaos::TrianglePairOutput<VertexBase>& output, LayerTrait const* layer_trait) const;
};







