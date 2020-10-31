#include <chaos/Chaos.h>

namespace chaos
{
	box2 GLTools::SetViewportWithAspect(glm::ivec2 const & size, float aspect)
	{
		box2 viewport = box2(std::make_pair(
			glm::vec2(0.0f, 0.0f),
			RecastVector<glm::vec2>(size)		
		));

		viewport = ShrinkBoxToAspect(viewport, aspect);
		SetViewport(viewport);
		return viewport;
	}

	bool GLTools::IsMatrixType(GLenum type)
	{
		// XXX : samplers type, maybe some error or some missing elements
		GLenum const matrix_types[] = {
			GL_FLOAT_MAT2, GL_FLOAT_MAT3, GL_FLOAT_MAT4,
			GL_FLOAT_MAT2x3, GL_FLOAT_MAT2x4, GL_FLOAT_MAT3x2, GL_FLOAT_MAT3x4, GL_FLOAT_MAT4x2, GL_FLOAT_MAT4x3,
			GL_DOUBLE_MAT2, GL_DOUBLE_MAT3, GL_DOUBLE_MAT4,
			GL_DOUBLE_MAT2x3, GL_DOUBLE_MAT2x4, GL_DOUBLE_MAT3x2, GL_DOUBLE_MAT3x4, GL_DOUBLE_MAT4x2, GL_DOUBLE_MAT4x3, 
			GL_NONE
		};

		for (int i = 0; matrix_types[i] != GL_NONE; ++i)
			if (type == matrix_types[i])
				return true;
		return false;
	}

	bool GLTools::IsSamplerType(GLenum type)
	{
		// XXX : samplers type, maybe some error or some missing elements
		GLenum const sampler_types[] = {
			GL_SAMPLER_1D, GL_SAMPLER_2D, GL_SAMPLER_3D, GL_SAMPLER_CUBE,
			GL_SAMPLER_1D_SHADOW, GL_SAMPLER_2D_SHADOW, GL_SAMPLER_1D_ARRAY,
			GL_SAMPLER_2D_ARRAY, GL_SAMPLER_1D_ARRAY_SHADOW, GL_SAMPLER_2D_ARRAY_SHADOW, GL_SAMPLER_CUBE_SHADOW,
			GL_INT_SAMPLER_1D, GL_INT_SAMPLER_2D, GL_INT_SAMPLER_3D, GL_INT_SAMPLER_CUBE, GL_INT_SAMPLER_1D_ARRAY,
			GL_INT_SAMPLER_2D_ARRAY, GL_UNSIGNED_INT_SAMPLER_1D, GL_UNSIGNED_INT_SAMPLER_2D, GL_UNSIGNED_INT_SAMPLER_3D,
			GL_UNSIGNED_INT_SAMPLER_CUBE, GL_UNSIGNED_INT_SAMPLER_1D_ARRAY, GL_UNSIGNED_INT_SAMPLER_2D_ARRAY,
			GL_SAMPLER_2D_RECT, GL_SAMPLER_2D_RECT_SHADOW, GL_SAMPLER_CUBE_MAP_ARRAY, GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW, GL_NONE
		};

		for (int i = 0; sampler_types[i] != GL_NONE; ++i)
			if (type == sampler_types[i])
				return true;
		return false;
	}

	int GLTools::GetEnumVectorArityImpl(GLenum type, GLenum v1, GLenum v2, GLenum v3, GLenum v4)
	{
		if (type == v1) return 1;
		if (type == v2) return 2;
		if (type == v3) return 3;
		if (type == v4) return 4;
		return 0;
	}

	int GLTools::GetEnumVectorArity(GLenum type)
	{
		int result = 0;
		if ((result = GetEnumVectorArityBool(type)) != 0)
			return result;
		if ((result = GetEnumVectorArityFloat(type)) != 0)
			return result;
		if ((result = GetEnumVectorArityDouble(type)) != 0)
			return result;
		if ((result = GetEnumVectorArityInt(type)) != 0)
			return result;
		if ((result = GetEnumVectorArityUnsignedInt(type)) != 0)
			return result;
		return result;
	}

	int GLTools::GetEnumVectorArityBool(GLenum type)
	{
		return GetEnumVectorArityImpl(type, GL_BOOL, GL_BOOL_VEC2, GL_BOOL_VEC3, GL_BOOL_VEC4);
	}

