#pragma once

#include <chaos/StandardHeaders.h>
#include <chaos/BitmapAtlas.h>
#include <chaos/GeometryFramework.h>
#include <chaos/Class.h>
#include <chaos/PrimitiveOutput.h>
#include <chaos/ParticleDefault.h>
#include <chaos/Hotpoint.h>

namespace chaos
{


	/**
	* ParticleTools : an utility class that deserve to bring some services for particles system
	*/

	namespace ParticleTools
	{
        /** get particle by index */
        ParticleDefault* GetParticle(ParticleAllocationBase* allocation, size_t index);
        ParticleDefault const* GetParticle(ParticleAllocationBase const* allocation, size_t index);
        /** get particle position */
        glm::vec2 GetParticlePosition(ParticleAllocationBase const* allocation, size_t index);
        /** get particle box */
        box2 GetParticleBox(ParticleAllocationBase const* allocation, size_t index);
        /** set the particle position */
        bool SetParticlePosition(ParticleAllocationBase* allocation, size_t index, glm::vec2 const& position);
        /** set the particle box */
        bool SetParticleBox(ParticleAllocationBase* allocation, size_t index, box2 const& box);

        /** fill the vertices array with 4 vertices corresponding to 1 QUAD */
        void GenerateBoxParticle(QuadPrimitive<VertexDefault>& primitive, ParticleCorners const& corners, ParticleTexcoords const& texcoords, float rotation = 0.0f, int flags = 0);

        void GenerateBoxParticle(QuadPrimitive<VertexDefault>& primitive, box2 const& box, ParticleTexcoords const& texcoords, float rotation = 0.0f, int flags = 0);

        /** fill the vertices array with 6 vertices corresponding to 1 TRIANGLE PAIR */
        void GenerateBoxParticle(TrianglePairPrimitive<VertexDefault>& primitive, ParticleCorners const& corners, ParticleTexcoords const& texcoords, float rotation = 0.0f, int flags = 0);

        void GenerateBoxParticle(TrianglePairPrimitive<VertexDefault>& primitive, box2 const& box, ParticleTexcoords const& texcoords, float rotation = 0.0f, int flags = 0);

		/** an utility method to generate the 4 vertex attributes for a Particle */
		void GenerateVertexTextureAttributes(glm::vec3* vertex_texcoords, ParticleTexcoords const& texcoords, int flags); // in order BL, BR, TR, TL
		/** an utility method to generate the 4 vertex attributes for a Particle */
		void GenerateVertexPositionAttributes(glm::vec2* vertex_positions, ParticleCorners const& corners, float rotation, int flags); // in order BL, BR, TR, TL
		/** an utility method to generate the 4 vertex attributes for a Particle */
		void GenerateVertexFlagAttributes(int* vertex_flags, int flags);  // in order BL, BR, TR, TL

	}; // namespace ParticleTools

}; // namespace chaos

