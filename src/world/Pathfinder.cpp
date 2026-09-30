#include "Pathfinder.h"
#include "World.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>
#include <unordered_map>

namespace{
struct Key{int x,y,z;bool operator==(const Key& other)const{return x==other.x&&y==other.y&&z==other.z;}};
struct KeyHash{std::size_t operator()(const Key& key)const{std::size_t h=static_cast<unsigned>(key.x);h=(h*16777619u)^static_cast<unsigned>(key.y);return(h*16777619u)^static_cast<unsigned>(key.z);}};
struct OpenNode{Key key;float score;bool operator<(const OpenNode& other)const{return score>other.score;}};
Key key(const glm::ivec3& value){return{value.x,value.y,value.z};}
glm::ivec3 position(const Key& value){return{value.x,value.y,value.z};}
float heuristic(const glm::ivec3& a,const glm::ivec3& b){return static_cast<float>(std::abs(a.x-b.x)+std::abs(a.z-b.z))+std::abs(a.y-b.y)*1.6f;}

bool isTraversableGround(const World& world,int x,int y,int z){
  const auto down=world.getBlock(x,y-1,z); const auto current=world.getBlock(x,y,z); const auto up=world.getBlock(x,y+1,z);
  return isSolid(down)&&!isSolid(current)&&!isSolid(up);
}

bool isNodeOpen(const World& world,const glm::ivec3& node){
  if(node.y<0||node.y>=256)return false;
  const auto below=world.getBlock(node.x,node.y-1,node.z);
  const auto current=world.getBlock(node.x,node.y,node.z);
  const auto above=world.getBlock(node.x,node.y+1,node.z);
  return isSolid(below)&&!isSolid(current)&&!isSolid(above);
}

bool isBlockedByCeiling(const World& world,const glm::ivec3& node){
  return isSolid(world.getBlock(node.x,node.y,node.z))||isSolid(world.getBlock(node.x,node.y+1,node.z));
}

bool isBlockedByDepartureCeiling(const World& world,const glm::ivec3& current,const glm::ivec3& next){
  if(next.y<=current.y)return false;
  return isSolid(world.getBlock(current.x,current.y+2,current.z))||isSolid(world.getBlock(next.x,next.y+1,next.z));
}

bool isSurroundedBySolid(const World& world,const glm::ivec3& position){
  static constexpr int directions[4][2]={{1,0},{-1,0},{0,1},{0,-1}};
  int blocked=0;
  for(const auto& direction:directions){
    const glm::ivec3 side{position.x+direction[0],position.y,position.z+direction[1]};
    if(!isNodeOpen(world,side) || isSolid(world.getBlock(side.x,side.y,side.z)) || isSolid(world.getBlock(side.x,side.y+1,side.z)))++blocked;
  }
  return blocked==4;
}

bool isBlockingStep(const World& world,const glm::ivec3& current,const glm::ivec3& next){
  if(!isNodeOpen(world,next))return true;
  const glm::ivec3 delta=next-current;
  if(std::abs(delta.x)+std::abs(delta.z)==0)return true;
  if(delta.x!=0&&delta.z!=0){
    const int dx=delta.x>0?1:-1,dz=delta.z>0?1:-1;
    if(!isNodeOpen(world,{current.x+dx,current.y,current.z})||!isNodeOpen(world,{current.x,current.y,current.z+dz}))return true;
  }
  return false;
}
}

struct Pathfinder::NodeInfo{
  bool walkable=false;
  bool blockAbove=false;
  bool hazardous=false;
  float cost=1.f;
};

bool Pathfinder::isWalkable(const World& world,const glm::ivec3& feet){
  return feet.y>=0&&feet.y<256&&isTraversableGround(world,feet.x,feet.y,feet.z);
}

Pathfinder::NodeInfo Pathfinder::makeNodeInfo(const World& world,const glm::ivec3& position){
  const auto type=world.getBlock(position.x,position.y,position.z);
  const auto belowType=world.getBlock(position.x,position.y-1,position.z);
  const auto aboveType=world.getBlock(position.x,position.y+1,position.z);
  const BlockProperties& block=world.getBlockProperties(position.x,position.y,position.z);
  const BlockProperties& below=world.getBlockProperties(position.x,position.y-1,position.z);
  NodeInfo info{};
  info.walkable = isSolid(belowType) && !isSolid(type) && !isSolid(aboveType);
  info.blockAbove = isSolid(type) || isSolid(aboveType);
  info.hazardous = block.isHazard;
  info.cost = std::max(1.f, block.movementCost);
  if((type==BlockType::GRASS||type==BlockType::DIRT||type==BlockType::SAND)&&!isSolid(type))info.cost=1.05f;
  if(isSolid(belowType)&&below.isHazard)info.cost += 3.f;
  return info;
}

