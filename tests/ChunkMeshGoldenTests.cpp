#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "world/Chunk.h"
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>

namespace {
struct Fingerprint {
  std::size_t vertices;
  std::size_t indices;
  std::uint64_t vertexHash;
  std::uint64_t indexHash;
};

std::uint64_t checksum(const void* data,std::size_t size){
  const auto* bytes=static_cast<const unsigned char*>(data);
  std::uint64_t hash=14695981039346656037ull;
  for(std::size_t i=0;i<size;++i){hash^=bytes[i];hash*=1099511628211ull;}
  return hash;
}

auto worldBlock(Chunk& chunk){
  const auto position=chunk.position();
  return [&chunk,position](int x,int y,int z){
    return chunk.getBlock(x-position.x*Chunk::SIZE_X,y,z-position.y*Chunk::SIZE_Z);
  };
}

Fingerprint meshFingerprint(Chunk& chunk){
  chunk.generateMesh(worldBlock(chunk));
  GLint vertexBuffer=0,vertexBytes=0;
  glGetIntegerv(GL_ARRAY_BUFFER_BINDING,&vertexBuffer);
  glGetBufferParameteriv(GL_ARRAY_BUFFER,GL_BUFFER_SIZE,&vertexBytes);
  std::vector<Vertex> vertices(static_cast<std::size_t>(vertexBytes)/sizeof(Vertex));
  glGetBufferSubData(GL_ARRAY_BUFFER,0,vertexBytes,vertices.data());

  chunk.render();
  GLint indexBytes=0;
  glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER,GL_BUFFER_SIZE,&indexBytes);
  std::vector<unsigned> indices(static_cast<std::size_t>(indexBytes)/sizeof(unsigned));
  glGetBufferSubData(GL_ELEMENT_ARRAY_BUFFER,0,indexBytes,indices.data());
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER,0);
  return {vertices.size(),indices.size(),checksum(vertices.data(),vertices.size()*sizeof(Vertex)),checksum(indices.data(),indices.size()*sizeof(unsigned))};
}

void makeSuperflat(Chunk& chunk){
  for(int z=0;z<Chunk::SIZE_Z;++z)for(int x=0;x<Chunk::SIZE_X;++x){
    chunk.setBlock(x,0,z,BlockType::BEDROCK);
    for(int y=1;y<=3;++y)chunk.setBlock(x,y,z,BlockType::DIRT);
    chunk.setBlock(x,4,z,BlockType::GRASS);
  }
}

}

int main(){
  static_assert(sizeof(Vertex)==sizeof(float)*6,"vertex buffer layout must remain six floats");
  static_assert(sizeof(unsigned)==sizeof(std::uint32_t),"index buffer layout must remain 32-bit");
  if(!glfwInit())return 77;
  glfwWindowHint(GLFW_VISIBLE,GLFW_FALSE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
  glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
  GLFWwindow* window=glfwCreateWindow(64,64,"Chunk mesh golden tests",nullptr,nullptr);
  if(!window){glfwTerminate();return 77;}
  glfwMakeContextCurrent(window);
  if(!gladLoadGL(reinterpret_cast<GLADloadfunc>(glfwGetProcAddress))){glfwDestroyWindow(window);glfwTerminate();return 77;}
  int result=0;
  {
    Chunk flat({0,0});makeSuperflat(flat);
    const Fingerprint flatExpected{3328,4992,0x3539234e5ebf9dd1ull,0x05ecadf1f8948b25ull};
    const auto flatActual=meshFingerprint(flat);
    if(flatActual.vertices!=flatExpected.vertices||flatActual.indices!=flatExpected.indices||
       flatActual.vertexHash!=flatExpected.vertexHash||flatActual.indexHash!=flatExpected.indexHash){
      std::cerr<<"superflat mesh output changed\n";result=1;
    }
    Chunk edited({0,0});makeSuperflat(edited);
    edited.setBlock(0,5,0,BlockType::OAK_LOG);
    edited.setBlock(0,6,0,BlockType::LEAVES);
    edited.setBlock(15,5,15,BlockType::STONE);
    edited.setBlock(15,6,15,BlockType::GLASS);
    const Fingerprint editedExpected{3392,5088,0x205dea95e3f352bdull,0x2aa844935d733725ull};
    const auto editedActual=meshFingerprint(edited);
    if(editedActual.vertices!=editedExpected.vertices||editedActual.indices!=editedExpected.indices||
       editedActual.vertexHash!=editedExpected.vertexHash||editedActual.indexHash!=editedExpected.indexHash){
      std::cerr<<"edge-edited mesh output changed\n";result=1;
    }
  }
  if(glGetError()!=GL_NO_ERROR){std::cerr<<"OpenGL error while reading mesh buffers\n";result=1;}
  glfwDestroyWindow(window);
  glfwTerminate();
  return result;
}
