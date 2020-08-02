CXXFLAGS=-std=c++17

OBJS = \
	src/chunk.o \
	src/handler.o \
	src/lingo.o \
	src/movie.o \
	src/projectorrays.o \
	src/stream.o \
	src/subchunk.o \
	src/util.o

projectorrays: $(OBJS)
	$(CXX) -o projectorrays $(CXXFLAGS) $(OBJS)

.PHONY: clean
clean:
	-rm projectorrays $(OBJS)
