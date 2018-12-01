#include <chaos/TiledMapTools.h>

namespace chaos
{
	bool TiledMapTools::AddIntoAtlasInput(TiledMap::Manager const * manager, BitmapAtlas::AtlasInput & input)
	{
		return AddIntoAtlasInput(manager, input.GetRootFolder());
	}

	bool TiledMapTools::AddIntoAtlasInput(TiledMap::TileSet const * tile_set, BitmapAtlas::AtlasInput & input)
	{
		return AddIntoAtlasInput(tile_set, input.GetRootFolder());
	}

	bool TiledMapTools::AddIntoAtlasInput(TiledMap::Map const * map, BitmapAtlas::AtlasInput & input)
	{
		return AddIntoAtlasInput(map, input.GetRootFolder());
	}

	bool TiledMapTools::AddIntoAtlasInput(TiledMap::Map const * map, BitmapAtlas::FolderInfoInput * folder_input)
	{
		assert(map != nullptr);
		assert(folder_input != nullptr);

		// insert all image layers in the map
		size_t image_layer = map->image_layers.size();
		for (size_t i = 0; i < image_layer; ++i)
		{
			TiledMap::ImageLayer const * image_layer = map->image_layers[i].get();
			if (image_layer != nullptr)
				continue;
			if (image_layer->image_path.size() > 0)
				folder_input->AddBitmap(image_layer->image_path, nullptr, 0);
		}
		return true;
	}

	bool TiledMapTools::AddIntoAtlasInput(TiledMap::TileSet const * tile_set, BitmapAtlas::FolderInfoInput * folder_input)
	{
		assert(tile_set != nullptr);
		assert(folder_input != nullptr);

		// the 'single' image for the whole tile set
		if (tile_set->image_path.size() > 0)
			folder_input->AddBitmap(tile_set->image_path, nullptr, 0);

		// enumerate all TileData
		size_t tile_count = tile_set->tiles.size();
		for (size_t j = 0; j < tile_count; ++j)
		{
			TiledMap::TileData const * tile_data = tile_set->tiles[j].get();
			if (tile_data == nullptr)
				continue;
			if (tile_data->image_path.size() > 0)
				folder_input->AddBitmap(tile_data->image_path, nullptr, 0);
		}
		return true;
	}

	bool TiledMapTools::AddIntoAtlasInput(TiledMap::Manager const * manager, BitmapAtlas::FolderInfoInput * folder_input)
	{
		assert(manager != nullptr);
		assert(folder_input != nullptr);

		// insert all images in any referenced in TiledSet
		size_t tile_set_count = manager->tile_sets.size();
		for (size_t i = 0; i < tile_set_count; ++i)
		{
			TiledMap::TileSet const * tile_set = manager->tile_sets[i].get();
			if (tile_set == nullptr)
				continue;
			if (!AddIntoAtlasInput(tile_set, folder_input))
				return false;
		}

		// images in the maps
		size_t map_count = manager->maps.size();
		for (size_t i = 0; i < map_count; ++i)
		{
			TiledMap::Map const * map = manager->maps[i].get();
			if (map == nullptr)
				continue;
			if (!AddIntoAtlasInput(map, folder_input))
				return false;
		}
		return true;
	}

	bool TiledMapTools::HasFlag(TiledMap::PropertyOwner const * property_owner, char const * property_name)
	{
		assert(property_owner != nullptr);
		// search in properties
		if (property_name != nullptr)
		{
			TiledMap::Property const * property = property_owner->FindProperty(property_name);
			if (property != nullptr)
			{
				bool const * property_bool = property->GetBoolProperty();
				if (property_bool != nullptr)
					return *property_bool;

				int const * property_int = property->GetIntProperty();
				if (property_int != nullptr)
					return (*property_int > 0);
			}
		}
		return false;
	}

	bool TiledMapTools::HasFlag(TiledMap::GeometricObject const * object_geometric, char const * name, char const * type, char const * property_name)
	{
		assert(object_geometric != nullptr);
		// name is an indicator
		if (name != nullptr && object_geometric->name == name)
			return true;
		// type is an indicator
		if (type != nullptr && object_geometric->type == type)
			return true;
		// search in properties
		TiledMap::PropertyOwner const * property_owner = object_geometric;
		if (HasFlag(property_owner, property_name))
			return true;
		return false;
	}

	bool TiledMapTools::IsWorldBoundingBox(TiledMap::GeometricObject const * object_geometric)
	{
		return HasFlag(object_geometric, "world_bounding_box", "world_bounding_box", "WORLD_BOUNDING_BOX");
	}

	bool TiledMapTools::IsLayerBoundingBox(TiledMap::GeometricObject const * object_geometric)
	{
		return HasFlag(object_geometric, "layer_bounding_box", "layer_bounding_box", "LAYER_BOUNDING_BOX");
	}

	bool TiledMapTools::IsTypedObject(TiledMap::GeometricObject const * object_geometric)
	{
		// the type of the object may be given by the field 'type'
		if (object_geometric != nullptr && object_geometric->type.length() > 0)
			return true;
		// or by a property 'OBJECT_TYPE'
		return HasFlag(object_geometric, nullptr, nullptr, "OBJECT_TYPE");
	}

	bool TiledMapTools::IsTriggerSurface(TiledMap::GeometricObject const * object_geometric)
	{
		return HasFlag(object_geometric, "trigger_surface", "trigger_surface", "TRIGGER_SURFACE");
	}

	bool TiledMapTools::IsPlayerStart(TiledMap::GeometricObject const * object_geometric)
	{
		return HasFlag(object_geometric, "player_start", "player_start", "PLAYER_START");
	}

	bool TiledMapTools::IsCamera(TiledMap::GeometricObject const * object_geometric)
	{
		return HasFlag(object_geometric, "camera", "camera", "CAMERA");
	}

	bool TiledMapTools::GetExplicitWorldBoundingBox(TiledMap::GeometricObject const * object_geometric, box2 & result, bool world_system)
	{
		if (object_geometric == nullptr)
			return false;
		TiledMap::GeometricObjectSurface const * object_surface = object_geometric->GetObjectSurface();
		if (object_surface == nullptr)
			return false;
		if (TiledMapTools::IsWorldBoundingBox(object_surface))
		{
			result = object_surface->GetBoundingBox(world_system);
			return true;
		}
		return false;
	}

	bool TiledMapTools::GetExplicitLayerBoundingBox(TiledMap::GeometricObject const * object_geometric, box2 & result, bool world_system) 
	{
		if (object_geometric == nullptr)
			return false;
		TiledMap::GeometricObjectSurface const * object_surface = object_geometric->GetObjectSurface();
		if (object_surface == nullptr)
			return false;
		if (TiledMapTools::IsLayerBoundingBox(object_surface))
		{
			result = object_surface->GetBoundingBox(world_system);
			return true;
		}
		return false;
	}

}; // namespace chaos
