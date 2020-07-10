include_directories(../)

find_package(gflags REQUIRED)

set(
    FILES
    obj.cpp
    ../flags.cpp
    ../flags.h
)

add_executable(obj ${FILES})
target_link_libraries(obj evulkan gflags)

configure_file("../compile.sh" "." COPYONLY)

configure_file("viking_room.png" "." COPYONLY)
configure_file("viking_room.obj" "." COPYONLY)
configure_file("shader.frag" "." COPYONLY)
configure_file("shader.vert" "." COPYONLY)