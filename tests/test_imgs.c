#include "../fui/clock.h"
#include "../fui/fbi.h"
#include "../fui/img/imgs.h"
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
    mock_fb.real_pixels = malloc(mock_fb.screen_size);
    if (!mock_fb.real_pixels) {
        return -1;
    }
    *state = &mock_fb;
    return 0;
}

static int teardown_framebuffer(void **state) {
    framebuffer_t *fb = *state;
    free(fb->real_pixels);
    return 0;
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
        cmocka_unit_test_setup_teardown(test_screenshot, setup_framebuffer, teardown_framebuffer),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}