	int GLTools::GetEnumVectorArityFloat(GLenum type)
	{
		return GetEnumVectorArityImpl(type, GL_FLOAT, GL_FLOAT_VEC2, GL_FLOAT_VEC3, GL_FLOAT_VEC4);
	}

	int GLTools::GetEnumVectorArityDouble(GLenum type)
	{
		return GetEnumVectorArityImpl(type, GL_DOUBLE, GL_DOUBLE_VEC2, GL_DOUBLE_VEC3, GL_DOUBLE_VEC4);
	}

	int GLTools::GetEnumVectorArityInt(GLenum type)
	{
		return GetEnumVectorArityImpl(type, GL_INT, GL_INT_VEC2, GL_INT_VEC3, GL_INT_VEC4);
	}

	int GLTools::GetEnumVectorArityUnsignedInt(GLenum type)
	{
		return GetEnumVectorArityImpl(type, GL_UNSIGNED_INT, GL_UNSIGNED_INT_VEC2, GL_UNSIGNED_INT_VEC3, GL_UNSIGNED_INT_VEC4);
	}

	bool GLTools::MapBuffers(GLuint vertex_buffer, GLuint index_buffer, size_t vb_size, size_t ib_size, std::pair<char*, GLuint*> & result) // shuxxx
	{
		result = std::make_pair(nullptr, nullptr);

		if (vertex_buffer != 0)
		{
			glNamedBufferData(vertex_buffer, vb_size, nullptr, GL_STATIC_DRAW); // shuxxx !!!
			result.first = (char *)glMapNamedBuffer(vertex_buffer, GL_WRITE_ONLY);
			if (result.first == nullptr)
				return false;
		}

		if (index_buffer != 0)
		{
			glNamedBufferData(index_buffer, ib_size, nullptr, GL_STATIC_DRAW);
			result.second = (GLuint *)glMapNamedBuffer(index_buffer, GL_WRITE_ONLY);
			if (result.second == nullptr && vertex_buffer != 0)
			{
				glUnmapNamedBuffer(vertex_buffer);
				return false;
			}
		}

		return true;
	}

	bool GLTools::GenerateVertexAndIndexBuffers(shared_ptr<GPUVertexArray> * vertex_array, shared_ptr<GPUBuffer> * vertex_buffer, shared_ptr<GPUBuffer> * index_buffer, bool in_dynamic_vertex_buffer, bool in_dynamic_index_buffer)
	{
		// release resource at destruction in case of failure 
		shared_ptr<GPUVertexArray> va;
		shared_ptr<GPUBuffer> vb;
		shared_ptr<GPUBuffer> ib;

		if (vertex_array != nullptr)
		{
			va = new GPUVertexArray(); // create a GL resource
			if (va == nullptr || !va->IsValid())
				return false;		
		}

		if (vertex_buffer != nullptr)
		{
			vb = new GPUBuffer(in_dynamic_vertex_buffer); // create a GL resource
			if (vb == nullptr || !vb->IsValid())
				return false;		
		}

		if (index_buffer != nullptr)
		{
			ib = new GPUBuffer(in_dynamic_index_buffer); // create a GL resource
			if (ib == nullptr || !ib->IsValid())
				return false;		
		}

		// success : validate pointers
		if (vertex_array != nullptr)
			*vertex_array = va;
		if (vertex_buffer != nullptr)
			*vertex_buffer = vb;
		if (index_buffer != nullptr)
			*index_buffer = ib;

		return true;
	}

	void GLTools::DisplayGenericInformation()
	{
		GLubyte const * renderer = glGetString(GL_RENDERER);
		if (renderer != nullptr)
			LogTools::Log("OpenGL RENDERER        : %s", renderer);

		GLubyte const * vendor = glGetString(GL_VENDOR);
		if (vendor != nullptr)
			LogTools::Log("OpenGL VENDOR          : %s", vendor);

		GLubyte const * version = glGetString(GL_VERSION);
		if (version != nullptr)
			LogTools::Log("OpenGL VERSION         : %s", version);

		GLubyte const * shading_version = glGetString(GL_SHADING_LANGUAGE_VERSION);
		if (shading_version != nullptr)
			LogTools::Log("OpenGL SHADING LANGAGE : %s", shading_version);
	}

