#include "fonts.h"
#include <stdio.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

static stbtt_fontinfo font;
static unsigned char ttf_buffer[2 * 1024 * 1024]; // 2MB buffer for font data

void load_font(const char *font_path) {
  FILE *font_file = fopen(font_path, "rb");
  if (!font_file) {
    perror("Error opening font file");
    return;
  }
  fread(ttf_buffer, 1, sizeof(ttf_buffer), font_file);
  fclose(font_file);

  stbtt_InitFont(&font, ttf_buffer, stbtt_GetFontOffsetForIndex(ttf_buffer, 0));
}

void draw_text(layer_t *layer, const char *text, int32_t x, int32_t y,
               float font_size, uint32_t color) {
  float scale = stbtt_ScaleForPixelHeight(&font, font_size);
  int32_t width, height, xoff, yoff;

  int32_t x_start = x;

  for (int32_t i = 0; text[i] != '\0'; i++) {
    if (text[i] == '\n') {
      y += font_size * scale * 30;
      x = x_start;
      continue;
    }
    int32_t advance, lsb;
    stbtt_GetCodepointHMetrics(&font, text[i], &advance, &lsb);
    unsigned char *bitmap = stbtt_GetCodepointBitmap(
        &font, scale, scale, text[i], &width, &height, &xoff, &yoff);
    for (int32_t j = 0; j < height; j++) {
      for (int32_t k = 0; k < width; k++) {
        if (bitmap[j * width + k]) {
          draw_pixel(layer, xoff + x + k, yoff + y + j, color);
        }
      }
    }
    x += advance * scale;

    stbtt_FreeBitmap(bitmap, NULL);
  }
}