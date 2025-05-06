#ifndef __FBI_H__
#define __FBI_H__

#include <linux/fb.h>
#include <stdbool.h>
#include <stdint.h>

#define DEFAULT_FB "/dev/fb0"

typedef struct framebuffer {
  int32_t fd;
  int32_t width;
  int32_t height;
  uint32_t line_length_pixels; // Number of pixels in a line (could be different
                               // from width because of padding)
  uint32_t total_pixels;
  uint32_t screen_size;
  uint32_t *real_pixels; // Pointer to the framebuffer in memory
  uint32_t *pixels;      // Will draw here and use dirty mask to update
  struct fb_var_screeninfo vinfo;
  struct fb_fix_screeninfo finfo;
} framebuffer_t;

typedef struct layer {
  int32_t x;
  int32_t y;
  int32_t width;
  int32_t height;
  uint32_t *pixels;
  float z_depth; // Z depth for layering
} layer_t;

typedef struct layer_list {
  layer_t *layers;
  int32_t count;
  int32_t *z_sorted_index;
} layer_list_t;

void load_framebuffer(framebuffer_t *fb, const char *device);
void destroy_framebuffer(framebuffer_t *fb);

// Layer managing
void create_layer_list(layer_list_t **list);
void destroy_layer_list(layer_list_t **list);
void add_layer(layer_list_t *list, int32_t x, int32_t y, int32_t width,
               int32_t height, float z_depth);
void sort_layers_z(layer_list_t *list);
void remove_layer(layer_list_t *list, int32_t index);
void clear_layer(layer_list_t *list, int32_t index);
void clear_all_layers(layer_list_t *list);
void draw_layer(layer_list_t *list, int32_t index, framebuffer_t *fb,
                bool margin, bool flush);
void draw_all_layers(layer_list_t *list, framebuffer_t *fb, bool margin,
                     bool flush);

// Drawing
void render_framebuffer(framebuffer_t *fb);
void clear_framebuffer(framebuffer_t *fb, uint32_t color);
void gray_clear_framebuffer(framebuffer_t *fb, uint8_t intensity);
void draw_pixel(layer_t *layer, int x, int y, uint32_t color);
void draw_pixel_fb(framebuffer_t *fb, int x, int y, uint32_t color);

#endif