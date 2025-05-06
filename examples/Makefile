CC = gcc
CFLAGS = -Wall $(shell pkg-config --cflags fui)
LDFLAGS = $(shell pkg-config --libs fui)

SRC_MAIN = main.c
TARGET_MAIN = main
SRC_BODIES = bodies.c
TARGET_BODIES = bodies

all: $(TARGET_MAIN) $(TARGET_BODIES)

$(TARGET_MAIN): $(SRC_MAIN)
	$(CC) -g $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(TARGET_BODIES): $(SRC_BODIES)
	$(CC) -g $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET_MAIN)
	rm -f $(TARGET_BODIES)

.PHONY: all clean