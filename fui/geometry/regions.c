#include "regions.h"
#include "../fbi.h"
#include "../primitives.h"

int32_t position_layer_location(layer_list_t *layer_list, int32_t x, int32_t y) {
    for (int32_t z_index = layer_list->count - 1; z_index >= 0; z_index--) {
        int32_t index = layer_list->z_sorted_index[z_index];
        layer_t *layer = &layer_list->layers[index];
        if (x >= layer->x && x < layer->x + layer->width && y >= layer->y && y < layer->y + layer->height) {
            draw_line(layer, 0, 0, layer->width - 1, 0, 0xFFFF0000);
            draw_line(layer, 0, layer->height - 1, layer->width - 1, layer->height - 1, 0xFFFF0000);
            draw_line(layer, 0, 0, 0, layer->height - 1, 0xFFFF0000);
            draw_line(layer, layer->width - 1, 0, layer->width - 1, layer->height - 1, 0xFFFF0000);
            return index;
        }
    }
    return -1;
}