include_directories(../)

add_executable(triangle triangle.cpp)
target_link_libraries(triangle evulkan)

configure_file("../compile.sh" "." COPYONLY)
configure_file("shader.frag" "." COPYONLY)
configure_file("shader.vert" "." COPYONLY)