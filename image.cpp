#include "image.h"

#include <SDL_stdinc.h>
#include <cstring>
#include <limits>
#include <vips/image.h>

Image::Image() {}

Image::Image(std::string filename) {
  _image = vips_image_new_from_file(filename.c_str(), NULL);
  vips_image_inplace(_image);
}

int Image::width() { return _image->Xsize; }

int Image::height() { return _image->Ysize; }

uint32_t Image::getPixel(int x, int y) {
  return *(uint32_t *)(VIPS_IMAGE_ADDR(_image, x, y));
}

uint32_t Image::getPixelUV(float x, float y) {
  return getPixel(x * width(), y * height());
}

glm::vec3 Image::getColor(int x, int y) {
  uint32_t color = getPixel(x, y);
  float r = uint8_t(color >> 0) / 255.0;
  float g = uint8_t(color >> 8) / 255.0;
  float b = uint8_t(color >> 16) / 255.0;
  return glm::vec3(r, g, b);
}

glm::vec3 Image::getColorUV(float x, float y) {
  return getColor(x * width(), y * height());
}

ImageBuffer::ImageBuffer() {}

ImageBuffer::ImageBuffer(int width, int height) {
  this->width = width;
  this->height = height;
  buffer.resize(width * height);
}

int ImageBuffer::size() { return height * width; }

void ImageBuffer::clear(uint32_t color) {
  SDL_memset4(buffer.data(), color, buffer.size());
}

void ImageBuffer::resize(int width, int height) {
  this->width = width;
  this->height = height;
  buffer.resize(width * height);
}

void ImageBuffer::set(int x, int y, uint32_t color) {
  if (y >= height || y < 0 || x >= width || x < 0)
    return;

  buffer[width * y + x] = color;
}

uint32_t *ImageBuffer::data() { return buffer.data(); }

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
