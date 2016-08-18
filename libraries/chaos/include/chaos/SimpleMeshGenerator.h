#pragma once

#include <chaos/GeometryFramework.h>
#include <chaos/StandardHeaders.h>
#include <chaos/SimpleMesh.h>
#include <chaos/MemoryBufferWriter.h>


namespace chaos
{

  /**
   * A class to describe requirement for a mesh
   */

class MeshGenerationRequirement
{
public:

  /** constructor */
  MeshGenerationRequirement():
    vertices_count(0), 
    indices_count(0){}

  /** test whether the requirement is valid */
  bool IsValid() const;

public: 

  /** number of vertices required */
  int vertices_count;
  /** number of indices required */
  int indices_count;
};

  /**
   * SimpleMeshGeneratorProxy : an object that is responsible for generating the mesh data
   */

class SimpleMeshGeneratorProxy
{
public:

  /** the destructor */
  virtual ~SimpleMeshGeneratorProxy(){}
  /** get requirement */
  virtual MeshGenerationRequirement GetRequirement() const = 0;
  /** get the vertex declaration */
  virtual void GenerateVertexDeclaration(VertexDeclaration & declaration) const = 0;
  /** get the mesh data */
  virtual void GenerateMeshData(std::vector<MeshPrimitive> & primitives, MemoryBufferWriter & vertices_buffer, MemoryBufferWriter & indices_buffer) const = 0;
};

  /**
   * A base class for mesh generators
   */

class SimpleMeshGenerator
{
public:

  /** generation function */
  boost::intrusive_ptr<SimpleMesh> GenerateMesh() const;

protected:

  /** generate a proxy */
  virtual SimpleMeshGeneratorProxy * CreateProxy() const = 0;
};

/**
 * QuadMeshGenerator : help defines mesh as simple quad
 */

class QuadMeshGenerator : public SimpleMeshGenerator
{
  friend class QuadMeshGeneratorProxy;

public:

  /** constructor */
  QuadMeshGenerator(box2 const & in_primitive): 
    primitive(in_primitive) {}

  /** generate a proxy */
  virtual SimpleMeshGeneratorProxy * CreateProxy() const;
 
protected:

  /** the box to generate */
  box2 primitive;
};

class QuadMeshGeneratorProxy : public SimpleMeshGeneratorProxy
{
public:

  /** the constructor */
  QuadMeshGeneratorProxy(QuadMeshGenerator const & in_generator): generator(in_generator){}
  /** get requirement */
  virtual MeshGenerationRequirement GetRequirement() const override;
  /** get the vertex declaration */
  virtual void GenerateVertexDeclaration(VertexDeclaration & declaration) const override;
  /** get the mesh data */
  virtual void GenerateMeshData(std::vector<MeshPrimitive> & primitives, MemoryBufferWriter & vertices_buffer, MemoryBufferWriter & indices_buffer) const override;

protected:

  /** the generator */
  QuadMeshGenerator generator;

  /** the vertices defining a face facing planes inside [-1, +1] */
  static glm::vec3 const vertices[4];
  /** the triangles indices defining a face facing planes */
  static GLuint const triangles[6];
};

/**
 * CubeMeshGenerator : help defines cube mesh 
 */

class CubeMeshGenerator : public SimpleMeshGenerator
{

  friend class CubeMeshGeneratorProxy;

public:

  /** constructor */
  CubeMeshGenerator(box3 const & in_primitive, bool in_with_normals):
    primitive(in_primitive),
    with_normals(in_with_normals){}

  /** generate a proxy */
  virtual SimpleMeshGeneratorProxy * CreateProxy() const;

protected:

  /** the box to generate */
  box3 primitive;
  /** whether normals are to be generated */
  bool with_normals;
};

/**
 * CubeMeshGeneratorProxy : proxy for CubeMeshGenerator
 */

class CubeMeshGeneratorProxy : public SimpleMeshGeneratorProxy
{
public:

  /** the constructor */
  CubeMeshGeneratorProxy(CubeMeshGenerator const & in_generator) : generator(in_generator) {}
  /** get requirement */
  virtual MeshGenerationRequirement GetRequirement() const override;
  /** get the vertex declaration */
  virtual void GenerateVertexDeclaration(VertexDeclaration & declaration) const override;
  /** get the mesh data */
  virtual void GenerateMeshData(std::vector<MeshPrimitive> & primitives, MemoryBufferWriter & vertices_buffer, MemoryBufferWriter & indices_buffer) const override;

protected:

  /** the generator */
  CubeMeshGenerator generator;

  /** the vertices defining a cube */
  static glm::vec3 const vertices[8];
  /** the triangles defining a cube */
  static GLuint const triangles[36];

  /** the vertices defining a cube */
  static glm::vec3 const vertices_with_normals[24 * 2];
  /** the triangles defining a cube */
  static GLuint const triangles_with_normals[36];
};


/**
* SphereMeshGenerator : help defines mesh as simple sphere
*/

class SphereMeshGenerator : public SimpleMeshGenerator
{

  friend class SphereMeshGeneratorProxy;

public:

  /** constructor */
  SphereMeshGenerator(sphere3 const & in_primitive, int in_subdivisions) :
    primitive(in_primitive),
    subdivisions(in_subdivisions){}

  /** generate a proxy */
  virtual SimpleMeshGeneratorProxy * CreateProxy() const;

protected:

  /** the sphere to generate */
  sphere3 primitive;
  /** number of subdivisions */
  int subdivisions;
};

/**
 * SphereMeshGeneratorProxy : proxy for SphereMeshGenerator
 */

class SphereMeshGeneratorProxy : public SimpleMeshGeneratorProxy
{
public:

  /** the constructor */
  SphereMeshGeneratorProxy(SphereMeshGenerator const & in_generator) : generator(in_generator) {}
  /** get requirement */
  virtual MeshGenerationRequirement GetRequirement() const override;
  /** get the vertex declaration */
  virtual void GenerateVertexDeclaration(VertexDeclaration & declaration) const override;
  /** get the mesh data */
  virtual void GenerateMeshData(std::vector<MeshPrimitive> & primitives, MemoryBufferWriter & vertices_buffer, MemoryBufferWriter & indices_buffer) const override;

protected:

  /** get a vertex on the sphere from polar angle */
  float3 GetSphereVertex(float alpha, float beta) const;

protected:

  SphereMeshGenerator generator;
};


}; // namespace chaos

