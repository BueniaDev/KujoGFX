cmake_minimum_required(VERSION 3.15)
project(KujoGFX)

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
elseif(UNIX AND NOT APPLE)
    message(WARNING "Linux support is HIGHLY experimental, and may not even compile correctly, so be EXTREMELY careful here!")
    message(STATUS "Operating system is Linux.")

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

add_library(kujogfx INTERFACE ${KUJOGFX_HEADERS})
target_include_directories(kujogfx INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(external)
target_link_libraries(kujogfx INTERFACE glad)

if (${KUJOGFX_PLATFORM} STREQUAL "Win32")
    target_link_libraries(kujogfx INTERFACE ntdll.lib d3d11.lib d3dcompiler.lib)
elseif (${KUJOGFX_PLATFORM} STREQUAL "Wayland")
    find_package(Wayland REQUIRED)
    target_include_directories(kujogfx INTERFACE ${WAYLAND_INCLUDE_DIRS})
    target_link_libraries(kujogfx INTERFACE ${WAYLAND_LIBRARIES})
    target_compile_definitions(kujogfx INTERFACE KUJOGFX_IS_WAYLAND)
elseif (${KUJOGFX_PLATFORM} STREQUAL "X11")
    find_package(X11 REQUIRED)

    target_link_libraries(kujogfx INTERFACE OpenGL::EGL OpenGL::OpenGL)

    target_include_directories(kujogfx INTERFACE ${X11_INCLUDE_DIR})
    target_link_libraries(kujogfx INTERFACE ${X11_LIBRARIES})
    target_compile_definitions(kujogfx INTERFACE KUJOGFX_IS_X11)
endif()

find_package(OpenGL REQUIRED)
target_link_libraries(kujogfx INTERFACE OpenGL::GL)

find_package(Vulkan REQUIRED)
target_link_libraries(kujogfx INTERFACE Vulkan::Vulkan)

find_package(glslang REQUIRED)
target_link_libraries(kujogfx INTERFACE glslang)

find_package(SPIRV-Tools REQUIRED)

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
add_subdirectory(spirv_test)