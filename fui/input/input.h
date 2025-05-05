#ifndef __INPUT_H__
#define __INPUT_H__

#include <libevdev-1.0/libevdev/libevdev.h>

#define INPUT_DIR "/dev/input"

int is_keyboard(struct libevdev *dev);
int is_mouse(struct libevdev *dev);
char *get_keyboard_path();
char *get_mouse_path();

#endif // __INPUT_H__