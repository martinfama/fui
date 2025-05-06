CC = gcc
CFLAGS = -Wall $(shell pkg-config --cflags fui)
LDFLAGS = $(shell pkg-config --libs fui)

SRC = main.c asteroids.c player.c
TARGET = main

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) -g $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(OBJ) $(TARGET)