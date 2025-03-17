#pragma once

#ifndef _GL_H_
#define _GL_H_

#include "image.h"
#include "shader.h"

#include <glm/glm.hpp>

namespace gl {
void line(int x0, int y0, int x1, int y1, ImageBuffer &image, uint32_t color);

void triangle(glm::vec3 *points, ImageBuffer &image, ZBuffer &buffer,
              uint32_t color);

void halfSpaceTriangle(glm::vec3 *points, ImageBuffer &image, ZBuffer &zubffer,
                       uint32_t color);

void halfSpaceTriangle(glm::vec3 *p, ImageBuffer &image, ZBuffer &zbuffer,
                       Shader &shader);

}; // namespace gl

#endif
