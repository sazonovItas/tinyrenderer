#pragma once

#ifndef _SHADER_H_
#define _SHADER_H_

#include <cstdint>
#include <glm/glm.hpp>

class TriangleShader {
public:
  struct Context {
    int lightCnt;
    glm::vec3 *lights;
    glm::vec3 *lightColors;

    glm::vec3 fragPos, fragNormal;
  };

  static uint32_t fragment(Context ctx);

private:
  Context _ctx;
};

class PhongShader {
public:
  struct Context {
    glm::vec3 viewPos;
    glm::vec3 fragPos, fragNorm;

    glm::vec3 lightPos, lightColor;
    glm::vec3 ambient, diffuse, specular;
    float shininess;
  };

  uint32_t fragment(Context ctx);

private:
  Context _ctx;
};

#endif
