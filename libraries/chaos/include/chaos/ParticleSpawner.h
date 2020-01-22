#pragma once

#include <chaos/StandardHeaders.h>

#include <chaos/ReferencedObject.h>
#include <chaos/BitmapAtlas.h>
#include <chaos/ParticleLayer.h>
#include <chaos/ParticleAccessor.h>

namespace chaos
{

    class ParticleSpawner : public ReferencedObject
    {
    public:

        /** simplest constructor */
        ParticleSpawner(ParticleLayerBase* in_particle_layer) :
            particle_layer(in_particle_layer)
        {
            assert(in_particle_layer != nullptr);
        }

        /** constructor with additionnal bitmap arguments */
        template<typename ...PARAMS>
        ParticleSpawner(ParticleLayerBase* in_particle_layer, PARAMS... params) :
            particle_layer(in_particle_layer)
        {
            SetBitmapInfo(params...);
        }

        /** change the bitmap info */
        template<typename BITMAP_NAME, typename FOLDER_NAME>
        void SetBitmapInfo(BITMAP_NAME bitmap_name, FOLDER_NAME folder_name)
        {
            // get the atlas
            BitmapAtlas::TextureArrayAtlas const* atlas = particle_layer->GetTextureAtlas();
            if (atlas == nullptr)
                return;
            // find the folder
            BitmapAtlas::FolderInfo const* bitmap_set = atlas->GetFolderInfo(folder_name, true);
            if (bitmap_set != nullptr)
                return;
            // get the bitmap
            bitmap_info = bitmap_set->GetBitmapInfo(bitmap_name, true);
        }

        /** change the bitmap info with just a bitmap name */
        template<typename BITMAP_NAME>
        void SetBitmapInfo(BITMAP_NAME bitmap_name)
        {
            SetBitmapInfo(bitmap_name, "sprites"); // folder "sprites" by default
        }

        /** simple spawn method */
        ParticleAllocationBase* SpawnParticles(size_t count, bool new_allocation);

        /** spawn + user initialization methods */
        template<typename INIT_PARTICLE_FUNC>
        ParticleAllocationBase* SpawnParticles(size_t count, bool new_allocation, INIT_PARTICLE_FUNC init_func)
        {
            ParticleAllocationBase* result = SpawnParticles(count, new_allocation);
            // call user initialization function
            if (result != nullptr)
            {
                size_t allocation_count = result->GetParticleCount();
                init_func(result->GetParticleAccessor(allocation_count - count, count));  // partial accessor, take the last particles in the array
            }
            return result;
        }

    protected:

        /** the spawn internal method (that just creates particles) */
        ParticleAllocationBase* DoSpawnParticles(size_t count, bool new_allocation);

    protected:

        /** the layer instance where to spawn the particles */
        ParticleLayerBase* particle_layer = nullptr;
        /** the bitmap info to use for the spawned particles */
        BitmapAtlas::BitmapInfo const* bitmap_info = nullptr;
    };


}; // namespace chaos

