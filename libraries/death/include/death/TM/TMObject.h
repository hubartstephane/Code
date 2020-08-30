#pragma once

#include <death/TM.h>

namespace death
{
	// ==========================================
	// TMObjectReferenceRegistry : store wanted reference to further resolving
	// ==========================================

	class TMObjectReferenceRegistry
	{

	};




	class TMObjectReference
	{
	public:

		TMObject* operator -> ()
		{
			if (id > 0)
			{


				id = -1; // do not search anymore for the object
			}
			return object.get();
		}

	protected:

		int id = 0;

		mutable chaos::weak_ptr<TMObject> object;
	};




	// =====================================
	// TMObject 
	// =====================================

	class TMObject : public chaos::Tickable, public chaos::JSONSerializable
	{
		DEATH_TILEDLEVEL_ALL_FRIENDS;

		CHAOS_OBJECT_DECLARE_CLASS2(TMObject, chaos::Tickable);

	public:

		/** get the layer instance owning this object */
		TMLayerInstance* GetLayerInstance() { return layer_instance; }
		/** get the layer instance owning this object */
		TMLayerInstance const* GetLayerInstance() const { return layer_instance; }

		/** whenever the level is being started */
		virtual void OnLevelStarted() {}
		/** whenever the Level is being stopped */
		virtual void OnLevelEnded() {}
		/** whenever the Level is being restarted */
		virtual void OnLevelRestart() {}

		/** get the layer ID (used for Checkpoints) */
		int GetObjectID() const { return id; }

		/** get the object bounding box */
		chaos::box2 GetBoundingBox(bool world_system) const;

		/** override */
		virtual bool SerializeFromJSON(nlohmann::json const& json) override;
		/** override */
		virtual bool SerializeIntoJSON(nlohmann::json& json) const override;

	protected:

		/** additionnal initialization */
		virtual bool Initialize(TMLayerInstance* in_layer_instance, chaos::TiledMap::GeometricObject const* in_geometric_object, TMObjectReferenceRegistry & reference_registry);
		/** enable the creation of additionnal particles */
		virtual bool IsParticleCreationEnabled() const;

	protected:

		/** id of the object (comming from chaos::TiledMap) */
		int id = 0;
		/** whether the particles created for this object should be under its ownership (instead of the layers) */
		bool particle_ownership = false;
		/** the bounding box of the object */
		chaos::box2 bounding_box;

		/** the allocations for this object */
		chaos::shared_ptr<chaos::ParticleAllocationBase> allocation;

		/** a reference to the layer instance */
		TMLayerInstance* layer_instance = nullptr;
	};

	// =====================================
	// TMCameraTemplate : where the player may start
	// =====================================

	class TMCameraTemplate : public TMObject
	{
		DEATH_TILEDLEVEL_ALL_FRIENDS;

		CHAOS_OBJECT_DECLARE_CLASS2(TMCameraTemplate, TMObject);

	protected:

		/** override */
		virtual bool Initialize(TMLayerInstance* in_layer_instance, chaos::TiledMap::GeometricObject const* in_geometric_object, TMObjectReferenceRegistry& reference_registry) override;
	};

	// =====================================
	// TMPlayerStart : where the player may start
	// =====================================

	class TMPlayerStart : public TMObject
	{
		DEATH_TILEDLEVEL_ALL_FRIENDS;

		CHAOS_OBJECT_DECLARE_CLASS2(TMPlayerStart, TMObject);

	protected:

		/** override */
		virtual bool Initialize(TMLayerInstance* in_layer_instance, chaos::TiledMap::GeometricObject const* in_geometric_object, TMObjectReferenceRegistry& reference_registry) override;
		/** override */
		virtual bool SerializeFromJSON(nlohmann::json const& json) override;
		/** override */
		virtual bool SerializeIntoJSON(nlohmann::json& json) const override;

	protected:

		/** the bitmap to use for this player */
		std::string bitmap_name;
	};

	// =====================================
	// TMTrigger : an object player can collide with (for moment, rectangle)
	// =====================================

	class TMTrigger : public TMObject
	{
		DEATH_TILEDLEVEL_ALL_FRIENDS;

		CHAOS_OBJECT_DECLARE_CLASS2(TMTrigger, TMObject);

	public:

		/** whether it is enabled or not */
		bool IsEnabled() const { return enabled; }
		/** change whether the trigger is enabled or not */
		void SetEnabled(bool in_enabled = true);

		/** whether it should be triggered a single time */
		bool IsTriggerOnce() const { return trigger_once; }
		/** change whether the trigger once is enabled or not */
		void SetTriggerOnce(bool in_trigger_once = true);

		/** search whether there is a collision given box */
		virtual bool IsCollisionWith(chaos::box2 const& other_box, chaos::CollisionType collision_type) const;

		/** override */
		virtual bool SerializeFromJSON(nlohmann::json const& json) override;
		/** override */
		virtual bool SerializeIntoJSON(nlohmann::json& json) const override;

	protected:

		/** override */
		virtual bool Initialize(TMLayerInstance* in_layer_instance, chaos::TiledMap::GeometricObject const* in_geometric_object, TMObjectReferenceRegistry& reference_registry) override;
		/** called whenever a collision with object is detected (returns true, if collision is handled successfully (=> important for TriggerOnce) */
		virtual bool OnCollisionEvent(float delta_time, chaos::Object* object, chaos::CollisionType event_type);

