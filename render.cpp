#include "render.h"
#include "gl.h"
#include "shader.h"

#include <glm/fwd.hpp>
#include <glm/geometric.hpp>

VertexTransformTask::VertexTransformTask(Context _ctx) : Task() {
  this->_ctx = _ctx;
}

void VertexTransformTask::doWork() {
  Model *model = _ctx.model;
  std::pair<int, int> range = _ctx.range;
  std::pair<int, int> rangeNorms = _ctx.rangeNorms;

  glm::mat4x4 mat = _ctx.viewport * _ctx.proj * _ctx.view * _ctx.transform;
  glm::mat4x4 world = _ctx.transform;

  for (int i = range.first; i < range.second; i++) {
    model->transformedVerts[i] = mat * glm::vec4(model->vert(i), 1.0);
    model->worldVerts[i] = world * glm::vec4(model->vert(i), 1.0);
  }

  for (int i = rangeNorms.first; i < rangeNorms.second; i++) {
    model->worldNorms[i] = world * glm::vec4(model->norm(i), 0.0);
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

      gl::line(v1.x, v1.y, v2.x, v2.y, *_ctx.image, _ctx.color);
    }
  }
}

RenderTriangleTask::RenderTriangleTask(Context _ctx) : Task() {
  this->_ctx = _ctx;
}

void RenderTriangleTask::doWork() {
  Model *model = _ctx.model;
  Image *image = _ctx.image;
  ZBuffer *zbuffer = _ctx.zbuffer;

  std::pair<int, int> range = _ctx.range;
  float zNear = _ctx.zNear, zFar = _ctx.zFar;

  TriangleShader shader;

  for (int iface = range.first; iface < range.second; iface++) {
    glm::vec4 v[3] = {
        model->transformedVerts[model->vertIdx(iface, 0)],
        model->transformedVerts[model->vertIdx(iface, 1)],
        model->transformedVerts[model->vertIdx(iface, 2)],
    };

    bool clip = false;

    for (int i = 0; i < 3; i++) {
      if (v[i].w < 0.0 || v[i].w < 0.0) {
        clip = true;
        break;
      }

      v[i].x /= v[i].w, v[i].y /= v[i].w, v[i].z /= v[i].w, v[i].w /= v[i].w;

      if (v[i].z < zNear || v[i].z > zFar) {
        clip = true;
        break;
      }
    }

    if (!clip) {
      glm::vec3 norm = glm::normalize(
          glm::cross((model->vert(iface, 0) - model->vert(iface, 1)),
                     (model->vert(iface, 2) - model->vert(iface, 0))));

      glm::vec3 worldV[3] = {
          model->worldVerts[model->vertIdx(iface, 0)],
          model->worldVerts[model->vertIdx(iface, 1)],
          model->worldVerts[model->vertIdx(iface, 2)],
      };

      glm::vec3 fragPos = (worldV[0] + worldV[1] + worldV[2]) / 3.0f;
      glm::vec3 viewDir = glm::normalize(fragPos - _ctx.viewPos);
      if (glm::dot(viewDir, norm) < 0) {
        continue;
      }

      TriangleShader::Context ctx = {
          .lightCnt = _ctx.lightCnt,
          .lights = _ctx.lightPositions.data(),
          .lightColors = _ctx.lightColors.data(),
          .fragPos = fragPos,
          .fragNormal = norm,
      };
      shader.setContext(ctx);

      glm::vec3 vs[3] = {v[0], v[1], v[2]};
      gl::halfSpaceTriangle(vs, *image, *zbuffer, shader.fragment({}));
    }
  }
}

RenderPhongTask::RenderPhongTask(Context _ctx) : Task() { this->_ctx = _ctx; }

void RenderPhongTask::doWork() {
  Model *model = _ctx.model;
  Image *image = _ctx.image;
  ZBuffer *zbuffer = _ctx.zbuffer;

  std::pair<int, int> range = _ctx.range;
  float zNear = _ctx.zNear, zFar = _ctx.zFar;

  PhongShader shader;

  for (int iface = range.first; iface < range.second; iface++) {
    glm::vec4 v[3] = {
        model->transformedVerts[model->vertIdx(iface, 0)],
        model->transformedVerts[model->vertIdx(iface, 1)],
        model->transformedVerts[model->vertIdx(iface, 2)],
    };

    bool clip = false;

    for (int i = 0; i < 3; i++) {
      if (v[i].w < 0.0) {
        clip = true;
        break;
      }

      v[i].x /= v[i].w, v[i].y /= v[i].w, v[i].z /= v[i].w, v[i].w /= v[i].w;

      if (v[i].z < zNear || v[i].z > zFar) {
        clip = true;
        break;
      }
    }

    if (!clip) {
      glm::vec3 normals[3] = {
          glm::normalize(model->norm(iface, 0)),
          glm::normalize(model->norm(iface, 1)),
          glm::normalize(model->norm(iface, 2)),
      };

      glm::vec3 worldVs[3] = {
          model->worldVerts[model->vertIdx(iface, 0)],
          model->worldVerts[model->vertIdx(iface, 1)],
          model->worldVerts[model->vertIdx(iface, 2)],
      };

      glm::vec3 norm = glm::normalize(
          glm::cross((worldVs[0] - worldVs[1]), (worldVs[2] - worldVs[1])));
      glm::vec3 faceCenter = (worldVs[0] + worldVs[1] + worldVs[2]) / 3.0f;
      glm::vec3 viewDir = glm::normalize(faceCenter - _ctx.viewPos);
      if (glm::dot(viewDir, norm) < 0) {
        continue;
      }

      PhongShader::Context ctx = {
          .normals = normals,
          .worldVs = worldVs,
          .viewPos = _ctx.viewPos,
          .lightPos = _ctx.lightPos,
          .lightColor = _ctx.lightColor,
          .ambient = _ctx.ambient,
          .diffuse = _ctx.diffuse,
          .specular = _ctx.specular,
          .shininess = _ctx.shininess,
      };
      shader.setContext(ctx);

      glm::vec3 vs[3] = {v[0], v[1], v[2]};
      gl::halfSpaceTriangle(vs, *image, *zbuffer, shader);
    }
  }
}
