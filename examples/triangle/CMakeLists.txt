include_directories(../)

set(
    FILES
    triangle.cpp
    ../util.h
)

set(
    SHADERS_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/shader.frag
    ${CMAKE_CURRENT_SOURCE_DIR}/shader.vert
)

set(
    SHADERS_BIN
    ${CMAKE_CURRENT_BINARY_DIR}/shader.frag
    ${CMAKE_CURRENT_BINARY_DIR}/shader.vert
)

set(
    SHADERS_SPV
    ${CMAKE_CURRENT_BINARY_DIR}/shader_frag.spv
    ${CMAKE_CURRENT_BINARY_DIR}/shader_vert.spv
)

add_custom_command(
    OUTPUT ${SHADERS_BIN}
    COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/shader* . && cp ${CMAKE_CURRENT_SOURCE_DIR}/compile.sh .
    COMMENT "Copying across shader source files."
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/shader.frag ${CMAKE_CURRENT_SOURCE_DIR}/shader.vert
)

add_custom_command(
    OUTPUT ${SHADERS_SPV}
    COMMAND ./compile.sh
    DEPENDS ${SHADERS_BIN}
)

add_custom_target(
    shaders ALL
    DEPENDS ${SHADERS_SPV}
)

add_executable(triangle ${FILES})
target_link_libraries(triangle evulkan)
add_dependencies(triangle shaders)

file(REMOVE ${SHADERS_BIN})