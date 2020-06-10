VULKAN_SDK_PATH = $(VULKAN_SDK)

IDIR = include
V_IDIR = vulkan/include
GL_IDIR = gl/include
ODIR = obj
SDIR = src

CC=g++
CFLAGS = -std=c++17 -I$(IDIR) -I$(GL_IDIR) -I$(V_IDIR)
GLEW_PATH = /usr/local/lib/
DEPS = $(V_IDIR)/evulkan.h $(GL_IDIR)/egl.h $(IDIR)/grid.h

LDFLAGS = `pkg-config --static --libs glfw3`
LDFLAGS += -L$(GLEW_PATH) -lGLEW -framework OpenGL -Lgl -lEGL
LDFLAGS += -L$(VULKAN_SDK_PATH)/lib -lvulkan -Lvulkan -lEVulkan
LDFLAGS += -L/usr/local/lib/ -lgflags

_OBJ = util.o bench.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

all: $(OBJ) vulkan gl

vulkan:
	cd vulkan && $(MAKE)

gl:
	cd gl && $(MAKE)

$(ODIR)/%.o: $(SDIR)/%.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	cd vulkan && $(MAKE) clean
	cd gl && $(MAKE) clean
	rm $(ODIR)/*.o

.PHONY: vulkan gl all