CXXFLAGS=-std=c++17 -Wall -Wextra -Isrc
LDFLAGS=-lz
LDFLAGS_RELEASE=-s -Os

ifeq ($(OS),Windows_NT)
	LDFLAGS+=-static -static-libgcc
endif

all: projectorrays

debug: CXXFLAGS+=-g -fsanitize=address
debug: LDFLAGS_RELEASE=
debug: projectorrays

OBJS = \
	src/main.o \
	src/common/fileio.o \
	src/common/log.o \
	src/common/stream.o \
	src/director/castmember.o \
	src/director/chunk.o \
	src/director/dirfile.o \
	src/director/handler.o \
	src/director/lingo.o \
	src/director/subchunk.o \
	src/director/util.o

projectorrays: $(OBJS)
	$(CXX) -o projectorrays $(CXXFLAGS) $(OBJS) $(LDFLAGS) $(LDFLAGS_RELEASE)

.PHONY: clean
clean:
	-rm projectorrays $(OBJS)
