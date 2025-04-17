project(glad)


if (WIN32)
    add_library(glad INTERFACE glad/gl.h glad/wgl.h)
else()
    add_library(glad INTERFACE glad/gl.h glad/egl.h)
endif()

target_include_directories(glad INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})