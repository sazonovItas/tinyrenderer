#pragma once

#ifndef _COLOR_H_
#define _COLOR_H_

#include <cstdint>

class Color {
public:
  Color();
  Color(uint8_t r, uint8_t g, uint8_t b);
  Color(float r, float g, float b);

  uint32_t color();

private:
  uint8_t _r, _g, _b, _a;
};

#endif
