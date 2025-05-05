#include "fui/colors.h"
#include "fui/debugui.h"
#include "fui/events.h"
#include "fui/fbi.h"
#include "fui/fonts.h"
#include "fui/geometry/regions.h"
#include "fui/img/imgs.h"
#include "fui/input/keyboard.h"
#include "fui/input/mouse.h"
#include "fui/primitives.h"
#include "fui/tty.h"
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <math.h>
#include <poll.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

typedef struct {
    layer_t *layer;
    int width, height;
    float zoom, move_x, move_y;
    int max_iter;
} mandelbrot_args_t;
pthread_t mandelbrot_thread_id;
bool mandelbrot_running = false;
bool stop_mandelbrot = false;
pthread_mutex_t mandelbrot_mutex = PTHREAD_MUTEX_INITIALIZER;

void draw_mandelbrot(layer_t *layer, int width, int height, float zoom, float move_x, float move_y, int max_iter) {
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            float zx = 1.5f * (i - width / 2) / (0.5f * zoom * width) + move_x;
            float zy = (j - height / 2) / (0.5f * zoom * height) + move_y;
            float x0 = 0.0f;
            float y0 = 0.0f;
            int iter = 0;
            while (x0 * x0 + y0 * y0 < 4 && iter < max_iter) {
                float tmp = x0 * x0 - y0 * y0 + zx;
                y0 = 2.0f * x0 * y0 + zy;
                x0 = tmp;
                iter++;
                if (stop_mandelbrot) {
                    return;
                }
            }
            // uint32_t color = (iter == max_iter) ? 0xFF000000 : hsl_to_rgb((float)iter / max_iter, 1.0f, 0.5f);
            // rescale color with log
            uint32_t color =
                (iter == max_iter) ? 0xFF000000 : hsl_to_rgb((float)log(iter + 1) / log(max_iter), 1.0f, 0.5f);
            draw_pixel(layer, i, j, color);
        }
    }
}

void *mandelbrot_thread(void *args) {
    mandelbrot_args_t *mandelbrot_args = (mandelbrot_args_t *)args;
    draw_mandelbrot(mandelbrot_args->layer, mandelbrot_args->width, mandelbrot_args->height, mandelbrot_args->zoom,
                    mandelbrot_args->move_x, mandelbrot_args->move_y, mandelbrot_args->max_iter);

    pthread_mutex_lock(&mandelbrot_mutex);
    mandelbrot_running = false;
    pthread_mutex_unlock(&mandelbrot_mutex);
}

void run_mandelbrot(layer_t *layer, int width, int height, float zoom, float move_x, float move_y, int max_iter) {
    mandelbrot_args_t *mandelbrot_args = malloc(sizeof(mandelbrot_args_t));
    mandelbrot_args->layer = layer;
    mandelbrot_args->width = width;
    mandelbrot_args->height = height;
    mandelbrot_args->zoom = zoom;
    mandelbrot_args->move_x = move_x;
    mandelbrot_args->move_y = move_y;
    mandelbrot_args->max_iter = max_iter;

    pthread_mutex_lock(&mandelbrot_mutex);
    if (!mandelbrot_running) {
        mandelbrot_running = true;
        pthread_create(&mandelbrot_thread_id, NULL, mandelbrot_thread, (void *)mandelbrot_args);
    }
    pthread_mutex_unlock(&mandelbrot_mutex);
}

