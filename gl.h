#pragma once

#include <glm/fwd.hpp>
#ifndef _GL_H_
#define _GL_H_

#include "image.h"

#include <glm/glm.hpp>

namespace gl {
glm::mat4x4 viewport(const int x, const int y, const int w, const int h);

void line(int x0, int y0, int x1, int y1, Image &image, uint32_t color);

glm::mat4x4 view(glm::vec3 forward, glm::vec3 right, glm::vec3 up);
}; // namespace gl

#endif
