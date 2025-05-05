#include "../fui/clock.h"
#include "../fui/debugui.h"
#include "../fui/fbi.h"
#include "../fui/img/imgs.h"
#include <cmocka.h>
#include <stdarg.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
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
    mock_fb.fd = -1; // Mock framebuffer does not need a file descriptor
    mock_fb.vinfo = (struct fb_var_screeninfo){0};
    mock_fb.finfo = (struct fb_fix_screeninfo){0};
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

    for (int i = 0; i < fb->total_pixels; i++) {
        assert_int_equal(((uint32_t *)fb->real_pixels)[i], color);
    }
}

static void test_draw_pixel(void **state) {
    framebuffer_t *fb = *state;
    int x = 100, y = 50;
    uint32_t color = 0xFFFF0000;
    draw_pixel_fb(fb, x, y, color);

    int index = y * fb->line_length_pixels + x;
    assert_int_equal(((uint32_t *)fb->pixels)[index], color);
}

static void test_render_framebuffer(void **state) {
    framebuffer_t *fb = *state;

    for (int i = 0; i < fb->total_pixels; i++) {
        ((uint32_t *)fb->pixels)[i] = i;
    }

    render_framebuffer(fb);

    for (int i = 0; i < fb->total_pixels; i++) {
        assert_int_equal(fb->real_pixels[i], i);
    }
}

static void test_debugui_init_cleanup(void **state) {
    (void)state;
    const char *log_file_path = "debugui.log";
    init_debugui(log_file_path);
    cleanup_debugui();
    assert_true(1);
}

static void test_screenshot(void **state) {
    framebuffer_t *fb = *state;
    const char *filename = "test_screenshot.png";

    screenshot(filename, fb);
    while (atomic_load(&taking_screenshot)) {
        sleep_us(1000);
    }

    FILE *file = fopen(filename, "r");
    assert_non_null(file);
    if (file) {
        fclose(file);
        remove(filename);
    }
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_clear_framebuffer, setup_framebuffer, teardown_framebuffer),
        cmocka_unit_test_setup_teardown(test_draw_pixel, setup_framebuffer, teardown_framebuffer),
        cmocka_unit_test_setup_teardown(test_render_framebuffer, setup_framebuffer, teardown_framebuffer),
        cmocka_unit_test(test_debugui_init_cleanup),
        cmocka_unit_test_setup_teardown(test_screenshot, setup_framebuffer, teardown_framebuffer),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
