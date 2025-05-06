#include "../fui/clock.h"
#include <cmocka.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>

static void test_get_time_ms(void **state) {
  (void)state;
  uint32_t time1 = get_time_ms();
  usleep(1000);
  uint32_t time2 = get_time_ms();
  assert_true(time2 > time1);
}

static void test_get_time_us(void **state) {
  (void)state;
  uint64_t time1 = get_time_us();
  usleep(1000);
  uint64_t time2 = get_time_us();
  assert_true(time2 > time1);
}

static void test_sleep_ms(void **state) {
  (void)state;
  uint32_t start = get_time_ms();
  sleep_ms(10);
  uint32_t end = get_time_ms();
  assert_true(end - start >= 10);
}

static void test_sleep_us(void **state) {
  (void)state;
  uint64_t start = get_time_us();
  sleep_us(10000);
  uint64_t end = get_time_us();
  assert_true(end - start >= 10000);
}

int main(void) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_get_time_ms),
      cmocka_unit_test(test_get_time_us),
      cmocka_unit_test(test_sleep_ms),
      cmocka_unit_test(test_sleep_us),
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}