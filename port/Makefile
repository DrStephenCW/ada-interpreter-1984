# Build Arcturus 1.6-A with gcc on 64-bit (or 32-bit) Linux, x86 or ARM.
# Debian/Ubuntu: apt install gcc make.  Amazon Linux/Fedora: dnf install gcc make.
CC      = gcc
LIBDIR ?= $(CURDIR)/lib/
# the 1984 code assumes chars are signed, as on the VAX (ARM's are not)
CFLAGS  = -std=gnu89 -g -O0 -fno-strict-aliasing -fcommon -fsigned-char -w \
          -Icompat -Isrc -include src/decls64.h -DARCTURUS_LIB='"$(LIBDIR)"'

SRCS = $(wildcard src/*.c) $(wildcard compat/*.c)
OBJS = $(patsubst %.c,build/%.o,$(notdir $(SRCS)))
vpath %.c src compat

ada: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

build/%.o: %.c src/decls64.h | build
	$(CC) $(CFLAGS) -c $< -o $@

build:
	mkdir -p build

clean:
	rm -rf build ada
