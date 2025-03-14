#pragma once

#ifndef _GL_H_
#define _GL_H_

#include "image.h"

#include <glm/glm.hpp>

namespace gl {
void line(int x0, int y0, int x1, int y1, Image &image, uint32_t color);

void triangle(glm::vec3 *points, Image &image, ZBuffer &buffer, uint32_t color);

void halfSpaceTriangle(glm::vec3 *points, Image &image, ZBuffer &zubffer,
                       uint32_t color);

void halfSpaceTriangle(glm::vec3 *p, Image &image, ZBuffer &zbuffer,
                       glm::vec3 *normals, glm::vec3 lightDir);
}; // namespace gl

#endif
