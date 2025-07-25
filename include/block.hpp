#pragma once
#include "glm/glm.hpp"
class Block
{
  public:
    glm::vec3 pos;
    Block() : pos(0.0f, 0.0f, 0.0f) {}
    Block(float x, float y, float z) : pos(x, y, z) {}
    Block(glm::vec3 pos) : pos(pos) {}
};
