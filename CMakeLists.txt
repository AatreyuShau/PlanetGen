cmake_minimum_required(VERSION 3.20)
project(PlanetGen VERSION 0.1.0 LANGUAGES CXX)

option(PLANETGEN_BUILD_VIEWER "Build the interactive X11 planet viewer" ON)

add_library(planetgen_core
    src/Noise.cpp
    src/Terrain.cpp
    src/Planet.cpp
)

target_include_directories(planetgen_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_compile_features(planetgen_core PUBLIC cxx_std_20)

target_compile_options(planetgen_core PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
)

if(PLANETGEN_BUILD_VIEWER)
    find_package(X11 REQUIRED)

    add_executable(planetgen_viewer app/main.cpp)
    target_link_libraries(planetgen_viewer PRIVATE planetgen_core X11::X11)
    target_compile_options(planetgen_viewer PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
    )
endif()
