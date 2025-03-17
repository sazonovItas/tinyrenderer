#pragma once

#ifndef _MODEL_H_
#define _MODEL_H_

#include "image.h"

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

  Image _diffuse;
  Image _specular;
  Image _normal;

  void triangulate(std::vector<int> &faceVertCnt);

public:
  Model(const std::string obj);

  std::vector<glm::vec4> worldVerts = {};
  std::vector<glm::vec4> worldNorms = {};
  std::vector<glm::vec4> transformedVerts = {};

  int nverts() const;
  int nfaces() const;
  int nnorms() const;

  glm::vec3 vert(const int i);
  glm::vec3 norm(const int i);

  glm::vec3 vert(const int iface, const int nthvert);
  int vertIdx(const int iface, const int nthvert);

  glm::vec3 norm(const int iface, const int nthnorm);
  int normIdx(const int iface, const int nthnorm);

  glm::vec2 uv(const int iface, const int nthuv);
  int uvIdx(const int iface, const int nthuv);

  void parseTextures(std::string diffuse, std::string normal,
                     std::string specular);

  Image diffuseMap() { return _diffuse; }
};

#endif
