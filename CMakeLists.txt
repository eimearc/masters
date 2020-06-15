cmake_minimum_required (VERSION 3.1)
project (evulkan)

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

set (SOURCES
    ${STB_INCLUDE_PATH}/gflags/gflags.h
    ${VULKANROOT}/src/buffer.cpp
    ${VULKANROOT}/src/descriptor.cpp
    ${VULKANROOT}/src/draw.cpp
    ${VULKANROOT}/src/evulkan_core.cpp
    ${VULKANROOT}/src/evulkan_util.cpp
    ${VULKANROOT}/src/instance.cpp
    ${VULKANROOT}/src/sync.cpp
    ${VULKANROOT}/src/run.cpp
    ${VULKANROOT}/src/swapchain.cpp
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

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${VULKANROOT}/lib)
set(CMAKE_INSTALL_PREFIX ${VULKANROOT}/_install)
add_library(evulkan SHARED ${SOURCES})
target_link_libraries(evulkan gflags) # Must only link to evulkan, otherwise get error in main.

install(TARGETS evulkan DESTINATION lib)
install(FILES ${VULKANROOT}/include/evulkan.h DESTINATION include)

set(
    MAIN_SOURCES
    ${VULKANROOT}/src/main.cpp
    ${UTILROOT}/src/bench.cpp
    ${UTILROOT}/src/util.cpp
)

add_executable(main ${MAIN_SOURCES})
target_link_libraries(main evulkan)
set_target_properties(main PROPERTIES RUNTIME_OUTPUT_DIRECTORY "../")
