#ifndef __MOUSE_H__
#define __MOUSE_H__

#include "../events.h"
typedef struct mouse_state {
    int max_x;
    int max_y;
    int x;
    int y;
    int left;
    int right;
    int middle;
    int scroll;
} mouse_state_t;

extern mouse_state_t global_mouse_state;

void init_mouse();
void cleanup_mouse();
void poll_mouse_events(EventQueue *event_queue);
void on_mouse_click(Event *event);
void on_mouse_move(Event *event);
void on_mouse_scroll(Event *event);

#endif