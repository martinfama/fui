CC = gcc
CFLAGS = -Wall $(shell pkg-config --cflags fui)
LDFLAGS = $(shell pkg-config --libs fui)

SRC_MAIN = main.c
TARGET_MAIN = main
SRC_BODIES = bodies.c
TARGET_BODIES = bodies
SRC_RENDER_SVG = render_svg.c
TARGET_RENDER_SVG = render_svg

all: $(TARGET_MAIN) $(TARGET_BODIES) $(TARGET_RENDER_SVG)

$(TARGET_MAIN): $(SRC_MAIN)
	$(CC) -g $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(TARGET_BODIES): $(SRC_BODIES)
	$(CC) -g $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(TARGET_RENDER_SVG): $(SRC_RENDER_SVG)
	$(CC) -g $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET_MAIN)
	rm -f $(TARGET_BODIES)
	rm -f $(TARGET_RENDER_SVG)

.PHONY: all clean