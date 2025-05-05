#ifndef __FONTS_H__
#define __FONTS_H__

#include "fbi.h"
#include "stb_truetype.h"
#include <stdint.h>

static stbtt_fontinfo font;
static unsigned char ttf_buffer[2 * 1024 * 1024]; // 2MB buffer for font data

void load_font(const char *font_path);
void draw_text(layer_t *layer, const char *text, int32_t x, int32_t y, float font_size, uint32_t color);

#endif