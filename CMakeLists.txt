cmake_minimum_required (VERSION 3.1)
project (evulkan)

find_package(PkgConfig REQUIRED)
set (CMAKE_CXX_STANDARD 17)

set(VULKANROOT .)
set(VULKAN_SDK $ENV{VULKAN_SDK})
set(STB_INCLUDE_PATH /usr/local/include)
set(STB_LIB_PATH /usr/local/lib)

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

# Add gflags.
find_package(gflags REQUIRED)

set (SOURCES
    ${VULKANROOT}/src/attachment.cpp
    ${VULKANROOT}/src/buffer.cpp
    ${VULKANROOT}/src/descriptor.cpp
    ${VULKANROOT}/src/device.cpp
    ${VULKANROOT}/src/pass.cpp
    ${VULKANROOT}/src/pipeline.cpp
    ${VULKANROOT}/src/shader.cpp
    ${VULKANROOT}/src/draw.cpp
    ${VULKANROOT}/src/evulkan_core.cpp
    ${VULKANROOT}/src/evulkan_util.cpp
    ${VULKANROOT}/src/instance.cpp
    ${VULKANROOT}/src/sync.cpp
    ${VULKANROOT}/src/swapchain.cpp
    ${VULKANROOT}/src/util.cpp
    ${VULKANROOT}/src/obj.cpp
    ${VULKANROOT}/src/texture.cpp
    ${VULKANROOT}/include/evulkan_core.h
    ${VULKANROOT}/include/descriptor.h
    ${VULKANROOT}/include/attachment.h
    ${VULKANROOT}/include/pipeline.h
    ${VULKANROOT}/include/pass.h
    ${VULKANROOT}/include/shader.h
    ${VULKANROOT}/include/swapchain.h
    ${VULKANROOT}/include/texture.h
    ${VULKANROOT}/include/buffer.h
    ${VULKANROOT}/include/device.h
    ${VULKANROOT}/include/evulkan_util.h
    ${VULKANROOT}/include/threadpool.h
    ${VULKANROOT}/include/vertex.h
)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${VULKANROOT}/lib)
set(CMAKE_INSTALL_PREFIX ${VULKANROOT}/_install)
add_library(evulkan SHARED ${SOURCES})

install(TARGETS evulkan DESTINATION lib)
install(FILES ${VULKANROOT}/include/evulkan.h DESTINATION include)

set(MAIN_SOURCES
    ${VULKANROOT}/src/main.cpp
    ${VULKANROOT}/include/app.h
    ${VULKANROOT}/src/app.cpp
    ${STB_INCLUDE_PATH}/gflags/gflags.h
    ${VULKANROOT}/include/flags.h
    ${VULKANROOT}/src/flags.cpp
    ${VULKANROOT}/include/cube.h
    ${VULKANROOT}/include/grid.h
)

add_executable(main ${MAIN_SOURCES})
target_link_libraries(main gflags) # Must only link to main, otherwise get error in main.
target_link_libraries(main evulkan)
set_target_properties(main PROPERTIES RUNTIME_OUTPUT_DIRECTORY "../")
