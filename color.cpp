#include "color.h"

#include <algorithm>

Color::Color() { _r = 0, _g = 0, _b = 0, _a = 0; }

Color::Color(uint8_t r, uint8_t g, uint8_t b) {
  _r = r, _g = g, _b = b, _a = 0;
}

Color::Color(float r, float g, float b) {
  _r = std::min(int(r * 255), 255);
  _g = std::min(int(g * 255), 255);
  _b = std::min(int(b * 255), 255);
  _a = 0;
}

uint32_t Color::color() {
  return (uint32_t(_a) << 24) + (uint32_t(_r) << 16) + (uint32_t(_g) << 8) +
         uint32_t(_b);
}
