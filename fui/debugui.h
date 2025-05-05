#ifndef __DEBUGUI_H__
#define __DEBUGUI_H__

#include "fbi.h"

void init_debugui(const char *log_file_path);
void cleanup_debugui();
void pdebugui(const char *text, ...);
void clear_debugui();
void render_debugui(layer_t *layer, int32_t x, int32_t y, float font_size);

void init_fps();
void update_fps();
void render_fps(layer_t *layer, int32_t x, int32_t y, float font_size);

#endif