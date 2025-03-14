#include "camera.h"
#include "geometry.h"
#include "image.h"
#include "model.h"
#include "render.h"
#include "thread_pool.h"

#include <GL/gl.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_video.h>
#include <glm/ext.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <iostream>
#include <stdexcept>

#define MODEL "models/dom.obj"

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

  Model *model;
  Image *image;
  ZBuffer *zbuffer;

  Camera camera;

  void initThreadPool() {
#define THREAD_COUNT 16
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
    model = new Model(MODEL);
    image = new Image(WIDTH, HEIGHT);
    zbuffer = new ZBuffer(WIDTH, HEIGHT);

    camera = Camera(glm::vec3(0.0, 0.0, 0.0), 15.0f);

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
        renderSDL();
    }
  }

  void renderSDL() {
    auto surface = SDL_GetWindowSurface(window);

    image->resize(surface->w, surface->h);
    zbuffer->resize(surface->w, surface->h);

    image->clear(0x00);
    zbuffer->clear();

    render();

    SDL_LockSurface(surface);
    SDL_memcpy4(surface->pixels, image->data(), image->size());
    SDL_UnlockSurface(surface);
    SDL_UpdateWindowSurface(window);
  }

  void render() {
#define Z_NEAR 0.1f
#define Z_FAR 100.0f
#define FOV 90.0f

    float aspect = float(image->width) / float(image->height);
    glm::mat4x4 proj = glm::perspective(FOV, aspect, Z_NEAR, Z_FAR);
    glm::mat4x4 view = camera.view();
    glm::mat4x4 viewport = geom::viewport(0, 0, image->width, image->height);

    threadPool->pause();

    VertexTransformTask::Context _vctx = {
        .model = model,
        .proj = proj,
        .view = view,
        .viewport = viewport,
    };

    int vertPerThread = model->nverts() / THREAD_COUNT;

    for (int i = 0; i < THREAD_COUNT - 1; i++) {
      _vctx.range = {i * vertPerThread, (i + 1) * vertPerThread};
      threadPool->add_task(VertexTransformTask(_vctx));
    }

    _vctx.range = {(THREAD_COUNT - 1) * vertPerThread, model->nverts()};
    threadPool->add_task(VertexTransformTask(_vctx));

    threadPool->wait();

    // RenderLineTask::Context _rctx = {
    //     .model = model,
    //     .image = image,
    //     .zNear = Z_NEAR,
    //     .zFar = Z_FAR,
    // };
    //
    // int facePerThread = model->nfaces() / THREAD_COUNT;
    //
    // for (int i = 0; i < THREAD_COUNT - 1; i++) {
    //   _rctx.range = {i * facePerThread, (i + 1) * facePerThread};
    //   threadPool->add_task(RenderLineTask(_rctx));
    // }
    //
    // _rctx.range = {(THREAD_COUNT - 1) * facePerThread, model->nfaces()};
    // threadPool->add_task(RenderLineTask(_rctx));
    //
    // threadPool->wait();

    // RenderTriangleTask::Context _rctx = {
    //     .model = model,
    //     .image = image,
    //     .zbuffer = zbuffer,
    //     .zNear = Z_NEAR,
    //     .zFar = Z_FAR,
    //     .lightDir = camera.eye(),
    // };
    //
    // int facePerThread = model->nfaces() / THREAD_COUNT;
    //
    // for (int i = 0; i < THREAD_COUNT; i++) {
    //   _rctx.range = {i * facePerThread, (i + 1) * facePerThread};
    //   threadPool->add_task(RenderTriangleTask(_rctx));
    //
    //   if (i == THREAD_COUNT - 1) {
    //     _rctx.range.second = model->nfaces();
    //   }
    // }
    //
    // threadPool->wait();

    RenderNormalsTask::Context _rctx = {
        .model = model,
        .image = image,
        .zbuffer = zbuffer,
        .zNear = Z_NEAR,
        .zFar = Z_FAR,
        .lightDir = camera.eye(),
    };

    int facePerThread = model->nfaces() / THREAD_COUNT;

    for (int i = 0; i < THREAD_COUNT; i++) {
      _rctx.range = {i * facePerThread, (i + 1) * facePerThread};
      threadPool->add_task(RenderNormalsTask(_rctx));

      if (i == THREAD_COUNT - 1) {
        _rctx.range.second = model->nfaces();
      }
    }

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
