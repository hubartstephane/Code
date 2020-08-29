#pragma once

#include <death/TM.h>

namespace death
{

	// =====================================
	// TMParticlePopulator : utility class to generate particles for a layer with a cache
	// =====================================

	class TMParticlePopulator
	{
		static size_t const PARTICLE_BUFFER_SIZE = 100;

	public:

		/** default constructor */
		TMParticlePopulator() = default;
		/** copy constructor (do not copy everything) */
		TMParticlePopulator(TMParticlePopulator const& src);

		/** initialize the object */
		bool Initialize(TMLayerInstance* in_layer_instance);
		/** insert a new particle */
		bool AddParticle(char const* bitmap_name, chaos::Hotpoint hotpoint, chaos::box2 particle_box, glm::vec4 const& color, float rotation, int particle_flags, int gid, bool keep_aspect_ratio);
		/** flush remaining particles */
		bool FlushParticles();

		/** get the final bounding box */
		chaos::box2 const& GetBoundingBox() const { return bounding_box; }
		/** get the particle allocation */
		chaos::ParticleAllocationBase* GetParticleAllocation() { return allocation; }

		/** copy operator */
		TMParticlePopulator& operator = (TMParticlePopulator const& src);

	protected:

		/** 'copy' the cached particle into the allocation (with type conversion) */
		bool FlushCachedParticlesToAllocation();

	protected:

		/** the concerned layer instance */
		TMLayerInstance* layer_instance = nullptr;
		/** the level owning this layer */
		death::TMLevel* level = nullptr;

		/** the texture atlas required */
		chaos::BitmapAtlas::TextureArrayAtlas const* texture_atlas = nullptr;
		/** the folder containing the bitmaps */
		chaos::BitmapAtlas::FolderInfo const* folder_info = nullptr;

		/** the allocation for all those particles */
		chaos::ParticleAllocationBase* allocation = nullptr;

		/** a cache of particles */
		TiledMapParticle particles[PARTICLE_BUFFER_SIZE];
		/** the cached number of particles */
		size_t particle_count = 0;
		/** a bounding box */
		chaos::box2 bounding_box;
	};

}; // namespace death
