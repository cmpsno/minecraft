#pragma once
#include "GpuMesh.h"
#include <map>
#include <utility>

class World;

class ChunkRenderer {
public:
  void render(World& world);
private:
  using ChunkKey=std::pair<int,int>;
  std::map<ChunkKey,GpuMesh> m_meshes;
};
