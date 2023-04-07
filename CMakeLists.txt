cmake_minimum_required(VERSION 3.17)

project(Galaxy)

# Set common variables.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Bin)

# Setup Nucleus
set(NUCLEUS_DIR ${CMAKE_SOURCE_DIR}/Libs/Nucleus2)
set(BILLIARDS_BUILD_GAME OFF)
set(NUCLEUS_INCLUDE_FORMAT OFF)
add_subdirectory(${NUCLEUS_DIR})

# For ImGui
include_directories(${NUCLEUS_DIR}/Libs/imgui/Include)
# For Protobuf
include_directories(${NUCLEUS_DIR}/Libs/Protobuf/src)

add_subdirectory(Sources)

# Set precompiled header from Nucleus
set(BUNDLE_PATH ${NUCLEUS_DIR}/Sources/Nucleus/Engine/Core/Bundle.h)
target_precompile_headers(Galaxy.Core PUBLIC ${BUNDLE_PATH})
target_precompile_headers(Galaxy PUBLIC ${BUNDLE_PATH})
