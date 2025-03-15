#include "image.h"

#include <SDL_stdinc.h>
#include <cstring>
#include <limits>

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
  buffer.resize(width * height);
}

void Image::set(int x, int y, uint32_t color) {
  if (y >= height || y < 0 || x >= width || x < 0)
    return;

  buffer[width * y + x] = color;
}

uint32_t *Image::data() { return buffer.data(); }

ZBuffer::ZBuffer() {}

ZBuffer::ZBuffer(int width, int height) {
  this->width = width;
  this->height = height;
  buffer.resize(width * height);
  locks.resize(width * height);
}

void ZBuffer::clear() {
  for (int i = buffer.size(); i--;) {
    buffer[i] = -std::numeric_limits<float>::max();
  }
}

int ZBuffer::size() { return height * width; }

void ZBuffer::resize(int width, int height) {
  this->width = width;
  this->height = height;
  buffer.resize(width * height);
  locks.resize(width * height);
}

bool ZBuffer::set(int x, int y, float z) {
  bool ok = false;

  locks[x + y * width].lock();
  if (buffer[int(x + y * width)] < 1.0 / z) {
    buffer[int(x + y * width)] = 1.0 / z;
    ok = true;
  }
  locks[x + y * width].unlock();

  return ok;
}
