#include "chaos/ChaosPCH.h"
#include "chaos/ChaosInternals.h"

namespace chaos
{
	namespace
	{
		constexpr size_t QUAD_INDEX_BUFFER_COUNT = 10000;
	};

	/**
	* GPUResourceManager
	**/

	GPUResourceManager::~GPUResourceManager()
	{
		Release();
	}

	void GPUResourceManager::Release()
	{
		textures.clear();
		programs.clear();
		render_materials.clear();
	}

	size_t GPUResourceManager::GetTextureCount() const
	{
		return textures.size();
	}

	GPUTexture * GPUResourceManager::GetTexture(size_t index)
	{
		return textures[index].get();
	}

	GPUTexture const * GPUResourceManager::GetTexture(size_t index) const
	{
		return textures[index].get();
	}

	GPUTexture * GPUResourceManager::FindTexture(ObjectRequest request)
	{
		return request.FindObject(textures);
	}

	GPUTexture const * GPUResourceManager::FindTexture(ObjectRequest request) const
	{
		return request.FindObject(textures);
	}

	GPUTexture * GPUResourceManager::FindTextureByPath(FilePathParam const & path)
	{
		return FindObjectByPath(path, textures);
	}

	GPUTexture const * GPUResourceManager::FindTextureByPath(FilePathParam const & path) const
	{
		return FindObjectByPath(path, textures);
	}

	size_t GPUResourceManager::GetProgramCount() const
	{
		return programs.size();
	}

	GPUProgram * GPUResourceManager::GetProgram(size_t index)
	{
		return programs[index].get();
	}

	GPUProgram const * GPUResourceManager::GetProgram(size_t index) const
	{
		return programs[index].get();
	}

	GPUProgram * GPUResourceManager::FindProgram(ObjectRequest request)
	{
		return request.FindObject(programs);
	}

	GPUProgram const * GPUResourceManager::FindProgram(ObjectRequest request) const
	{
		return request.FindObject(programs);
	}

	GPUProgram * GPUResourceManager::FindProgramByPath(FilePathParam const & path)
	{
		return FindObjectByPath(path, programs);
	}

	GPUProgram const * GPUResourceManager::FindProgramByPath(FilePathParam const & path) const
	{
		return FindObjectByPath(path, programs);
	}

	size_t GPUResourceManager::GetRenderMaterialCount() const
	{
		return render_materials.size();
	}

	GPURenderMaterial * GPUResourceManager::GetRenderMaterial(size_t index)
	{
		return render_materials[index].get();
	}

	GPURenderMaterial const * GPUResourceManager::GetRenderMaterial(size_t index) const
	{
		return render_materials[index].get();
	}

	GPURenderMaterial * GPUResourceManager::FindRenderMaterial(ObjectRequest request)
	{
		if (GPURenderMaterial* result = request.FindObject(render_materials))
			return result;
		if (GPUProgram* program = request.FindObject(programs))
			return program->GetDefaultMaterial();
		return nullptr;
	}

	GPURenderMaterial const * GPUResourceManager::FindRenderMaterial(ObjectRequest request) const
	{
		if (GPURenderMaterial const* result = request.FindObject(render_materials))
			return result;
		if (GPUProgram const* program = request.FindObject(programs))
			return program->GetDefaultMaterial();
		return nullptr;
	}

	GPURenderMaterial * GPUResourceManager::FindRenderMaterialByPath(FilePathParam const & path)
	{
		return FindObjectByPath(path, render_materials);
	}

	GPURenderMaterial const * GPUResourceManager::FindRenderMaterialByPath(FilePathParam const & path) const
	{
		return FindObjectByPath(path, render_materials);
	}

	GPUTexture * GPUResourceManager::LoadTexture(FilePathParam const & path, char const * name, GenTextureParameters const & texture_parameters)
	{
		return GPUTextureLoader(this).LoadObject(path, name, texture_parameters);
	}

	GPUProgram * GPUResourceManager::LoadProgram(FilePathParam const & path, char const * name)
	{
		return GPUProgramLoader(this).LoadObject(path, name);
	}

	GPURenderMaterial * GPUResourceManager::LoadRenderMaterial(FilePathParam const & path, char const * name)
	{
		GPURenderMaterialLoaderReferenceSolver solver;

		GPURenderMaterial * result = GPURenderMaterialLoader(this, &solver).LoadObject(path, name);
		if (result != nullptr)
			solver.ResolveReferences(this);
		return result;
	}


#if 0
	bool GPUResourceManager::LoadManager(FilePathParam const & path)
	{
		nlohmann::json json;
		if (!JSONTools::LoadJSONFile(path, json, LoadFileFlag::RECURSIVE))
			return true;
		return InitializeFromConfiguration(json);
	}
#endif

