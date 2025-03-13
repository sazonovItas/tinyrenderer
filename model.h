#pragma once

#ifndef _MODEL_H_
#define _MODEL_H_

#include <glm/glm.hpp>
#include <string>
#include <vector>

struct Index {
  int vert;
  int norm;
  int uv;
};

class Model {
  std::vector<glm::vec3> verts = {};
  std::vector<glm::vec3> norms = {};
  std::vector<glm::vec2> uvs = {};
  std::vector<Index> faces = {};

  void triangulate(std::vector<int> &faceVertCnt);

public:
  Model();
  Model(const std::string filename);

  std::vector<glm::vec4> transformedVerts = {};
  std::vector<glm::vec4> transformedNorms = {};

  int nverts() const;
  int nfaces() const;
  int nnroms() const;

  glm::vec3 vert(const int i);
  glm::vec3 norm(const int i);

  glm::vec3 vert(const int iface, const int nthvert);
  int vertIdx(const int iface, const int nthvert);

  glm::vec3 norm(const int iface, const int nthnorm);
  int normIdx(const int iface, const int nthnorm);
};

#endif
