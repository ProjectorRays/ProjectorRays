CXXFLAGS=-std=c++17 -Wall -Wextra

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
	$(CXX) -o projectorrays $(CXXFLAGS) -lz $(OBJS)

.PHONY: clean
clean:
	-rm projectorrays $(OBJS)
