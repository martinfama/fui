CC = gcc
CFLAGS = -I../fui -Wall -Wextra -g
LDFLAGS = -lcmocka -lm -levdev

FUI_SRCS = $(shell find ../fui -name '*.c' -not -name 'test_*.c')
FUI_OBJS = $(FUI_SRCS:.c=.o)

TESTS = test_clock test_debugui test_fbi test_imgs

OBJS = $(TESTS:=.o) $(FUI_OBJS)

all: $(TESTS)

$(TESTS): %: %.o $(FUI_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TESTS) $(OBJS)

.PHONY: all clean
