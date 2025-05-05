#include "mouse.h"
#include "../clock.h"
#include "../debugui.h"
#include "input.h"
#include <errno.h>
#include <fcntl.h>
#include <libevdev-1.0/libevdev/libevdev.h>
#include <linux/input.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

mouse_state_t global_mouse_state;
static int mouse_fd = -1;
static struct libevdev *mouse_dev = NULL;

void init_mouse(int max_x, int max_y) {
    char *mouse_path = get_mouse_path();
    mouse_fd = open(mouse_path, O_RDONLY | O_NONBLOCK);
    if (mouse_fd < 0) {
        perror("Cannot open mouse");
        close(mouse_fd);
        exit(EXIT_FAILURE);
    }
    free(mouse_path);
    int rc = libevdev_new_from_fd(mouse_fd, &mouse_dev);
    if (rc < 0) {
        fprintf(stderr, "Failed to init libevdev: %s\n", strerror(-rc));
        close(mouse_fd);
        exit(EXIT_FAILURE);
    }

    register_event_handler(MOUSE_CLICK_EVENT, on_mouse_click);
    register_event_handler(MOUSE_MOVE_EVENT, on_mouse_move);
    register_event_handler(MOUSE_SCROLL_EVENT, on_mouse_scroll);
    register_poll_function(poll_mouse_events);

    global_mouse_state.max_x = max_x;
    global_mouse_state.max_y = max_y;
    global_mouse_state.x = max_x / 2;
    global_mouse_state.y = max_y / 2;
    global_mouse_state.left = 0;
    global_mouse_state.right = 0;
    global_mouse_state.middle = 0;
    global_mouse_state.scroll = 0;
}

void cleanup_mouse() {
    if (mouse_dev) {
        libevdev_free(mouse_dev);
        mouse_dev = NULL;
    }
    if (mouse_fd >= 0) {
        close(mouse_fd);
        mouse_fd = -1;
    }
}

void poll_mouse_events(EventQueue *event_queue) {
    static struct input_event ev;
    int rc;
    while ((rc = libevdev_next_event(mouse_dev, LIBEVDEV_READ_FLAG_NORMAL, &ev)) == LIBEVDEV_READ_STATUS_SUCCESS) {
        if (ev.type == EV_REL) {
            if (ev.code == REL_X || ev.code == REL_Y) {
                MouseMoveEvent move_event = {.timestamp = ev.time.tv_sec * 1000000 + ev.time.tv_usec,
                                             .dx = ev.code == REL_X ? ev.value : 0,
                                             .dy = ev.code == REL_Y ? ev.value : 0};
                Event event = {.type = MOUSE_MOVE_EVENT, .data.move = move_event};
                push_event(event_queue, &event);
            } else if (ev.code == REL_WHEEL) {
                MouseScrollEvent scroll_event = {.timestamp = ev.time.tv_sec * 1000000 + ev.time.tv_usec,
                                                 .value = ev.value};
                Event event = {.type = MOUSE_SCROLL_EVENT, .data.scroll = scroll_event};
                push_event(event_queue, &event);
            }
        } else if (ev.type == EV_KEY) {
            if (ev.code == BTN_LEFT || ev.code == BTN_RIGHT || ev.code == BTN_MIDDLE) {
                MouseClickEvent click_event = {.timestamp = ev.time.tv_sec * 1000000 + ev.time.tv_usec,
                                               .x = global_mouse_state.x,
                                               .y = global_mouse_state.y,
                                               .button = ev.code};
                Event event = {.type = MOUSE_CLICK_EVENT, .data.click = click_event};
                push_event(event_queue, &event);
            }
        }
    }
}

void on_mouse_click(Event *event) {
    if (event->type == MOUSE_CLICK_EVENT) {
        MouseClickEvent *click_event = &event->data.click;
        global_mouse_state.x = click_event->x;
        global_mouse_state.y = click_event->y;
        switch (click_event->button) {
        case BTN_LEFT:
            global_mouse_state.left = 1 - global_mouse_state.left;
            break;
        case BTN_RIGHT:
            global_mouse_state.right = 1 - global_mouse_state.right;
            break;
        case BTN_MIDDLE:
            global_mouse_state.middle = 1 - global_mouse_state.middle;
            break;
        default:
            break;
        }
    }
}

void on_mouse_move(Event *event) {
    if (event->type == MOUSE_MOVE_EVENT) {
        MouseMoveEvent *move_event = &event->data.move;
        global_mouse_state.x += move_event->dx;
        global_mouse_state.y += move_event->dy;
        if (global_mouse_state.x < 0)
            global_mouse_state.x = 0;
        if (global_mouse_state.x > global_mouse_state.max_x)
            global_mouse_state.x = global_mouse_state.max_x;
        if (global_mouse_state.y < 0)
            global_mouse_state.y = 0;
        if (global_mouse_state.y > global_mouse_state.max_y)
            global_mouse_state.y = global_mouse_state.max_y;
    }
}

void on_mouse_scroll(Event *event) {
    if (event->type == MOUSE_SCROLL_EVENT) {
        MouseScrollEvent *scroll_event = &event->data.scroll;
        global_mouse_state.scroll += scroll_event->value;
    }
}