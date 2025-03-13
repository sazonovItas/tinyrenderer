#pragma once

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

#endif
