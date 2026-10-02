#pragma once
#include <glad/gl.h>
#include "Vertex.h"
#include <vector>

class Mesh {
public:
  Mesh() = default; ~Mesh();
  Mesh(const Mesh&) = delete; Mesh& operator=(const Mesh&) = delete;
  void update(const std::vector<Vertex>& vertices, const std::vector<unsigned>& indices);
  void render() const;
private:
  GLuint m_vao=0,m_vbo=0,m_ebo=0; GLsizei m_count=0;
};
