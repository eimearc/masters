cmake_minimum_required (VERSION 3.1)
project (evulkan)

find_package(PkgConfig REQUIRED)
find_package(Microsoft.GSL CONFIG) # Type safety.
set (CMAKE_CXX_STANDARD 17)

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

# Add GLFW libraries.
find_package(glfw3 3.3 REQUIRED)
link_libraries(glfw)

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

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${VULKANROOT}/lib)
set(CMAKE_INSTALL_PREFIX ${VULKANROOT}/_install)
add_library(evulkan SHARED ${SOURCES})

install(TARGETS evulkan DESTINATION lib)
install(FILES ${VULKANROOT}/include/evulkan.h DESTINATION include)

# Build examples.
add_subdirectory(examples)

# Build tests.
add_subdirectory(tests)