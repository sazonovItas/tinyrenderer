#include "render.h"
#include "gl.h"

VertexTransformTask::VertexTransformTask(Context _ctx) : Task() {
  this->_ctx = _ctx;
}

void VertexTransformTask::doWork() {
  Model *model = _ctx.model;
  std::pair<int, int> range = _ctx.range;

  for (int i = range.first; i < range.second; i++) {
    model->transformed[i] = _ctx.viewport * _ctx.proj * _ctx.view *
                            _ctx.transform * glm::vec4(model->vert(i), 1.0);
  }
}

RenderLineTask::RenderLineTask(Context _ctx) : Task() { this->_ctx = _ctx; }

void RenderLineTask::doWork() {
  Model *model = _ctx.model;
  std::pair<int, int> range = _ctx.range;
  int width = _ctx.image->width, height = _ctx.image->height;

  for (int iface = range.first; iface < range.second; iface++) {
    glm::vec4 v[3] = {
        model->transformed[model->vertIdx(iface, 0)],
        model->transformed[model->vertIdx(iface, 1)],
        model->transformed[model->vertIdx(iface, 2)],
    };

    for (int i = 0; i < 3; i++) {
      glm::vec4 v1 = v[i], v2 = v[(i + 1) % 3];

      if (v1.w < 0.0 || v2.w < 0.0)
        continue;

      v1.x /= v1.w, v1.y /= v1.w, v1.z /= v1.w, v1.w /= v1.w;
      v2.x /= v2.w, v2.y /= v2.w, v2.z /= v2.w, v2.w /= v2.w;

      if (v1.z < 0.1 || v1.z > 100.0 || v2.z < 0.1 || v2.z > 100.0)
        continue;

      glm::vec2 mnV = glm::vec2(std::min(v1.x, v2.x), std::min(v1.y, v2.y));
      glm::vec2 mxV = glm::vec2(std::max(v1.x, v2.x), std::max(v1.y, v2.y));

      if (mxV.x < 0 || mxV.y < 0 || mnV.x >= width || mnV.y >= height)
        continue;

      gl::line(v1.x, v1.y, v2.x, v2.y, *_ctx.image, 0x00FFFFFF);
    }
  }
}
