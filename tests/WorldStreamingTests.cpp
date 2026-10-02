#include "world/World.h"
#include <iostream>

namespace{int fail(const char* message){std::cerr<<message<<'\n';return 1;}}

int main(){
  World world;world.setTaskBudgets(1,1,0);const glm::vec3 center{500.5f,8.f,500.5f};world.update(center);
  if(world.loadedChunkCount()!=1)return fail("streaming exceeded the one-generation frame budget");
  if(!world.isChunkLoadedAt(500,500))return fail("nearest player chunk was not generated first");
  if(world.skyLight(500,8,500)!=15)return fail("generated chunk did not receive its budgeted lighting pass");
  if(world.isChunkReady(31,31))return fail("chunk became render-ready before its mesh task completed");
  const auto firstPending=world.pendingTaskCount();world.update(center);
  if(world.loadedChunkCount()!=2)return fail("second update did not process exactly one more generation");
  if(world.pendingTaskCount()>firstPending+1)return fail("streaming queue accumulated duplicate tasks");

  world.setTaskBudgets(0,0,0);world.update({900.5f,8.f,900.5f});
  if(world.loadedChunkCount()!=0)return fail("distant streamed chunks were not culled");
  const auto distantPending=world.pendingTaskCount();world.update({900.5f,8.f,900.5f});
  if(world.pendingTaskCount()!=distantPending)return fail("repeated updates duplicated pending generation tasks");

  World meshWorld;meshWorld.setTaskBudgets(1,1,1);meshWorld.update({8.f,8.f,8.f});
  if(!meshWorld.isChunkReady(0,0))return fail("budgeted CPU mesh build did not mark its chunk ready");
  auto uploads=meshWorld.takeChunkMeshUploads();
  if(uploads.size()!=1||uploads.front().mesh.indices.empty())return fail("completed CPU mesh was not handed off for upload");
  if(!meshWorld.takeChunkMeshUploads().empty())return fail("CPU mesh upload hand-off was not drained");
  const auto initialIndexCount=uploads.front().mesh.indices.size();
  if(!meshWorld.setBlock(8,5,8,BlockType::STONE))return fail("mesh edit was rejected");
  meshWorld.update({8.f,8.f,8.f});
  uploads=meshWorld.takeChunkMeshUploads();
  if(uploads.size()!=1||uploads.front().mesh.indices.size()==initialIndexCount)
    return fail("dirty chunk did not hand off its rebuilt CPU mesh");
  return 0;
}
