#ifndef __PRIMITIVES_H__
#define __PRIMITIVES_H__

#include "fbi.h"
#include <stdbool.h>

void draw_line(layer_t *layer, int x1, int y1, int x2, int y2, uint32_t color);
void draw_rectangle(layer_t *layer, int x, int y, int width, int height,
                    uint32_t color, bool filled);
void draw_circle(layer_t *layer, int x, int y, int radius, uint32_t color,
                 bool filled);
void draw_aa_line(layer_t *layer, int x1, int y1, int x2, int y2,
                  uint32_t color);
void draw_aa_circle(layer_t *layer, int x, int y, int radius, uint32_t color,
                    bool filled);
void draw_triangle(layer_t *layer, int x1, int y1, int x2, int y2, int x3,
                   int y3, uint32_t color, bool filled);

#endif