#include <chaos/GPUVertexDeclaration.h>

namespace chaos
{

	int GPUVertexDeclarationEntry::GetComponentCount() const
	{
		int component_count = (type & 7);
		return component_count; 
	}

	GLenum GPUVertexDeclarationEntry::GetComponentType() const
	{
		int component_type = (type >> 3) & 7;
		if (component_type == TYPE_FLOAT)
			return GL_FLOAT;
		else if (component_type == TYPE_DOUBLE)
			return GL_DOUBLE;
		else if (component_type == TYPE_HALF)
			return GL_HALF_FLOAT;
		else if (component_type == TYPE_BYTE)
			return GL_BYTE;
		else if (component_type == TYPE_INT)
			return GL_INT;
		return GL_INVALID_ENUM;
	}

	int GPUVertexDeclarationEntry::GetEntrySize() const
	{
		int component_type = (type >> 3) & 7;

		int component_size = 0;
		if (component_type == TYPE_FLOAT)
			component_size = sizeof(float);
		else if (component_type == TYPE_DOUBLE)
			component_size = sizeof(double);
		else if (component_type == TYPE_HALF)
			component_size = sizeof(float) / 2;
		else if (component_type == TYPE_BYTE)
			component_size = sizeof(unsigned char);
		else if (component_type == TYPE_INT)
			component_size = sizeof(int32_t);

		int component_count = (type & 7);

		return component_count * component_size; 
	}

	void GPUVertexDeclaration::Push(int semantic, int semantic_index, int type)
	{
		int offset = 0;
		if (entries.size() > 0)
		{
			GPUVertexDeclarationEntry const & previous_entry = entries.back();
			offset = previous_entry.offset + previous_entry.GetEntrySize();
		}

		GPUVertexDeclarationEntry entry;
		entry.semantic       = semantic;
		entry.semantic_index = semantic_index;
		entry.type           = type;
		entry.offset         = offset;
		entries.push_back(entry);
	}

	GPUVertexDeclarationEntry * GPUVertexDeclaration::GetEntry(int semantic, int semantic_index)
	{
		for (auto & entry : entries)
			if (entry.semantic == semantic && (entry.semantic_index < 0 || entry.semantic_index == semantic_index))
				return &entry;
		return nullptr;
	}

	GPUVertexDeclarationEntry const * GPUVertexDeclaration::GetEntry(int semantic, int semantic_index) const
	{
		for (auto const & entry : entries)
			if (entry.semantic == semantic && (entry.semantic_index < 0 || entry.semantic_index == semantic_index))
				return &entry;
		return nullptr;
	}

	int GPUVertexDeclaration::GetVertexSize() const
	{
		int result = 0;
		for (auto const & entry : entries)
			result += entry.GetEntrySize();
		return result;
	}

	int GPUVertexDeclaration::GetSemanticCount(int semantic) const
	{
		int result = -1;
		for (auto const & entry : entries)
			if (entry.semantic == semantic)
				result = std::max(result, entry.semantic_index);
		return result + 1;
	}

	int GPUVertexDeclaration::GetPositionCount() const
	{
		return GetSemanticCount(SEMANTIC_POSITION);
	}

	int GPUVertexDeclaration::GetColorCount() const
	{
		return GetSemanticCount(SEMANTIC_COLOR);
	}

	int GPUVertexDeclaration::GetTextureCount() const
	{
		return GetSemanticCount(SEMANTIC_TEXCOORD);
	}

	int GPUVertexDeclaration::GetBoneCount() const
	{
		return std::min(GetSemanticCount(SEMANTIC_BONEINDEX), GetSemanticCount(SEMANTIC_BONEWEIGHT));
	}


}; // namespace chaos
