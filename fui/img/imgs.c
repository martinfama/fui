#include "imgs.h"
#include "../fbi.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#define NANOSVG_ALL_COLOR_KEYWORDS
#define NANOSVG_IMPLEMENTATION
#include "nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvgrast.h"

void render_svg(const char *filename, layer_t *layer, int32_t x, int32_t y,
                int32_t width, int32_t height, uint32_t background_color) {
  NSVGrasterizer *r = nsvgCreateRasterizer();
  NSVGimage *image = nsvgParseFromFile(filename, "px", 96.0f);
  if (image == NULL) {
    fprintf(stderr, "Error: could not open SVG file %s\n", filename);
    return;
  }
  int img_w = (int)image->width;
  int img_h = (int)image->height;
  float scale_x = (float)width / img_w;
  float scale_y = (float)height / img_h;
  float scale = fminf(scale_x, scale_y);
  unsigned char *dst = malloc(img_w * img_h * 4);
  nsvgRasterize(r, image, 0, 0, scale, dst, img_w, img_h, img_w * 4);
  for (int32_t j = 0; j < img_h; j++) {
    for (int32_t i = 0; i < img_w; i++) {
      unsigned char *src = &dst[(j * img_w + i) * 4];
      draw_pixel(layer, i + x, j + y, background_color);
      draw_pixel(layer, i + x, j + y,
                 (src[3] << 24) | (src[0] << 16) | (src[1] << 8) | src[2]);
    }
  }
  free(dst);
  nsvgDelete(image);
  nsvgDeleteRasterizer(r);
}