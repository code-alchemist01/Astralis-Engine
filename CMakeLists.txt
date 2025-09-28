cmake_minimum_required(VERSION 3.21)
project(procedural_universe VERSION 1.0.0 LANGUAGES CXX)

# C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Create extern directory for dependencies
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/extern)

# Include FetchContent for dependency management
include(FetchContent)

# GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# OpenGL - using core profile without extension loader

# GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
)
FetchContent_MakeAvailable(glm)

# spdlog
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.12.0
)
FetchContent_MakeAvailable(spdlog)

# FastNoiseLite
FetchContent_Declare(
    FastNoiseLite
    GIT_REPOSITORY https://github.com/Auburn/FastNoiseLite.git
    GIT_TAG v1.1.1
)
FetchContent_MakeAvailable(FastNoiseLite)

# ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.90.1
)
FetchContent_MakeAvailable(imgui)

# Create ImGui library
set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC 
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${CMAKE_BINARY_DIR}/_deps/glfw-src/include
)
target_link_libraries(imgui PUBLIC glfw)

# Source files
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS 
    "src/*.cpp"
    "src/*.hpp"
)

# Create executable
add_executable(procedural_universe ${SOURCES})

# Include directories
target_include_directories(procedural_universe PRIVATE 
    src
    extern
    extern/glad/include
    ${CMAKE_BINARY_DIR}/_deps/glfw-src/include
    ${CMAKE_BINARY_DIR}/_deps/glm-src
    ${CMAKE_BINARY_DIR}/_deps/spdlog-src/include
    ${CMAKE_BINARY_DIR}/_deps/fastnoiselite-src/Cpp
    ${CMAKE_BINARY_DIR}/_deps/imgui-src
    ${CMAKE_BINARY_DIR}/_deps/imgui-src/backends
)

# Link libraries
target_link_libraries(procedural_universe PRIVATE 
    glfw
    glm::glm
    spdlog::spdlog
    imgui
)

# Platform-specific libraries
if(WIN32)
    target_link_libraries(procedural_universe PRIVATE opengl32)
endif()

# Compiler flags
if(MSVC)
    target_compile_options(procedural_universe PRIVATE /W4 /permissive-)
    # Enable multi-processor compilation
    target_compile_options(procedural_universe PRIVATE /MP)
else()
    target_compile_options(procedural_universe PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Copy assets to build directory
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})

# Set working directory for debugging
set_target_properties(procedural_universe PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)