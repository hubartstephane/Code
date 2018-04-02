﻿#include <chaos/RenderMaterial.h>


namespace chaos
{
  RenderMaterial::RenderMaterial()
  {

  }

  RenderMaterial::~RenderMaterial()
  {
    Release();
  }

  void RenderMaterial::Release()
  {
    program = nullptr;
    uniform_provider = nullptr;
    parent_material = nullptr;
  }


  void RenderMaterial::SetProgram(GPUProgram * in_program)
  {
    program = in_program;
  }

  void RenderMaterial::SetParentMaterial(RenderMaterial * in_parent)
  {
    parent_material = in_parent;

#if _DEBUG
    // ensure no cycle parenting
    RenderMaterial * rm = in_parent;
    while (rm != nullptr)
    {
      assert(rm != this);
      rm = rm->parent_material.get(); 
    }
#endif
  }

  void RenderMaterial::UseMaterial(GPUProgramVariableProvider const * in_uniform_provider) const
  {
    // go through the hierarchy until we get the program
    RenderMaterial const * rm = this;
    while (rm != nullptr && rm->program == nullptr)
      rm = rm->parent_material.get();
    if (rm == nullptr)
      return;

    // use the program
    rm->program->UseProgram(nullptr, nullptr);

    GPUProgramData const & program_data = rm->program->GetProgramData();
    program_data.BindUniforms(this, in_uniform_provider);
  }
  
}; // namespace chaos
