#include "colors.h"
#include <math.h>
#include <stdint.h>

static float f(int n, float h, float a, float l) {
  float k = fmod(n + h / 30, 12);
  return l - a * fmax(fmin(k - 3, fmin(9 - k, 1)), -1);
}

uint32_t hsl_to_rgb(float h, float s, float l) {
  h = fmod(h, 1.0f) * 360.0f;
  float r, g, b;
  if (s == 0) {
    r = g = b = l; // Achromatic
  } else {
    float a = s * fmin(l, 1 - l);
    r = f(0, h, a, l);
    g = f(8, h, a, l);
    b = f(4, h, a, l);
  }
  return (0xFF << 24) | ((uint32_t)(r * 255)) << 16 |
         ((uint32_t)(g * 255)) << 8 | ((uint32_t)(b * 255));
}

uint32_t parametrized_rainbow(float t) {
  float h = fmod(t, 1.0);
  float s = 1.0;
  float l = 0.5;
  return hsl_to_rgb(h, s, l);
}