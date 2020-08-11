include_directories(../)
include_directories(.)

set(
    FILES
    main.cpp
    triangle.h
    ../util.h
    bench.h
)

add_executable(bench ${FILES})
target_link_libraries(bench evulkan)

configure_file("../compile.sh" "." COPYONLY)
configure_file("shader.frag" "." COPYONLY)
configure_file("shader.vert" "." COPYONLY)