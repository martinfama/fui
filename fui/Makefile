PREFIX = /usr/local
LIBDIR = $(PREFIX)/lib
INCLUDEDIR = $(PREFIX)/include/fui
PKGCONFIGDIR = $(PREFIX)/lib/pkgconfig
PKGCONFIGFILE = fui.pc

CC = gcc
CFLAGS = -Wall -Iinclude -O2
AR = ar
ARFLAGS = rcs
LFLAGS = -levdev -lm -lasound
LIBNAME = libfui.a

HEADERS = $(shell find . -name '*.h')
SRC = $(shell find . -name '*.c')
OBJ = $(SRC:.c=.o)

all: lib/$(LIBNAME)

lib/$(LIBNAME): $(OBJ)
	mkdir -p lib
	$(AR) $(ARFLAGS) $@ $^

%.o: %.c
	$(CC) -g $(CFLAGS) $(LFLAGS) -c $< -o $@

install: lib/$(LIBNAME)
	mkdir -p $(LIBDIR) $(INCLUDEDIR) $(PKGCONFIGDIR)
	install -m 644 lib/$(LIBNAME) $(LIBDIR)
	@$(foreach file, $(HEADERS), install -D $(file) $(INCLUDEDIR)/$(file);)
	install -m 644 $(PKGCONFIGFILE) $(PKGCONFIGDIR)
	@echo "Add $(PKGCONFIGDIR) to PKG_CONFIG_PATH to use pkg-config. Run: export PKG_CONFIG_PATH=$(PKGCONFIGDIR):\$$PKG_CONFIG_PATH"

uninstall:
	@echo "Uninstalling $(LIBNAME) from $(LIBDIR), headers from $(INCLUDEDIR), and pkg-config file from $(PKGCONFIGDIR)..."
	rm -f $(LIBDIR)/$(LIBNAME)
	rm -f $(shell find $(INCLUDEDIR) -type f -name '*.h')
	rm -f $(PKGCONFIGDIR)/$(PKGCONFIGFILE)
	@echo "Uninstall complete."

clean:
	rm -f $(OBJ)

help:
	@echo "Usage: make [target]"
	@echo "Targets:"
	@echo "  all        - Build the library"
	@echo "  install    - Install the library and headers"
	@echo "  uninstall  - Uninstall the library and headers"
	@echo "  clean      - Remove object files and library"
	@echo "  help       - Show this help message"

.PHONY: all install uninstall clean help