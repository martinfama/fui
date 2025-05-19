PREFIX = /usr/local
LIBDIR = $(PREFIX)/lib
INCLUDEDIR = $(PREFIX)/include
PKGCONFIGDIR = $(PREFIX)/lib/pkgconfig
PKGCONFIGFILE = fui.pc

CC = gcc
CFLAGS = -Wall -Iinclude -O2
AR = ar
ARFLAGS = rcs
LFLAGS = -levdev -lm -lasound
LIBNAME = libfui.a

HEADERS = $(shell find ./fui/ -name '*.h')
SRC = $(shell find ./fui/ -name '*.c')
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
	rm -rf $(INCLUDEDIR)/fui
	rm -f $(PKGCONFIGDIR)/$(PKGCONFIGFILE)
	@echo "Uninstall complete."

clean:
	rm -f $(OBJ)

format:
	clang-format -i $(shell find . -name '*.c' -o -name '*.h')

help:
	@echo "Usage: make [target]"
	@echo "Targets:"
	@echo "  all        - Build the library"
	@echo "  install    - Install the library and headers"
	@echo "  uninstall  - Uninstall the library and headers"
	@echo "  clean      - Remove object files and library"
	@echo "  help       - Show this help message"
	@echo "  format     - Format source code using clang-format"

.PHONY: all install uninstall clean format help