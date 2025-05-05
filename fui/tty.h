#ifndef __TTY_H__
#define __TTY_H__

#include <fcntl.h>
#include <linux/kd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <linux/fb.h>

struct termios orig_termios;

void reset_input_mode() {
    char *tty_name = ttyname(STDIN_FILENO);
    if (tty_name == NULL) {
        perror("Error getting tty name");
        return;
    }
    int tty_fd = open(tty_name, O_RDWR);
    ioctl(tty_fd, KDSETMODE, KD_TEXT);
    close(tty_fd);
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
    printf("\e[?25h");  // Show cursor again
    fflush(stdout);
}

void set_input_mode() {
    struct termios new_termios;

    // Save original terminal settings
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(reset_input_mode);  // Restore on exit

    new_termios = orig_termios;

    // Disable canonical mode and echo
    new_termios.c_lflag &= ~(ICANON | ECHO);

    // Set the new settings
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);

    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);

    printf("\e[?25l");  // Hide cursor
    
    char *tty_name = ttyname(STDIN_FILENO);
    if (tty_name == NULL) {
        perror("Error getting tty name");
        return;
    }
    int tty_fd = open(tty_name, O_RDWR);
    ioctl(tty_fd, KDSETMODE, KD_GRAPHICS);
}

#endif