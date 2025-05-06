#include "fbi.h"
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <termios.h>
#include <unistd.h>

void load_framebuffer(framebuffer_t *fb, const char *device) {
    framebuffer_t fb_info;
    fb_info.fd = -1;
    fb_info.width = 0;
    fb_info.height = 0;
    fb_info.line_length_pixels = 0;
    fb_info.total_pixels = 0;
    fb_info.screen_size = 0;
    fb_info.real_pixels = NULL;
    fb_info.pixels = NULL;
    fb_info.vinfo = (struct fb_var_screeninfo){0};
    fb_info.finfo = (struct fb_fix_screeninfo){0};

    int fb_d = open(device, O_RDWR);
    if (fb_d == -1) {
        perror("Error opening framebuffer device");
        destroy_framebuffer(&fb_info);
    }
    fb_info.fd = fb_d;

    struct fb_var_screeninfo vinfo;
    if (ioctl(fb_d, FBIOGET_VSCREENINFO, &vinfo)) {
        perror("Error reading variable information");
        destroy_framebuffer(&fb_info);
    }
    fb_info.vinfo = vinfo;

    struct fb_fix_screeninfo finfo;
    if (ioctl(fb_d, FBIOGET_FSCREENINFO, &finfo)) {
        perror("Error reading fixed information");
        destroy_framebuffer(&fb_info);
    }
    fb_info.finfo = finfo;

    fb_info.width = vinfo.xres_virtual;
    fb_info.height = vinfo.yres_virtual;
    fb_info.line_length_pixels = finfo.line_length / (vinfo.bits_per_pixel / 8);
    fb_info.total_pixels = vinfo.xres_virtual * vinfo.yres_virtual;
    fb_info.screen_size = finfo.smem_len;

    fb_info.real_pixels = mmap(NULL, fb_info.screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb_info.fd, 0);
    if (fb_info.real_pixels == MAP_FAILED) {
        perror("Error mapping framebuffer to memory");
        destroy_framebuffer(&fb_info);
    }
    fb_info.pixels = malloc(fb_info.screen_size);
    if (fb_info.pixels == NULL) {
        perror("Error allocating memory for framebuffer");
        destroy_framebuffer(&fb_info);
    }

    fb->fd = fb_info.fd;
    fb->width = fb_info.width;
    fb->height = fb_info.height;
    fb->line_length_pixels = fb_info.line_length_pixels;
    fb->total_pixels = fb_info.total_pixels;
    fb->screen_size = fb_info.screen_size;
    fb->real_pixels = fb_info.real_pixels;
    fb->pixels = fb_info.pixels;
    fb->vinfo = fb_info.vinfo;
    fb->finfo = fb_info.finfo;
}

void destroy_framebuffer(framebuffer_t *fb) {
    free(fb->pixels);
    fb->pixels = NULL;
    if (fb->real_pixels != MAP_FAILED) {
        munmap(fb->real_pixels, fb->screen_size);
    }
    fb->real_pixels = NULL;
    if (fb->fd != -1) {
        close(fb->fd);
    }
}

void render_framebuffer(framebuffer_t *fb) {
    // update with dirty mask
    // for (int i = 0; i < fb->total_pixels; i++) {
    //     if (fb->pixels[i] != fb->real_pixels[i]) {
    //         fb->real_pixels[i] = fb->pixels[i];
    //     }
    // }
    // directly copy the entire array to the framebuffer
    memcpy(fb->real_pixels, fb->pixels, fb->screen_size);
}

void clear_framebuffer(framebuffer_t *fb, uint32_t color) {
    for (int i = 0; i < fb->total_pixels; i++) {
        fb->real_pixels[i] = color;
    }
}

void gray_clear_framebuffer(framebuffer_t *fb, uint8_t intensity) {
    intensity = (0xFF << 24) | (intensity << 16) | (intensity << 8) | intensity; // ARGB format
    memset(fb->pixels, intensity, fb->screen_size);
}

void draw_pixel_fb(framebuffer_t *fb, int x, int y, uint32_t color) {
    if (x < 0 || x >= fb->width || y < 0 || y >= fb->height) {
        return;
    }

    // if alpha is FF, just draw the pixel
    if ((color >> 24) == 0xFF) {
        fb->pixels[y * fb->line_length_pixels + x] = color;
        return;
    }

    uint32_t existing_color = fb->pixels[y * fb->line_length_pixels + x];
    uint8_t alpha = (color >> 24) & 0xFF;
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;
    uint8_t existing_r = (existing_color >> 16) & 0xFF;
    uint8_t existing_g = (existing_color >> 8) & 0xFF;
    uint8_t existing_b = existing_color & 0xFF;
    uint8_t new_r = (r * alpha + existing_r * (255 - alpha)) / 255;
    uint8_t new_g = (g * alpha + existing_g * (255 - alpha)) / 255;
    uint8_t new_b = (b * alpha + existing_b * (255 - alpha)) / 255;

    fb->pixels[y * fb->line_length_pixels + x] = (new_r << 16) | (new_g << 8) | new_b;
}

void draw_pixel(layer_t *layer, int x, int y, uint32_t color) {
    if (x < 0 || x >= layer->width || y < 0 || y >= layer->height) {
        return;
    }
    layer->pixels[y * layer->width + x] = color;
}