	bool GPUResourceManager::InitializeFromConfiguration()
	{
		return ReadConfigurableProperties(ReadConfigurablePropertiesContext::INITIALIZATION);
	}

	bool GPUResourceManager::LoadTexturesFromConfiguration(JSONReadConfiguration config)
	{
		return LoadObjectsFromConfiguration<true>(
			"textures",
			config,
			GPUTextureLoader(this));
	}

	bool GPUResourceManager::LoadProgramsFromConfiguration(JSONReadConfiguration config)
	{
		return LoadObjectsFromConfiguration<true>(
			"programs",
			config,
			GPUProgramLoader(this));
	}

	bool GPUResourceManager::LoadMaterialsFromConfiguration(JSONReadConfiguration config)
	{
		GPURenderMaterialLoaderReferenceSolver solver; // finalize the missing references

		bool result = LoadObjectsFromConfiguration<true>(
			"rendermaterials",
			config,
			GPURenderMaterialLoader(this, &solver));
		if (result)
			solver.ResolveReferences(this);
		return result;
	}

	bool GPUResourceManager::RefreshGPUResources(GPUResourceManager * other_gpu_manager)
	{
		GPUResourceManagerReloadData reload_data;

		assert(other_gpu_manager != nullptr);
		if (!RefreshTextures(other_gpu_manager, reload_data))
			return false;
		if (!RefreshPrograms(other_gpu_manager, reload_data))
			return false;
		if (!RefreshMaterial(other_gpu_manager, reload_data))
			return false;
		return true;
	}

	// We are working with managers (the normal and a temporary)
	// We iterate over all resources:
	//   -If we find in TEMP a resource that is not in NORMAL, we simply add it to NORMAL too (the resource is in both managers until destruction of TEMP)
	//   -If a TEMP resource has an equivalent (PATH or NAME) in NORMAL, we want to update NORMAL with data in TEMP resource
	//    (a swap of all data will do the trick) => this is STEAL case
	//
	// We have now to patch all resources. All internal references to an object that has been stolen must be replaced
	// (GPURenderMaterial is the only concerned while it can have reference to textures and programs)

	template<typename FIND_BY_REQUEST, typename FIND_BY_PATH, typename RESOURCE_VECTOR, typename SWAP_OBJECTS_FUNC>
	void RefreshObjects(FIND_BY_REQUEST find_by_request, FIND_BY_PATH find_by_path, RESOURCE_VECTOR resource_vector, GPUResourceManager * gpu_manager, GPUResourceManager * other_gpu_manager, SWAP_OBJECTS_FUNC swap_objects)
	{
		auto & ori_objects = gpu_manager->*resource_vector;
		auto & new_objects = other_gpu_manager->*resource_vector;

		// update all resources that are newer (same name or same path)
		size_t ori_count = ori_objects.size();
		for (size_t i = 0; i < ori_count; ++i)
		{
			// original object
			auto * ori_object = ori_objects[i].get();
			if (ori_object == nullptr)
				continue;
			// find corresponding resource in new manager
			decltype(ori_object) other_object = nullptr;
			if (ori_object->GetName() != nullptr)
				other_object = (other_gpu_manager->*find_by_request)(ori_object->GetName());
			if (other_object == nullptr)
				other_object = (other_gpu_manager->*find_by_path)(ori_object->GetPath());
			if (other_object == nullptr)
				continue;

			// swap the resources
			swap_objects(ori_object, other_object);
		}
		// add new resources
		size_t new_count = new_objects.size();
		for (size_t i = 0; i < new_count; ++i)
		{
			// original object
			auto * new_object = new_objects[i].get();
			if (new_object == nullptr)
				continue;
			// find whether this object is new
			decltype(new_object) ori_object = nullptr;
			if (new_object->GetName() != nullptr)
				ori_object = (gpu_manager->*find_by_request)(new_object->GetName());
			if (ori_object == nullptr)
				ori_object = (gpu_manager->*find_by_path)(new_object->GetPath());
			if (ori_object != nullptr)
				continue;
			// copy the object in the original manager
			ori_objects.push_back(new_object);
		}
		// resources that have been destroyed are simply ignored (we have no real way to be sure they are not used)
	}


