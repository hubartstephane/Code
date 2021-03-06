﻿#include <chaos/Chaos.h>

namespace chaos
{

	GPUProgram::GPUProgram(GLuint in_id): 
		program_id(in_id)
	{
		if (in_id != 0)
			program_data = GPUProgramData::GetData(in_id);
	}

	GPUProgram::~GPUProgram()
	{
		Release();
	}

	void GPUProgram::Release()
	{
		if (program_id != 0)
		{
			glDeleteProgram(program_id);
			program_id = 0;
			program_data.Clear();
		}
	}

	bool GPUProgram::UseProgram(class GPUProgramProviderBase const * uniform_provider) const
	{
		if (!IsValid())
			return false;
		glUseProgram(program_id);
		program_data.BindUniforms(uniform_provider);
		return true;
	}

}; // namespace chaos
