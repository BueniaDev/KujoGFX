project(01-clear)

set(CLEAR_SOURCES
	01_clear.cpp)

add_executable(01-clear ${CLEAR_SOURCES})
target_link_libraries(01-clear PUBLIC kujogfx)

if (${KUJOGFX_PLATFORM} STREQUAL "Emscripten")
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
else()
    find_package(SDL3 REQUIRED)
    target_compile_options(01-clear PRIVATE ${COMPILE_OPTIONS})
    target_link_libraries(01-clear PUBLIC SDL3::SDL3)
endif()