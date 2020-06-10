STB_INCLUDE_PATH = /usr/local/include/

IDIR = include
IDIR_EXT = ../include
ODIR = obj
ODIR_EXT = ../obj
SDIR = src
SDIR_EXT = ../src

CC=g++
CFLAGS = -std=c++17 -I$(STB_INCLUDE_PATH) -I$(IDIR) -I$(IDIR_EXT)
GLEW_PATH = /usr/local/lib/
LDFLAGS = `pkg-config --static --libs glfw3` -L$(GLEW_PATH) -lGLEW -framework OpenGL  -L/usr/local/lib/ -lgflags

_DEPS = egl.h
_DEPS_EXT = cube.h grid.h vertex.h util.h flags.h bench.h threadpool.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))
DEPS += $(patsubst %,$(IDIR_EXT)/%,$(_DEPS_EXT))

_OBJ = egl.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

_OBJ_EXT = util.o flags.o bench.o
OBJ_EXT = $(patsubst %,$(ODIR_EXT)/%,$(_OBJ_EXT))

.PHONY: test clean

gl: $(OBJ) $(OBJ_EXT) $(ODIR)/main.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(ODIR)/%.o: $(SDIR)/%.cpp $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

$(ODIR_EXT)/%.o: $(SDIR_EXT)/%.cpp $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

libEGL.a: $(OBJ) $(OBJ_EXT)
	ar rcs $@ $^

libs: libEGL.a

clean:
	rm gl || true
	rm $(ODIR)/*.o || true
	rm libEGL.a || true
