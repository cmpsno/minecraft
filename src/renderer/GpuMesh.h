#pragma once
#include "../world/ChunkMeshData.h"
#include <glad/gl.h>

class GpuMesh {
public:
  GpuMesh()=default;
  ~GpuMesh();
  GpuMesh(const GpuMesh&)=delete;
  GpuMesh& operator=(const GpuMesh&)=delete;
  void upload(const ChunkMeshData& mesh);
  void draw()const;
private:
  GLuint m_vao=0,m_vbo=0,m_ebo=0;
  GLsizei m_count=0;
};
