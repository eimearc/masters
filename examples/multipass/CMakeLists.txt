include_directories(../)

find_package(gflags REQUIRED)

set(
    FILES
    cube.h
    grid.h
    multipass.cpp
    ../flags.cpp
    ../util.h
)

set(
    SHADERS_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/pass_0.frag"
    "${CMAKE_CURRENT_SOURCE_DIR}/pass_0.vert"
    "${CMAKE_CURRENT_SOURCE_DIR}/pass_1.frag"
    "${CMAKE_CURRENT_SOURCE_DIR}/pass_1.vert"
    "${COMPILE_SRC_SCRIPT}"
)

set(
    SHADERS_BIN
    "${CMAKE_CURRENT_BINARY_DIR}/pass_0.frag"
    "${CMAKE_CURRENT_BINARY_DIR}/pass_0.vert"
    "${CMAKE_CURRENT_BINARY_DIR}/pass_1.frag"
    "${CMAKE_CURRENT_BINARY_DIR}/pass_1.vert"
    "${CMAKE_CURRENT_BINARY_DIR}/compile.sh"
)

set(
    SHADERS_SPV
    "${CMAKE_CURRENT_BINARY_DIR}/pass_0_frag.spv"
    "${CMAKE_CURRENT_BINARY_DIR}/pass_0_vert.spv"
    "${CMAKE_CURRENT_BINARY_DIR}/pass_1_frag.spv"
    "${CMAKE_CURRENT_BINARY_DIR}/pass_1_vert.spv"
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
    shaders_multipass ALL
    DEPENDS ${SHADERS_SPV}
)

add_executable(multipass ${FILES})
target_link_libraries(multipass evulkan gflags)
add_dependencies(multipass shaders_multipass)