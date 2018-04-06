#pragma once

#include <chaos/StandardHeaders.h>
#include <chaos/ReferencedObject.h>
#include <chaos/VertexDeclaration.h>
#include <chaos/GPUProgramData.h>
#include <chaos/GPUProgram.h>
#include <chaos/GPUProgramProvider.h>
#include <chaos/VertexArray.h>
#include <chaos/VertexBuffer.h>
#include <chaos/IndexBuffer.h>
#include <chaos/RenderMaterial.h>
#include <chaos/GPUResource.h>
#include <chaos/DrawPrimitive.h>

namespace chaos
{

	/**
	* SimpleMesh : how it says
	*/

	class SimpleMesh : public GPUResource
	{

	protected:

		class VertexArrayInfo
		{
		public:

			/** the program we want to be notify of the releasing */
			GPUProgram const * program = nullptr;
			/** the vertex array for that program */
			boost::intrusive_ptr<VertexArray> vertex_array;
		};

	public:

		/** constructor */
		SimpleMesh() = default;
		/** destructor */
		~SimpleMesh();

		/** render the primitive (base_instance is an offset applyed to gl_InstanceID) */
		void Render(GPUProgram const * program, GPUProgramProviderBase const * uniform_provider = nullptr, int instance_count = 0, int base_instance = 0) const;
		/** render the primitive (base_instance is an offset applyed to gl_InstanceID) */
		void Render(RenderMaterial const * material, GPUProgramProviderBase const * uniform_provider = nullptr, int instance_count = 0, int base_instance = 0) const;

		/** should bind index buffer and vertex buffer, as musch as for the vertex declaration */
		void SetVertexBufferOffset(GLintptr vertex_buffer_offset);
		/** offset the index or vertex position */
		void ShiftPrimitivesIndexAndVertexPosition(int vb_offset, int ib_offset);

	protected:

		/** cleaning the object */
		virtual bool DoRelease() override;
		/** find or create a vertex array for a given program */
		VertexArray const * GetOrCreateVertexArrayForProgram(GPUProgram const * program) const;

	public:

		/** self descriptive */
		VertexDeclaration declaration;
		/** the primitives to render */
		std::vector<DrawPrimitive> primitives;

		/** self descriptive */
		boost::intrusive_ptr<VertexBuffer> vertex_buffer;
		/** self descriptive */
		boost::intrusive_ptr<IndexBuffer> index_buffer;
		/** in the vertex buffer (that may be shared by other simpled mesh), the offset from the begining given to it */
		GLintptr vertex_buffer_offset = 0;

	protected:

		/** the vertex binding depends on the program that is used. This is a map that make relation between program / vertex array */
		mutable std::vector<VertexArrayInfo> vertex_array_info;

	};

}; // namespace chaos
