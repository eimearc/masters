# evulkan; a Vulkan graphics library
Bournemouth University MSc Computer Animation and Visual Effects 2019/2020 master's project.

Vulkan is a low-level graphics and compute API which aims to provide users with faster draw speeds
by removing overhead from the driver. The user is expected to explicitly provide the details previously
generated by the driver. The resulting extra code can be difficult to understand and taxing to write
for beginners, leading to the need for a helper library - evulkan.

evulkan is for newcomers to Vulkan and helps with setting up some of the more esoteric components 
of Vulkan including the swapchain, synchronization objects and the command buffers. It is suited to
multithreaded programming.

## Requirements

evulkan requires the tinyobjloader library (https://github.com/tinyobjloader/tinyobjloader) and 
the stb_image library (https://github.com/nothings/stb).

## Installation

evulkan is built and installed using CMake. Create a build directory at the root of the folder,
change to that directory, run `cmake ..`, followed by `make`, followed by `make install`. See
the instructions below for more information.

```shell
$ mkdir build
$ cd build
$ cmake ..
$ make
$ make install
```
The evulkan library will then be installed to your computer, along with the necessary header files.

## Use

The `examples` directory within the resulting `build` directory contain examples of how to use
evulkan. You can run the examples as following:

```shell
 triangle$ ./triangle
multipass$ ./multipass --num_threads 3
      obj$ ./ obj
```