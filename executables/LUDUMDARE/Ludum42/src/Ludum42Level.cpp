#include "Ludum42Level.h"

#include <chaos/GLMTools.h>

death::GameLevelInstance * LudumGameplayLevel::DoCreateLevelInstance()
{
	return new LudumGameplayLevelInstance(game);
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

LudumGameplayLevel::LudumGameplayLevel(class LudumGame * in_game):
	LudumLevel(in_game)
{
}

LudumGameplayLevelInstance::LudumGameplayLevelInstance(class LudumGame * in_game):
	LudumLevelInstance(in_game)
{
}

// =============================================================

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
	game->UnSpawnPlayer();
}

chaos::ParticleLayer * LudumGameplayLevelInstance::ObjectToParticleLayer(int object_type) const
{
	int layer_id = -1;
	if (object_type == LudumGame::OBJECT_TYPE_PLAYER)
		layer_id = LudumGame::PLAYER_LAYER_ID;
	else if (object_type == LudumGame::OBJECT_TYPE_PLANET)
		layer_id = LudumGame::PLANETS_LAYER_ID;

	if (layer_id > 0)
		return game->GetParticleManager()->FindLayer(layer_id);

	return nullptr;
}

chaos::ParticleAllocation * LudumGameplayLevelInstance::FindOrAllocationForObjectType(int object_type)
{
	// find the allocation
	auto it = allocations.find(object_type);
	if (it != allocations.end())
		return it->second.get();

	// create the allocation
	chaos::ParticleLayer * particle_layer = ObjectToParticleLayer(object_type);
	if (particle_layer == nullptr)
		return nullptr;

	chaos::ParticleAllocation * allocation = particle_layer->SpawnParticles(0);
	if (allocation == nullptr)
		return nullptr;
	allocations[object_type] = allocation;

	return allocation;
}

bool HasExplicitWorldBounds(chaos::TiledMap::GeometricObjectSurface const * object_surface)
{
	// name is an indicator
	if (object_surface->name == "world_bounds")
		return true;
	// property is another indicator
	chaos::TiledMap::Property const * property = object_surface->FindProperty("WORLD_BOUNDS");
	if (property == nullptr)
		return false;

	bool const * property_bool = property->GetBoolProperty();
	if (property_bool != nullptr)
		return *property_bool;

	int const * property_int = property->GetIntProperty();
	if (property_int != nullptr)
		return (*property_int > 0);

	return false;
}

bool GetExplicitWorldBounds(chaos::TiledMap::GeometricObject const * object_geometric, chaos::box2 & result)
{
	if (object_geometric == nullptr)
		return false;

	chaos::TiledMap::GeometricObjectSurface const * object_surface = object_geometric->GetObjectSurface();
	if (object_surface == nullptr)
		return false;
	if (HasExplicitWorldBounds(object_surface))
	{
		result = object_surface->GetBoundingBox();
		return true;
	}
	return false;
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

	// compute the new world
	bool explicit_world_bounds = false;
	chaos::box2 world_bounds; 


	for (size_t i = 0 ; i < tiled_map->object_layers.size(); ++i)
	{
		chaos::TiledMap::ObjectLayer const * object_layer = tiled_map->object_layers[i].get();
		if (object_layer == nullptr)
			continue;

		for (size_t j = 0 ; j < object_layer->geometric_objects.size(); ++j)
		{
			chaos::TiledMap::GeometricObject const * object = object_layer->geometric_objects[j].get();
			if (object == nullptr)
				continue;

			if (!explicit_world_bounds)
				explicit_world_bounds = GetExplicitWorldBounds(object, world_bounds);

			chaos::TiledMap::GeometricObjectTile const * object_tile = object->GetObjectTile();
			if (object_tile == nullptr)
				continue;

			chaos::TiledMap::TileInfo tile_info = tiled_map->FindTileInfo(object_tile->gid);
			if (tile_info.tiledata == nullptr)
				continue;

			chaos::box2 b;
			b.half_size = object_tile->size * 0.5f;

			
			chaos::box2 bbb = chaos::box2(std::make_pair(object_tile->position, object_tile->size));
			
			object_tile->rotation;

			

			object_tile = object_tile;
			object = object;
		}
	}


	//

	for (size_t i = 0 ; i < tiled_map->tile_layers.size(); ++i)
	{
		chaos::TiledMap::TileLayer const * tile_layer = tiled_map->tile_layers[i].get();
		if (tile_layer == nullptr)
			continue;

		glm::vec2 tile_size = chaos::GLMTools::RecastVector<glm::vec2>(tiled_map->tile_size);
		for (size_t j = 0 ; j < tile_layer->tile_indices.size() ; ++j)
		{
			int tile_indice = tile_layer->tile_indices[j];
			if (tile_indice <= 0)
				continue;

			chaos::TiledMap::TileInfo tile_info = tiled_map->FindTileInfo(tile_indice);
			if (tile_info.tiledata != nullptr)
			{	
				chaos::BitmapAtlas::BitmapEntry const * entry = bitmap_set->GetEntry(tile_info.tiledata->atlas_key.c_str());
				if (entry == nullptr)
					continue;

				glm::vec2 position = chaos::GLMTools::RecastVector<glm::vec2>(tile_layer->GetTileCoordinate(j) + tile_layer->offset);
				position.y = -position.y;

				ParticleObject new_particle;
				new_particle.bounding_box.position = position * tile_size;
				new_particle.bounding_box.half_size = 0.5f * chaos::GLMTools::RecastVector<glm::vec2>(tile_info.tiledata->image_size);
				new_particle.texcoords = chaos::ParticleTools::GetParticleTexcoords(*entry, texture_atlas->GetAtlasDimension());
				new_particle.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

				int const * object_type = tile_info.tiledata->FindPropertyInt("OBJECT_TYPE");
				if (object_type == nullptr)
					continue;

				if (*object_type == LudumGame::OBJECT_TYPE_PLAYER)
				{
					ParticlePlayer player_particle;
					(ParticleObject&)player_particle = new_particle;

					int const * atlas_size_x = tile_info.tiledata->FindPropertyInt("ATLAS_SIZE_X");
					if (atlas_size_x != nullptr)
						player_particle.atlas_dimension.x = *atlas_size_x;

					int const * atlas_size_y = tile_info.tiledata->FindPropertyInt("ATLAS_SIZE_Y");
					if (atlas_size_y != nullptr)
						player_particle.atlas_dimension.y = *atlas_size_y;

					float const * atlas_frequency = tile_info.tiledata->FindPropertyFloat("ATLAS_FREQUENCY");
					if (atlas_frequency != nullptr)
						player_particle.frequency = *atlas_frequency;

					int const * skip_last = tile_info.tiledata->FindPropertyInt("ATLAS_SKIP_LAST");
					if (skip_last != nullptr)
						player_particle.skip_last = *skip_last;

					//player_particle.delta_image = abs(rand());

					game->SpawnPlayer(player_particle);
					continue;
				}

				// create an allocation
				chaos::ParticleAllocation * allocation = FindOrAllocationForObjectType(*object_type);
				if (allocation == nullptr)
					continue;

				if (!allocation->AddParticles(1))
					continue;

				chaos::ParticleAccessor<ParticleObject> particles = allocation->GetParticleAccessor<ParticleObject>();
				particles[particles.GetCount() - 1] = new_particle;

				if (!explicit_world_bounds)
					world_bounds = world_bounds | new_particle.bounding_box;
			}
		}
	}

	world_box = world_bounds;
}


