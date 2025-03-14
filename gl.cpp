#include "gl.h"
#include "geometry.h"
#include <cmath>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <limits>

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

void gl::triangle(glm::vec3 *points, Image &image, ZBuffer &zbuffer,
                  uint32_t color) {
  glm::vec2 bboxmin(std::numeric_limits<float>::max(),
                    std::numeric_limits<float>::max());
  glm::vec2 bboxmax(-std::numeric_limits<float>::max(),
                    -std::numeric_limits<float>::max());
  glm::vec2 clamp(image.width, image.height);

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 2; j++) {
      bboxmin[j] = std::max(0.0f, std::min(bboxmin[j], points[i][j]));
      bboxmax[j] =
          std::min(clamp[j], std::max(bboxmax[j], points[i][j] + 0.5f));
    }
  }

  glm::vec3 P;
  for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x += 1) {
    for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y += 1) {

      glm::vec3 bc = geom::barycentric(points[0], points[1], points[2], P);
      if (bc.x < 0 || bc.y < 0 || bc.z < 0)
        continue;

      P.z = 0;
      for (int i = 0; i < 3; i++)
        P.z += points[i][2] * bc[i];

      if (zbuffer.set(std::ceil(P.x), std::ceil(P.y), P.z)) {
        image.set(std::ceil(P.x), std::ceil(P.y), color);
      }
    }
  }
}

void gl::halfSpaceTriangle(glm::vec3 *p, Image &image, ZBuffer &zbuffer,
                           uint32_t color) {
  int xMin =
      std::max(0, (int)std::ceil(std::min(p[0].x, std::min(p[1].x, p[2].x))));
  int yMin =
      std::max(0, (int)std::ceil(std::min(p[0].y, std::min(p[1].y, p[2].y))));
  int xMax = std::min(
      image.width, (int)std::ceil(std::max(p[0].x, std::max(p[1].x, p[2].x))));
  int yMax = std::min(
      image.height, (int)std::ceil(std::max(p[0].y, std::max(p[1].y, p[2].y))));

  glm::vec2 p1p2 = p[1] - p[0];
  glm::vec2 p2p3 = p[2] - p[1];
  glm::vec2 p3p1 = p[0] - p[2];

  float denom = 1 / geom::peerDot(-p3p1, p1p2);

  bool f1 = p2p3.y > 0 || (p2p3.y == 0 && p2p3.x < 0);
  bool f2 = p3p1.y > 0 || (p3p1.y == 0 && p3p1.x < 0);
  bool f3 = p1p2.y > 0 || (p1p2.y == 0 && p1p2.x < 0);

  glm::vec2 pp1 = {p[0].x - xMin, p[0].y - yMin};
  glm::vec2 pp2 = {p[1].x - xMin, p[1].y - yMin};
  glm::vec2 pp3 = {p[2].x - xMin, p[2].y - yMin};

  glm::vec3 b0 = denom * glm::vec3{
                             geom::peerDot(pp3, pp2),
                             geom::peerDot(pp1, pp3),
                             geom::peerDot(pp2, pp1),
                         };

  glm::vec3 dbdx = denom * glm::vec3{
                               pp3.y - pp2.y,
                               pp1.y - pp3.y,
                               pp2.y - pp1.y,
                           };

  glm::vec3 dbdy = denom * glm::vec3{
                               pp2.x - pp3.x,
                               pp3.x - pp1.x,
                               pp1.x - pp2.x,
                           };

  for (int y = yMin; y < yMax; y++, b0 += dbdy) {
    auto b = b0;
    for (int x = xMin; x < xMax; x++, b += dbdx) {
      if ((b.x > 0 && b.y > 0 && b.z >= 0) || (b.x == 0 && f1) ||
          (b.y == 0 && f2) || (b.z == 0 && f3)) {
        float z = b.x * p[0].z + b.y * p[1].z + p[2].z * b.z;

        if (zbuffer.set(x, y, z)) {
          image.set(x, y, color);
        }
      }
    }
  }
}

void gl::halfSpaceTriangle(glm::vec3 *p, Image &image, ZBuffer &zbuffer,
                           glm::vec3 *normals, glm::vec3 lightDir) {
  int xMin =
      std::max(0, (int)std::ceil(std::min(p[0].x, std::min(p[1].x, p[2].x))));
  int yMin =
      std::max(0, (int)std::ceil(std::min(p[0].y, std::min(p[1].y, p[2].y))));
  int xMax = std::min(
      image.width, (int)std::ceil(std::max(p[0].x, std::max(p[1].x, p[2].x))));
  int yMax = std::min(
      image.height, (int)std::ceil(std::max(p[0].y, std::max(p[1].y, p[2].y))));

  glm::vec2 p1p2 = p[1] - p[0];
  glm::vec2 p2p3 = p[2] - p[1];
  glm::vec2 p3p1 = p[0] - p[2];

  float denom = 1 / geom::peerDot(-p3p1, p1p2);

  bool f1 = p2p3.y > 0 || (p2p3.y == 0 && p2p3.x < 0);
  bool f2 = p3p1.y > 0 || (p3p1.y == 0 && p3p1.x < 0);
  bool f3 = p1p2.y > 0 || (p1p2.y == 0 && p1p2.x < 0);

  glm::vec2 pp1 = {p[0].x - xMin, p[0].y - yMin};
  glm::vec2 pp2 = {p[1].x - xMin, p[1].y - yMin};
  glm::vec2 pp3 = {p[2].x - xMin, p[2].y - yMin};

  glm::vec3 b0 = denom * glm::vec3{
                             geom::peerDot(pp3, pp2),
                             geom::peerDot(pp1, pp3),
                             geom::peerDot(pp2, pp1),
                         };

  glm::vec3 dbdx = denom * glm::vec3{
                               pp3.y - pp2.y,
                               pp1.y - pp3.y,
                               pp2.y - pp1.y,
                           };

  glm::vec3 dbdy = denom * glm::vec3{
                               pp2.x - pp3.x,
                               pp3.x - pp1.x,
                               pp1.x - pp2.x,
                           };

  for (int y = yMin; y < yMax; y++, b0 += dbdy) {
    auto b = b0;
    for (int x = xMin; x < xMax; x++, b += dbdx) {
      if ((b.x > 0 && b.y > 0 && b.z >= 0) || (b.x == 0 && f1) ||
          (b.y == 0 && f2) || (b.z == 0 && f3)) {
        float z = b.x * p[0].z + b.y * p[1].z + p[2].z * b.z;

        glm::vec3 norm = b.x * normals[0] + b.y * normals[1] + b.z * normals[2];
        float intensity =
            std::max(0.1f, std::min(1.0f, glm::dot(norm, -lightDir)));

        if (zbuffer.set(x, y, z)) {
          image.set(x, y,
                    (int(intensity * 255) << 16) + (int(intensity * 255) << 8) +
                        int(intensity * 255));
        }
      }
    }
  }
}
