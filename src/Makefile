CPPFLAGS=-std=c++17
LDFLAGS=
LIBS=-lc -lc++

OBJS = \
	chunk.o \
	handler.o \
	lingo.o \
	movie.o \
	projectorrays.o \
	stream.o \
	subchunk.o \
	util.o

projectorrays: $(OBJS)
	$(LD) -o projectorrays $(LDLAGS) $(OBJS) $(LIBS)
