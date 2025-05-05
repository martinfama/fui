#include "input.h"
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int is_keyboard(struct libevdev *dev) {
    return libevdev_has_event_type(dev, EV_KEY) && libevdev_has_event_code(dev, EV_KEY, KEY_A) &&
           libevdev_has_event_code(dev, EV_KEY, KEY_ENTER);
}

int is_mouse(struct libevdev *dev) {
    return libevdev_has_event_type(dev, EV_REL) && libevdev_has_event_code(dev, EV_REL, REL_X) &&
           libevdev_has_event_code(dev, EV_REL, REL_Y) && libevdev_has_event_code(dev, EV_KEY, BTN_LEFT);
}

char *get_keyboard_path() {
    DIR *dp = opendir(INPUT_DIR);
    if (!dp) {
        perror("opendir /dev/input");
        return NULL;
    }

    struct dirent *entry;
    char path[256];

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

        if (is_keyboard(dev)) {
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

char *get_mouse_path() {
    DIR *dp = opendir(INPUT_DIR);
    if (!dp) {
        perror("opendir /dev/input");
        return NULL;
    }

    struct dirent *entry;
    char path[256];

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

        if (is_mouse(dev)) {
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