int main() {
    srand(time(NULL));

    set_input_mode();

    framebuffer_t fb;
    load_framebuffer(&fb, DEFAULT_FB);
    if (fb.fd == -1) {
        reset_input_mode();
        fprintf(stderr, "Error loading framebuffer\n");
        return 1;
    }

    load_font("../fonts/basis33.ttf");
    init_debugui("log.txt");
    init_fps();

    EventQueue event_queue;
    init_event_queue(&event_queue, EVENT_QUEUE_CAPACITY);
    init_keyboard();
    init_mouse(fb.width, fb.height);
    start_polling(&event_queue);

    layer_list_t *layer_list = NULL;
    create_layer_list(&layer_list);
    if (layer_list == NULL) {
        destroy_framebuffer(&fb);
        reset_input_mode();
        fprintf(stderr, "Error creating layer list\n");
        return 1;
    }
    add_layer(layer_list, 0, 0, fb.line_length_pixels, fb.height, 0.0f);
    add_layer(layer_list, 100, 100, 400, 400, 0.5f);
    add_layer(layer_list, 400, 400, 200, 200, 1.0f);
    add_layer(layer_list, 600, 100, 600, 400, 2.0f);

    layer_t *drawing_layer = &layer_list->layers[1];
    int x_prev = 0;
    int y_prev = 0;
    bool drawing = false;
    float rainbow_cycle = 0.0f;
    float rainbow_speed = 0.0001f;

    layer_t *mandelbrot_layer = &layer_list->layers[3];
    mandelbrot_args_t mandelbrot_args = {.layer = mandelbrot_layer,
                                         .width = mandelbrot_layer->width,
                                         .height = mandelbrot_layer->height,
                                         .zoom = 1.0f,
                                         .move_x = 0.25f,
                                         .move_y = 0.5f,
                                         .max_iter = 2000};

    bool taking_screenshot = false;

    clear_framebuffer(&fb, 0xFF000000);

    for (float h = 0.0f; h < 1.0f; h += 0.1f) {
        uint32_t color = hsl_to_rgb(h, 1.0f, 0.5f);
        draw_rectangle(drawing_layer, (int)(h * 500) + 100, 500, 5, 5, color, true);
    }

    while (1) {
        process_event_queue(&event_queue);

        int32_t mouse_layer_index = position_layer_location(layer_list, global_mouse_state.x, global_mouse_state.y);

        gray_clear_framebuffer(&fb, 0x00);

        clear_layer(layer_list, 0);

        for (int x = 50; x < 150; x++) {
            draw_line(&layer_list->layers[2], x, 50, x, 150, parametrized_rainbow(rainbow_cycle + x * 0.01f));
        }

        draw_circle(&layer_list->layers[0], global_mouse_state.x, global_mouse_state.y, 4, 0xFFFF0000, true);
        if (global_mouse_state.x >= drawing_layer->x &&
            global_mouse_state.x < drawing_layer->x + drawing_layer->width &&
            global_mouse_state.y >= drawing_layer->y &&
            global_mouse_state.y < drawing_layer->y + drawing_layer->height) {
            if (global_mouse_state.left) {
                if (!drawing) {
                    drawing = true;
                    x_prev = global_mouse_state.x - drawing_layer->x;
                    y_prev = global_mouse_state.y - drawing_layer->y;
                }
                int32_t x_new = global_mouse_state.x - drawing_layer->x;
                int32_t y_new = global_mouse_state.y - drawing_layer->y;
                uint32_t color = parametrized_rainbow(rainbow_cycle);
                draw_line(drawing_layer, x_prev, y_prev, x_new, y_new, color);
                float dx = x_new - x_prev;
                float dy = y_new - y_prev;
                float distance = sqrt(dx * dx + dy * dy);
                rainbow_cycle += distance * rainbow_speed;
                x_prev = x_new;
                y_prev = y_new;
            } else {
                if (drawing) {
                    drawing = false;
                }
            }
        } else {
            if (drawing) {
                drawing = false;
            }
        }

        if (global_mouse_state.middle && !taking_screenshot) {
            screenshot("screenshot.png", &fb);
            screenshot_region("screenshot_region.png", &fb, 100, 100, 400, 400);
            pdebugui("Screenshot taken");
            taking_screenshot = true;
        }
        if (!global_mouse_state.middle) {
            taking_screenshot = false;
        }

        if (global_mouse_state.right) {
            float temp = layer_list->layers[1].z_depth;
            layer_list->layers[1].z_depth = layer_list->layers[2].z_depth;
            layer_list->layers[2].z_depth = temp;
            sort_layers_z(layer_list);
        }

        if (!mandelbrot_running) {
            mandelbrot_args.zoom += 0.5f;
            mandelbrot_args.max_iter += 500;
            run_mandelbrot(mandelbrot_layer, mandelbrot_args.width, mandelbrot_args.height, mandelbrot_args.zoom,
                           mandelbrot_args.move_x, mandelbrot_args.move_y, mandelbrot_args.max_iter);
        }

        update_fps();
        render_debugui(&layer_list->layers[0], 10, 10, 20.0f);
        render_fps(&layer_list->layers[0], 10, layer_list->layers[0].height - 20, 20.0f);

        draw_all_layers(layer_list, &fb, false, false);
        render_framebuffer(&fb);

        char c;
        ssize_t n = read(STDIN_FILENO, &c, 1);
        if (n > 0) {
            if (c == 27) {
                break;
            }
        }
    }

    pthread_mutex_lock(&mandelbrot_mutex);
    if (mandelbrot_running) {
        pthread_cancel(mandelbrot_thread_id);
        mandelbrot_running = false;
        stop_mandelbrot = true;
    }
    pthread_mutex_unlock(&mandelbrot_mutex);
    pthread_join(mandelbrot_thread_id, NULL);

    cleanup_debugui();

    destroy_layer_list(&layer_list);
    destroy_framebuffer(&fb);

    cleanup_polling();
    destroy_event_queue(&event_queue);
    cleanup_mouse();
    cleanup_keyboard();

    reset_input_mode();

    return 0;
}