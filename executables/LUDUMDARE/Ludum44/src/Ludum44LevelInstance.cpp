#include "Ludum44Level.h"
#include "Ludum44LevelInstance.h"
#include "Ludum44Game.h"
#include "Ludum44Player.h"
#include "Ludum44GameInstance.h"

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
		cameras[i]->AddComponent(new death::ShakeCameraComponent(0.15f, 0.05f, 0.15f, true, true));
		//cameras[i]->AddComponent(new death::ShakeCameraComponent(0.5f, 10.0f, 0.15f, false, false));
	}
}

bool LudumLevelInstance::IsLevelCompleted(bool & loop_levels) const
{
	loop_levels = false;

	return level_completed;
}

bool LudumLevelInstance::CanCompleteLevel() const
{


	return true;
}

bool LudumLevelInstance::DoTick(double delta_time)
{
	death::TiledMap::LevelInstance::DoTick(delta_time);

	// get the game
	LudumGame * ludum_game = GetLudumGame();
	if (ludum_game == nullptr)
		return true;
	// get the PLAYER 0
	LudumPlayer * player = GetLudumPlayer(0);
	if (player == nullptr)
		return true;
	// get the camera
	death::Camera * camera = GetCamera(0);
	if (camera == nullptr)
		return true;
	// get the camera boc without effects
	chaos::box2 camera_box = camera->GetCameraBox(false);
	if (IsGeometryEmpty(camera_box))
		return true;
	// get the player box
	chaos::box2 player_box = player->GetPlayerBox();
	if (IsGeometryEmpty(player_box))
		return true;

	// compute scroll for both camera and player
	float scroll_displacement = scroll_factor * ludum_game->scroll_factor * camera_speed * (float)delta_time;
	float camera_y = camera_box.position.y + scroll_displacement; // the final wanted camera Y

	// the camera follows the player in X & Y direction
	chaos::box2 safe_camera = camera_box;
	safe_camera.half_size *= camera->GetSafeZone();
	chaos::RestrictToInside(safe_camera, player_box, true);
	camera_box.position = safe_camera.position;

	// force the camera Y position and player is now forced to be in Y camera range
	camera_box.position.y = camera_y;
	player_box.position.y += scroll_displacement;

	// force the player Y to be in camera box (X will not change because already in correct range from previous restrictionp)
	safe_camera = camera_box;
	safe_camera.half_size *= camera->GetSafeZone();
	chaos::RestrictToInside(safe_camera, player_box, false);

	// restrict camera to world
	chaos::box2 world = GetBoundingBox();
	if (!IsGeometryEmpty(world))
	{
		chaos::RestrictToInside(world, camera_box, false);
		chaos::RestrictToInside(world, player_box, false);
	}

	// apply the compute result
	camera->SetCameraBox(camera_box);
	player->SetPlayerBox(player_box);

	return true;
}

void LudumLevelInstance::OnLevelStarted()
{
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

	// change the camera speed
	static float DEFAULT_CAMERA_SPEED = 100.0f;
	camera_speed = DEFAULT_CAMERA_SPEED;

	death::TiledMap::Level const * level = GetTiledLevel();
	if (level != nullptr)
	{
		camera_speed = level->GetTiledMap()->FindPropertyFloat("CAMERA_SPEED", DEFAULT_CAMERA_SPEED);
	}

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

	LudumPlayer * ludum_player = auto_cast(player);
	if (ludum_player == nullptr)
		return;
	ludum_player->fire_allocation = nullptr;
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

	scroll_factor = ludum_checkpoint->scroll_factor;

#if 0
	// enable all triggers
	death::TiledMap::LayerInstance * trigger_layer_instance = FindLayerInstance("Zones");
	if (trigger_layer_instance != nullptr)
	{
		size_t count = trigger_layer_instance->GetTriggerSurfaceCount();
		for (size_t i = 0; i < count; ++i)
		{
			death::TiledMap::TriggerSurfaceObject * trigger = trigger_layer_instance->GetTriggerSurface(i);
			if (trigger != nullptr)
				trigger->SetEnabled(true);
		}
	}
#endif
	// destroy all bullets
	death::TiledMap::LayerInstance * fire_layer_instance = FindLayerInstance("fire");
	if (fire_layer_instance != nullptr)
	{
		chaos::ParticleLayerBase * particle_layer = fire_layer_instance->GetParticleLayer();
		if (particle_layer != nullptr)
			particle_layer->ClearAllAllocations();
	}
	// destroy all enemies
	death::TiledMap::LayerInstance * enemies_layer_instance = FindLayerInstance("Enemies");
	if (enemies_layer_instance != nullptr)
	{
		chaos::ParticleLayerBase * particle_layer = enemies_layer_instance->GetParticleLayer();
		if (particle_layer != nullptr)
			particle_layer->ClearAllAllocations();
	}

	return true;
}

bool LudumLevelInstance::DoSaveIntoCheckpoint(death::LevelCheckpoint * checkpoint) const
{
	LudumLevelCheckpoint * ludum_checkpoint = auto_cast(checkpoint);
	if (ludum_checkpoint == nullptr)
		return false;

	if (!death::TiledMap::LevelInstance::DoSaveIntoCheckpoint(ludum_checkpoint))
		return false;

	ludum_checkpoint->scroll_factor = scroll_factor;


	return true;
}


