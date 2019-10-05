#include "Ludum45Level.h"
#include "Ludum45LevelInstance.h"
#include "Ludum45Game.h"
#include "Ludum45Player.h"
#include "Ludum45GameInstance.h"

#include <chaos/GLMTools.h>
#include <chaos/ParticleDefault.h>
#include <chaos/GeometryFramework.h>

// =============================================================
// LudumLevelInstance implementation
// =============================================================

DEATH_GAMEFRAMEWORK_IMPLEMENT_LEVELINSTANCE(Ludum);

LudumLevelInstance::LudumLevelInstance(LudumGame * in_game):
	game(in_game)
{
	assert(in_game != nullptr); 
}

void LudumLevelInstance::CreateCameras()
{
	death::TiledMap::LevelInstance::CreateCameras();

	size_t camera_count = cameras.size();
	for (size_t i = 0; i < camera_count; ++i)
	{
		cameras[i]->SetSafeZone(glm::vec2(0.6f, 0.8f));
		cameras[i]->AddComponent(new death::FollowPlayerCameraComponent(0));
		cameras[i]->AddComponent(new death::ShakeCameraComponent(0.15f, 0.05f, 0.15f, true, true));
		cameras[i]->AddComponent(new death::SoundListenerCameraComponent());
	}
}

bool LudumLevelInstance::CanCompleteLevel() const
{


	return true;
}

bool LudumLevelInstance::DoTick(double delta_time)
{
	death::TiledMap::LevelInstance::DoTick(delta_time);
	RestrictPlayerToWorld(0);
	return true;
}

void LudumLevelInstance::OnLevelStarted()
{
	// !!! Should be automatic no ??

	// shuludum

	// create the fire particle layer if necessary
	death::TiledMap::LayerInstance * layer_instance = FindLayerInstance("fire");
	if (layer_instance != nullptr)
		layer_instance->CreateParticleLayer();











	// super call
	death::TiledMap::LevelInstance::OnLevelStarted();
}

bool LudumLevelInstance::Initialize(death::Game * in_game, death::GameLevel * in_level)
{
	if (!death::TiledMap::LevelInstance::Initialize(in_game, in_level))
		return false;




	return true;
}

void LudumLevelInstance::OnPlayerEntered(death::Player * player)
{
	death::TiledMap::LevelInstance::OnPlayerEntered(player);

	LudumPlayer * ludum_player = auto_cast(player);
	if (ludum_player == nullptr)
		return;

	death::TiledMap::LayerInstance * layer_instance = FindLayerInstance("fire");
	if (layer_instance == nullptr)
		return;

	chaos::ParticleLayerBase * fire_layer = layer_instance->GetParticleLayer();
	if (fire_layer != nullptr)
		ludum_player->fire_allocation = fire_layer->SpawnParticles(0);
}

void LudumLevelInstance::OnPlayerLeaved(death::Player * player)
{
	death::TiledMap::LevelInstance::OnPlayerLeaved(player);


}








death::LevelCheckpoint * LudumLevelInstance::DoCreateCheckpoint() const
{
	return new LudumLevelCheckpoint();
}

bool LudumLevelInstance::DoLoadFromCheckpoint(death::LevelCheckpoint const * checkpoint)
{
	LudumLevelCheckpoint const * ludum_checkpoint = auto_cast(checkpoint);
	if (ludum_checkpoint == nullptr)
		return false;

	if (!death::TiledMap::LevelInstance::DoLoadFromCheckpoint(ludum_checkpoint))
		return false;


	return true;
}

bool LudumLevelInstance::DoSaveIntoCheckpoint(death::LevelCheckpoint * checkpoint) const
{
	LudumLevelCheckpoint * ludum_checkpoint = auto_cast(checkpoint);
	if (ludum_checkpoint == nullptr)
		return false;

	if (!death::TiledMap::LevelInstance::DoSaveIntoCheckpoint(ludum_checkpoint))
		return false;






	return true;
}

