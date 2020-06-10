VULKAN_SDK_PATH = $(VULKAN_SDK)
STB_INCLUDE_PATH = /usr/local/include/

IDIR = include
IDIR_EXT = ../include
ODIR = obj
ODIR_EXT = ../obj
SDIR = src
SDIR_EXT = ../src

CC=g++
CFLAGS = -std=c++17 -I$(VULKAN_SDK_PATH)/include -I$(STB_INCLUDE_PATH) -I$(STB_INCLUDE_PATH)gflags/ -I$(IDIR) -I$(IDIR_EXT)
LDFLAGS = `pkg-config --static --libs glfw3` -L$(VULKAN_SDK_PATH)/lib -lvulkan -L/usr/local/lib/ -lgflags
LDFLAGS += -L/usr/local/lib/ -lboost_thread-mt

_DEPS = evulkan.h evulkan_core.h evulkan_util.h
_DEPS_EXT = cube.h grid.h vertex.h util.h flags.h threadpool.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))
DEPS += $(patsubst %,$(IDIR_EXT)/%,$(_DEPS_EXT))

_OBJ =  descriptor.o instance.o render.o run.o buffer.o evulkan_core.o update_scene.o draw.o evulkan_util.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

_OBJ_EXT = util.o flags.o bench.o
OBJ_EXT = $(patsubst %,$(ODIR_EXT)/%,$(_OBJ_EXT))

.PHONY: test clean vulkan

vulkan: $(ODIR)/main.o $(OBJ) $(OBJ_EXT)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(ODIR)/%.o: $(SDIR)/%.cpp $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

$(ODIR_EXT)/%.o: $(SDIR_EXT)/%.cpp $(DEPS_EXT)
	$(CC) $(CFLAGS) -c -o $@ $<

libEVulkan.a: $(OBJ) $(OBJ_EXT)
	ar rcs $@ $^

libs: libEVulkan.a

clean:
	rm vulkan || true
	rm $(ODIR)/*.o || true
	rm libEVulkan.a || true
