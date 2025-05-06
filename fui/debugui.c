#include "debugui.h"
#include "clock.h"
#include "fbi.h"
#include "fonts.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static int debugui_line_count = 10;
static char **debugui_text_lines = NULL;
static FILE *log_file = NULL;

void set_string(char **arr, const char *str) {
  if (str == NULL) {
    *arr = NULL;
    return;
  }
  if (*arr != NULL) {
    free(*arr);
  }
  *arr = malloc(strlen(str) + 1);
  if (*arr != NULL) {
    strcpy(*arr, str);
  }
}

void init_debugui(const char *log_file_path) {
  debugui_text_lines = malloc(sizeof(char *) * debugui_line_count);
  for (int i = 0; i < debugui_line_count; i++) {
    debugui_text_lines[i] = NULL;
  }
  log_file = fopen(log_file_path, "a");
  if (log_file == NULL) {
    fprintf(stderr, "Error opening log file: %s\n", log_file_path);
    return;
  }
}

void cleanup_debugui() {
  for (int i = 0; i < debugui_line_count; i++) {
    if (debugui_text_lines[i] != NULL) {
      free(debugui_text_lines[i]);
    }
  }
  free(debugui_text_lines);
  debugui_text_lines = NULL;

  if (log_file != NULL) {
    fclose(log_file);
    log_file = NULL;
  }
}

void pdebugui(const char *text, ...) {
  va_list args;
  va_start(args, text);
  // Format the text, and get the size so that we can allocate the right amount
  // of memory
  va_list args_copy;
  va_copy(args_copy, args);
  int size = vsnprintf(NULL, 0, text, args_copy);
  va_end(args_copy);
  char *formatted_text = (char *)malloc(size + 1);

  vsnprintf(formatted_text, size + 1, text, args);
  va_end(args);

  // Shift the lines up
  for (int i = debugui_line_count - 1; i > 0; i--) {
    set_string(&debugui_text_lines[i], debugui_text_lines[i - 1]);
  }
  set_string(&debugui_text_lines[0], formatted_text);

  if (log_file != NULL) {
    fprintf(log_file, "%s\n", debugui_text_lines[0]);
    fflush(log_file);
  }
  free(formatted_text);
}

void clear_debugui() {
  for (int i = 0; i < debugui_line_count; i++) {
    debugui_text_lines[i] = NULL;
  }
}

void render_debugui(layer_t *layer, int32_t x, int32_t y, float font_size) {
  for (int i = 0; i < debugui_line_count; i++) {
    if (debugui_text_lines[i] != NULL) {
      // set alpha to 0xFF for bottom line and slide to 0x00 for top line
      uint32_t alpha = (uint32_t)(0xFF - (i * (0xFF / debugui_line_count)));
      uint32_t color = (alpha << 24) | (0x00FFFFFF);
      int y_offset = (int)((debugui_line_count - i) * font_size);
      draw_text(layer, debugui_text_lines[i], x, y + y_offset, font_size,
                color);
    }
  }
}

static int running_average_window = 10;
static int frame_count;
static uint32_t frame_times[10];
static uint32_t last_time;
static double fps;

void init_fps() {
  last_time = get_time_us();
  frame_count = 0;
  fps = 0;
  for (int i = 0; i < running_average_window; i++) {
    frame_times[i] = 0;
  }
}

void update_fps() {
  uint32_t current_time = get_time_us();
  uint32_t delta_time = current_time - last_time;
  last_time = current_time;

  for (int i = running_average_window - 1; i > 0; i--) {
    frame_times[i] = frame_times[i - 1];
  }
  frame_times[0] = delta_time;
  frame_count++;

  if (frame_count >= running_average_window) {
    double sum = 0;
    for (int i = 0; i < running_average_window; i++) {
      sum += frame_times[i];
    }
    fps = running_average_window / (sum / 1000000.0);
  }
}

void render_fps(layer_t *layer, int32_t x, int32_t y, float font_size) {
  int size = snprintf(NULL, 0, "FPS: %.2f", fps);
  char *text = (char *)malloc(size + 1);
  snprintf(text, size + 1, "FPS: %.2f", fps);
  draw_text(layer, text, x, y, font_size, 0xFFFFFFFF);
  free(text);

  // for (int i = 0; i < running_average_window; i++)
  // {
  //     pdebugui("Frame %d: %d us", i, frame_times[i]);
  // }
}