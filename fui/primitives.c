#include "primitives.h"
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

void draw_line(layer_t *layer, int x1, int y1, int x2, int y2, uint32_t color) {
    // Bresenham's line algorithm
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        draw_pixel(layer, x1, y1, color);
        if (x1 == x2 && y1 == y2) {
            break;
        }
        int err2 = err * 2;
        if (err2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (err2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

void draw_rectangle(layer_t *layer, int x, int y, int width, int height, uint32_t color, bool filled) {
    if (x + width < 0 || x >= layer->width || y + height < 0 || y >= layer->height) {
        return;
    }

    if (filled) {
        for (int i = y; i < y + height && i < layer->height; i++) {
            for (int j = x; j < x + width && j < layer->width; j++) {
                draw_pixel(layer, j, i, color);
            }
        }
    } else {
        for (int i = 0; i < width; i++) {
            draw_pixel(layer, x + i, y, color);
            draw_pixel(layer, x + i, y + height - 1, color);
        }
        for (int i = 0; i < height; i++) {
            draw_pixel(layer, x, y + i, color);
            draw_pixel(layer, x + width - 1, y + i, color);
        }
    }
}

void draw_circle(layer_t *layer, int x, int y, int radius, uint32_t color, bool filled) {
    if (x + radius < 0 || x - radius >= layer->width || y + radius < 0 || y - radius >= layer->height) {
        return;
    }

    if (filled) {
        int r2 = radius * radius;
        int area = r2 << 2;
        int rr = radius << 1;

        for (int i = 0; i < area; i++) {
            int tx = (i % rr) - radius;
            int ty = (i / rr) - radius;

            if (tx * tx + ty * ty <= r2) {
                int px = x + tx;
                int py = y + ty;

                if (px >= 0 && px < layer->width && py >= 0 && py < layer->height) {
                    draw_pixel(layer, px, py, color);
                }
            }
        }
    } else {
        int x0 = radius;
        int y0 = 0;
        int err = 0;

        while (x0 >= y0) {
            draw_pixel(layer, x + x0, y + y0, color);
            draw_pixel(layer, x + y0, y + x0, color);
            draw_pixel(layer, x - y0, y + x0, color);
            draw_pixel(layer, x - x0, y + y0, color);
            draw_pixel(layer, x - x0, y - y0, color);
            draw_pixel(layer, x - y0, y - x0, color);
            draw_pixel(layer, x + y0, y - x0, color);
            draw_pixel(layer, x + x0, y - y0, color);

            if (err <= 0) {
                err += 2 * ++y0 + 1;
            }
            if (err > 0) {
                err -= 2 * --x0;
            }
        }
    }
}

void draw_aa_line(layer_t *layer, int x1, int y1, int x2, int y2, uint32_t color) {
    // Anti-aliased line drawing using Xiaolin Wu's algorithm
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    int e2;
    int ed = dx + dy == 0 ? 1 : sqrt((float)(dx * dx + dy * dy));

    while (true) {
        color = (color & 0x00FFFFFF) | ((255 * abs(err - dx + dy) / ed) << 24);
        draw_pixel(layer, x1, y1, color);

        e2 = err;
        if (2 * e2 >= -dx) {
            if (x1 == x2)
                break;
            if (e2 + dy < ed) {
                uint32_t temp_color = (color & 0x00FFFFFF) | ((255 * (e2 + dy) / ed) << 24);
                draw_pixel(layer, x1, y1 - sy, temp_color); // Adjust for inverted y-coordinates
            }
            err -= dy;
            x1 += sx;
        }
        if (2 * e2 <= dy) {
            if (y1 == y2)
                break;
            if (dx - e2 < ed) {
                uint32_t temp_color = (color & 0x00FFFFFF) | ((255 * (dx - e2) / ed) << 24);
                draw_pixel(layer, x1 + sx, y1, temp_color); // Correct x1 instead of x2
            }
            err += dx;
            y1 += sy;
        }
    }
}

void draw_aa_circle(layer_t *layer, int x, int y, int radius, uint32_t color, bool filled) {
    // Anti-aliased circle drawing using Wu's algorithm
    int x0 = radius;
    int y0 = 0;
    int err = 0;

    while (x0 >= y0) {
        draw_aa_line(layer, x + x0, y + y0, x - x0, y + y0, color);
        draw_aa_line(layer, x + y0, y + x0, x - y0, y + x0, color);
        draw_aa_line(layer, x - x0, y - y0, x + x0, y - y0, color);
        draw_aa_line(layer, x - y0, y - x0, x + y0, y - x0, color);

        if (err <= 0) {
            err += 2 * ++y0 + 1;
        }
        if (err > 0) {
            err -= 2 * --x0;
        }
    }
}

void draw_triangle(layer_t *layer, int x1, int y1, int x2, int y2, int x3, int y3, uint32_t color, bool filled) {
    if (filled) {
        // Fill the triangle using scanline algorithm
        if (y1 > y2) {
            int tmp = y1;
            y1 = y2;
            y2 = tmp;
            tmp = x1;
            x1 = x2;
            x2 = tmp;
        }
        if (y1 > y3) {
            int tmp = y1;
            y1 = y3;
            y3 = tmp;
            tmp = x1;
            x1 = x3;
            x3 = tmp;
        }
        if (y2 > y3) {
            int tmp = y2;
            y2 = y3;
            y3 = tmp;
            tmp = x2;
            x2 = x3;
            x3 = tmp;
        }

        // Calculate slopes
        float slope1 = (float)(x2 - x1) / (y2 - y1);
        float slope2 = (float)(x3 - x1) / (y3 - y1);
        float slope3 = (float)(x3 - x2) / (y3 - y2);

        for (int i = y1; i <= y3; i++) {
            int startX, endX;
            if (i < y2) {
                startX = x1 + slope1 * (i - y1);
                endX = x1 + slope2 * (i - y1);
            } else {
                startX = x2 + slope3 * (i - y2);
                endX = x1 + slope2 * (i - y1);
            }
            for (int j = startX; j <= endX; j++) {
                draw_pixel(layer, j, i, color);
            }
        }
    } else {
        draw_line(layer, x1, y1, x2, y2, color);
        draw_line(layer, x2, y2, x3, y3, color);
        draw_line(layer, x3, y3, x1, y1, color);
    }
}