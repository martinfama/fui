PREFIX = /usr/local
LIBDIR = $(PREFIX)/lib
INCLUDEDIR = $(PREFIX)/include/fui

CC = gcc
CFLAGS = -Wall -Iinclude -O2
AR = ar
ARFLAGS = rcs
LFLAGS = -levdev -lm -lasound
LIBNAME = libfui.a

SRC = $(shell find . -name '*.c')
OBJ = $(SRC:.c=.o)

all: lib/$(LIBNAME)

lib/$(LIBNAME): $(OBJ)
	mkdir -p lib
	$(AR) $(ARFLAGS) $@ $^

%.o: %.c
	$(CC) -g $(CFLAGS) $(LFLAGS) -c $< -o $@

install: lib/$(LIBNAME)
	mkdir -p $(LIBDIR) $(INCLUDEDIR)
	install -d $(LIBDIR)
	install -m 644 lib/$(LIBNAME) $(LIBDIR)
	install -d $(INCLUDEDIR)
	@$(foreach file, $(shell find . -type f -name '*.h'), \
		install -D $(file) $(DESTDIR)$(INCLUDEDIR)/$(file);)

uninstall:
	@echo "Uninstalling $(LIBNAME) from $(LIBDIR) and headers from $(INCLUDEDIR)"
	rm -f $(LIBDIR)/$(LIBNAME)
	rm -f $(shell find $(INCLUDEDIR) -type f -name '*.h')
	@echo "Uninstall complete."

clean:
	rm -f $(OBJ) $(TARGET)