	bool GPUResourceManager::RefreshTextures(GPUResourceManager * other_gpu_manager, GPUResourceManagerReloadData & reload_data)
	{
		assert(other_gpu_manager != nullptr);

		GPUTexture * (GPUResourceManager::*find_by_request)(ObjectRequest) = &GPUResourceManager::FindTexture;
		GPUTexture * (GPUResourceManager::*find_by_path)(FilePathParam const &) = &GPUResourceManager::FindTextureByPath;

		std::vector<shared_ptr<GPUTexture>> GPUResourceManager::*resource_vector = &GPUResourceManager::textures;

		RefreshObjects(find_by_request, find_by_path, resource_vector, this, other_gpu_manager, [&reload_data](GPUTexture * ori_object, GPUTexture * other_object)
		{
			// each time there is a reference to other_object, replace it with ori_object
			// (while ori_object has capture other's data)
			reload_data.texture_map[other_object] = ori_object;

			// XXX : we cannot simply copy texture_id => this would produce a double deletion of OpenGL resource
			std::swap(ori_object->texture_id, other_object->texture_id);
			std::swap(ori_object->file_timestamp, other_object->file_timestamp);
			std::swap(ori_object->texture_description, other_object->texture_description);
		});
		return true;
	}

	bool GPUResourceManager::RefreshPrograms(GPUResourceManager * other_gpu_manager, GPUResourceManagerReloadData & reload_data)
	{
		assert(other_gpu_manager != nullptr);

		GPUProgram * (GPUResourceManager::*find_by_request)(ObjectRequest) = &GPUResourceManager::FindProgram;
		GPUProgram * (GPUResourceManager::*find_by_path)(FilePathParam const &) = &GPUResourceManager::FindProgramByPath;

		std::vector<shared_ptr<GPUProgram>> GPUResourceManager::*resource_vector = &GPUResourceManager::programs;

		RefreshObjects(find_by_request, find_by_path, resource_vector, this, other_gpu_manager, [&reload_data](GPUProgram * ori_object, GPUProgram * other_object)
		{
			// each time there is a reference to other_object, replace it with ori_object
			// (while ori_object has capture other's data)
			reload_data.program_map[other_object] = ori_object;

			// XXX : we cannot simply copy program_id => this would produce a double deletion of OpenGL resource
			std::swap(ori_object->program_id, other_object->program_id);
			std::swap(ori_object->file_timestamp, other_object->file_timestamp);
			std::swap(ori_object->program_data, other_object->program_data);
		});

		return true;
	}

	bool GPUProgramReplaceTextureAction::DoProcess(char const * name, GPUTexture const * value, GPUProgramProviderInterface const * provider) const
	{
		// XXX : remove constness ! Maybe a better way to do so
		GPUProgramProviderTexture * provider_texture = const_cast<GPUProgramProviderTexture *>(dynamic_cast<GPUProgramProviderTexture const *>(provider));
		if (provider_texture != nullptr)
		{
			auto it = reload_data.texture_map.find(provider_texture->value.get());
			if (it != reload_data.texture_map.end())
				provider_texture->value = it->second;
		}
		return false; // continue for all other textures
	}

	void GPUResourceManager::PatchRenderMaterialRecursive(GPURenderMaterialInfo * material_info, GPUResourceManagerReloadData & reload_data)
	{
		if (material_info == nullptr)
			return;

		// patch textures (uniforms)
		GPUProgramReplaceTextureAction action(reload_data);
		material_info->uniform_provider.ProcessAction(nullptr, action);

		// patch program
		auto it_program = reload_data.program_map.find(material_info->program.get());
		if (it_program != reload_data.program_map.end())
			material_info->program = it_program->second;

		// patch parent_material
		auto it_parent = reload_data.render_material_map.find(material_info->parent_material.get());
		if (it_parent != reload_data.render_material_map.end())
			material_info->parent_material = it_parent->second;

		// recursively look at renderpasses
		for (GPURenderMaterialInfoEntry & entry : material_info->renderpasses)
			PatchRenderMaterialRecursive(entry.material_info.get(), reload_data);
	}

