#ifndef __IMGS_H__
#define __IMGS_H__

#include "../fbi.h"
#include "stb_image.h"
#include "stb_image_resize2.h"
#include "stb_image_write.h"
#include <stdatomic.h>

extern atomic_bool taking_screenshot;

void *_screenshot_region(void *args);
void screenshot_region(const char *filename, framebuffer_t *fb, int32_t x,
                       int32_t y, int32_t width, int32_t height);
void screenshot(const char *filename, framebuffer_t *fb);

#endif