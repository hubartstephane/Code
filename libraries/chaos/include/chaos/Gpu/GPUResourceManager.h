namespace chaos
{
#ifdef CHAOS_FORWARD_DECLARATION

	class GPUResourceManagerReloadData;
	class GPUProgramReplaceTextureAction;
	class GPUResourceManager;

#elif !defined CHAOS_TEMPLATE_IMPLEMENTATION

	/**
	 * GPUResourceManagerReloadData : a structure used whenever we can to reload all data in manager
	 */

	class CHAOS_API GPUResourceManagerReloadData
	{
	public:

		/** texture that have captured data from new manager */
		std::map<GPUTexture*, GPUTexture*> texture_map;
		/** programs that have captured data from new manager */
		std::map<GPUProgram*, GPUProgram*> program_map;
		/** materials that have captured data from new manager */
		std::map<GPURenderMaterial*, GPURenderMaterial*> render_material_map;
	};

	/**
	* GPUProgramReplaceTextureAction : an action used to replace any texture in TextureProvider by one in the replacement map
	*/

	class CHAOS_API GPUProgramReplaceTextureAction : public GPUProgramAction
	{
	public:

		/** constructor */
		GPUProgramReplaceTextureAction(GPUResourceManagerReloadData& in_reload_data) :
			reload_data(in_reload_data) {}

	protected:

		/** override */
		virtual bool DoProcess(char const* name, GPUTexture const* value, GPUProgramProviderInterface const* provider) const override;

	protected:

		/** the reload data */
		GPUResourceManagerReloadData& reload_data;
	};

	/**
	* GPUResourceManager : a manager to store different kinds of (can be depend) resources
	**/

	class CHAOS_API GPUResourceManager : public ResourceManager, public ConfigurableInterface
	{
		friend class GPUTextureLoader;
		friend class GPUProgramLoader;
		friend class GPURenderMaterialLoader;
		friend class GPURenderMaterialLoaderReferenceSolver;

	public:

		/** destructor */
		virtual ~GPUResourceManager();
		/** release the resources */
		virtual void Release();

		/** load a texture */
		GPUTexture* LoadTexture(FilePathParam const& path, char const* name = nullptr, GenTextureParameters const& texture_parameters = {});
		/** load a program */
		GPUProgram* LoadProgram(FilePathParam const& path, char const* name = nullptr);
		/** load a material */
		GPURenderMaterial* LoadRenderMaterial(FilePathParam const& path, char const* name = nullptr);

		/** get the number of textures */
		size_t GetTextureCount() const;
		/** get a texture by its index */
		GPUTexture* GetTexture(size_t index);
		/** get a texture by its index */
		GPUTexture const* GetTexture(size_t index) const;
		/** find a texture by its name */
		GPUTexture* FindTexture(ObjectRequest request);
		/** find a texture by its name */
		GPUTexture const* FindTexture(ObjectRequest request) const;
		/** find a texture by its path */
		GPUTexture* FindTextureByPath(FilePathParam const& path);
		/** find a texture by its path */
		GPUTexture const* FindTextureByPath(FilePathParam const& path) const;


		/** get the number of programs */
		size_t GetProgramCount() const;
		/** get a program by its index */
		GPUProgram* GetProgram(size_t index);
		/** get a program by its index */
		GPUProgram const* GetProgram(size_t index) const;
		/** find a program by its name */
		GPUProgram* FindProgram(ObjectRequest request);
		/** find a program by its name */
		GPUProgram const* FindProgram(ObjectRequest request) const;
		/** find a program by its path */
		GPUProgram* FindProgramByPath(FilePathParam const& path);
		/** find a program by its path */
		GPUProgram const* FindProgramByPath(FilePathParam const& path) const;


		/** get the number of materials */
		size_t GetRenderMaterialCount() const;
		/** get a material by its index */
		GPURenderMaterial* GetRenderMaterial(size_t index);
		/** get a material by its index */
		GPURenderMaterial const* GetRenderMaterial(size_t index) const;
		/** find a render material by its name */
		GPURenderMaterial* FindRenderMaterial(ObjectRequest request);
		/** find a render material by its name */
		GPURenderMaterial const* FindRenderMaterial(ObjectRequest request) const;
		/** find a render material by its path */
		GPURenderMaterial* FindRenderMaterialByPath(FilePathParam const& path);
		/** find a render material by its path */
		GPURenderMaterial const* FindRenderMaterialByPath(FilePathParam const& path) const;


#if 0
		/** initialize the manager from a file */
		virtual bool LoadManager(FilePathParam const& path);
#endif


		/** loading from a JSON object */
		virtual bool InitializeFromConfiguration();
		/** merge all resources with incomming manager */
		virtual bool RefreshGPUResources(GPUResourceManager* other_gpu_manager);

		/** Initialize internal resources */
		virtual bool InitializeInternalResources();
		/** get an index buffer for quad rendering (returns the number of quad that can be renderer with this buffer) */
		GPUBuffer* GetQuadIndexBuffer(size_t* result_quad_count);
		/** get quad simple mesh */
		GPUMesh* GetQuadMesh();

	protected:

		/** override */
		virtual bool OnReadConfigurableProperties(JSONReadConfiguration config, ReadConfigurablePropertiesContext context) override;

		/** load the textures from configuration */
		virtual bool LoadTexturesFromConfiguration(JSONReadConfiguration config);
		/** load the programs from configuration */
		virtual bool LoadProgramsFromConfiguration(JSONReadConfiguration config);
		/** load the materials from configuration */
		virtual bool LoadMaterialsFromConfiguration(JSONReadConfiguration config);

		/** merge all resources with incomming manager */
		virtual bool RefreshTextures(GPUResourceManager* other_gpu_manager, GPUResourceManagerReloadData& reload_data);
		/** merge all resources with incomming manager */
		virtual bool RefreshPrograms(GPUResourceManager* other_gpu_manager, GPUResourceManagerReloadData& reload_data);
		/** merge all resources with incomming manager */
		virtual bool RefreshMaterial(GPUResourceManager* other_gpu_manager, GPUResourceManagerReloadData& reload_data);

		/** recursively patch all materials due to refreshing */
		void PatchRenderMaterialRecursive(GPURenderMaterialInfo* material_info, GPUResourceManagerReloadData& reload_data);

	protected:

		/** the textures */
		std::vector<shared_ptr<GPUTexture>> textures;
		/** the programs */
		std::vector<shared_ptr<GPUProgram>> programs;
		/** the render materials */
		std::vector<shared_ptr<GPURenderMaterial>> render_materials;

		/** the fullscreen quad mesh */
		shared_ptr<GPUMesh> quad_mesh;
		/** the quad to triangle_pair index rendering */
		shared_ptr<GPUBuffer> quad_index_buffer;
	};

#endif

}; // namespace chaos