	bool GPUResourceManager::RefreshMaterial(GPUResourceManager * other_gpu_manager, GPUResourceManagerReloadData & reload_data)
	{
		assert(other_gpu_manager != nullptr);

		GPURenderMaterial * (GPUResourceManager::*find_by_request)(ObjectRequest) = &GPUResourceManager::FindRenderMaterial;
		GPURenderMaterial * (GPUResourceManager::*find_by_path)(FilePathParam const &) = &GPUResourceManager::FindRenderMaterialByPath;

		std::vector<shared_ptr<GPURenderMaterial>> GPUResourceManager::*resource_vector = &GPUResourceManager::render_materials;

		RefreshObjects(find_by_request, find_by_path, resource_vector, this, other_gpu_manager, [&reload_data](GPURenderMaterial * ori_object, GPURenderMaterial * other_object)
		{
			// each time there is a reference to other_object, replace it with ori_object
			// (while ori_object has capture other's data)
			reload_data.render_material_map[other_object] = ori_object;

			std::swap(ori_object->file_timestamp, other_object->file_timestamp);

			std::swap(ori_object->material_info, other_object->material_info);
#if 0
			std::swap(ori_object->material_info->parent_material, other_object->material_info->parent_material);
			std::swap(ori_object->material_info->program, other_object->material_info->program);
			std::swap(ori_object->material_info->uniform_provider.children_providers, other_object->material_info->uniform_provider.children_providers);
			std::swap(ori_object->material_info->renderpasses, other_object->material_info->renderpasses);
			std::swap(ori_object->material_info->hidden, other_object->material_info->hidden);
			std::swap(ori_object->material_info->filter, other_object->material_info->filter);
			std::swap(ori_object->material_info->hidden_specified, other_object->material_info->hidden_specified);
			std::swap(ori_object->material_info->filter_specified, other_object->material_info->filter_specified);
#endif
		});

		// patching references (texures, programs, parent_materials)
		size_t count = render_materials.size();
		for (size_t i = 0; i < count; ++i)
			PatchRenderMaterialRecursive(render_materials[i]->material_info.get(), reload_data);
		return true;
	}

	bool GPUResourceManager::InitializeInternalResources()
	{
		// generate the quad mesh
		GPUMultiMeshGenerator generators;

		box2 box = box2(glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 1.0f));
		generators.AddGenerator(new GPUQuadMeshGenerator(box), quad_mesh);
		if (!generators.GenerateMeshes())
			return false;

		// generate the quad list index buffer (used in primitive rendering)
		quad_index_buffer = new GPUBuffer(true);
		if (quad_index_buffer == nullptr)
			return false;

		size_t buffer_size = 6 * QUAD_INDEX_BUFFER_COUNT * sizeof(GLuint); // quad becomes a triangle pair
		// reserve a buffer
		quad_index_buffer->SetBufferData(nullptr, buffer_size);
		// fill the buffer
		GLuint* buffer = (GLuint*)quad_index_buffer->MapBuffer(0, 0, false, true);
		if (buffer == nullptr)
			return false;

		for (size_t i = 0; i < QUAD_INDEX_BUFFER_COUNT; ++i)
		{
			size_t start = i * 4;

			buffer[0] = (GLuint)(start + 0);
			buffer[1] = (GLuint)(start + 1);
			buffer[2] = (GLuint)(start + 2);
			buffer[3] = (GLuint)(start + 3);
			buffer[4] = (GLuint)(start + 0);
			buffer[5] = (GLuint)(start + 2);
			buffer += 6;
		}
		quad_index_buffer->UnMapBuffer();
		// this buffer will never be given to any GPUBufferPool
		quad_index_buffer->IncrementUsageCount();

		return true;
	}

	GPUBuffer* GPUResourceManager::GetQuadIndexBuffer(size_t* result_quad_count)
	{
		if (result_quad_count != nullptr)
			*result_quad_count = QUAD_INDEX_BUFFER_COUNT;
		return quad_index_buffer.get();
	}

	GPUMesh* GPUResourceManager::GetQuadMesh()
	{
		return quad_mesh.get();
	}

	bool GPUResourceManager::OnReadConfigurableProperties(JSONReadConfiguration config, ReadConfigurablePropertiesContext context)
	{
		if (context == ReadConfigurablePropertiesContext::INITIALIZATION)
		{
			if (!LoadTexturesFromConfiguration(config))
				return false;
			if (!LoadProgramsFromConfiguration(config))
				return false;
			if (!LoadMaterialsFromConfiguration(config))
				return false;
			return true;
		}
		else if (context == ReadConfigurablePropertiesContext::HOT_RELOAD)
		{
			// create a temporary manager
			if (shared_ptr<GPUResourceManager> other_gpu_resource_manager = new GPUResourceManager) // destroyed at the end of the function
			{
				if (other_gpu_resource_manager->StartManager())
				{
					other_gpu_resource_manager->InitializeFromConfiguration();
					RefreshGPUResources(other_gpu_resource_manager.get());
					other_gpu_resource_manager->StopManager();
					return true;
				}
			}
		}
		return false;
	}

}; // namespace chaos
