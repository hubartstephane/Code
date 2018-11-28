#pragma once

#include <death/GameLevel.h>
#include <death/TiledMapParticle.h>

#include <chaos/StandardHeaders.h>
#include <chaos/GeometryFramework.h>
#include <chaos/ReferencedObject.h>
#include <chaos/TiledMap.h>
#include <chaos/Renderable.h>
#include <chaos/Tickable.h>
#include <chaos/ParticleManager.h>
#include <chaos/ParticleDefault.h>
#include <chaos/BitmapAtlas.h>

namespace death
{

	namespace TiledMap
	{
		// ==============================================================
		// FORWARD DECLARATION / FRIENDSHIP MACROS
		// ==============================================================

		// all classes in this file
#define DEATH_TILEDLEVEL_CLASSES (Level) (LevelInstance) (LayerInstance) (GeometricObject) (CameraObject) (PlayerStartObject) (TriggerSurfaceObject) (BaseObject) (LayerInstanceParticlePopulator)

		// forward declaration
#define DEATH_TILEDLEVEL_FORWARD_DECL(r, data, elem) class elem;
		BOOST_PP_SEQ_FOR_EACH(DEATH_TILEDLEVEL_FORWARD_DECL, _, DEATH_TILEDLEVEL_CLASSES)

			// friendship macro
#define DEATH_TILEDLEVEL_FRIEND_DECL(r, data, elem) friend class elem;
#define DEATH_TILEDLEVEL_ALL_FRIENDS BOOST_PP_SEQ_FOR_EACH(DEATH_TILEDLEVEL_FRIEND_DECL, _, DEATH_TILEDLEVEL_CLASSES)

		// =====================================
		// BaseObject : a base object for special game entities
		// =====================================

		class BaseObject : public chaos::ReferencedObject, public chaos::NamedObject
		{
			DEATH_TILEDLEVEL_ALL_FRIENDS

		public:

			/** constructor */
			BaseObject(LayerInstance * in_layer_instance);

			/** get the layer instance owning this object */
			LayerInstance * GetLayerInstance() { return layer_instance; }
			/** get the layer instance owning this object */
			LayerInstance const * GetLayerInstance() const { return layer_instance; }

		protected:

			/** a reference to the layer instance */
			LayerInstance * layer_instance = nullptr;
		};

		// =====================================
		// GeometricObject 
		// =====================================

		class GeometricObject : public BaseObject
		{
			DEATH_TILEDLEVEL_ALL_FRIENDS

		public:

			/** constructor */
			GeometricObject(LayerInstance * in_layer_instance);

			/** get the geometric object corresponding to this */
			chaos::TiledMap::GeometricObject * GetGeometricObject() { return geometric_object.get(); }
			/** get the geometric object corresponding to this */
			chaos::TiledMap::GeometricObject const * GetGeometricObject() const { return geometric_object.get(); }

		protected:

			/** initialization */
			virtual bool Initialize(chaos::TiledMap::GeometricObject * in_geometric_object);

		protected:

			/** the associated geometric object */
			boost::intrusive_ptr<chaos::TiledMap::GeometricObject> geometric_object;
		};

		// =====================================
		// CameraObject : where the player may start
		// =====================================

		class CameraObject : public GeometricObject
		{
			DEATH_TILEDLEVEL_ALL_FRIENDS

		public:

			/** constructor */
			CameraObject(LayerInstance * in_layer_instance);

		protected:

			/** override */
			virtual bool Initialize(chaos::TiledMap::GeometricObject * in_geometric_object) override;
		};

		// =====================================
		// PlayerStartObject : where the player may start
		// =====================================

		class PlayerStartObject : public GeometricObject
		{
			DEATH_TILEDLEVEL_ALL_FRIENDS

		public:

			/** constructor */
			PlayerStartObject(LayerInstance * in_layer_instance);

		protected:

			/** override */
			virtual bool Initialize(chaos::TiledMap::GeometricObject * in_geometric_object) override;
		};

		// =====================================
		// TriggerSurfaceObject : an object player can collide with (for moment, rectangle)
		// =====================================

		class TriggerSurfaceObject : public GeometricObject
		{
			DEATH_TILEDLEVEL_ALL_FRIENDS

