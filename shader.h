#pragma once

#ifndef _SHADER_H_
#define _SHADER_H_

#include "image.h"

#include <cstdint>
#include <glm/glm.hpp>

class Shader {
public:
  virtual uint32_t fragment(glm::vec3 position) = 0;
};

class TriangleShader : public Shader {
public:
  struct Context {
    int lightCnt;
    glm::vec3 *lights;
    glm::vec3 *lightColors;

    glm::vec3 fragPos, fragNormal;
  };

  void setContext(Context ctx);
  uint32_t fragment(glm::vec3 position) override;

private:
  Context _ctx;
};

class PhongShader : public Shader {
public:
  struct Context {
    glm::vec3 *normals;
    glm::vec3 *worldVs;

    glm::vec3 viewPos;

    glm::vec3 lightPos, lightColor;
    glm::vec3 ambient, diffuse, specular;
    float shininess;
  };

  void setContext(Context ctx);
  uint32_t fragment(glm::vec3 position) override;

private:
  Context _ctx;
};

class TextureShader : public Shader {
public:
  struct Context {
    glm::vec4 *vs;
    glm::vec2 *uvs;

    glm::vec3 viewPos;

    glm::vec3 lightPos, lightColor;
    Image diffuse, specular, normal;
  };

  void setContext(Context ctx);
  uint32_t fragment(glm::vec3 position) override;

private:
  Context _ctx;
};

#endif
