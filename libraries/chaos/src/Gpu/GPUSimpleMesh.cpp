#include <chaos/Chaos.h>

namespace chaos
{
	GPUSimpleMesh::~GPUSimpleMesh()
	{
		Release();
	}

	void GPUSimpleMesh::ShiftPrimitivesIndexAndVertexPosition(int vb_offset, int ib_offset)
	{
		if (vb_offset != 0 || ib_offset != 0)
			for (auto & primitive : primitives)
				primitive.ShiftIndexAndVertexPosition(vb_offset, ib_offset);
	}

	void GPUSimpleMesh::Release()
	{
		vertex_buffer = nullptr;
		index_buffer = nullptr;
        vertex_array_cache = nullptr;
		vertex_declaration = nullptr;

		primitives.clear();
	}

	void GPUSimpleMesh::Render(GPURenderer * renderer, GPUProgram const * program, GPUProgramProviderBase const * uniform_provider, GPURenderParams const & render_params)
	{
		// early exit
		if (program == nullptr)
			return;
		// use program
		program->UseProgram(uniform_provider);
		// do the rendering
		DoRender(renderer, program, render_params);
		// reset the program		
		glUseProgram(0);
	}

	void GPUSimpleMesh::Render(GPURenderer * renderer, GPURenderMaterial const * material, GPUProgramProviderBase const * uniform_provider, GPURenderParams const & render_params)
	{
		// early exit
        if (vertex_declaration == nullptr)
            return;
		if (material == nullptr)
			return;
		// get the program for the material, use it and bind its uniforms
		GPUProgram const * program = material->UseMaterial(uniform_provider, render_params);
		if (program == nullptr)
			return;
		// do the rendering
		DoRender(renderer, program, render_params);
		// reset the program
		glUseProgram(0);
	}

	void GPUSimpleMesh::DoRender(GPURenderer * renderer, GPUProgram const * program, GPURenderParams const & render_params)
	{
		assert(program != nullptr);
        // create a vertex array cache if necessary
        if (vertex_array_cache == nullptr)
            vertex_array_cache = new GPUVertexArrayCache;
		// find the vertex array to use
		GPUVertexArray const * vertex_array = vertex_array_cache->FindOrCreateVertexArray(renderer, program, vertex_buffer.get(), index_buffer.get(), vertex_declaration.get(), vertex_buffer_offset);
		if (vertex_array == nullptr)
			return;
		// bind the vertex array
		glBindVertexArray(vertex_array->GetResourceID());
		// render the primitives
		for (auto const & primitive : primitives)
			renderer->Draw(primitive, render_params.instancing);
		// unbind the vertex array
		glBindVertexArray(0);
	}

	void GPUSimpleMesh::SetVertexBufferOffset(GLintptr in_vertex_buffer_offset)
	{
		vertex_buffer_offset = in_vertex_buffer_offset;
	}

}; // namespace chaos
