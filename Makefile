VERSION_NUMBER=0.2.1
GIT_SHA=$(shell git rev-parse --short HEAD)

CPPFLAGS+=-DVERSION_NUMBER=$(VERSION_NUMBER) -DGIT_SHA=$(GIT_SHA)
CXXFLAGS+=-std=c++17 -Wall -Wextra -Isrc
LDLIBS+=-lz -lmpg123
LDFLAGS_RELEASE+=-s -Os

BINARY=projectorrays

ifeq ($(OS),Windows_NT)
# shlwapi is required by mpg123
	LDLIBS+=-lshlwapi
	LDFLAGS+=-static -static-libgcc
	BINARY=projectorrays.exe
endif

FONTMAPS = $(wildcard fontmaps/*.txt)
FONTMAP_HEADERS = $(patsubst %.txt,%.h,$(FONTMAPS))

.PHONY: all
all: $(BINARY)

fontmaps/%.h: $(patsubst %.h,%.txt,$@)
	xxd -i $(patsubst %.h,%.txt,$@) > $@

OBJS = \
	src/main.o \
	src/common/codewriter.o \
	src/common/fileio.o \
	src/common/json.o \
	src/common/log.o \
	src/common/options.o \
	src/common/stream.o \
	src/common/util.o \
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

$(BINARY): $(OBJS)
	$(CXX) -o $(BINARY) $(CPPFLAGS) $(CXXFLAGS) $(OBJS) $(LDFLAGS) $(LDFLAGS_RELEASE) $(LDLIBS)

debug: CXXFLAGS+=-g -fsanitize=address
debug: LDFLAGS_RELEASE=
debug: $(BINARY)

release: CPPFLAGS+=-DRELEASE_BUILD
release: $(BINARY)

.PHONY: clean
clean:
	-rm $(BINARY) $(FONTMAP_HEADERS) $(OBJS)
