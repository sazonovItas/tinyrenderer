#include "gl.h"
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/geometric.hpp>

glm::mat4x4 gl::viewport(const int x, const int y, const int w, const int h) {
  return glm::transpose(glm::mat4x4(glm::vec4(w / 2.0, 0.0, 0.0, x + w / 2.0),
                                    glm::vec4(0.0, -h / 2.0, 0.0, y + h / 2.0),
                                    glm::vec4(0.0, 0.0, 1.0, 0.0),
                                    glm::vec4(0.0, 0.0, 0.0, 1.0)));
};

glm::mat4x4 gl::view(glm::vec3 forward, glm::vec3 right, glm::vec3 up) {
  forward = glm::normalize(forward);
  right = glm::normalize(right);
  up = glm::normalize(up);

  return glm::transpose(glm::mat4x4(glm::vec4(right, 0.0), glm::vec4(up, 0.0),
                                    glm::vec4(forward, 0.0),
                                    glm::vec4(0.0, 0.0, 0.0, 1.0)));
}

void gl::line(int x0, int y0, int x1, int y1, Image &image, uint32_t color) {
  bool steep = false;
  if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
    std::swap(x0, y0);
    std::swap(x1, y1);
    steep = true;
  }
  if (x0 > x1) {
    std::swap(x0, x1);
    std::swap(y0, y1);
  }
  int dx = x1 - x0;
  int dy = y1 - y0;
  int derror2 = std::abs(dy) * 2;
  int error2 = 0;
  int y = y0;
  for (int x = x0; x <= x1; x++) {
    if (steep) {
      image.set(y, x, color);
    } else {
      image.set(x, y, color);
    }
    error2 += derror2;
    if (error2 > dx) {
      y += (y1 > y0 ? 1 : -1);
      error2 -= dx * 2;
    }
  }
}