	protected:

		/** flag whether to object is enabled or not */
		bool enabled = true;
		/** flag whether to can only trigger once */
		bool trigger_once = false;
		/** outside box factor (a factor applyed to bounding box to detect whether the player is going outside of the range) */
		float outside_box_factor = 1.0f;

		/** whenever the trigger-enter event has happened */
		bool enter_event_triggered = false;
	};

	// =================================================
	// TMNotificationTrigger
	// =================================================

	class TMNotificationTrigger : public TMTrigger
	{
		DEATH_TILEDLEVEL_ALL_FRIENDS;

		CHAOS_OBJECT_DECLARE_CLASS2(TMNotificationTrigger, TMTrigger);

	public:

		/** override */
		virtual bool IsParticleCreationEnabled() const override;
		/** override */
		virtual bool SerializeFromJSON(nlohmann::json const& json) override;
		/** override */
		virtual bool SerializeIntoJSON(nlohmann::json& json) const override;

	protected:

		/** override */
		virtual bool Initialize(TMLayerInstance* in_layer_instance, chaos::TiledMap::GeometricObject const* in_geometric_object, TMObjectReferenceRegistry& reference_registry) override;
		/** override */
		virtual bool OnCollisionEvent(float delta_time, chaos::Object* object, chaos::CollisionType event_type) override;

	protected:

		/** the notification to display */
		std::string notification_string;
		/** the notification lifetime */
		float notification_lifetime = 5.0f;
		/** destroy whenever collision is finished */
		bool stop_when_collision_over = false;
		/** collision with player instead of camera */
		bool player_collision = true;
	};

	// =================================================
	// TMCheckpointTrigger
	// =================================================

	class TMCheckpointTrigger : public TMTrigger
	{
		DEATH_TILEDLEVEL_ALL_FRIENDS;

		CHAOS_OBJECT_DECLARE_CLASS2(TMCheckpointTrigger, TMTrigger);

	public:

		/** override */
		virtual bool IsParticleCreationEnabled() const override;

	protected:

		/** override */
		virtual bool Initialize(TMLayerInstance* in_layer_instance, chaos::TiledMap::GeometricObject const* in_geometric_object, TMObjectReferenceRegistry& reference_registry) override;
		/** override */
		virtual bool OnCollisionEvent(float delta_time, chaos::Object* object, chaos::CollisionType event_type) override;
	};

	// =================================================
	// TMSoundTrigger : an object that play a sound
	// =================================================

	class TMSoundTrigger : public TMTrigger
	{
		DEATH_TILEDLEVEL_ALL_FRIENDS;

		CHAOS_OBJECT_DECLARE_CLASS2(TMSoundTrigger, TMTrigger);

	public:

		/** override */
		virtual bool IsParticleCreationEnabled() const override;
		/** override */
		virtual bool SerializeFromJSON(nlohmann::json const& json) override;
		/** override */
		virtual bool SerializeIntoJSON(nlohmann::json& json) const override;

	protected:

		/** override */
		virtual bool Initialize(TMLayerInstance* in_layer_instance, chaos::TiledMap::GeometricObject const* in_geometric_object, TMObjectReferenceRegistry& reference_registry) override;
		/** override */
		virtual bool OnCollisionEvent(float delta_time, chaos::Object* object, chaos::CollisionType event_type) override;

		/** the sound creation method */
		chaos::Sound* CreateSound() const;

	protected:

		/** the name of the sound to play */
		std::string sound_name;

		/** a ratio applyed to max_distance to compute the min_distance of the sound (for 3D sounds) */
		float min_distance_ratio = 0.3f;
		/** 3D sound */
		bool is_3D_sound = false;
		/** whether the sound is to be looping or play only once */
		bool looping = true;
		/** timer for far 3D sound before entering pause */
		float pause_timer_when_too_far = 0.0f;
		/** 3D sound */
		bool stop_when_collision_over = false;

		/** the sound being played */
		chaos::shared_ptr<chaos::Sound> sound;
	};


	// =================================================
	// TMChangeLevelTrigger
	// =================================================

	class TMChangeLevelTrigger : public TMTrigger
	{
		DEATH_TILEDLEVEL_ALL_FRIENDS;

		CHAOS_OBJECT_DECLARE_CLASS2(TMChangeLevelTrigger, TMTrigger);

	public:

		/** override */
		virtual bool SerializeFromJSON(nlohmann::json const& json) override;
		/** override */
		virtual bool SerializeIntoJSON(nlohmann::json& json) const override;

	protected:

		/** override */
		virtual bool Initialize(TMLayerInstance* in_layer_instance, chaos::TiledMap::GeometricObject const* in_geometric_object, TMObjectReferenceRegistry& reference_registry) override;
		/** override */
		virtual bool IsParticleCreationEnabled() const override;
		/** override */
		virtual bool OnCollisionEvent(float delta_time, chaos::Object* object, chaos::CollisionType event_type) override;

	protected:

		/** the destination level */
		std::string level_name;
		/** the destination player start */
		std::string player_start_name;
	};

}; // namespace death
