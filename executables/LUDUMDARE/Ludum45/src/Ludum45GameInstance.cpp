#include "Ludum45GameInstance.h"
#include "Ludum45Game.h"
#include "Ludum45Level.h"
#include "Ludum45LevelInstance.h"
#include "Ludum45Player.h"

#include <death/SoundContext.h>

DEATH_GAMEFRAMEWORK_IMPLEMENT_GAMEINSTANCE(Ludum);

LudumGameInstance::LudumGameInstance(death::Game * in_game) : 
	death::GameInstance(in_game)
{
	
}

death::Player * LudumGameInstance::DoCreatePlayer()
{
	return new LudumPlayer(this);
}

bool LudumGameInstance::DoCheckGameOverCondition() // shuludum : mututaliser le code
{
	LudumPlayer * ludum_player = GetLudumPlayer(0);
	if (ludum_player != nullptr)
	{
		if (ludum_player->GetCurrentLife() <= 0.0f) // no more energy => go to checkpoint
		{
			// game over mandatory (last life about to be removed)
			int life_count = ludum_player->GetLifeCount();
			if (life_count <= 1)
				return true;
			// keep some values to restore later
			int score = ludum_player->GetScore();
			// try to go to checkpoint
			if (!RestartFromRespawnCheckpoint())
				return true;
			// update player values after death
			ludum_player->SetScore(score, false);
			ludum_player->SetLifeCount(life_count - 1, false); 
			ludum_player->OnLifeLost();			
		}
	}
	return false;
}

bool LudumGameInstance::DoSaveIntoCheckpoint(death::GameCheckpoint * checkpoint) const
{
	if (!death::GameInstance::DoSaveIntoCheckpoint(checkpoint))
		return false;

	return true;
}

bool LudumGameInstance::DoLoadFromCheckpoint(death::GameCheckpoint const * checkpoint)
{
	if (!death::GameInstance::DoLoadFromCheckpoint(checkpoint))
		return false;

	return true;
}

ParticleFire * LudumGameInstance::FireProjectile(chaos::ParticleAllocationBase * allocation, chaos::box2 const & ref_box, chaos::BitmapAtlas::BitmapLayout const & layout, float ratio_to_box, int count, char const * sound_name, float delta_rotation, bool player_ownership, float velocity, float offset_rotation)
{
	if (count <= 0)
		return nullptr;
	if (allocation == nullptr)
		return nullptr;

	LudumGame * ludum_game = GetLudumGame();
	if (ludum_game == nullptr)
		return nullptr;
	if (!allocation->AddParticles(count))
		return nullptr;
	chaos::ParticleAccessor<ParticleFire> particles = allocation->GetParticleAccessor();
	if (particles.GetCount() == 0)
		return nullptr;


	ParticleFire * result = &particles[particles.GetCount() - count];

	// compute the bounding box for all particles
	chaos::box2 particle_box = ref_box;

	particle_box.half_size = ratio_to_box * ref_box.half_size;
	particle_box = chaos::AlterBoxToAspect(particle_box, chaos::MathTools::CastAndDiv<float>(layout.width, layout.height), true);	
	particle_box.position = ref_box.position;

	// compute texcoords for all particles
	chaos::ParticleTexcoords texcoords = chaos::ParticleTools::GetParticleTexcoords(layout);


	float rotation = 0.0f - delta_rotation * (float)(count / 2);

	for (int i = 0 ; i < count ; ++i)
	{
		result[i].bounding_box = particle_box;	
		result[i].texcoords = texcoords;
		result[i].color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

		int scroll_direction = 1;
		float particle_orientation = rotation + offset_rotation;
		float particle_velocity_orientation = offset_rotation + rotation + (float)M_PI_2;

		glm::vec2 direction = glm::vec2(std::cos(particle_velocity_orientation), std::sin(particle_velocity_orientation));
		result[i].velocity  = velocity * direction;
		result[i].rotation  = particle_orientation;

		result[i].bounding_box.position += direction * ref_box.half_size[scroll_direction];

		result[i].player_ownership = player_ownership;

		rotation += delta_rotation;
	}

	if (sound_name != nullptr)	
		ludum_game->Play(sound_name, false, false, 0.0f, death::SoundContext::LEVEL);

	return result;
}

void LudumGameInstance::FireExplosion(chaos::box2 const & ref_box)
{
    death::TiledMap::LayerInstance* layer_instance = GetLudumLevelInstance()->FindLayerInstance("Explosions");
    if (layer_instance == nullptr)
        return;

    chaos::ParticleSpawner spawner = layer_instance->GetParticleSpawner("explosion");
    if (!spawner.IsValid())
        return;

    chaos::BitmapAtlas::BitmapInfo const* explosion_info = spawner.GetBitmapInfo();

    spawner.SpawnParticles(1, false, [ref_box, explosion_info](chaos::ParticleAccessorBase<ParticleExplosion> accessor) 
    {
        for (ParticleExplosion& particle : accessor)
        {
            particle.bounding_box = ref_box;
            particle.bounding_box.half_size.x = particle.bounding_box.half_size.y = std::max(ref_box.half_size.x, ref_box.half_size.y);
            particle.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
            particle.explosion_info = explosion_info;
        }
    });
}

void LudumGameInstance::OnLevelChanged(death::GameLevel * new_level, death::GameLevel * old_level, death::GameLevelInstance * new_level_instance)
{
	// super method
	death::GameInstance::OnLevelChanged(new_level, old_level, new_level_instance);
}

void LudumGameInstance::OnPlayerEntered(death::Player * player)
{
	death::GameInstance::OnPlayerEntered(player);

	LudumGame * ludum_game = GetLudumGame();
	if (ludum_game == nullptr)
		return;

	LudumPlayer * ludum_player = auto_cast(player);
	if (ludum_player == nullptr)
		return;

	// find bitmap set
	chaos::BitmapAtlas::FolderInfo const * bitmap_set = ludum_game->GetTextureAtlas()->GetFolderInfo("sprites");
	if (bitmap_set != nullptr)
	{
		chaos::BitmapAtlas::BitmapInfo const * fire_info = bitmap_set->GetBitmapInfo("fire");
		if (fire_info == nullptr)
			return;

		ludum_player->fire_bitmap_layout = *fire_info;
	}
}
