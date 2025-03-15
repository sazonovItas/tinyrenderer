#include "shader.h"
#include "color.h"

#include <cmath>
#include <glm/geometric.hpp>

uint32_t TriangleShader::fragment(glm::vec3 position) {
  glm::vec3 intensity(0.1, 0.1, 0.1);

  for (int i = 0; i < _ctx.lightCnt; i++) {
    glm::vec3 dir = glm::normalize(_ctx.fragPos - _ctx.lights[i]);
    float lightIntensity = std::max(0.0f, glm::dot(_ctx.fragNormal, dir));
    intensity += lightIntensity * _ctx.lightColors[i];
  }

  float mxIntensity =
      std::max(intensity[0], std::max(intensity[1], intensity[2]));
  if (mxIntensity > 1.0) {
    intensity /= mxIntensity;
  }

  return Color(intensity.x, intensity.y, intensity.z).color();
}

void TriangleShader::setContext(Context ctx) { _ctx = ctx; }

uint32_t PhongShader::fragment(glm::vec3 position) {
  glm::vec3 fragPos = _ctx.worldVs[0] * position.x +
                      _ctx.worldVs[1] * position.y +
                      _ctx.worldVs[2] * position.z;
  glm::vec3 fragNorm = glm::normalize(position.x * _ctx.normals[0] +
                                      position.y * _ctx.normals[1] +
                                      position.z * _ctx.normals[2]);

  glm::vec3 lightDir = glm::normalize(_ctx.lightPos - fragPos);
  glm::vec3 viewDir = glm::normalize(_ctx.viewPos - fragPos);

  float diffuse = std::max(0.0f, glm::dot(fragNorm, lightDir));
  glm::vec3 diffuseColor = _ctx.lightColor * (diffuse * _ctx.diffuse);

  glm::vec3 reflectDir = glm::reflect(-lightDir, fragNorm);
  float specular =
      pow(std::max(glm::dot(viewDir, reflectDir), 0.0f), _ctx.shininess);
  glm::vec3 specularColor = _ctx.lightColor * (specular * _ctx.specular);

  glm::vec3 intensity = _ctx.ambient + diffuseColor + specularColor;

  return Color(intensity.x, intensity.y, intensity.z).color();
}

void PhongShader::setContext(Context ctx) { _ctx = ctx; }
