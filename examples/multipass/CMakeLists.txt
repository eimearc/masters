include_directories(../)

find_package(gflags REQUIRED)

set(
    FILES
    multipass.cpp
    ../app.cpp
    ../app.h
    ../flags.cpp
    ../flags.h
    ../grid.h
)

add_executable(multipass ${FILES})
target_link_libraries(multipass evulkan gflags)

configure_file("../compile.sh" "." COPYONLY)
file(GLOB SHADERS_F *.frag)
file(GLOB SHADERS_V *.vert)
foreach(SHADER ${SHADERS_F})
    configure_file(${SHADER} "." COPYONLY)
endforeach()
foreach(SHADER ${SHADERS_V})
    configure_file(${SHADER} "." COPYONLY)
endforeach()