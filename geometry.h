#pragma once

#ifndef _GEOMETRY_H_
#define _GEOMETRY_H_

#include <glm/glm.hpp>

namespace geom {
glm::mat4x4 viewport(const int x, const int y, const int w, const int h);

glm::mat4x4 view(glm::vec3 forward, glm::vec3 right, glm::vec3 up);

glm::vec3 barycentric(glm::vec3 A, glm::vec3 B, glm::vec3 C, glm::vec3 P);

float peerdot(glm::vec2 a, glm::vec2 b);

glm::vec3 calcTangent(glm::vec3 *vs, glm::vec2 *uvs);
}; // namespace geom

#endif
