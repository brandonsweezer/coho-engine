cmake_minimum_required(VERSION 3.1...3.25)
project(
    Renderer
    VERSION 0.1.0
    LANGUAGES CXX C
)
add_subdirectory(SDL2)
add_subdirectory(webgpu)
add_subdirectory(sdl2webgpu)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src")
file(GLOB_RECURSE SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)

add_executable(Renderer
    ${SRC_FILES}
)

target_include_directories(Renderer PRIVATE .)
target_compile_definitions(Renderer PRIVATE RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resources")
target_link_libraries(Renderer PRIVATE SDL2::SDL2 webgpu sdl2webgpu)

target_copy_webgpu_binaries(Renderer)
set_target_properties(Renderer PROPERTIES
    CXX_STANDARD 17
    COMPILE_WARNING_AS_ERROR ON
)

# Define a custom function to copy SDL2 binaries
function(target_copy_sdl2_binaries target)
    # CMAKE_BINARY_DIR is /build
    file(GLOB SDL2_DLLS "${CMAKE_BINARY_DIR}/SDL2/Debug/*.dll")
    
    foreach(dll ${SDL2_DLLS})
        add_custom_command(
            TARGET ${target}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${dll} $<TARGET_FILE_DIR:${target}>
        )
    endforeach()
endfunction()

target_copy_sdl2_binaries(Renderer)


if (MSVC)
    target_compile_options(Renderer PRIVATE /W4)
    # Ignore a warning that GLM requires to bypass
	# Disable warning C4201: nonstandard extension used: nameless struct/union
	target_compile_options(Renderer PUBLIC /wd4201)
	# Disable warning C4305: truncation from 'int' to 'bool' in 'if' condition
	target_compile_options(Renderer PUBLIC /wd4305)

	# Ignore a warning that stb_image requires to bypass
	# Disable warning C4244: conversion from 'int' to 'short', possible loss of data
	target_compile_options(Renderer PUBLIC /wd4244)
else()
    target_compile_options(Renderer PRIVATE -Wall -Wextra -pedantic)
endif()

