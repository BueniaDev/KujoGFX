project(01-clear)

set(CLEAR_SOURCES
	01_clear.cpp)

add_executable(${PROJECT_NAME} ${CLEAR_SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/examples/utils)
target_link_libraries(${PROJECT_NAME} PUBLIC kujogfx)

# Hack to speed up linking of Emscripten release builds
# (see https://github.com/WebAssembly/binaryen/issues/7705)

if (${KUJOGFX_PLATFORM} STREQUAL "Emscripten")
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-O1")
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-sASSERTIONS -sERROR_ON_WASM_CHANGES_AFTER_LINK -sWASM_BIGINT -O1")
else()
    find_package(SDL3 REQUIRED)
    target_link_libraries(${PROJECT_NAME} PUBLIC SDL3::SDL3)
endif()