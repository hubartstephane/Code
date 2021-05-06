#ifdef CHAOS_FORWARD_DECLARATION

namespace chaos
{
	class ParticleBackground;
	class ParticleBackgroundLayerTrait;

}; // namespace chaos

#elif defined CHAOS_TEMPLATE_IMPLEMENTATION


namespace chaos
{
    template<typename VERTEX_TYPE>
    void ParticleToPrimitives(ParticleBackground const& particle, PrimitiveOutput<VERTEX_TYPE>& output)
    {
        QuadPrimitive<VERTEX_TYPE> quad = output.AddQuads();
        ParticleToPrimitive(particle, quad);
    }

    template<typename VERTEX_TYPE>
    void ParticleToPrimitive(ParticleBackground const& particle, QuadPrimitive<VERTEX_TYPE>& primitive)
    {
        primitive[0].position = glm::vec2(-1.0f, -1.0f);
        primitive[1].position = glm::vec2(+1.0f, -1.0f);
        primitive[2].position = glm::vec2(+1.0f, +1.0f);
        primitive[3].position = glm::vec2(-1.0f, +1.0f);

        for (VERTEX_TYPE& vertex : primitive)
        {
            glm::vec2 texcoord = vertex.position * 0.5f + glm::vec2(0.5f, 0.5f);

            vertex.texcoord.x = texcoord.x;
            vertex.texcoord.y = texcoord.y;
            vertex.texcoord.z = 0.0f;
            vertex.color = particle.color;
        }
    }

    template<typename VERTEX_TYPE>
    void ParticleToPrimitive(ParticleBackground const& particle, TrianglePairPrimitive<VERTEX_TYPE>& primitive)
    {
        primitive[0].position = glm::vec2(-1.0f, -1.0f);
        primitive[1].position = glm::vec2(+1.0f, -1.0f);
        primitive[2].position = glm::vec2(-1.0f, +1.0f);

        primitive[3].position = glm::vec2(-1.0f, +1.0f);
        primitive[4].position = glm::vec2(+1.0f, -1.0f);
        primitive[5].position = glm::vec2(+1.0f, +1.0f);

        for (VERTEX_TYPE& vertex : primitive)
        {
            glm::vec2 texcoord = vertex.position * 0.5f + glm::vec2(0.5f, 0.5f);

            vertex.texcoord.x = texcoord.x;
            vertex.texcoord.y = texcoord.y;
            vertex.texcoord.z = 0.0f;
            vertex.color = particle.color;
        }
    }

}; // namespace chaos


#else

namespace chaos
{

	// ===========================================================================
	// Background particle system
	// ===========================================================================

	class ParticleBackground
	{
	public:

		glm::vec4 color;
	};

	class ParticleBackgroundLayerTrait : public ParticleLayerTrait<ParticleBackground, VertexDefault>
	{
	public:

		/** constructor */
		ParticleBackgroundLayerTrait(){ dynamic_particles = dynamic_vertices = false;}
		/** copy constructor */
		ParticleBackgroundLayerTrait(ParticleBackgroundLayerTrait const& src) = default;
	};

	/** output primitive */
    template<typename VERTEX_TYPE>
	void ParticleToPrimitives(ParticleBackground const& particle, PrimitiveOutput<VERTEX_TYPE>& output);
	/** generates 1 quad from one particle */
    template<typename VERTEX_TYPE>
	void ParticleToPrimitive(ParticleBackground const& particle, QuadPrimitive<VERTEX_TYPE>& primitive);
	/** generates 1 triangle pair from one particle */
    template<typename VERTEX_TYPE>
	void ParticleToPrimitive(ParticleBackground const& particle, TrianglePairPrimitive<VERTEX_TYPE>& primitive);

	CHAOS_REGISTER_CLASS1(ParticleBackground);

}; //namespace chaos

#endif // CHAOS_FORWARD_DECLARATION