		public:

			/** constructor */
			TriggerSurfaceObject(LayerInstance * in_layer_instance);

			/** whether it is enabled or not */
			bool IsEnabled() const { return enabled; }
			/** change whether the trigger is enabled or not */
			void SetEnabled(bool in_enabled = true) { enabled = in_enabled;}
		
			/** get the trigger ID */
			int GetTriggerID() const { return trigger_id; }

			/** get the object bounding box */
			chaos::box2 GetBoundingBox() const;

		protected:

			/** override */
			virtual bool Initialize(chaos::TiledMap::GeometricObject * in_geometric_object) override;

			/** called whenever a collision with player is detected (returns false, if loop is to be broken) */
			virtual bool OnPlayerCollision(double delta_time, chaos::ParticleDefault::Particle * player_particle);
			
		protected:

			/** flag whether to object is enabled or not */
			bool enabled = true;
			/** an ID that helps make classification */
			int trigger_id = 0;
		};

		// =====================================
		// Level : a level described by a tiledmap
		// =====================================

		class Level : public GameLevel
		{
			DEATH_TILEDLEVEL_ALL_FRIENDS

		public:

			/** constructor */
			Level();

			/** initialization from tiled_map */
			virtual bool Initialize(chaos::TiledMap::Map * in_tiled_map);

			/** get the tiled map */
			chaos::TiledMap::Map * GetTiledMap() { return tiled_map.get(); }
			/** get the tiled map */
			chaos::TiledMap::Map const * GetTiledMap() const { return tiled_map.get(); }

		protected:

			/** create a level instance for that level user specified function */
			virtual GameLevelInstance * DoCreateLevelInstance(Game * in_game) override;

			/** create a TriggerSurface specializable method */
			virtual TriggerSurfaceObject * DoCreateTriggerSurface(LayerInstance * in_layer_instance);
			/** create a Camera specializable method */
			virtual CameraObject * DoCreateCamera(LayerInstance * in_layer_instance);
			/** create a PlayerStartObject specializable method */
			virtual PlayerStartObject * DoCreatePlayerStart(LayerInstance * in_layer_instance);
			/** create a PlayerStartObject specializable method */
			virtual LayerInstance * DoCreateLayerInstance(LevelInstance * in_level_instance);

			/** create a TriggerSurface 'entry point' */
			TriggerSurfaceObject * CreateTriggerSurface(LayerInstance * in_layer_instance, chaos::TiledMap::GeometricObject * in_geometric_object);
			/** create a camera 'entry point' */
			CameraObject * CreateCamera(LayerInstance * in_layer_instance, chaos::TiledMap::GeometricObject * in_geometric_object);
			/** create a player start 'entry point' */
			PlayerStartObject * CreatePlayerStart(LayerInstance * in_layer_instance, chaos::TiledMap::GeometricObject * in_geometric_object);
			/** create a layer instance 'entry point' */
			LayerInstance * CreateLayerInstance(LevelInstance * in_level_instance, chaos::TiledMap::LayerBase * in_layer);

			/** get the folder in which bitmaps are stored in Game::Atlas */
			virtual chaos::BitmapAtlas::FolderInfo const * GetFolderInfo(LayerInstance * layer_instance) const;
			/** get the atlas to use for the rendering */
			virtual chaos::BitmapAtlas::TextureArrayAtlas const * GetTextureAtlas(LayerInstance * layer_instance) const;
			/** create a particle layer */
			virtual chaos::ParticleLayer * CreateParticleLayer(LayerInstance * layer_instance);

			/** called whenever a collision between player and tile happens */
			virtual bool OnPlayerTileCollision(double delta_time, chaos::ParticleDefault::Particle * player_particle, TileParticle * particle);

			/** the default program when not specified */
			virtual chaos::GPUProgram * GenDefaultRenderProgram();
			/** the default material when not specified */
			virtual chaos::GPURenderMaterial * GetDefaultRenderMaterial();

		protected:

			/** the tiled map corresponding to this level */
			boost::intrusive_ptr<chaos::TiledMap::Map> tiled_map;
		};

		// =====================================
		// LayerInstance : instance of a Layer
		// =====================================

