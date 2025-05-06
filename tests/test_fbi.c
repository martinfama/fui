#include "../fui/fbi.h"
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>

framebuffer_t mock_fb;

static int setup_framebuffer(void **state) {
  mock_fb.width = 800;
  mock_fb.height = 600;
  mock_fb.line_length_pixels = 800;
  mock_fb.total_pixels = mock_fb.width * mock_fb.height;
  mock_fb.screen_size = mock_fb.total_pixels * sizeof(uint32_t);
  mock_fb.pixels = malloc(mock_fb.screen_size);
  if (!mock_fb.pixels) {
    return -1;
  }
  mock_fb.real_pixels = malloc(mock_fb.screen_size);
  if (!mock_fb.real_pixels) {
    free(mock_fb.pixels);
    return -1;
  }
  *state = &mock_fb;
  return 0;
}

static int teardown_framebuffer(void **state) {
  framebuffer_t *fb = *state;
  free(fb->pixels);
  free(fb->real_pixels);
  return 0;
}

static void test_clear_framebuffer(void **state) {
  framebuffer_t *fb = *state;
  uint32_t color = 0xFF00FF00;
  clear_framebuffer(fb, color);

  for (uint32_t i = 0; i < fb->total_pixels; i++) {
    assert_int_equal(((uint32_t *)fb->real_pixels)[i], color);
  }
}

int main(void) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test_setup_teardown(test_clear_framebuffer, setup_framebuffer,
                                      teardown_framebuffer),
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}