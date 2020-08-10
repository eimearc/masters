# Add GLFW libraries.
find_package(glfw3 3.3 REQUIRED)
link_libraries(glfw)

add_subdirectory(multipass)
add_subdirectory(obj)
add_subdirectory(triangle)