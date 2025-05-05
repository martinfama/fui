CC = gcc
CFLAGS = -Wall
LDFLAGS = -l:libfui.a -lm -levdev -lasound
INCLUDE_FLAGS := -I/usr/local/include/fui

CFLAGS += $(INCLUDE_FLAGS)

SRC = main.c asteroids.c player.c
TARGET = main

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) -g $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(OBJ) $(TARGET)