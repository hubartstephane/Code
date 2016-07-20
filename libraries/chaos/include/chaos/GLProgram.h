#pragma once

#include <chaos/StandardHeaders.h>
#include <chaos/GraphicResource.h>

namespace chaos
{

class GLProgram : public GraphicResource
{
public:

  /** constructor */
  GLProgram();
  /** destructor */
  virtual ~GLProgram();

  /** cleaning the object */
  virtual void Release();

  /** returns the GL name of the resource */
  GLuint GetResourceID() const { return program_id; }

  /** returns true whether the resource is valid */
  bool IsValid() const { return glIsProgram(program_id) == GL_TRUE;}

protected:

  /** the resource id */
  GLuint program_id;
};

}; // namespace chaos
