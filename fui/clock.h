#ifndef __CLOCK_H__
#define __CLOCK_H__

#include <stdint.h>

uint32_t get_time_ms();
uint64_t get_time_us();
void sleep_ms(uint32_t ms);
void sleep_us(uint64_t us);

#endif