	char const * GLTools::GLenumToString(GLenum type)
	{
#define CHAOS_GLENUM_TO_STRING(x) if (type == x) return #x
		CHAOS_GLENUM_TO_STRING(GL_BYTE);
		CHAOS_GLENUM_TO_STRING(GL_UNSIGNED_BYTE);
		CHAOS_GLENUM_TO_STRING(GL_SHORT);
		CHAOS_GLENUM_TO_STRING(GL_UNSIGNED_SHORT);
		CHAOS_GLENUM_TO_STRING(GL_INT);
		CHAOS_GLENUM_TO_STRING(GL_UNSIGNED_INT);
		CHAOS_GLENUM_TO_STRING(GL_FLOAT);
		CHAOS_GLENUM_TO_STRING(GL_2_BYTES);
		CHAOS_GLENUM_TO_STRING(GL_3_BYTES);
		CHAOS_GLENUM_TO_STRING(GL_4_BYTES);
		CHAOS_GLENUM_TO_STRING(GL_DOUBLE);
		CHAOS_GLENUM_TO_STRING(GL_FLOAT_VEC2);
		CHAOS_GLENUM_TO_STRING(GL_FLOAT_VEC3);
		CHAOS_GLENUM_TO_STRING(GL_FLOAT_VEC4);
		CHAOS_GLENUM_TO_STRING(GL_INT_VEC2);
		CHAOS_GLENUM_TO_STRING(GL_INT_VEC3);
		CHAOS_GLENUM_TO_STRING(GL_INT_VEC4);
		CHAOS_GLENUM_TO_STRING(GL_BOOL);
		CHAOS_GLENUM_TO_STRING(GL_BOOL_VEC2);
		CHAOS_GLENUM_TO_STRING(GL_BOOL_VEC3);
		CHAOS_GLENUM_TO_STRING(GL_BOOL_VEC4);
		CHAOS_GLENUM_TO_STRING(GL_FLOAT_MAT2);
		CHAOS_GLENUM_TO_STRING(GL_FLOAT_MAT3);
		CHAOS_GLENUM_TO_STRING(GL_FLOAT_MAT4);
		CHAOS_GLENUM_TO_STRING(GL_SAMPLER_1D);
		CHAOS_GLENUM_TO_STRING(GL_SAMPLER_2D);
		CHAOS_GLENUM_TO_STRING(GL_SAMPLER_3D);
		CHAOS_GLENUM_TO_STRING(GL_SAMPLER_CUBE);
		CHAOS_GLENUM_TO_STRING(GL_SAMPLER_1D_SHADOW);
		CHAOS_GLENUM_TO_STRING(GL_SAMPLER_2D_SHADOW);
		CHAOS_GLENUM_TO_STRING(GL_SAMPLER_1D_ARRAY);
		CHAOS_GLENUM_TO_STRING(GL_SAMPLER_2D_ARRAY);
		CHAOS_GLENUM_TO_STRING(GL_SAMPLER_1D_ARRAY_SHADOW);
		CHAOS_GLENUM_TO_STRING(GL_SAMPLER_2D_ARRAY_SHADOW);
		CHAOS_GLENUM_TO_STRING(GL_SAMPLER_CUBE_SHADOW);
		CHAOS_GLENUM_TO_STRING(GL_UNSIGNED_INT_VEC2);
		CHAOS_GLENUM_TO_STRING(GL_UNSIGNED_INT_VEC3);
		CHAOS_GLENUM_TO_STRING(GL_UNSIGNED_INT_VEC4);
		CHAOS_GLENUM_TO_STRING(GL_INT_SAMPLER_1D);
		CHAOS_GLENUM_TO_STRING(GL_INT_SAMPLER_2D);
		CHAOS_GLENUM_TO_STRING(GL_INT_SAMPLER_3D);
		CHAOS_GLENUM_TO_STRING(GL_INT_SAMPLER_CUBE);
		CHAOS_GLENUM_TO_STRING(GL_INT_SAMPLER_1D_ARRAY);
		CHAOS_GLENUM_TO_STRING(GL_INT_SAMPLER_2D_ARRAY);
		CHAOS_GLENUM_TO_STRING(GL_UNSIGNED_INT_SAMPLER_1D);
		CHAOS_GLENUM_TO_STRING(GL_UNSIGNED_INT_SAMPLER_2D);
		CHAOS_GLENUM_TO_STRING(GL_UNSIGNED_INT_SAMPLER_3D);
		CHAOS_GLENUM_TO_STRING(GL_UNSIGNED_INT_SAMPLER_CUBE);
		CHAOS_GLENUM_TO_STRING(GL_UNSIGNED_INT_SAMPLER_1D_ARRAY);
		CHAOS_GLENUM_TO_STRING(GL_UNSIGNED_INT_SAMPLER_2D_ARRAY);

		CHAOS_GLENUM_TO_STRING(GL_DEBUG_SEVERITY_HIGH);
		CHAOS_GLENUM_TO_STRING(GL_DEBUG_SEVERITY_MEDIUM);
		CHAOS_GLENUM_TO_STRING(GL_DEBUG_SEVERITY_LOW);
		CHAOS_GLENUM_TO_STRING(GL_DEBUG_SEVERITY_NOTIFICATION);

		CHAOS_GLENUM_TO_STRING(GL_DEBUG_TYPE_ERROR);
		CHAOS_GLENUM_TO_STRING(GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR);
		CHAOS_GLENUM_TO_STRING(GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR);
		CHAOS_GLENUM_TO_STRING(GL_DEBUG_TYPE_PORTABILITY);
		CHAOS_GLENUM_TO_STRING(GL_DEBUG_TYPE_PERFORMANCE);
		CHAOS_GLENUM_TO_STRING(GL_DEBUG_TYPE_OTHER);
		CHAOS_GLENUM_TO_STRING(GL_DEBUG_TYPE_MARKER);
		CHAOS_GLENUM_TO_STRING(GL_DEBUG_TYPE_PUSH_GROUP);
		CHAOS_GLENUM_TO_STRING(GL_DEBUG_TYPE_POP_GROUP);

		CHAOS_GLENUM_TO_STRING(GL_DEBUG_SOURCE_API);
		CHAOS_GLENUM_TO_STRING(GL_DEBUG_SOURCE_WINDOW_SYSTEM);
		CHAOS_GLENUM_TO_STRING(GL_DEBUG_SOURCE_SHADER_COMPILER);
		CHAOS_GLENUM_TO_STRING(GL_DEBUG_SOURCE_THIRD_PARTY);
		CHAOS_GLENUM_TO_STRING(GL_DEBUG_SOURCE_APPLICATION);
		CHAOS_GLENUM_TO_STRING(GL_DEBUG_SOURCE_OTHER);

#undef CHAOS_GLENUM_TO_STRING

		return "Unknown";
	}

