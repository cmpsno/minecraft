#include "ChunkRenderer.h"
#include "../world/World.h"
#include <set>

void ChunkRenderer::render(World& world){
  std::set<ChunkKey> loaded;
  for(const auto& position:world.loadedChunkPositions())loaded.emplace(position.x,position.y);
  for(auto it=m_meshes.begin();it!=m_meshes.end();)
    if(!loaded.count(it->first))it=m_meshes.erase(it);
    else ++it;

  for(const auto& upload:world.takeChunkMeshUploads()){
    const ChunkKey key{upload.position.x,upload.position.y};
    m_meshes.try_emplace(key).first->second.upload(upload.mesh);
  }
  for(const auto& mesh:m_meshes)mesh.second.draw();
}
