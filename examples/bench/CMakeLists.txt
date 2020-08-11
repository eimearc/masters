include_directories(../)
include_directories(../multipass)
include_directories(.)

set(
    FILES
    main.cpp
    bench.h
    multipass.h
    obj.h
    triangle.h
    ../util.h
)

add_executable(bench ${FILES})
target_link_libraries(bench evulkan)

set(MODEL_DIR ./models)
set(TEXTURE_DIR ./textures)
set(SHADER_DIR ./shaders)

configure_file(${MODEL_DIR}/viking_room.obj "." COPYONLY)
configure_file(${TEXTURE_DIR}/viking_room.png "." COPYONLY)
configure_file(${SHADER_DIR}/obj_vert.spv "." COPYONLY)
configure_file(${SHADER_DIR}/obj_frag.spv "." COPYONLY)
configure_file(${SHADER_DIR}/pass_0_vert.spv "." COPYONLY)
configure_file(${SHADER_DIR}/pass_0_frag.spv "." COPYONLY)
configure_file(${SHADER_DIR}/pass_1_vert.spv "." COPYONLY)
configure_file(${SHADER_DIR}/pass_1_frag.spv "." COPYONLY)
configure_file(${SHADER_DIR}/triangle_vert.spv "." COPYONLY)
configure_file(${SHADER_DIR}/triangle_frag.spv "." COPYONLY)