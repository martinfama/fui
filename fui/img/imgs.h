#ifndef __IMGS_H__
#define __IMGS_H__

#include "../fbi.h"

void draw_bitmap(layer_t *layer, uint32_t *bitmap, int32_t x, int32_t y,
                 int32_t width, int32_t height);
void render_svg(const char *filename, layer_t *layer, int32_t x, int32_t y,
                int32_t width, int32_t height, uint32_t background_color);

#endif // __IMGS_H__