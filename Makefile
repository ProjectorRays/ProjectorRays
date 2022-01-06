CXXFLAGS=-std=c++17 -Wall -Wextra -Isrc
LDFLAGS=-lz -lmpg123
LDFLAGS_RELEASE=-s -Os

ifeq ($(OS),Windows_NT)
	LDFLAGS+=-static -static-libgcc
endif

FONTMAPS = $(wildcard fontmaps/*.txt)
FONTMAP_HEADERS = $(patsubst %.txt,%.h,$(FONTMAPS))

fontmaps/%.h: $(patsubst %.h,%.txt,$@)
	xxd -i $(patsubst %.h,%.txt,$@) > $@

OBJS = \
	src/main.o \
	src/common/fileio.o \
	src/common/log.o \
	src/common/stream.o \
	src/director/castmember.o \
	src/director/chunk.o \
	src/director/dirfile.o \
	src/director/fontmap.o \
	src/director/guid.o \
	src/director/handler.o \
	src/director/lingo.o \
	src/director/sound.o \
	src/director/subchunk.o \
	src/director/util.o

src/director/fontmap.o: $(FONTMAP_HEADERS)

projectorrays: $(OBJS)
	$(CXX) -o projectorrays $(CXXFLAGS) $(OBJS) $(LDFLAGS) $(LDFLAGS_RELEASE)

all: projectorrays

debug: CXXFLAGS+=-g -fsanitize=address
debug: LDFLAGS_RELEASE=
debug: projectorrays

.PHONY: clean
clean:
	-rm projectorrays $(FONTMAP_HEADERS) $(OBJS)
