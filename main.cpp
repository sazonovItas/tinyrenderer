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
#include <SDL_keyboard.h>
#include <SDL_keycode.h>
#include <glm/ext.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <iostream>
#include <stdexcept>

#define MODEL_FILE "models/cube.obj"
#define MODEL_DIFFUSE "models/albedo.png"

#define LIGHT_COUNT 2
#define CAMERA_RADIUS 10.0f

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
  enum RenderMode {
    RenderLines,
    RenderTriangles,
    RenderPhongLight,
    RenderTexture
  };

  SDL_Window *window;
  RenderMode currentRenderMode = RenderLines;

  MT::ThreadPool *threadPool;

  Model *model;
  ImageBuffer *image;
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

  std::vector<glm::vec3> lightPositions;
  std::vector<glm::vec3> lightColors;

  void mainLoop() {
    model = new Model(MODEL_FILE);
    image = new ImageBuffer(WIDTH, HEIGHT);
    zbuffer = new ZBuffer(WIDTH, HEIGHT);

    model->parseTextures(MODEL_DIFFUSE, "", "");

    camera = Camera(glm::vec3(0.0, 0.0, 0.0), CAMERA_RADIUS);

    lightPositions.resize(3);
    lightPositions[1] = {2.0, 2.0, 0.0};
    lightPositions[2] = {-2.0, 2.0, 0.0};

    lightColors.resize(3);
    lightColors[0] = {1.0, 1.0, 1.0};
    lightColors[1] = {0.5, 0.0, 0.0};
    lightColors[2] = {0.0, 0.5, 0.0};

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

        if (event.type == SDL_KEYDOWN) {
          switch (event.key.keysym.sym) {
          case SDLK_1:
            currentRenderMode = RenderLines;
            break;
          case SDLK_2:
            currentRenderMode = RenderTriangles;
            break;
          case SDLK_3:
            currentRenderMode = RenderPhongLight;
            break;
          case SDLK_4:
            currentRenderMode = RenderTexture;
            break;
          }

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
    glm::mat4x4 proj =
        glm::perspective(glm::radians(FOV), aspect, Z_NEAR, Z_FAR);
    glm::mat4x4 view = camera.view();
    glm::mat4x4 viewport = geom::viewport(0, 0, image->width, image->height);

    threadPool->pause();

    VertexTransformTask::Context _vctx = {
        .model = model,
        .proj = proj,
        .view = view,
        .viewport = viewport,
    };

    int vertsPerThread = model->nverts() / THREAD_COUNT;
    int normsPerThread = model->nnorms() / THREAD_COUNT;

    for (int i = 0; i < THREAD_COUNT; i++) {
      _vctx.range = {i * vertsPerThread, (i + 1) * vertsPerThread};
      _vctx.rangeNorms = {i * normsPerThread, (i + 1) * normsPerThread};

      if (i == THREAD_COUNT - 1) {
        _vctx.range.second = model->nverts();
        _vctx.rangeNorms.second = model->nnorms();
      }

      threadPool->addTask(VertexTransformTask(_vctx));
    }

    threadPool->wait();

    switch (currentRenderMode) {
    case RenderLines:
      renderLines();
      break;
    case RenderTriangles:
      renderTriangles();
      break;
    case RenderPhongLight:
      renderPhongLight();
      break;
    case RenderTexture:
      renderTexture();
      break;
    }
  }

  void renderLines() {
    RenderLineTask::Context _rctx = {
        .model = model,
        .image = image,
        .zNear = Z_NEAR,
        .zFar = Z_FAR,
        .color = 0x00FFFFFF,
    };

    int facePerThread = model->nfaces() / THREAD_COUNT;

    for (int i = 0; i < THREAD_COUNT; i++) {
      _rctx.range = {i * facePerThread, (i + 1) * facePerThread};
      if (i == THREAD_COUNT - 1) {
        _rctx.range.second = model->nfaces();
      }

      threadPool->addTask(RenderLineTask(_rctx));
    }

    threadPool->wait();
  }

  void renderTriangles() {
    lightPositions[0] = camera.position();

    RenderTriangleTask::Context _rctx = {
        .model = model,
        .image = image,
        .zbuffer = zbuffer,

        .zNear = Z_NEAR,
        .zFar = Z_FAR,

        .lightCnt = LIGHT_COUNT + 1,
        .lightColors = lightColors,
        .lightPositions = lightPositions,

        .viewPos = camera.position(),
    };

    int facePerThread = model->nfaces() / THREAD_COUNT;

    for (int i = 0; i < THREAD_COUNT; i++) {
      _rctx.range = {i * facePerThread, (i + 1) * facePerThread};
      if (i == THREAD_COUNT - 1) {
        _rctx.range.second = model->nfaces();
      }

      threadPool->addTask(RenderTriangleTask(_rctx));
    }

    threadPool->wait();
  }

  void renderPhongLight() {
    RenderPhongTask::Context _rctx = {
        .model = model,
        .image = image,
        .zbuffer = zbuffer,
        .zNear = Z_NEAR,
        .zFar = Z_FAR,
        .lightPos = camera.position(),
        .lightColor = lightColors[0],
        .viewPos = camera.position(),
    };

    int facePerThread = model->nfaces() / THREAD_COUNT;

    for (int i = 0; i < THREAD_COUNT; i++) {
      _rctx.range = {i * facePerThread, (i + 1) * facePerThread};
      if (i == THREAD_COUNT - 1) {
        _rctx.range.second = model->nfaces();
      }

      threadPool->addTask(RenderPhongTask(_rctx));
    }

    threadPool->wait();
  }

  void renderTexture() {
    RenderTextureTask::Context _rctx = {
        .model = model,
        .image = image,
        .zbuffer = zbuffer,
        .zNear = Z_NEAR,
        .zFar = Z_FAR,
        .lightPos = camera.position(),
        .lightColor = lightColors[0],
        .viewPos = camera.position(),
    };

    int facePerThread = model->nfaces() / THREAD_COUNT;

    for (int i = 0; i < THREAD_COUNT; i++) {
      _rctx.range = {i * facePerThread, (i + 1) * facePerThread};
      if (i == THREAD_COUNT - 1) {
        _rctx.range.second = model->nfaces();
      }

      threadPool->addTask(RenderTextureTask(_rctx));
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
