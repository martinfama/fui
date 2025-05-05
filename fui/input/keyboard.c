#include "keyboard.h"
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

keyboard_state_t global_keyboard_state;
static int keyboard_fd = -1;
static struct libevdev *keyboard_dev = NULL;

void init_keyboard() {
    char *keyboard_path = get_keyboard_path();
    keyboard_fd = open(keyboard_path, O_RDONLY | O_NONBLOCK);
    if (keyboard_fd < 0) {
        perror("Cannot open keyboard");
        close(keyboard_fd);
        exit(EXIT_FAILURE);
    }
    free(keyboard_path);
    int rc = libevdev_new_from_fd(keyboard_fd, &keyboard_dev);
    if (rc < 0) {
        fprintf(stderr, "Failed to init libevdev: %s\n", strerror(-rc));
        close(keyboard_fd);
        exit(EXIT_FAILURE);
    }

    register_event_handler(KEYBOARD_EVENT, on_keypress);
    register_poll_function(poll_keyboard_events);
}

void cleanup_keyboard() {
    if (keyboard_dev) {
        libevdev_free(keyboard_dev);
        keyboard_dev = NULL;
    }
    if (keyboard_fd >= 0) {
        close(keyboard_fd);
        keyboard_fd = -1;
    }
}

void poll_keyboard_events(EventQueue *event_queue) {
    static struct input_event ev;
    int rc;
    while ((rc = libevdev_next_event(keyboard_dev, LIBEVDEV_READ_FLAG_NORMAL, &ev)) == LIBEVDEV_READ_STATUS_SUCCESS) {
        if (ev.type == EV_KEY) {
            KeyboardEvent keyboard_event = {
                .timestamp = ev.time.tv_sec * 1000000 + ev.time.tv_usec, .keycode = ev.code, .value = ev.value};
            Event event = {.type = KEYBOARD_EVENT, .data.keyboard = keyboard_event};
            push_event(event_queue, &event);
        }
    }
}

void on_keypress(Event *event) {
    if (event->type == KEYBOARD_EVENT) {
        KeyboardEvent *keyboard_event = &event->data.keyboard;
        if (keyboard_event->value == 1 || keyboard_event->value == 2) { // Key pressed
            global_keyboard_state.key_state[keyboard_event->keycode] = 1;
        } else if (keyboard_event->value == 0) { // Key released
            global_keyboard_state.key_state[keyboard_event->keycode] = 0;
        }
    }
}