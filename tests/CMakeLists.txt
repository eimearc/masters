include_directories(../include)

find_package(GTest REQUIRED)

enable_testing()

set(
    FILES
    attachment_test.cpp
    buffer_test.cpp
    command_test.cpp
    descriptor_test.cpp
    device_test.cpp
    framebuffer_test.cpp
    main.cpp
    obj_test.cpp
    pass_test.cpp
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    link_directories(/usr/local/lib)
endif()

add_executable(tests ${FILES})
target_link_libraries(tests evulkan gtest)

configure_file("shader_frag.spv" "." COPYONLY)
configure_file("shader_vert.spv" "." COPYONLY)
configure_file("tri.obj" "." COPYONLY)