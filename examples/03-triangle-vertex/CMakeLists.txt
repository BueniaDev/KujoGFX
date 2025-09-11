project(03-triangle-vertex)

set(EX_SOURCES
	03_triangle_vertex.cpp)

add_executable(03-triangle-vertex ${EX_SOURCES})
target_link_libraries(03-triangle-vertex PUBLIC kujogfx)

if (${KUJOGFX_PLATFORM} STREQUAL "Emscripten")
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
else()
    find_package(SDL3 REQUIRED)
    target_compile_options(03-triangle-vertex PRIVATE ${COMPILE_OPTIONS})
    target_link_libraries(03-triangle-vertex PUBLIC SDL3::SDL3)
endif()