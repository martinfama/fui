#ifndef __COLORS_H__
#define __COLORS_H__

#include <stdint.h>

uint32_t hsl_to_rgb(float h, float s, float l);
uint32_t hue_to_rgb(float p, float q, float t);
uint32_t parametrized_rainbow(float t);

#endif