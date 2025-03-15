#pragma once

#include <glm/ext/matrix_transform.hpp>
#ifndef _RENDER_H_
#define _RENDER_H_

#include "image.h"
#include "model.h"
#include "thread_pool.h"

#include <glm/glm.hpp>

class VertexTransformTask : public MT::Task {
public:
  struct Context {
    Model *model;

    glm::mat4x4 transform = glm::identity<glm::mat4x4>();
    glm::mat4x4 proj, view, viewport;

    std::pair<int, int> range;
    std::pair<int, int> rangeNorms;
  };

  VertexTransformTask(Context _ctx);

  void doWork() override;

private:
  Context _ctx;
};

class RenderLineTask : public MT::Task {
public:
  struct Context {
    Model *model;
    Image *image;

    float zNear, zFar;

    uint32_t color;

    std::pair<int, int> range;
  };

  RenderLineTask(Context _ctx);

  void doWork() override;

private:
  Context _ctx;
};

class RenderTriangleTask : public MT::Task {
public:
  struct Context {
    Model *model;
    Image *image;
    ZBuffer *zbuffer;

    float zNear, zFar;

    int lightCnt = 0;
    std::vector<glm::vec3> lightColors;
    std::vector<glm::vec3> lightPositions;
    glm::vec3 viewPos;

    std::pair<int, int> range;
  };

  RenderTriangleTask(Context _ctx);

  void doWork() override;

private:
  Context _ctx;
};

class RenderPhongTask : public MT::Task {
public:
  struct Context {
    Model *model;
    Image *image;
    ZBuffer *zbuffer;

    float zNear, zFar;

    glm::vec3 lightPos, lightColor, viewPos;
    glm::vec3 ambient, diffuse, specular;
    float shininess;

    std::pair<int, int> range;
  };

  RenderPhongTask(Context _ctx);

  void doWork() override;

private:
  Context _ctx;
};

#endif
