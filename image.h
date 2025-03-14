#pragma once

#include "spinlock.h"
#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <cstdint>
#include <vector>

class Image {
  std::vector<uint32_t> buffer;

public:
  int width;
  int height;

  Image();
  Image(int width, int height);

  int size();
  void clear(uint32_t color);
  void resize(int width, int height);
  void set(int x, int y, uint32_t color);

  uint32_t *data();
};

class ZBuffer {
  std::vector<float> buffer;
  spinlock lock[4];

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
