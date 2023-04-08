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

set(CMAKE_MODULE_PATH ${NUCLEUS_DIR}/cmake)
include(SetupBuildOptions)
SetupBuildOptions()

add_compile_options(/MD)

# For ImGui
include_directories(${NUCLEUS_DIR}/Libs/imgui/Include)
# For Protobuf
include_directories(${NUCLEUS_DIR}/Libs/Protobuf/src)

set(BUNDLE_PATH ${NUCLEUS_DIR}/Sources/Nucleus/Engine/Core/Bundle.h)

function(ConfigureGalaxyModule name sources)
	target_compile_definitions(${name} PRIVATE NUCLEUS_BUILD_SHARED)
    target_precompile_headers(${name} PUBLIC ${BUNDLE_PATH})
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Source Files" FILES ${sources})
    set_target_properties(${name} PROPERTIES FOLDER "Galaxy")
endfunction()

add_subdirectory(Sources)
