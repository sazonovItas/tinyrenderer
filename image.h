#pragma once

#include <vips/image.h>
#ifndef _IMAGE_H_
#define _IMAGE_H_

#include "spinlock.h"

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <vips/vips.h>

class ImageBuffer {
  std::vector<uint32_t> buffer;

public:
  int width;
  int height;

  ImageBuffer();
  ImageBuffer(int width, int height);

  int size();
  void clear(uint32_t color);
  void resize(int width, int height);
  void set(int x, int y, uint32_t color);

  uint32_t *data();
};

class Image {
  VipsImage *_image;

public:
  Image();
  Image(std::string filename);

  int width();
  int height();

  uint32_t getPixel(int x, int y);
  glm::vec3 getColor(int x, int y);

  uint32_t getPixelUV(float x, float y);
  glm::vec3 getColorUV(float x, float y);
};

class ZBuffer {
  std::vector<float> buffer;
  std::vector<spinlock> locks;

public:
  int width;
  int height;

  ZBuffer();
  ZBuffer(int width, int height);

  int size();
  void clear();
  void resize(int width, int height);
  bool set(int x, int y, float z);

  float *data();
};

#endif
