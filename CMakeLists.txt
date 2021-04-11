cmake_minimum_required(VERSION 3.17)

project(Galaxy)

# Set common variables.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Bin)

# Setup Nucleus
set(LIBS_DIR ${CMAKE_SOURCE_DIR}/Libs)
set(NUCLEUS_DIR ${LIBS_DIR}/Nucleus/Sources/Nucleus)

#TODO: Make common startup system.
# For Application and WinApplication.
set(GAME_DIR ${LIBS_DIR}/Nucleus/Sources/Game)
include_directories(${GAME_DIR})

add_subdirectory(${NUCLEUS_DIR})
add_subdirectory(Sources)

# Set precompiled header from Nucleus to main target.
target_precompile_headers(Galaxy REUSE_FROM Nucleus)
