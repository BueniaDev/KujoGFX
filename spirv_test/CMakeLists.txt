project(spriv_test)

set(TEST_SOURCES
	spirv_test.cpp)

add_executable(spirv_test ${TEST_SOURCES})
target_link_libraries(spirv_test PUBLIC kujogfx)