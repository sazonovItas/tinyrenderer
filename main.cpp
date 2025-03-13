#include "camera.h"
#include "gl.h"
#include "image.h"
#include "model.h"
#include "thread_pool.h"

#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_video.h>
#include <SDL_timer.h>
#include <cstdint>
#include <glm/ext.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <iostream>
#include <stdexcept>

uint32_t WIDTH = 1920;
uint32_t HEIGHT = 1080;

class App {
public:
  void run() {
    initThreadPool();
    initWindow();
    mainLoop();
    cleanup();
  }

private:
  SDL_Window *window;
  MT::ThreadPool *threadPool;

  void initThreadPool() {
#define THREAD_COUNT 8
    threadPool = new MT::ThreadPool(THREAD_COUNT);
  }

  void initWindow() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) !=
        0) {
      throw std::runtime_error("failed to init sdl");
    }

    SDL_WindowFlags windowFlags =
        (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    window =
        SDL_CreateWindow("ACG", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                         WIDTH, HEIGHT, windowFlags);
    if (window == nullptr) {
      throw std::runtime_error("failed to create window with sdl");
    }
  }

  void mainLoop() {
    Model *model = new Model("models/house.obj");
    Image *image = new Image(WIDTH, HEIGHT);
    Camera camera(glm::vec3(0.0, 0.0, 0.0), 10.0f);

    bool loop = true;
    while (loop) {
      bool rerender = false;

      SDL_Event event;
      while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT)
          loop = false;

        if (event.type == SDL_WINDOWEVENT &&
            event.window.event == SDL_WINDOWEVENT_CLOSE &&
            event.window.windowID == SDL_GetWindowID(window))
          loop = false;

        if (event.type == SDL_MOUSEWHEEL) {
          camera.zoom(event.wheel.y * 0.1f);
          rerender = true;
        }

        if (event.type == SDL_MOUSEMOTION &&
            event.button.button == SDL_BUTTON(SDL_BUTTON_LEFT)) {
          camera.rotate(
              glm::radians(-event.motion.x * event.motion.xrel * 0.0003f),
              glm::radians(event.motion.y * event.motion.yrel * 0.0003f));
          rerender = true;
        }

        if (event.type == SDL_WINDOWEVENT &&
            event.window.event == SDL_WINDOWEVENT_EXPOSED) {
          rerender = true;
        }
      }

      if (rerender)
        renderSDL(model, image, camera);
    }
  }

  void renderSDL(Model *model, Image *image, Camera &camera) {
    auto surface = SDL_GetWindowSurface(window);

    image->resize(surface->w, surface->h);
    image->clear(0x00);

    render(model, image, camera);

    SDL_memcpy4(surface->pixels, image->data(), image->size());
    SDL_UpdateWindowSurface(window);
  }

  class VertexTransformTask : public MT::Task {
  public:
    struct Context {
      Model *model;

      glm::mat4x4 proj, view;

      std::pair<int, int> range;
    };

    VertexTransformTask(Context _ctx) : Task() { this->_ctx = _ctx; }

    void doWork() override {
      Model *model = _ctx.model;
      std::pair<int, int> range = _ctx.range;

      for (int i = range.first; i < range.second; i++) {
        model->transformed[i] =
            _ctx.proj * _ctx.view * glm::vec4(model->vert(i), 1.0);
      }
    }

  private:
    Context _ctx;
  };

  class RenderTask : public MT::Task {
  public:
    struct Context {
      Model *model;
      Image *image;

      glm::mat4x4 viewport;
      std::pair<int, int> range;
    };

    RenderTask(Context _ctx) : Task() { this->_ctx = _ctx; }

    void doWork() override {
      Model *model = _ctx.model;
      std::pair<int, int> range = _ctx.range;

      for (int iface = range.first; iface < range.second; iface++) {
        glm::vec4 v[3] = {
            model->transformed[model->vertIdx(iface, 0)],
            model->transformed[model->vertIdx(iface, 1)],
            model->transformed[model->vertIdx(iface, 2)],
        };
        v[0].x /= v[0].w, v[0].y /= v[0].w, v[0].z /= v[0].w,
            v[0].w /= abs(v[0].w);
        v[1].x /= v[1].w, v[1].y /= v[1].w, v[1].z /= v[1].w,
            v[1].w /= abs(v[1].w);
        v[2].x /= v[2].w, v[2].y /= v[2].w, v[2].z /= v[2].w,
            v[2].w /= abs(v[2].w);

        for (int i = 0; i < 3; i++) {
          glm::vec4 v1 = v[i], v2 = v[(i + 1) % 3];

          if (v1.z < 0.1 || v1.z > 100.0 || v2.z < 0.1 || v2.z > 100.0 ||
              v1.w < 0.0 || v2.w < 0.0)
            continue;

          glm::vec2 mnV = glm::vec2(std::min(v1.x, v2.x), std::min(v1.y, v2.y));
          glm::vec2 mxV = glm::vec2(std::max(v1.x, v2.x), std::max(v1.y, v2.y));

          if (mxV.x < -1.0 || mxV.y < -1.0 || mnV.x > 1.0 || mnV.y > 1.0)
            continue;

          v1 = _ctx.viewport * v1;
          v2 = _ctx.viewport * v2;

          gl::line(v1.x, v1.y, v2.x, v2.y, *_ctx.image, 0x00FFFFFF);
        }
      }
    }

  private:
    Context _ctx;
  };

  void render(Model *model, Image *image, Camera &camera) {
    float aspect = float(image->width) / float(image->height);
    glm::mat4x4 proj = glm::perspective(90.0f, aspect, 0.1f, 100.0f);
    glm::mat4x4 view = camera.view();
    glm::mat4x4 viewport = gl::viewport(0, 0, image->width, image->height);

    VertexTransformTask::Context _vctx = {
        .model = model,
        .proj = proj,
        .view = view,
    };

    int vertPerThread = model->nverts() / THREAD_COUNT;

    for (int i = 0; i < THREAD_COUNT - 1; i++) {
      _vctx.range = {i * vertPerThread, (i + 1) * vertPerThread};
      threadPool->add_task(VertexTransformTask(_vctx));
    }

    _vctx.range = {(THREAD_COUNT - 1) * vertPerThread, model->nverts()};
    threadPool->add_task(VertexTransformTask(_vctx));

    threadPool->wait();

    RenderTask::Context _rctx = {
        .model = model,
        .image = image,
        .viewport = viewport,
    };

    int facePerThread = model->nfaces() / THREAD_COUNT;

    for (int i = 0; i < THREAD_COUNT - 1; i++) {
      _rctx.range = {i * facePerThread, (i + 1) * facePerThread};
      threadPool->add_task(RenderTask(_rctx));
    }

    _rctx.range = {(THREAD_COUNT - 1) * facePerThread, model->nfaces()};
    threadPool->add_task(RenderTask(_rctx));

    threadPool->wait();
  }

  void cleanup() {
    delete threadPool;

    SDL_DestroyWindow(window);
    SDL_Quit();
  }
};

int main(int argc, char *argv[]) {
  App app;

  try {
    app.run();
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
