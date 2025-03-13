#include "image.h"

#include <SDL_stdinc.h>
#include <cstring>

Image::Image() {}

Image::Image(int width, int height) {
  this->width = width;
  this->height = height;
  buffer.resize(width * height);
}

int Image::size() { return height * width; }

void Image::clear(uint32_t color) {
  SDL_memset4(buffer.data(), color, buffer.size());
}

void Image::resize(int width, int height) {
  this->width = width;
  this->height = height;
  if (width * height > buffer.size())
    buffer.resize(width * height);
}

void Image::set(int x, int y, uint32_t color) {
  if (y >= height || y < 0 || x >= width || x < 0) {
    return;
  }

  buffer[width * y + x] = color;
}

uint32_t *Image::data() { return buffer.data(); }
