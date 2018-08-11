#include "Ludum42Level.h"

#include <chaos/GLMTools.h>

death::GameLevelInstance * LudumGameplayLevel::DoCreateLevelInstance()
{
	return new LudumGameplayLevelInstance(game);
}

death::GameLevelInstance * LudumNarrativeLevel::DoCreateLevelInstance()
{
	return new LudumNarrativeLevelInstance(game);
}

// =============================================================

LudumLevel::LudumLevel(class LudumGame * in_game):
	game(in_game)
{
}

LudumLevelInstance::LudumLevelInstance(class LudumGame * in_game):
	game(in_game)
{
}

LudumNarrativeLevel::LudumNarrativeLevel(class LudumGame * in_game):
	LudumLevel(in_game)
{
}

LudumNarrativeLevelInstance::LudumNarrativeLevelInstance(class LudumGame * in_game):
	LudumLevelInstance(in_game)
{
}

LudumGameplayLevel::LudumGameplayLevel(class LudumGame * in_game):
	LudumLevel(in_game)
{
}

LudumGameplayLevelInstance::LudumGameplayLevelInstance(class LudumGame * in_game):
	LudumLevelInstance(in_game)
{
}

// =============================================================

LudumNarrativeLevel * LudumNarrativeLevelInstance::GetLudumLevel()
{
	return dynamic_cast<LudumNarrativeLevel*>(GetLevel());
}

LudumNarrativeLevel const * LudumNarrativeLevelInstance::GetLudumLevel() const
{
	return dynamic_cast<LudumNarrativeLevel const *>(GetLevel());
}

LudumGameplayLevel * LudumGameplayLevelInstance::GetLudumLevel()
{
	return dynamic_cast<LudumGameplayLevel*>(GetLevel());
}

LudumGameplayLevel const * LudumGameplayLevelInstance::GetLudumLevel() const
{
	return dynamic_cast<LudumGameplayLevel const *>(GetLevel());
}

// =============================================================

void LudumGameplayLevelInstance::OnLevelEnded()
{
	allocations.clear();
	game->player_allocations = nullptr;
}

chaos::ParticleLayer * LudumGameplayLevelInstance::LevelLayerToParticleLayer(chaos::TiledMap::TileLayer const * level_layer) const
{
	std::string const * particle_layer_name = level_layer->FindPropertyString("PARTICLE_LAYER");
	if (particle_layer_name == nullptr || particle_layer_name->length() == 0)
		return nullptr;

	int layer_id = -1;

	if (_strcmpi(particle_layer_name->c_str(), "GROUND") == 0)
		layer_id = LudumGame::GROUND_LAYER_ID;
	else if (_strcmpi(particle_layer_name->c_str(), "WALLS") == 0)
		layer_id = LudumGame::WALLS_LAYER_ID;
	else if (_strcmpi(particle_layer_name->c_str(), "OBJECTS") == 0)
		layer_id = LudumGame::GAMEOBJECT_LAYER_ID;

	if (layer_id > 0)
		return game->GetParticleManager()->FindLayer(layer_id);

	return nullptr;
}

void LudumGameplayLevelInstance::OnLevelStarted()
{
	allocations.clear();

	LudumGameplayLevel const * ludum_level = GetLudumLevel();
	if (ludum_level == nullptr)
		return;

	chaos::TiledMap::Map const * tiled_map = ludum_level->tiled_map.get();
	if (tiled_map == nullptr)
		return;


	chaos::BitmapAtlas::TextureArrayAtlas const * texture_atlas = game->GetTextureAtlas();
	if (texture_atlas == nullptr)
		return;

	chaos::BitmapAtlas::BitmapSet const * bitmap_set = texture_atlas->GetBitmapSet("sprites");
	if (bitmap_set == nullptr)
		return;

	static float scale = 0.05f;

	chaos::box2 wb; // compute the world box

	for (size_t i = 0 ; i < tiled_map->tile_layers.size(); ++i)
	{
		chaos::TiledMap::TileLayer * tile_layer = tiled_map->tile_layers[i].get();
		if (tile_layer == nullptr)
			continue;

		chaos::ParticleLayer * particle_layer = LevelLayerToParticleLayer(tile_layer);
		if (particle_layer == nullptr)
			return;

		size_t tile_count = tile_layer->GetNonEmptyTileCount();
		if (tile_count == 0)
			continue;

		chaos::ParticleAllocation * allocation = particle_layer->SpawnParticles((int)tile_count);
		if (allocation == nullptr)
			continue;

		size_t k = 0;
		chaos::ParticleAccessor<ParticleObject> particles = allocation->GetParticleAccessor<ParticleObject>();

		glm::vec2 tile_size = chaos::GLMTools::RecastVector<glm::vec2>(tiled_map->tile_size);
		for (size_t j = 0 ; j < tile_layer->tile_indices.size() ; ++j)
		{
			int tile_indice = tile_layer->tile_indices[j];
			if (tile_indice <= 0)
				continue;
			
			glm::vec2 position = chaos::GLMTools::RecastVector<glm::vec2>(tile_layer->GetTileCoordinate(j) + tile_layer->offset);

			chaos::TiledMap::TileInfo tile_info = tiled_map->FindTileInfo(tile_indice);
			if (tile_info.tiledata != nullptr)
			{			
				chaos::BitmapAtlas::BitmapEntry const * entry = bitmap_set->GetEntry(tile_info.tiledata->atlas_key.c_str());
				if (entry == nullptr)
					continue;

				ParticleObject & particle = particles[k++];
				
				particle.bounding_box.position = scale * position * tile_size;
				particle.bounding_box.half_size = scale * 0.5f * chaos::GLMTools::RecastVector<glm::vec2>(tile_info.tiledata->image_size);

				particle.texcoords = chaos::ParticleTools::GetParticleTexcoords(*entry, texture_atlas->GetAtlasDimension());
				particle.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

				wb = wb | particle.bounding_box;
			}
		}

		allocation->Resize(k);
		allocations.push_back(allocation);
	}

	world_box = wb;
}

// =============================================================

void LudumNarrativeLevelInstance::OnLevelEnded()
{
}

void LudumNarrativeLevelInstance::OnLevelStarted()
{
}