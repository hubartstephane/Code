namespace chaos
{
#ifdef CHAOS_FORWARD_DECLARATION

	template<typename VERTEX_TYPE>
	class GPUDrawInterface;

#elif !defined CHAOS_TEMPLATE_IMPLEMENTATION

	/**
	 * GPUDrawInterface : this is a primitive that have its own GPUDynamicMesh embedded for directed drawing
	 */

	template<typename VERTEX_TYPE>
	class GPUDrawInterface : public PrimitiveOutput<VERTEX_TYPE>
	{
	public:

		/** default/minimum number of vertices allocation (same value than PrimitiveOutput's) */
		static constexpr size_t MIN_VERTEX_ALLOCATION = 100;

		/** constructor */
		GPUDrawInterface(ObjectRequest in_render_material_request, size_t in_vertex_requirement_evaluation = MIN_VERTEX_ALLOCATION) :
			PrimitiveOutput<VERTEX_TYPE>(&dynamic_mesh, GetBufferPool(), GetVertexDeclaration(), in_render_material_request, in_vertex_requirement_evaluation)
		{
			dynamic_mesh.SetVertexArrayCache(GetVertexArrayCache());
		}
		/** constructor */
		GPUDrawInterface(GPURenderMaterial* in_render_material, size_t in_vertex_requirement_evaluation = MIN_VERTEX_ALLOCATION) :
			PrimitiveOutput<VERTEX_TYPE>(&dynamic_mesh, GetBufferPool(), GetVertexDeclaration(), (in_render_material != nullptr) ? in_render_material : DefaultScreenSpaceProgram::GetMaterial(), in_vertex_requirement_evaluation)
		{
			dynamic_mesh.SetVertexArrayCache(GetVertexArrayCache());
		}

		/** destructor */
		virtual ~GPUDrawInterface()
		{
			dynamic_mesh.Clear(GetBufferPool());
		}

		/** flush and display the pending content */
		int Display(GPURenderer* renderer, GPUProgramProviderBase const* uniform_provider, GPURenderParams const& render_params)
		{
			this->Flush();
			int result = dynamic_mesh.Display(renderer, uniform_provider, render_params);
			dynamic_mesh.Clear(GetBufferPool());
			return result;
		}

		/** extract the mesh for external purpose */
		GPUDynamicMesh* ExtractMesh()
		{
			GPUDynamicMesh* result = new GPUDynamicMesh();
			if (result != nullptr)
			{
				this->Flush();
				swap(*result, dynamic_mesh);
				dynamic_mesh.SetVertexArrayCache(GetVertexArrayCache());
			}
			return result;
		}

		/** getter on the mesh */
		GPUDynamicMesh& GetDynamicMesh() { return dynamic_mesh; }
		/** getter on the mesh */
		GPUDynamicMesh const& GetDynamicMesh() const { return dynamic_mesh; }
		/** clear the dynamic mesh */
		void Clear()
		{
			dynamic_mesh.Clear(GetBufferPool());
		}

	protected:

		/** gets the shared GPUBufferPool to improve GPUBuffer allocation */
		static GPUBufferPool* GetBufferPool()
		{
			static shared_ptr<GPUBufferPool> result = new GPUBufferPool();
			return result.get();
		}
		/** gets the shared GPUVertexDeclaration */
		static GPUVertexDeclaration* GetVertexDeclaration()
		{
			static shared_ptr<GPUVertexDeclaration> result;
			if (result == nullptr)
			{
				result = new GPUVertexDeclaration;
				GetTypedVertexDeclaration(result.get(), boost::mpl::identity<VERTEX_TYPE>());
			}
			return result.get();
		}
		/** gets the shared GPUVertexArrayCache */
		static GPUVertexArrayCache* GetVertexArrayCache()
		{
			static shared_ptr<GPUVertexArrayCache> result = new GPUVertexArrayCache();
			return result.get();
		}

	protected:

		/** the internal buffer */
		GPUDynamicMesh dynamic_mesh;
	};

#endif

}; // namespace chaos