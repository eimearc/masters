cmake_minimum_required (VERSION 3.1)
project (evulkan)

find_package(PkgConfig REQUIRED)
set (CMAKE_CXX_STANDARD 14)

option(BUILD_EXAMPLES "Build the examples" ON)
option(BUILD_TESTS "Build the tests" ON)

# add_definitions(-DEVK_NDEBUG) # Uncomment to turn off assertations.

set(VULKANROOT .)
set(VULKAN_SDK $ENV{VULKAN_SDK})
set(STB_INCLUDE_PATH /usr/local/include)
set(STB_LIB_PATH /usr/local/lib)

set(VULKAN_INCLUDE ${VULKANROOT}/include)
set(VULKAN_SRC ${VULKANROOT}/src)

# Add headers.
include_directories (./include)
include_directories (${VULKAN_SDK}/include)
include_directories (${STB_INCLUDE_PATH})

# Add Vulkan libraries.
find_package(Vulkan REQUIRED)
link_libraries(Vulkan::Vulkan)

set (SOURCES
    ${VULKAN_SRC}/attachment.cpp
    ${VULKAN_SRC}/buffer.cpp
    ${VULKAN_SRC}/command.cpp
    ${VULKAN_SRC}/descriptor.cpp
    ${VULKAN_SRC}/device.cpp
    ${VULKAN_SRC}/draw.cpp
    ${VULKAN_SRC}/framebuffer.cpp
    ${VULKAN_SRC}/pass.cpp
    ${VULKAN_SRC}/pipeline.cpp
    ${VULKAN_SRC}/shader.cpp
    ${VULKAN_SRC}/sync.cpp
    ${VULKAN_SRC}/swapchain.cpp
    ${VULKAN_SRC}/sync.cpp
    ${VULKAN_SRC}/util.cpp
    ${VULKAN_SRC}/obj.cpp
    ${VULKAN_SRC}/texture.cpp
    ${VULKAN_SRC}/vertexinput.cpp
    ${VULKAN_INCLUDE}/threadpool.h
    ${VULKAN_INCLUDE}/vertex.h
)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${VULKANROOT}/lib) # TODO: Remove
set(CMAKE_INSTALL_PREFIX ${VULKANROOT}/_install) # TODO: Remove
add_library(evulkan SHARED ${SOURCES})

install(TARGETS evulkan DESTINATION lib)
install(FILES ${VULKANROOT}/include/evulkan.h DESTINATION include)

# Build examples.
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif(BUILD_EXAMPLES)

# Build tests.
if(BUILD_TESTS)
    add_subdirectory(tests)
endif(BUILD_TESTS)