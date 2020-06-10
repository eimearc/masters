cmake_minimum_required (VERSION 3.1)
project (vulkan)

find_package(PkgConfig REQUIRED)
set (CMAKE_CXX_STANDARD 17)

set(VULKANROOT .)
set(UTILROOT .)
set(VULKAN_SDK $ENV{VULKAN_SDK})
set(STB_INCLUDE_PATH /usr/local/include)
set(STB_LIB_PATH /usr/local/lib)

# Add headers.
include_directories (./include)
include_directories (${UTILROOT}/include)
include_directories (${VULKAN_SDK}/include)
include_directories (${STB_INCLUDE_PATH})

# Add GLFW libraries.
find_package(glfw3 3.3 REQUIRED)
link_libraries(glfw)

# Add Vulkan libraries.
find_package(Vulkan REQUIRED)
link_libraries(Vulkan::Vulkan)

# Add gflags.
find_package(gflags REQUIRED)
link_libraries(gflags)

set (SOURCES
    ${STB_INCLUDE_PATH}/gflags/gflags.h
    ${VULKANROOT}/src/buffer.cpp
    ${VULKANROOT}/src/descriptor.cpp
    ${VULKANROOT}/src/draw.cpp
    ${VULKANROOT}/src/evulkan_core.cpp
    ${VULKANROOT}/src/evulkan_util.cpp
    ${VULKANROOT}/src/instance.cpp
    ${VULKANROOT}/src/main.cpp
    ${VULKANROOT}/src/render.cpp
    ${VULKANROOT}/src/run.cpp
    ${VULKANROOT}/src/update_scene.cpp
    ${VULKANROOT}/include/evulkan_core.h
    ${VULKANROOT}/include/evulkan_util.h
    ${VULKANROOT}/include/evulkan.h
    ${UTILROOT}/src/bench.cpp
    ${UTILROOT}/src/flags.cpp
    ${UTILROOT}/src/util.cpp
    ${UTILROOT}/include/bench.h
    ${UTILROOT}/include/util.h
    ${UTILROOT}/include/flags.h
    ${UTILROOT}/include/cube.h
    ${UTILROOT}/include/grid.h
    ${UTILROOT}/include/threadpool.h
    ${UTILROOT}/include/vertex.h
)

add_executable(vulkan ${SOURCES})
set_target_properties(vulkan PROPERTIES RUNTIME_OUTPUT_DIRECTORY "../")
