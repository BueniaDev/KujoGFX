cmake_minimum_required(VERSION 3.15)
project(KujoGFX)

include(FetchContent)

# Require C++17 (and position independent code)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

if (WIN32)
    message(STATUS "Operating system is Windows.")
    set(KUJOGFX_PLATFORM "Win32")
    set(KUJOGFX_VULKAN 1)
    if (CMAKE_CXX_COMPILER_ID STREQUAL GNU)
	message(STATUS "Compiler is MinGW.")
	set(COMPILE_OPTIONS "-Wall")
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL MSVC)
	message(WARNING "MSVC support is HIGHLY experimental, and may not even compile correctly, so be EXTREMELY careful here!")
	message(STATUS "Compiler is MSVC.")
	set(COMPILE_OPTIONS "/W4")
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL Clang)
	message(WARNING "Clang support on Windows is HIGHLY experimental, and may not even compile correctly, so be EXTREMELY careful here!")
	message(STATUS "Compiler is Clang.")
	set(COMPILE_OPTIONS "-Wall")
    else()
	message(SEND_ERROR "Compiler not supported.")
	return()
    endif()
elseif(EMSCRIPTEN)
    message(WARNING "Emscripten support is HIGHLY experimental, and may not even compile correctly, so be EXTREMELY careful here!")
    message(STATUS "Operating system is Emscripten.")
    set(KUJOGFX_PLATFORM "Emscripten")
    set(KUJOGFX_VULKAN 0)

    if (CMAKE_CXX_COMPILER_ID STREQUAL Clang)
	message(STATUS "Compiler is Clang.")
	set(COMPILE_OPTIONS "-Wall")
    else()
	message(SEND_ERROR "Compiler not supported.")
	return()
    endif()
elseif(UNIX AND NOT APPLE)
    message(WARNING "Linux support is HIGHLY experimental, and may not even compile correctly, so be EXTREMELY careful here!")
    message(STATUS "Operating system is Linux.")
    set(KUJOGFX_VULKAN 1)

    set(display_server_info $ENV{XDG_SESSION_TYPE})

    message(STATUS ${display_server_info})

    if ("${display_server_info}" STREQUAL "x11")
	set(KUJOGFX_PLATFORM "X11")
    elseif ("${display_server_info}" STREQUAL "wayland")
	set(KUJOGFX_PLATFORM "Wayland")
    else()
	message(WARNING "Could not determine display server type, assuming X11...")
	set(KUJOGFX_PLATFORM "X11")
    endif()

    if (CMAKE_CXX_COMPILER_ID STREQUAL GNU)
	message(STATUS "Compiler is GCC.")
	set(COMPILE_OPTIONS "-Wall")
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL Clang)
	message(STATUS "Compiler is Clang.")
	set(COMPILE_OPTIONS "-Wall")
    else()
	message(SEND_ERROR "Compiler not supported.")
	return()
    endif()
elseif(APPLE)
    message(WARNING "MacOS support is HIGHLY experimental, and may not even compile correctly, so be EXTREMELY careful here!")
    message(STATUS "Operating system is MacOS.")
    set(KUJOGFX_PLATFORM "MacOS")
    set(KUJOGFX_VULKAN 1)
    if (CMAKE_CXX_COMPILER_ID STREQUAL AppleClang)
	message(STATUS "Compiler is AppleClang.")
	set(COMPILE_OPTIONS "-Wall")
    else()
	message(SEND_ERROR "Compiler not supported.")
	return()
    endif()
else()
    message(SEND_ERROR "Operating system not supported.")
    return()
endif()

set(KUJOGFX_HEADERS kujogfx.h)

message(LOG "${CMAKE_CURRENT_SOURCE_DIR}")
add_library(kujogfx INTERFACE ${KUJOGFX_HEADERS})
target_include_directories(kujogfx INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(external)
target_link_libraries(kujogfx INTERFACE glad)

find_package(OpenGL REQUIRED)
target_link_libraries(kujogfx INTERFACE OpenGL::GL)

if (${KUJOGFX_PLATFORM} STREQUAL "Win32")
    target_link_libraries(kujogfx INTERFACE ntdll.lib d3d11.lib d3dcompiler.lib)
elseif (${KUJOGFX_PLATFORM} STREQUAL "Wayland")
    find_package(Wayland REQUIRED)
    target_include_directories(kujogfx INTERFACE ${WAYLAND_INCLUDE_DIRS})
    target_link_libraries(kujogfx INTERFACE ${WAYLAND_LIBRARIES})
    target_compile_definitions(kujogfx INTERFACE KUJOGFX_IS_WAYLAND)
elseif (${KUJOGFX_PLATFORM} STREQUAL "X11")
    find_package(X11 REQUIRED)
    target_include_directories(kujogfx INTERFACE ${X11_INCLUDE_DIR})
    target_link_libraries(kujogfx INTERFACE ${X11_LIBRARIES})
    target_compile_definitions(kujogfx INTERFACE KUJOGFX_IS_X11)
endif()

if (KUJOGFX_VULKAN)
find_package(Vulkan REQUIRED)
target_link_libraries(kujogfx INTERFACE Vulkan::Vulkan)
endif()

# Fetch dependencies

FetchContent_Declare(
    SPIRV_Headers
    GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Headers.git
    GIT_TAG vulkan-sdk-1.4.321.0
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(SPIRV_Headers)

FetchContent_Declare(
    spirv_cross
    GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Cross.git
    GIT_TAG vulkan-sdk-1.4.321.0
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(spirv_cross)

FetchContent_Declare(
    spirv-tools
    GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Tools.git
    GIT_TAG vulkan-sdk-1.4.321.0
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(spirv-tools)

FetchContent_Declare(
    glslang
    GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git
    GIT_TAG vulkan-sdk-1.4.321.0
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(glslang)

target_include_directories(kujogfx INTERFACE "${SPIRV_Headers_SOURCE_DIR}/include")

target_link_libraries(kujogfx INTERFACE glslang)

set(SPIRV_LIBRARIES
	SPIRV-Tools
	SPIRV-Tools-opt
	spirv-cross-core
	spirv-cross-cpp
	spirv-cross-glsl
	spirv-cross-hlsl
)

target_link_libraries(kujogfx INTERFACE ${SPIRV_LIBRARIES})

add_subdirectory(examples/01-clear)
add_subdirectory(examples/02-triangle)
add_subdirectory(examples/03-triangle-vertex)
add_subdirectory(spirv_test)
add_subdirectory(spirv_reflection)