		class LayerInstance : public chaos::Renderable
		{
			DEATH_TILEDLEVEL_ALL_FRIENDS

		protected:

			/** protected constructor */
			LayerInstance(LevelInstance * in_level_instance);

		public:

			/** get the tiled layer */
			chaos::TiledMap::LayerBase * GetTiledLayer() { return layer.get(); }
			/** get the tiled layer */
			chaos::TiledMap::LayerBase const * GetTiledLayer() const { return layer.get(); }

			/** get the level (for this layer) */
			Level * GetTypedLevel();
			/** get the level (for this layer) */
			Level const * GetTypedLevel() const;

			/** get the game */
			Game * GetGame();
			/** get the game */
			Game const * GetGame() const;

			/** find the player start from its name */
			PlayerStartObject * FindPlayerStart(char const * name);
			/** find the player start from its name */
			PlayerStartObject const * FindPlayerStart(char const * name) const;

			/** find the camera from its name */
			CameraObject * FindCamera(char const * name);
			/** find the camera from its name */
			CameraObject const * FindCamera(char const * name) const;

			/** find the trigger surface from its name */
			TriggerSurfaceObject * FindTriggerSurface(char const * name);
			/** find the trigger surface from its name */
			TriggerSurfaceObject const * FindTriggerSurface(char const * name) const;

			/** get the bounding box for the level */
			chaos::box2 const & GetBoundingBox() const { return bounding_box; }

			/** create a particle allocation for the layer */
			chaos::ParticleAllocation * CreateParticleAllocation();

			/** get whether player collisions are enabled on that layer */
			bool ArePlayerCollisionEnabled() const { return player_collision_enabled; }
			/** change whether collisions with player are to be test on that layer */
			void SetPlayerCollisionEnabled(bool in_player_collision_enabled){ player_collision_enabled = in_player_collision_enabled; }

			/** get whether trigger surfaces are enabled on that layer */
			bool AreTriggerSurfacesEnabled() const { return trigger_surfaces_enabled; }
			/** change whether trigger surfaces are enabled on that layer */
			void SetTriggerSurfacesEnabled(bool in_trigger_surfaces_enabled) { trigger_surfaces_enabled = in_trigger_surfaces_enabled; }

			/** get whether collisions with tiles are enabled on that layer */
			bool AreTileCollisionsEnabled() const { return tile_collisions_enabled; }
			/** change whether collisions with tiles are enabled on that layer */
			void SetTileCollisionsEnabled(bool in_tile_collisions_enabled) { tile_collisions_enabled = in_tile_collisions_enabled; }

		protected:

			/** initialization */
			virtual bool Initialize(chaos::TiledMap::LayerBase * in_layer);
			/** find render material according to its name */
			chaos::GPURenderMaterial * FindRenderMaterial(char const * material_name);

			/** override */
			virtual bool DoTick(double delta_time) override;
			/** override */
			virtual int DoDisplay(chaos::GPUProgramProviderBase const * uniform_provider, chaos::RenderParams const & render_params) const override;

			/** search all collision with the player (tiles/TriggerSurfaceObject) */
			virtual void ComputePlayerCollision(double delta_time);
			/** compute trigger collisions with surface triggers */
			virtual void ComputePlayerCollisionWithSurfaceTriggers(double delta_time, chaos::ParticleDefault::Particle * player_particle);
			/** compute collisions between players and tiles */
			virtual void ComputePlayerTileCollisions(double delta_time, chaos::ParticleDefault::Particle * player_particle);

			/** specialized layer */
			bool InitializeLayer(chaos::TiledMap::ImageLayer * image_layer);
			/** specialized layer */
			bool InitializeLayer(chaos::TiledMap::ObjectLayer * object_layer);
			/** specialized layer */
			bool InitializeLayer(chaos::TiledMap::TileLayer * tile_layer);

		protected:

			/** displacement ratio relatively to the main layer */
			float displacement_ratio = 1.0f;
			/** whether the layer is to be repeated infinitely in X direction */
			bool wrap_x = false;
			/** whether the layer is to be repeated infinitely in Y direction */
			bool wrap_y = false;
			/** material name */
			std::string material_name;

