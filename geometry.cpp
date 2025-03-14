#include "geometry.h"
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>

glm::mat4x4 geom::viewport(const int x, const int y, const int w, const int h) {
  return glm::transpose(glm::mat4x4(
      glm::vec4(w / 2.0, 0.0, 0.0, x + w / 2.0 + 0.5),
      glm::vec4(0.0, -h / 2.0, 0.0, y + h / 2.0 + 0.5),
      glm::vec4(0.0, 0.0, 1.0, 0.0), glm::vec4(0.0, 0.0, 0.0, 1.0)));
};

glm::mat4x4 geom::view(glm::vec3 forward, glm::vec3 right, glm::vec3 up) {
  forward = glm::normalize(forward);
  right = glm::normalize(right);
  up = glm::normalize(up);

  return glm::transpose(glm::mat4x4(glm::vec4(right, 0.0), glm::vec4(up, 0.0),
                                    glm::vec4(forward, 0.0),
                                    glm::vec4(0.0, 0.0, 0.0, 1.0)));
}

glm::vec3 geom::barycentric(glm::vec3 A, glm::vec3 B, glm::vec3 C,
                            glm::vec3 P) {
  glm::vec3 AC = glm::vec3(C.x - A.x, C.y - A.y, 0.0);
  glm::vec3 AB = glm::vec3(B.x - A.x, B.y - A.y, 0.0);

  float ABC = (C.x - A.x) * (B.y - A.y) - (C.y - A.y) * (B.x - A.x);
  float BCP = (C.x - P.x) * (B.y - P.y) - (C.y - P.y) * (B.x - P.x);
  float CAP = (A.x - P.x) * (C.y - P.y) - (C.x - P.x) * (A.y - P.y);

  float u = BCP / ABC, v = CAP / ABC;

  if (std::abs(1.0f - u - v) > 1e-2)
    return glm::vec3(u, v, 1.0f - u - v);

  return glm::vec3(-1, 1, 1);
}

float geom::peerDot(glm::vec2 a, glm::vec2 b) { return a.x * b.y - a.y * b.x; }
