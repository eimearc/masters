include_directories(../)
include_directories(../multipass)
include_directories(.)

set(
    FILES
    main.cpp
    bench.h
    triangle.h
    multipass.h
    ../util.h
)

add_executable(bench ${FILES})
target_link_libraries(bench evulkan)

configure_file("triangle_vert.spv" "." COPYONLY)
configure_file("triangle_frag.spv" "." COPYONLY)
configure_file("pass_0_vert.spv" "." COPYONLY)
configure_file("pass_0_frag.spv" "." COPYONLY)
configure_file("pass_1_vert.spv" "." COPYONLY)
configure_file("pass_1_frag.spv" "." COPYONLY)