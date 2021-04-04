CXXFLAGS=-std=c++17 -Wall -Wextra
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
	src/castmember.o \
	src/chunk.o \
	src/handler.o \
	src/lingo.o \
	src/movie.o \
	src/projectorrays.o \
	src/stream.o \
	src/subchunk.o \
	src/util.o

projectorrays: $(OBJS)
	$(CXX) -o projectorrays $(CXXFLAGS) $(OBJS) $(LDFLAGS) $(LDFLAGS_RELEASE)

.PHONY: clean
clean:
	-rm projectorrays $(OBJS)