void create_layer_list(layer_list_t **list) {
    *list = malloc(sizeof(layer_list_t));
    if (list == NULL) {
        perror("Error allocating memory for layer list");
        return;
    }
    (*list)->count = 0;
    (*list)->z_sorted_index = NULL;

    (*list)->layers = malloc(0);
    if ((*list)->layers == NULL) {
        perror("Error allocating memory for layers");
        return;
    }
    (*list)->z_sorted_index = malloc(0);
    if ((*list)->z_sorted_index == NULL) {
        perror("Error allocating memory for z-sorted index");
        free((*list)->layers);
        return;
    }
}

void destroy_layer_list(layer_list_t **list) {
    if (list != NULL && *list != NULL) {
        for (int i = 0; i < (*list)->count; i++) {
            if ((*list)->layers[i].pixels != NULL) {
                free((*list)->layers[i].pixels);
                (*list)->layers[i].pixels = NULL;
            }
        }
        free((*list)->layers);
        (*list)->layers = NULL;
        free((*list)->z_sorted_index);
        (*list)->z_sorted_index = NULL;
        free(*list);
        *list = NULL;
    }
}

void sort_layers_z(layer_list_t *list) {
    // Generate a sorted index based on z_depth
    for (int i = 0; i < list->count; i++) {
        list->z_sorted_index[i] = i;
    }
    for (int i = 0; i < list->count - 1; i++) {
        for (int j = 0; j < list->count - i - 1; j++) {
            if (list->layers[list->z_sorted_index[j]].z_depth > list->layers[list->z_sorted_index[j + 1]].z_depth) {
                int32_t temp = list->z_sorted_index[j];
                list->z_sorted_index[j] = list->z_sorted_index[j + 1];
                list->z_sorted_index[j + 1] = temp;
            }
        }
    }
}

void add_layer(layer_list_t *list, int32_t x, int32_t y, int32_t width, int32_t height, float z_depth) {
    layer_t *new_layers = realloc(list->layers, (list->count + 1) * sizeof(layer_t));
    if (new_layers == NULL) {
        perror("Error reallocating memory for layers");
        return;
    }
    list->layers = new_layers;
    list->layers[list->count].x = x;
    list->layers[list->count].y = y;
    list->layers[list->count].width = width;
    list->layers[list->count].height = height;
    list->layers[list->count].pixels = malloc(width * height * sizeof(uint32_t));
    list->layers[list->count].z_depth = z_depth;
    if (list->layers[list->count].pixels == NULL) {
        perror("Error allocating memory for layer pixels");
        return;
    }
    memset(list->layers[list->count].pixels, 0, width * height * sizeof(uint32_t));
    list->count++;
    list->z_sorted_index = realloc(list->z_sorted_index, list->count * sizeof(int32_t));
    if (list->z_sorted_index == NULL) {
        perror("Error reallocating memory for z-sorted index");
        return;
    }
    sort_layers_z(list);
}

void remove_layer(layer_list_t *list, int32_t index) {
    if (index < 0 || index >= list->count) {
        return;
    }
    free(list->layers[index].pixels);
    list->layers[index].pixels = NULL;
    for (int i = index; i < list->count - 1; i++) {
        list->layers[i] = list->layers[i + 1];
    }
    list->count--;
    list->layers = realloc(list->layers, list->count * sizeof(layer_t));
    if (list->layers == NULL) {
        perror("Error reallocating memory for layers");
        return;
    }
    list->z_sorted_index = realloc(list->z_sorted_index, list->count * sizeof(int32_t));
    if (list->z_sorted_index == NULL) {
        perror("Error reallocating memory for z-sorted index");
        return;
    }
    sort_layers_z(list);
}

void clear_layer(layer_list_t *list, int32_t index) {
    if (index < 0 || index >= list->count) {
        return;
    }
    memset(list->layers[index].pixels, 0, list->layers[index].width * list->layers[index].height * sizeof(uint32_t));
}

void clear_all_layers(layer_list_t *list) {
    for (int i = 0; i < list->count; i++) {
        clear_layer(list, i);
    }
}

void draw_layer(layer_list_t *list, int32_t index, framebuffer_t *fb, bool margin, bool flush) {
    if (index < 0 || index >= list->count) {
        return;
    }
    layer_t *layer = &list->layers[index];
    for (int y = 0; y < layer->height; y++) {
        for (int x = 0; x < layer->width; x++) {
            // check if the pixel is different from the current fb pixel value
            if (flush) {
                draw_pixel_fb(fb, layer->x + x, layer->y + y, layer->pixels[y * layer->width + x]);
            } else if (layer->pixels[y * layer->width + x] !=
                       fb->pixels[(layer->y + y) * fb->line_length_pixels + (layer->x + x)]) {
                draw_pixel_fb(fb, layer->x + x, layer->y + y, layer->pixels[y * layer->width + x]);
            }
        }
    }
    if (margin) {
        // Draw a border around the layer
        for (int x = 0; x < layer->width; x++) {
            draw_pixel_fb(fb, layer->x + x, layer->y, 0xFFFFFFFF);
            draw_pixel_fb(fb, layer->x + x, layer->y + layer->height - 1, 0xFFFFFFFF);
        }
        for (int y = 0; y < layer->height; y++) {
            draw_pixel_fb(fb, layer->x, layer->y + y, 0xFFFFFFFF);
            draw_pixel_fb(fb, layer->x + layer->width - 1, layer->y + y, 0xFFFFFFFF);
        }
    }
}

void draw_all_layers(layer_list_t *list, framebuffer_t *fb, bool margin, bool flush) {
    for (int i = 0; i < list->count; i++) {
        draw_layer(list, list->z_sorted_index[i], fb, margin, flush);
    }
}
