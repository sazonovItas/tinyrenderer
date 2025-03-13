#include "model.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

Model::Model() {}

Model::Model(const std::string filename) {
  std::ifstream in;

  in.open(filename, std::ifstream::in);

  if (in.fail())
    throw std::runtime_error("failed to open file");

  std::string line;
  while (!in.eof()) {
    std::getline(in, line);
    std::istringstream iss(line.c_str());

    char trash;
    if (!line.compare(0, 2, "o ")) {
      iss >> trash;

      std::string name;
      iss >> name;
    } else if (!line.compare(0, 2, "v ")) {
      iss >> trash;

      glm::vec3 v;
      for (int i : {0, 1, 2})
        iss >> v[i];

      verts.push_back(v);
    } else if (!line.compare(0, 3, "vn ")) {
      iss >> trash >> trash;

      glm::vec3 n;
      for (int i : {0, 1, 2})
        iss >> n[i];

      norms.push_back(glm::normalize(n));
    } else if (!line.compare(0, 2, "f ")) {
      iss >> trash;

      int v, uv, n, cnt = 0;

      while (iss >> v >> trash >> uv >> trash >> n) {
        faces.push_back({.vert = v - 1, .norm = n - 1, .uv = uv - 1});
        cnt++;
      }

      if (cnt != 3) {
        throw std::runtime_error("object is not triangulized");
      }
    }
  }

  in.close();

  triangulate();

  transformed = std::vector<glm::vec4>(verts.size());
  for (int i = 0; i < transformed.size(); i++) {
    transformed[i] = glm::vec4(verts[i], 1.0);
  }
}

void Model::triangulate() {}

int Model::nverts() const { return verts.size(); }

int Model::nnroms() const { return norms.size(); }

int Model::nfaces() const { return faces.size() / 3; }

glm::vec3 Model::vert(const int i) { return verts[i]; }

glm::vec3 Model::norm(const int i) { return norms[i]; }

glm::vec3 Model::vert(const int iface, const int nthvert) {
  return verts[faces[iface * 3 + nthvert].vert];
}

int Model::vertIdx(const int iface, const int nthvert) {
  return faces[iface * 3 + nthvert].vert;
}

glm::vec3 Model::norm(const int iface, const int nthnorm) {
  return norms[faces[iface * 3 + nthnorm].norm];
}

int Model::normIdx(const int iface, const int nthnorm) {
  return faces[iface * 3 + nthnorm].norm;
}
