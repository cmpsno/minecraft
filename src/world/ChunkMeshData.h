#pragma once
#include "../renderer/Vertex.h"
#include <vector>

struct ChunkMeshData {
  std::vector<Vertex> vertices;
  std::vector<unsigned> indices;
};
