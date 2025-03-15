#include "render.h"
#include "gl.h"
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>

VertexTransformTask::VertexTransformTask(Context _ctx) : Task() {
  this->_ctx = _ctx;
}

void VertexTransformTask::doWork() {
  Model *model = _ctx.model;
  std::pair<int, int> range = _ctx.range;
  std::pair<int, int> rangeNorms = _ctx.rangeNorms;

  for (int i = range.first; i < range.second; i++) {
    model->transformedVerts[i] = _ctx.viewport * _ctx.proj * _ctx.view *
                                 _ctx.transform *
                                 glm::vec4(model->vert(i), 1.0);
  }

  for (int i = rangeNorms.first; i < rangeNorms.second; i++) {
    model->transformedNorms[i] =
        _ctx.transform * glm::vec4(model->norm(i), 0.0);
  }
}

RenderLineTask::RenderLineTask(Context _ctx) : Task() { this->_ctx = _ctx; }

void RenderLineTask::doWork() {
  Model *model = _ctx.model;
  std::pair<int, int> range = _ctx.range;
  int width = _ctx.image->width, height = _ctx.image->height;
  float zNear = _ctx.zNear, zFar = _ctx.zFar;

  for (int iface = range.first; iface < range.second; iface++) {
    glm::vec4 v[3] = {
        model->transformedVerts[model->vertIdx(iface, 0)],
        model->transformedVerts[model->vertIdx(iface, 1)],
        model->transformedVerts[model->vertIdx(iface, 2)],
    };

    for (int i = 0; i < 3; i++) {
      glm::vec4 v1 = v[i], v2 = v[(i + 1) % 3];

      if (v1.w < 0.0 || v2.w < 0.0)
        continue;

      v1.x /= v1.w, v1.y /= v1.w, v1.z /= v1.w, v1.w /= v1.w;
      v2.x /= v2.w, v2.y /= v2.w, v2.z /= v2.w, v2.w /= v2.w;

      if (v1.z < zNear || v1.z > zFar || v2.z < zNear || v2.z > zFar)
        continue;

      glm::vec2 mnV = glm::vec2(std::min(v1.x, v2.x), std::min(v1.y, v2.y));
      glm::vec2 mxV = glm::vec2(std::max(v1.x, v2.x), std::max(v1.y, v2.y));

      if (mxV.x < 0 || mxV.y < 0 || mnV.x >= width || mnV.y >= height)
        continue;

      gl::line(v1.x, v1.y, v2.x, v2.y, *_ctx.image, 0x00FFFFFF);
    }
  }
}

RenderTriangleTask::RenderTriangleTask(Context _ctx) : Task() {
  this->_ctx = _ctx;
}

void RenderTriangleTask::doWork() {
  Model *model = _ctx.model;
  std::pair<int, int> range = _ctx.range;
  float zNear = _ctx.zNear, zFar = _ctx.zFar;

  for (int iface = range.first; iface < range.second; iface++) {
    glm::vec4 v[3] = {
        model->transformedVerts[model->vertIdx(iface, 0)],
        model->transformedVerts[model->vertIdx(iface, 1)],
        model->transformedVerts[model->vertIdx(iface, 2)],
    };

    bool clip = true;

    for (int i = 0; i < 3; i++) {
      if (v[i].w < 0.0 || v[i].w < 0.0) {
        clip = false;
        break;
      }

      v[i].x /= v[i].w, v[i].y /= v[i].w, v[i].z /= v[i].w, v[i].w /= v[i].w;

      if (v[i].z < zNear || v[i].z > zFar) {
        clip = false;
        break;
      }
    }

    if (clip) {
      glm::vec3 norm = glm::normalize(
          glm::cross((model->vert(iface, 0) - model->vert(iface, 1)),
                     (model->vert(iface, 2) - model->vert(iface, 0))));

      glm::vec3 worldV[3] = {
          model->vert(iface, 0),
          model->vert(iface, 1),
          model->vert(iface, 2),
      };
      glm::vec3 faceCenter = (worldV[0] + worldV[1] + worldV[2]) / 3.0f;
      glm::vec3 viewDir = glm::normalize(faceCenter - _ctx.viewPos);

      if (glm::dot(viewDir, norm) < 0) {
        continue;
      }

      glm::vec3 intensity(0.1, 0.1, 0.1);
      glm::vec3 vs[3] = {v[0], v[1], v[2]};

      for (int i = 0; i < _ctx.lightCnt; i++) {
        glm::vec3 lightDir =
            glm::normalize(faceCenter - _ctx.lightPositions[i]);
        float lightIntensity =
            std::max(0.0f, std::min(1.0f, glm::dot(norm, lightDir)));
        intensity += lightIntensity * _ctx.lightColors[i];
      }

      float maxIntensity =
          std::max(intensity[0], std::max(intensity[1], intensity[2]));

      if (maxIntensity > 1.0) {
        intensity /= maxIntensity;
      }

      gl::halfSpaceTriangle(vs, *_ctx.image, *_ctx.zbuffer,
                            (int(intensity[0] * 255) << 16) +
                                (int(intensity[1] * 255) << 8) +
                                int(intensity[2] * 255));
    }
  }
}

RenderSpecTask::RenderSpecTask(Context _ctx) : Task() { this->_ctx = _ctx; }

void RenderSpecTask::doWork() {
  Model *model = _ctx.model;
  std::pair<int, int> range = _ctx.range;
  float zNear = _ctx.zNear, zFar = _ctx.zFar;

  for (int iface = range.first; iface < range.second; iface++) {
    glm::vec4 v[3] = {
        model->transformedVerts[model->vertIdx(iface, 0)],
        model->transformedVerts[model->vertIdx(iface, 1)],
        model->transformedVerts[model->vertIdx(iface, 2)],
    };

    bool clip = true;

    for (int i = 0; i < 3; i++) {
      if (v[i].w < 0.0 || v[i].w < 0.0) {
        clip = false;
        break;
      }

      v[i].x /= v[i].w, v[i].y /= v[i].w, v[i].z /= v[i].w, v[i].w /= v[i].w;

      if (v[i].z < zNear || v[i].z > zFar) {
        clip = false;
        break;
      }
    }

    if (clip) {
      glm::vec3 normals[3] = {
          glm::normalize(model->norm(iface, 0)),
          glm::normalize(model->norm(iface, 1)),
          glm::normalize(model->norm(iface, 2)),
      };

      glm::vec3 worldV[3] = {
          model->vert(iface, 0),
          model->vert(iface, 1),
          model->vert(iface, 2),
      };
      glm::vec3 norm = glm::normalize(
          glm::cross((worldV[0] - worldV[1]), (worldV[2] - worldV[1])));
      glm::vec3 faceCenter = (worldV[0] + worldV[1] + worldV[2]) / 3.0f;
      glm::vec3 viewDir = glm::normalize(faceCenter - _ctx.viewPos);

      if (glm::dot(viewDir, norm) < 0) {
        continue;
      }

      glm::vec3 vs[3] = {v[0], v[1], v[2]};

      gl::halfSpaceTriangle(vs, *_ctx.image, *_ctx.zbuffer, worldV, normals,
                            _ctx.lightPos, _ctx.lightColor, _ctx.viewPos,
                            _ctx.ambient, _ctx.diffuse, _ctx.specular,
                            _ctx.shininess);
    }
  }
}