	CHAOS_HELP_TEXT(CMD, "-GLDebugNotifications");
#if _WIN32 || _WIN64
	CHAOS_HELP_TEXT(CMD, "-GLDebugBreak");
#endif

	void WINAPI GLTools::DebugMessageHandler(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar * msg, const void * user_data)
	{
		if (severity != GL_DEBUG_SEVERITY_NOTIFICATION || Application::HasApplicationCommandLineFlag("-GLDebugNotifications")) // CMDLINE
		{
			char const * source_str = GLenumToString(source);
			char const * type_str = GLenumToString(type);
			char const * severity_str = GLenumToString(severity);

			LogTools::Log("OpenGL warning message   [%s]", msg);
			LogTools::Log("               severity  [%s]", severity_str);
			LogTools::Log("               source    [%s]", source_str);
			LogTools::Log("               type      [%s]", type_str);
			LogTools::Log("               id        [%08x]", id);

			//DebugTools::DisplayCallStack(std::cout);

#if _WIN32 || _WIN64
			if (Application::HasApplicationCommandLineFlag("-GLDebugBreak")) // CMDLINE
				DebugBreak();
#endif
		}
	}

	void GLTools::SetDebugMessageHandler()
	{
		if (glDebugMessageCallback == nullptr) // some low profile cannot use debug features
			return;

		glDebugMessageCallback(DebugMessageHandler, nullptr);

#if _DEBUG
		//glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB); // this make debug events called immediately (Can BREAK in visual studio)
#endif
	}

}; // namespace chaos