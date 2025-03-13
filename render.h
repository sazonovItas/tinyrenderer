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

    std::pair<int, int> range;
  };

  RenderLineTask(Context _ctx);

  void doWork() override;

private:
  Context _ctx;
};

#endif
