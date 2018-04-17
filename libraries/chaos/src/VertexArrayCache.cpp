#include <chaos/VertexArrayCache.h>
#include <chaos/GLTools.h>


namespace chaos
{
	VertexArrayCacheEntry::~VertexArrayCacheEntry()
	{
		GPUProgramToVertexArrayCacheCallbacks * callbacks = program_destruction_callback;
		if (callbacks != nullptr)
		{
			program_destruction_callback = nullptr;
			// invalidate the destruction callback
			callbacks->cache_entry = nullptr; 
			// destroy the callback			
			if (program != nullptr)
				program->RemoveReleaseCallback(callbacks); 
		}	
	}

	VertexArray * VertexArrayCache::FindVertexArray(GPUProgram * program)
	{
		// early exit
		if (program == nullptr)
			return nullptr;

		// test whether the program id match
		size_t count = entries.size();
		for (size_t i = 0 ; i < count ; ++i)
		{
			VertexArrayCacheEntry * entry = entries[i].get();
			if (entry != nullptr && entry->program == program)
				return entry->vertex_array.get();		
		}	
		return nullptr;
	}

	VertexArray * VertexArrayCache::FindOrCreateVertexArray(GPUProgram * program, VertexBuffer * vertex_buffer, IndexBuffer * index_buffer, VertexDeclaration & declaration)
	{
		// early exit
		if (program == nullptr)
			return nullptr;

		// find exisiting data
		VertexArray * result = FindVertexArray(program);
		if (result != nullptr)
			return result;

		// create the vertex array
		boost::intrusive_ptr<VertexArray> new_vertex_array;
		if (!GLTools::GenerateVertexAndIndexBuffersObject(&new_vertex_array, nullptr, nullptr))
			return nullptr;

		// bind attributes
		GPUProgramData const & data = program->GetProgramData();

		GLuint va = new_vertex_array->GetResourceID();
		data.BindAttributes(va, declaration, nullptr);

		// set the vertex buffer
		if (vertex_buffer != nullptr)  // simple mesh only use one vertex_buffer : binding_index is always 0
		{
			GLuint   binding_index = 0;
			GLintptr offset = 0;
			glVertexArrayVertexBuffer(va, binding_index, vertex_buffer->GetResourceID(), offset, declaration.GetVertexSize());
		}

		// set the index buffer
		if (index_buffer != nullptr)
			glVertexArrayElementBuffer(va, index_buffer->GetResourceID());

		// create the destruction callback
		GPUProgramToVertexArrayCacheCallbacks * new_callback = new GPUProgramToVertexArrayCacheCallbacks();
		if (new_callback == nullptr)
			return nullptr;

		// create the entry in the cache
		VertexArrayCacheEntry * new_entry = new VertexArrayCacheEntry();
		if (new_entry == nullptr)
		{
			delete(new_callback);
			return nullptr;
		}
		// populate data
		new_callback->cache_entry = new_entry;

		new_entry->program_destruction_callback = new_callback;
		new_entry->program = program;
		new_entry->cache   = this;
		new_entry->vertex_array = new_vertex_array;

		// insert the callbacks in the program
		program->AddReleaseCallback(new_callback);
		// insert the entry
		entries.push_back(new_entry);

		return new_vertex_array.get();
	}

	void VertexArrayCache::RemoveEntry(VertexArrayCacheEntry * entry)
	{
		assert(entry != nullptr);
		size_t count = entries.size();
		for (size_t i = 0 ; i < count ; ++i)
		{
			if (entries[i] == entry)
			{
				if (i != count - 1)
					entries[i] = entries[count - 1];
				entries.pop_back();
				break;
			}		
		}	
	}

	bool GPUProgramToVertexArrayCacheCallbacks::OnResourceReleased(GPUResource * object, bool destruction)
	{
		VertexArrayCacheEntry * entry = cache_entry;
		if (entry != nullptr)
		{
			cache_entry = nullptr;
			// invalidate the entry
			entry->program = nullptr;
			entry->program_destruction_callback = nullptr;
			// remove the entry from the cache
			if (entry->cache != nullptr)
				entry->cache->RemoveEntry(entry);
			
		}
		return true; // this destruction callback is to be removed
	}



}; // namespace chaos
