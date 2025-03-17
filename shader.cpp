#include "shader.h"
#include "color.h"

#include <cmath>
#include <glm/common.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/color_space.hpp>
#include <glm/matrix.hpp>

uint32_t TriangleShader::fragment(glm::vec3 position) {
  glm::vec3 intensity(0.1, 0.1, 0.1);

  for (int i = 0; i < _ctx.lightCnt; i++) {
    glm::vec3 dir = glm::normalize(_ctx.fragPos - _ctx.lights[i]);
    float lightIntensity = std::max(0.0f, glm::dot(_ctx.fragNormal, dir));
    intensity += lightIntensity * _ctx.lightColors[i];
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

  return Color(intensity.r, intensity.g, intensity.b).color();
}

void PhongShader::setContext(Context ctx) { _ctx = ctx; }

uint32_t TextureShader::fragment(glm::vec3 position) {
  glm::vec3 fragPos = _ctx.worldVs[0] * position.x +
                      _ctx.worldVs[1] * position.y +
                      _ctx.worldVs[2] * position.z;
  glm::vec3 fragNorm = glm::normalize(position.x * _ctx.normals[0] +
                                      position.y * _ctx.normals[1] +
                                      position.z * _ctx.normals[2]);
  glm::vec2 uv = (position.x * _ctx.uvs[0] / _ctx.vs[0].w +
                  position.y * _ctx.uvs[1] / _ctx.vs[1].w +
                  position.z * _ctx.uvs[2] / _ctx.vs[2].w) /
                 (position.x / _ctx.vs[0].w + position.y / _ctx.vs[1].w +
                  position.z / _ctx.vs[2].w);

  uv = glm::clamp(uv, glm::vec2(0.0, 0.0), glm::vec2(1.0, 1.0));

  glm::vec3 diffuse = _ctx.diffuse.getColorUV(uv.x, 1.0 - uv.y);
  glm::vec3 specular = _ctx.specular.getColorUV(uv.x, 1.0 - uv.y);

  glm::vec3 tangent = glm::normalize(
      _ctx.tangent - glm::dot(_ctx.tangent, fragNorm) * fragNorm);
  glm::vec3 bitangent = glm::cross(fragNorm, tangent);
  glm::mat3x3 TBN(tangent, bitangent, fragNorm);
  glm::vec3 normal =
      TBN * (2.0f * _ctx.normal.getColorUV(uv.x, 1.0 - uv.y) - 1.0f);

  glm::vec3 lightDir = glm::normalize(_ctx.lightPos - fragPos);
  glm::vec3 viewDir = glm::normalize(_ctx.viewPos - fragPos);

  glm::vec3 reflectDir = glm::reflect(-lightDir, normal);
  float specularK = pow(std::max(glm::dot(viewDir, reflectDir), 0.0f), 0.6f);
  glm::vec3 specularColor = _ctx.lightColor * (specularK * specular);

  float diffuseK = std::max(0.0f, glm::dot(normal, lightDir));
  glm::vec3 diffuseColor = _ctx.lightColor * (diffuseK * diffuse);

  glm::vec3 intensity = diffuseColor + specularColor;

  return Color(intensity.r, intensity.g, intensity.b).color();
}

void TextureShader::setContext(Context ctx) { _ctx = ctx; }
