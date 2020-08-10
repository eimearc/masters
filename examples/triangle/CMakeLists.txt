include_directories(../)

set(
    FILES
    triangle.cpp
    ../util.h
)

add_executable(triangle ${FILES})
target_link_libraries(triangle evulkan)

configure_file("../compile.sh" "." COPYONLY)
configure_file("shader.frag" "." COPYONLY)
configure_file("shader.vert" "." COPYONLY)