			/** the level instance this object belongs to */
			LevelInstance * level_instance = nullptr;

			/** the tiled layer corresponding to this object */
			boost::intrusive_ptr<chaos::TiledMap::LayerBase> layer;
			/** the particle layer */
			boost::intrusive_ptr<chaos::ParticleLayer> particle_layer;

			/** the player starts */
			std::vector<boost::intrusive_ptr<PlayerStartObject>> player_starts;
			/** the player cameras */
			std::vector<boost::intrusive_ptr<CameraObject>> cameras;
			/** the trigger surface */
			std::vector<boost::intrusive_ptr<TriggerSurfaceObject>> trigger_surfaces;

			/** the bounding box of the layer */
			chaos::box2 bounding_box;

			/** whether collision with player are to be tested with that layer */
			bool player_collision_enabled = true;
			/** whether trigger surfaces are enabled on that layer */
			bool trigger_surfaces_enabled = true;
			/** whether collisions with tiles are enabled on that layer */
			bool tile_collisions_enabled = true;
		};

		// =====================================
		// LevelInstance : instance of a Level
		// =====================================

		class LevelInstance : public GameLevelInstance
		{
			DEATH_TILEDLEVEL_ALL_FRIENDS

		public:

			/** level getter with a cast */
			Level * GetTypedLevel();
			/** level getter with a cast */
			Level const * GetTypedLevel() const;

			/** get the tiled map */
			chaos::TiledMap::Map * GetTiledMap();
			/** get the tiled map */
			chaos::TiledMap::Map const * GetTiledMap() const;

			/** get the game */
			Game * GetGame() { return game; }
			/** get the game */
			Game const * GetGame() const { return game; }

			/** find the camera from its name */
			CameraObject * FindCamera(char const * name);
			/** find the camera from its name */
			CameraObject const * FindCamera(char const * name) const;
			/** find the player start from its name */
			PlayerStartObject * FindPlayerStart(char const * name);
			/** find the player start from its name */
			PlayerStartObject const * FindPlayerStart(char const * name) const;
			/** find the trigger surface from its name */
			TriggerSurfaceObject * FindTriggerSurface(char const * name);
			/** find the trigger surface from its name */
			TriggerSurfaceObject const * FindTriggerSurface(char const * name) const;

			/** get the bounding box for the level */
			chaos::box2 const & GetBoundingBox() const { return bounding_box; }

		protected:

			/** override */
			virtual bool Initialize(Game * in_game) override;
			/** override */
			virtual bool DoTick(double delta_time) override;
			/** override */
			virtual int DoDisplay(chaos::GPUProgramProviderBase const * uniform_provider, chaos::RenderParams const & render_params) const override;

			/** search all collision with the player (tiles/TriggerSurfaceObject) */
			virtual void ComputePlayerCollision(double delta_time);

			/** override */
			virtual void OnLevelStarted() override;
			/** override */
			virtual void OnLevelEnded() override;

			/** create the camera */
			virtual void CreateCamera();
			/** destroy the player allocation */
			virtual void UnSpawnPlayer();
			/** create the player allocation */
			virtual void SpawnPlayer();

			/** create the particle manager */
			virtual bool CreateParticleManager(Game * in_game);
			/** create the layers instances */
			virtual bool CreateLayerInstances(Game * in_game);
			/** compute the bounding box of the level */
			virtual void ComputeBoundingBox();

			/** the default material when not specified */
			virtual chaos::GPURenderMaterial * GetDefaultRenderMaterial();

		protected:

			/** a reference uppon the game */
			Game * game = nullptr;

			/** the particle manager used to render the world */
			boost::intrusive_ptr<chaos::ParticleManager> particle_manager;
			/** the layers */
			std::vector<boost::intrusive_ptr<LayerInstance>> layer_instances;
			/** the default render material */
			boost::intrusive_ptr<chaos::GPURenderMaterial> default_material;
		};

	}; // namespace TiledMap

		 // undefine macros
#undef DEATH_TILEDMAP_CLASSES
#undef DEATH_TILEDMAP_FORWARD_DECL
#undef DEATH_TILEDMAP_FRIEND_DECL
#undef DEATH_TILEDMAP_ALL_FRIENDS

}; // namespace death
