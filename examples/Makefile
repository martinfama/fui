CC = gcc
CFLAGS = -Wall
LDFLAGS = -Lfui -l:libfui.a -lm -levdev

INCLUDE_DIRS := $(shell find fui -type f -name '*.h' -exec dirname {} \; | sort -u)
INCLUDE_FLAGS := $(foreach dir,$(INCLUDE_DIRS),-I$(dir))
CFLAGS += $(INCLUDE_FLAGS)

SRC = main.c
TARGET = main

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) -g $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(OBJ) $(TARGET)