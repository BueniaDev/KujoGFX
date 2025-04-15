project(02-triangle)

set(EX_SOURCES
	02_triangle.cpp)

add_executable(02-triangle ${EX_SOURCES})
target_link_libraries(02-triangle PUBLIC kujogfx)

find_package(SDL3 REQUIRED)
target_compile_options(02-triangle PRIVATE ${COMPILE_OPTIONS})

target_link_libraries(02-triangle PUBLIC SDL3::SDL3)