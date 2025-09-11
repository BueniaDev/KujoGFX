project(spriv_relfection)

set(REFLECTION_SOURCES
	spirv_reflection.cpp)

add_executable(spirv_reflection ${REFLECTION_SOURCES})
target_link_libraries(spirv_reflection PUBLIC kujogfx)