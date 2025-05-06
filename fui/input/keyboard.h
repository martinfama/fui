#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include "../events.h"

#define MAX_KEYCODE 256
typedef struct keyboard_state {
  uint8_t key_state[MAX_KEYCODE]; // 0 = released, 1 = pressed
} keyboard_state_t;

extern keyboard_state_t global_keyboard_state;

void init_keyboard();
void cleanup_keyboard();
void poll_keyboard_events(EventQueue *event_queue);
void on_keypress(Event *event);

#endif