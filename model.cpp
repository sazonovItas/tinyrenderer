#include "model.h"
#include "geometry.h"

#include <fstream>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

Model::Model() {}

Model::Model(const std::string filename) {
  std::ifstream in;

  in.open(filename, std::ifstream::in);

  if (in.fail())
    throw std::runtime_error("failed to open file");

  std::vector<int> faceVertCnt;

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

      faceVertCnt.push_back(cnt);
    }
  }

  in.close();

  triangulate(faceVertCnt);

  transformedVerts = std::vector<glm::vec4>(verts.size());
  for (int i = 0; i < verts.size(); i++) {
    transformedVerts[i] = glm::vec4(verts[i], 1.0);
  }

  worldVerts = std::vector<glm::vec4>(verts.size());
  for (int i = 0; i < verts.size(); i++) {
    worldVerts[i] = glm::vec4(verts[i], 1.0);
  }

  worldNorms = std::vector<glm::vec4>(norms.size());
  for (int i = 0; i < worldNorms.size(); i++) {
    worldNorms[i] = glm::vec4(norms[i], 0.0);
  }
}

void Model::triangulate(std::vector<int> &faceVertCnt) {
  std::vector<Index> trianguleatedFaces;

  int offset = 0;
  for (auto vertCnt : faceVertCnt) {
    int l = offset, r = offset + vertCnt;

    std::vector<Index> face(faces.begin() + l, faces.begin() + r);
    while (face.size() > 3) {

      for (int i = 0; i < face.size(); i++) {
        int vCnt = face.size();
        glm::vec3 beforeV[3] = {
            verts[face[i].vert],
            verts[face[(i + 1) % vCnt].vert],
            verts[face[(i + 2) % vCnt].vert],
        };

        glm::vec3 forward = glm::normalize(
            glm::cross(beforeV[0] - beforeV[1], beforeV[2] - beforeV[1]));
        glm::vec3 right = beforeV[1] - beforeV[0];
        glm::vec3 up = glm::cross(forward, right);

        auto triangleMat = geom::view(forward, right, up);

        glm::vec3 v[3] = {
            triangleMat * glm::vec4(beforeV[0], 1.0),
            triangleMat * glm::vec4(beforeV[1], 1.0),
            triangleMat * glm::vec4(beforeV[2], 1.0),
        };

        float angle = glm::cross(v[0] - v[1], v[2] - v[1]).z;

        if (angle < 0.0) {
          continue;
        }

        bool ok = true;

        for (int j = (i + 3) % vCnt; j != i; j = (j + 1) % vCnt) {
          glm::vec3 vert = triangleMat * glm::vec4(verts[face[j].vert], 1.0);

          auto bc = geom::barycentric(v[0], v[1], v[2], vert);
          if (bc.x >= 0 && bc.y >= 0 && bc.z >= 0) {
            ok = false;
            break;
          }
        }

        if (ok) {
          trianguleatedFaces.push_back(face[i]);
          trianguleatedFaces.push_back(face[(i + 1) % vCnt]);
          trianguleatedFaces.push_back(face[(i + 2) % vCnt]);
          face.erase(face.begin() + (i + 1) % vCnt);
          break;
        }
      }
    }

    for (auto f : face)
      trianguleatedFaces.push_back(f);

    offset += vertCnt;
  }

  faces = trianguleatedFaces;
}

int Model::nverts() const { return verts.size(); }

int Model::nnorms() const { return norms.size(); }

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
