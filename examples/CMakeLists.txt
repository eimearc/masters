# Add GLFW libraries.
find_package(glfw3 3.3 REQUIRED)
link_libraries(glfw)

set(EXAMPLE_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(COMPILE_SRC_SCRIPT "${EXAMPLE_SRC_DIR}/compile.sh")

add_subdirectory(bench)
add_subdirectory(multipass)
add_subdirectory(obj)
add_subdirectory(triangle)