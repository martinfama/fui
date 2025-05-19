#include "imgs.h"
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <stdatomic.h>

atomic_bool taking_screenshot = false;

typedef struct {
  const char *filename;
  uint32_t *pixels;
  int32_t width;
  int32_t height;
  int32_t line_length_pixels;
} ScreenshotArgs;

void *_screenshot_region(void *args) {
  ScreenshotArgs *screenshot_args = (ScreenshotArgs *)args;
  stbi_write_png(screenshot_args->filename, screenshot_args->width,
                 screenshot_args->height, 4, screenshot_args->pixels,
                 screenshot_args->line_length_pixels * 4);
  free(screenshot_args->pixels);
  free(screenshot_args);
  atomic_store(&taking_screenshot, false);
  return NULL;
}

void screenshot_region(const char *filename, framebuffer_t *fb, int32_t x,
                       int32_t y, int32_t width, int32_t height) {
  if (x < 0 || y < 0 || x + width > fb->line_length_pixels ||
      y + height > fb->height) {
    fprintf(stderr, "Invalid screenshot region\n");
    return;
  }

  uint32_t *region_pixels = malloc(width * height * sizeof(uint32_t));
  if (!region_pixels) {
    fprintf(stderr, "Failed to allocate memory for region pixels\n");
    return;
  }
  if (width != fb->width || height != fb->height) {
    for (int32_t row = 0; row < height; row++) {
      memcpy(region_pixels + row * width,
             fb->real_pixels + (y + row) * fb->line_length_pixels + x,
             width * sizeof(uint32_t));
    }
  } else
    memcpy(region_pixels, fb->real_pixels, width * height * sizeof(uint32_t));

  // now, we have the issue that for the framebuffer, the order is ARGB, but for
  // the png, it is ABGR so we need to convert the order. we also just set alpha
  // to 0xFF
  for (uint32_t i = 0; i < width * height; i++) {
    uint32_t pixel = region_pixels[i];
    uint8_t r = (pixel >> 16) & 0xFF;
    uint8_t g = (pixel >> 8) & 0xFF;
    uint8_t b = pixel & 0xFF;
    region_pixels[i] = (0xFF << 24) | (b << 16) | (g << 8) | r;
  }

  ScreenshotArgs *args = malloc(sizeof(ScreenshotArgs));
  if (!args) {
    fprintf(stderr, "Failed to allocate memory for screenshot arguments\n");
    free(region_pixels);
    return;
  }
  args->filename = filename;
  args->pixels = region_pixels;
  args->width = width;
  args->height = height;
  args->line_length_pixels = width;

  atomic_store(&taking_screenshot, true);
  pthread_t thread;
  if (pthread_create(&thread, NULL, _screenshot_region, args) != 0) {
    atomic_store(&taking_screenshot, false);
    fprintf(stderr, "Error creating screenshot thread\n");
    free(region_pixels);
    free(args);
    return;
  }
  pthread_detach(thread);
}

void screenshot(const char *filename, framebuffer_t *fb) {
  screenshot_region(filename, fb, 0, 0, fb->line_length_pixels, fb->height);
}