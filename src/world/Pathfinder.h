#pragma once
#include <glm/glm.hpp>
#include <vector>

class World;

struct PathNode {
  int x=0,y=0,z=0;
  float cost=0.f;
  bool operator==(const PathNode& other)const{return x==other.x&&y==other.y&&z==other.z;}
};

struct Path {
  std::vector<PathNode> nodes;
  bool empty()const{return nodes.empty();}
  std::size_t size()const{return nodes.size();}
  PathNode back()const{return nodes.empty()?PathNode{}:nodes.back();}
};

class Pathfinder{
public:
  static std::vector<glm::ivec3> findPath(const World& world,const glm::ivec3& start,const glm::ivec3& target,int maxRange=16,int maxVisited=256);
  static Path findPathData(const World& world,const glm::ivec3& start,const glm::ivec3& target,int maxRange=16,int maxVisited=256);
  static bool isWalkable(const World& world,const glm::ivec3& feet);

private:
  struct NodeInfo;
  static NodeInfo makeNodeInfo(const World& world,const glm::ivec3& position);
  static float terrainCost(const World& world,const glm::ivec3& position,const glm::ivec3& previous);
};
