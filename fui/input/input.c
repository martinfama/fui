#include "input.h"
#include <dirent.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef int (*device_match)(struct libevdev *dev);

static char *get_input_device(device_match match) {
    DIR *dp = opendir(INPUT_DIR);
    if (!dp) {
        perror("opendir /dev/input");
        return NULL;
    }

    struct dirent *entry;
    char path[PATH_MAX];

    while ((entry = readdir(dp))) {
        if (strncmp(entry->d_name, "event", 5) != 0)
            continue;

        snprintf(path, sizeof(path), "%s/%s", INPUT_DIR, entry->d_name);
        int fd = open(path, O_RDONLY | O_NONBLOCK);
        if (fd < 0)
            continue;

        struct libevdev *dev = NULL;
        if (libevdev_new_from_fd(fd, &dev) < 0) {
            close(fd);
            continue;
        }

        if (match(dev)) {
            libevdev_free(dev);
            closedir(dp);
            return strdup(path);
        }

        libevdev_free(dev);
        close(fd);
    }

    closedir(dp);
    return NULL;
}

int is_keyboard(struct libevdev *dev) {
    return libevdev_has_event_type(dev, EV_KEY) && libevdev_has_event_code(dev, EV_KEY, KEY_A) &&
           libevdev_has_event_code(dev, EV_KEY, KEY_ENTER);
}

int is_mouse(struct libevdev *dev) {
    return libevdev_has_event_type(dev, EV_REL) && libevdev_has_event_code(dev, EV_REL, REL_X) &&
           libevdev_has_event_code(dev, EV_REL, REL_Y) && libevdev_has_event_code(dev, EV_KEY, BTN_LEFT);
}

int is_touchpad(struct libevdev *dev) {
    return libevdev_has_event_type(dev, BTN_TOOL_FINGER) &&
           libevdev_has_event_code(dev, BTN_TOOL_FINGER, BTN_TOOL_FINGER) && libevdev_has_event_type(dev, EV_ABS) &&
           libevdev_has_event_code(dev, EV_ABS, ABS_PRESSURE) &&
           libevdev_has_event_code(dev, EV_ABS, ABS_MT_POSITION_X);
}

char *get_keyboard_path() {
    char *path = get_input_device(is_keyboard);
    if (!path) {
        fprintf(stderr, "No keyboard found\n");
    }
    return path;
}

char *get_mouse_path() {
    char *path = get_input_device(is_mouse);
    if (!path) {
        fprintf(stderr, "No mouse found\n");
    }
    return path;
}

char *get_touchpad_path() {
    char *path = get_input_device(is_touchpad);
    if (!path) {
        fprintf(stderr, "No touchpad found\n");
    }
    return path;
}