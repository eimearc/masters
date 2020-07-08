cmake_minimum_required (VERSION 3.1)
project (examples)

find_package(PkgConfig REQUIRED)
set (CMAKE_CXX_STANDARD 17)

set(VULKANROOT .)
set(VULKAN_SDK $ENV{VULKAN_SDK})
set(STB_INCLUDE_PATH /usr/local/include)
set(STB_LIB_PATH /usr/local/lib)

include_directories (${VULKAN_SDK}/include)
include_directories (${STB_INCLUDE_PATH})
include_directories (../include)

# Add GLFW libraries.
find_package(glfw3 3.3 REQUIRED)
link_libraries(glfw)

# Add Vulkan libraries.
find_package(Vulkan REQUIRED)
link_libraries(Vulkan::Vulkan)

# Add gflags library.
find_package(gflags REQUIRED)

link_directories(../build/lib)

set(COMMON_SRC
    app.cpp
    app.h
    cube.h
    flags.h
    flags.cpp
    grid.h
    ${STB_INCLUDE_PATH}/gflags/gflags.h
    ../include/evulkan_core.h)

set(MULTIPASS_SOURCES multipass.cpp ${COMMON_SRC})
add_executable(multipass ${MULTIPASS_SOURCES})
target_link_libraries(multipass evulkan gflags)
set_target_properties(multipass PROPERTIES RUNTIME_OUTPUT_DIRECTORY "../")

set(OBJ_SOURCES obj.cpp ${COMMON_SRC})
add_executable(obj ${OBJ_SOURCES})
target_link_libraries(obj evulkan gflags)
set_target_properties(obj PROPERTIES RUNTIME_OUTPUT_DIRECTORY "../")
