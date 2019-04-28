#pragma once


#include "Ludum44Particles.h"
#include "Ludum44Game.h"
#include "Ludum44GameInstance.h"
#include "Ludum44LevelInstance.h"

#include <chaos/CollisionFramework.h>
#include <chaos/ClassTools.h>
#include <chaos/ParticleTools.h>

chaos::GPUVertexDeclaration GetTypedVertexDeclaration(boost::mpl::identity<VertexBase>)
{
	chaos::GPUVertexDeclaration result;
	result.Push(chaos::SEMANTIC_POSITION, 0, chaos::TYPE_FLOAT2);
	result.Push(chaos::SEMANTIC_TEXCOORD, 0, chaos::TYPE_FLOAT3);
	result.Push(chaos::SEMANTIC_COLOR, 0, chaos::TYPE_FLOAT4);
	return result;
}

// ===========================================================================
// ParticlePlayerTrait
// ===========================================================================

size_t ParticlePlayerTrait::ParticleToVertices(ParticlePlayer const * p, VertexBase * vertices, size_t vertices_per_particle, LayerTrait const * layer_trait) const
{
	size_t result = chaos::ParticleDefault::ParticleTrait::ParticleToVertices(p, vertices, vertices_per_particle);
	return result;
}


bool ParticlePlayerTrait::UpdateParticle(float delta_time, ParticlePlayer * particle, LayerTrait const * layer_trait) const
{
	particle->bounding_box.position += delta_time * particle->velocity;


	return false;
}



// =====================================
// TileParticleTraitExt
// =====================================

bool TileParticleTraitExt::UpdateParticle(float delta_time, death::TiledMap::TileParticle * particle)
{
	if (particle->gid == 0)
	{
		particle->color.a -= delta_time;
		if (particle->color.a <= 0.0f) // fade out the particle
			return true;	
	}
	return false;
}








// ===========================================================================
// ParticleLifeTrait
// ===========================================================================


bool ParticleLifeTrait::UpdateParticle(float delta_time, ParticleLife * particle, LayerTrait const * layer_trait) const
{

	return false;
}

size_t ParticleLifeTrait::ParticleToVertices(ParticleLife const * particle, VertexBase * vertices, size_t vertices_per_particle, LayerTrait const * layer_trait) const
{
	return chaos::ParticleDefault::ParticleTrait::ParticleToVertices(particle, vertices, vertices_per_particle);
}

// ===========================================================================
// ParticleFireTrait
// ===========================================================================

ParticleFireUpdateData ParticleFireTrait::BeginUpdateParticles(float delta_time, ParticleFire * particle, size_t count, LayerTrait const * layer_trait) const
{
	ParticleFireUpdateData result;
	if (count > 0)
	{
		result.camera_box = layer_trait->game->GetLudumLevelInstance()->GetCameraBox();
	}
	return result;
}

bool ParticleFireTrait::UpdateParticle(float delta_time, ParticleFire * particle, ParticleFireUpdateData const & update_data, LayerTrait const * layer_trait) const
{
	if (!chaos::Collide(update_data.camera_box, particle->bounding_box)) // destroy the particle outside the camera frustum (works for empty camera)
		return true;	

	particle->bounding_box.position += delta_time * particle->velocity;

	return false; // do not destroy the particle
}

size_t ParticleFireTrait::ParticleToVertices(ParticleFire const * particle, VertexBase * vertices, size_t vertices_per_particle, LayerTrait const * layer_trait) const
{
	chaos::ParticleTools::GenerateBoxParticle(particle->bounding_box, particle->texcoords, vertices, particle->rotation);
	// copy the color in all triangles vertex
	for (size_t i = 0; i < 6; ++i)
		vertices[i].color = particle->color;
	return 6;
}

// ===========================================================================
// ParticleEnemyTrait
// ===========================================================================

ParticleEnemyUpdateData ParticleEnemyTrait::BeginUpdateParticles(float delta_time, ParticleEnemy * particle, size_t count, LayerTrait const * layer_trait) const
{
	ParticleEnemyUpdateData result;
	if (count > 0)
	{
		result.camera_box = layer_trait->game->GetLudumLevelInstance()->GetCameraBox();
		result.camera_box.half_size *= 3.0f;
	}
	return result;
}

bool ParticleEnemyTrait::UpdateParticle(float delta_time, ParticleEnemy * particle, ParticleEnemyUpdateData const & update_data, LayerTrait const * layer_trait) const
{
	// destroy the particle if no life
	if (particle->life <= 0.0f)
		return true;
	// destroy the particle if outside a BIG camera box
	if (!chaos::Collide(update_data.camera_box, particle->bounding_box)) // destroy the particle outside the camera frustum (works for empty camera)
		return true;	

	// apply velocity
	particle->bounding_box.position += delta_time * particle->velocity;
	// apply rotation
	particle->rotation += delta_time * particle->rotation_speed;

	return false; // do not destroy the particle
}

size_t ParticleEnemyTrait::ParticleToVertices(ParticleEnemy const * particle, VertexBase * vertices, size_t vertices_per_particle, LayerTrait const * layer_trait) const
{
	chaos::ParticleTools::GenerateBoxParticle(particle->bounding_box, particle->texcoords, vertices, particle->rotation);
	// select wanted color
	glm::vec4 c = (particle->just_touched)?
		glm::vec4(0.0f, 0.0f, 0.0f, 0.0f):
		particle->color;
	((ParticleEnemy *)particle)->just_touched = false; // HACK : just render for one frame
	// copy the color in all triangles vertex
	for (size_t i = 0; i < 6; ++i)
		vertices[i].color = c;
	return 6;
}

