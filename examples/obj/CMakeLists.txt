find_package(gflags REQUIRED)
include_directories(../)

set(
    FILES
    obj.cpp
    ../flags.cpp
    ../util.h
)

set(
    SHADERS_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/shader.frag"
    "${CMAKE_CURRENT_SOURCE_DIR}/shader.vert"
    "${COMPILE_SRC_SCRIPT}"
)

set(
    SHADERS_BIN
    "${CMAKE_CURRENT_BINARY_DIR}/shader.frag"
    "${CMAKE_CURRENT_BINARY_DIR}/shader.vert"
    "${CMAKE_CURRENT_BINARY_DIR}/compile.sh"
)

set(
    SHADERS_SPV
    "${CMAKE_CURRENT_BINARY_DIR}/shader_frag.spv"
    "${CMAKE_CURRENT_BINARY_DIR}/shader_vert.spv"
)

add_custom_command(
    OUTPUT ${SHADERS_BIN}
    COMMAND cp ${SHADERS_SRC} .
    COMMENT "Copying across shader source files."
    DEPENDS ${SHADERS_SRC}
)

add_custom_command(
    OUTPUT ${SHADERS_SPV}
    COMMAND ./compile.sh && rm ${SHADERS_BIN}
    COMMENT "Compiling shader files and removing redundant files."
    DEPENDS ${SHADERS_BIN}
)

add_custom_target(
    shaders_obj ALL
    DEPENDS ${SHADERS_SPV}
)

add_executable(obj ${FILES})
target_link_libraries(obj evulkan gflags)
add_dependencies(obj shaders_obj)

configure_file("viking_room.png" "." COPYONLY)
configure_file("viking_room.obj" "." COPYONLY)