float Pathfinder::terrainCost(const World& world,const glm::ivec3& position,const glm::ivec3& previous){
  const auto here=world.getBlock(position.x,position.y,position.z);
  const auto below=world.getBlock(position.x,position.y-1,position.z);
  float cost=1.f;
  if(here==BlockType::GRASS||here==BlockType::DIRT)cost=1.05f;
  if(here==BlockType::SAND)cost=1.3f;
  if(here==BlockType::LEAVES)cost=1.5f;
  if(isSolid(below)&&world.getBlockProperties(position.x,position.y-1,position.z).isHazard)cost+=4.f;
  if(position.y!=previous.y)cost+=0.5f + std::abs(position.y-previous.y)*0.4f;
  if(world.getBlock(position.x,position.y+1,position.z)==BlockType::AIR)cost += 0.05f;
  return cost;
}

Path Pathfinder::findPathData(const World& world,const glm::ivec3& start,const glm::ivec3& target,int maxRange,int maxVisited){
  Path path;
  const auto nodes=findPath(world,start,target,maxRange,maxVisited);
  path.nodes.reserve(nodes.size());
  for(const auto& node:nodes)path.nodes.push_back({node.x,node.y,node.z,0.f});
  return path;
}

std::vector<glm::ivec3> Pathfinder::findPath(const World& world,const glm::ivec3& start,const glm::ivec3& target,int maxRange,int maxVisited){
  if(maxRange<=0||maxVisited<=0)return{};
  if(!isWalkable(world,start)||!isWalkable(world,target))return{};
  std::priority_queue<OpenNode> open;
  std::unordered_map<Key,float,KeyHash> cost;
  std::unordered_map<Key,Key,KeyHash> parent;
  const Key startKey=key(start),targetKey=key(target);
  cost[startKey]=0.f;
  open.push({startKey,heuristic(start,target)});
  int visited=0;
  while(!open.empty()&&visited<maxVisited){
    const Key currentKey=open.top().key; open.pop();
    const glm::ivec3 current=position(currentKey); ++visited;
    if(currentKey==targetKey){
      std::vector<glm::ivec3> path;
      Key step=currentKey;
      while(!(step==startKey)){
        path.push_back(position(step));
        step=parent.at(step);
      }
      path.push_back(start);
      std::reverse(path.begin(),path.end());
      return path;
    }
    static constexpr int directions[8][3]={{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1},{1,0,1},{-1,0,-1}};
    for(const auto& direction:directions){
      glm::ivec3 next{current.x+direction[0],current.y+direction[1],current.z+direction[2]};
      const int dx=std::abs(next.x-start.x),dz=std::abs(next.z-start.z),dy=std::abs(next.y-start.y);
      if(dx>maxRange||dz>maxRange||dy>maxRange)continue;
      if((next.x!=current.x||next.z!=current.z)&&!isNodeOpen(world,next)){
        if(isNodeOpen(world,next+glm::ivec3(0,1,0)) && !isBlockedByCeiling(world,next+glm::ivec3(0,1,0)) && !isBlockedByDepartureCeiling(world,current,next+glm::ivec3(0,1,0))) {
          next.y += 1;
        } else if(isNodeOpen(world,next-glm::ivec3(0,1,0)) && !isBlockedByDepartureCeiling(world,current,next-glm::ivec3(0,1,0))) {
          next.y -= 1;
        } else {
          continue;
        }
      }
      if(!isNodeOpen(world,next))continue;
      if(isBlockedByCeiling(world,next))continue;
      if(next.y>current.y && isBlockedByDepartureCeiling(world,current,next))continue;
      if(std::abs(next.x-current.x)>0 && std::abs(next.z-current.z)>0 && isSurroundedBySolid(world,next))continue;
      const float stepCost = terrainCost(world,next,current);
      if(stepCost<=0.f)continue;
      const float nextCost = cost[currentKey] + stepCost;
      const Key nextKey=key(next);
      const auto existing=cost.find(nextKey);
      if(existing!=cost.end()&&existing->second<=nextCost)continue;
      cost[nextKey]=nextCost;
      parent[nextKey]=currentKey;
      open.push({nextKey,nextCost+heuristic(next,target)});
    }
  }
  return{};
}
