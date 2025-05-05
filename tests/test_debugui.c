#include "../fui/debugui.h"
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>

static void test_debugui_init_cleanup(void **state) {
    (void)state;
    const char *log_file_path = "debugui_test.log";
    init_debugui(log_file_path);
    cleanup_debugui();
    assert_true(1);
}

static void test_pdebugui(void **state) {
    (void)state;
    const char *log_file_path = "debugui_test.log";
    init_debugui(log_file_path);
    pdebugui("Test message: %d", 42);
    cleanup_debugui();
    assert_true(1);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_debugui_init_cleanup),
        cmocka_unit_test(test_pdebugui),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}