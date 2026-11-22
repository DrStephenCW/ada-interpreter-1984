# Build Arcturus 1.6-A on 32-bit-capable Linux.  Needs gcc with -m32
# (Debian/Ubuntu: apt install gcc-multilib).
CC      = gcc
LIBDIR ?= $(CURDIR)/lib/
CFLAGS  = -m32 -std=gnu89 -g -O0 -fno-strict-aliasing -fcommon -w -Icompat -Isrc \
          -DARCTURUS_LIB='"$(LIBDIR)"'
TAPE   ?= ../ada
REBUILT?= ..

SRCS = $(wildcard src/*.c) $(wildcard compat/*.c)
OBJS = $(patsubst %.c,build/%.o,$(notdir $(SRCS)))
vpath %.c src compat

ada: $(OBJS)
	$(CC) -m32 $(LDFLAGS) -o $@ $(OBJS)

build/%.o: %.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build:
	mkdir -p build

clean:
	rm -rf build ada
