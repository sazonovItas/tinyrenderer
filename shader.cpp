#include "shader.h"
#include <cmath>
#include <glm/geometric.hpp>

uint32_t TriangleShader::fragment(Context ctx) {
  glm::vec3 intensity(0.1, 0.1, 0.1);

  for (int i = 0; i < ctx.lightCnt; i++) {
    glm::vec3 dir = glm::normalize(ctx.fragPos - ctx.lights[i]);
    float lightIntensity = std::max(0.0f, glm::dot(ctx.fragNormal, dir));
    intensity += lightIntensity * ctx.lightColors[i];
  }

  float mxIntensity =
      std::max(intensity[0], std::max(intensity[1], intensity[2]));
  if (mxIntensity > 1.0) {
    intensity /= mxIntensity;
  }

  uint32_t color = (int(intensity[0] * 255) << 16) +
                   (int(intensity[1] * 255) << 8) + int(intensity[2] * 255);

  return color;
}

uint32_t PhongShader::fragment(Context ctx) {
  glm::vec3 lightDir = glm::normalize(ctx.lightPos - ctx.fragPos);
  glm::vec3 viewDir = glm::normalize(ctx.viewPos - ctx.fragPos);

  float diffuse = std::max(0.0f, glm::dot(ctx.fragNorm, lightDir));
  glm::vec3 diffuseColor = ctx.lightColor * (diffuse * ctx.diffuse);

  glm::vec3 reflectDir = glm::reflect(-lightDir, ctx.fragNorm);
  float specular =
      pow(std::max(glm::dot(viewDir, reflectDir), 0.0f), ctx.shininess);
  glm::vec3 specularColor = ctx.lightColor * (specular * ctx.specular);

  glm::vec3 intensity = ctx.ambient + diffuseColor + specularColor;

  uint32_t color = (std::min(int(intensity[0] * 255), 255) << 16) +
                   (std::min(int(intensity[1] * 255), 255) << 8) +
                   std::min(int(intensity[2] * 255), 255);

  return color;
}
