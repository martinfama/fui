#include "clock.h"
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

uint32_t get_time_ms() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (uint32_t)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
}
uint64_t get_time_us() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (uint64_t)(tv.tv_sec * 1000000 + tv.tv_usec);
}
void sleep_ms(uint32_t ms) { usleep(ms * 1000); }
void sleep_us(uint64_t us) { usleep(us); }