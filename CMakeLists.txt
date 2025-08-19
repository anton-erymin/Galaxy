cmake_minimum_required(VERSION 3.17)

project(Galaxy)

# Set common variables.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Bin)

# Setup Nucleus
set(NUCLEUS_DIR ${CMAKE_SOURCE_DIR}/Libs/Nucleus)
set(NUCLEUS_BUILD_GAME OFF)
set(NUCLEUS_BUILD_SHARED ON)
set(NUCLEUS_INCLUDE_FORMAT OFF)
set(NUCLEUS_ENABLE_PATH_TRACER OFF)
# Overload launch module name
set(NUCLEUS_LAUNCH_MODULE_NAME "Galaxy")
# And main gameplay logic header file
set(NUCLEUS_GAME_MODULE_NAME "Galaxy.GalaxySimulator")
set(NUCLEUS_GAME_MODULE_CLASS_NAME "GameModule")
set(NUCLEUS_GAME_MODULE_HEADER_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Sources/GalaxySimulator/GalaxyModule.h")

# Finally add Nucleus
add_subdirectory(${NUCLEUS_DIR})

SetupBuildOptions()

# For ImGui
include_directories(${NUCLEUS_DIR}/Libs/imgui/Include)
# For Protobuf
include_directories(${NUCLEUS_DIR}/Libs/Protobuf/src)
# For ankerl
include_directories(${NUCLEUS_DIR}/Libs/ankerl)

set(BUNDLE_PATH ${NUCLEUS_DIR}/Sources/Nucleus/Engine/Core/Bundle.h)

function(ConfigureGalaxyModule name sources)
	target_compile_definitions(${name} PRIVATE GAMEPLAY_SHARED)    
	target_include_directories(${name} PUBLIC ${CMAKE_SOURCE_DIR}/Sources)
	target_include_directories(${name} PUBLIC ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
    target_precompile_headers(${name} PUBLIC ${BUNDLE_PATH})
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Source Files" FILES ${sources})
    set_target_properties(${name} PROPERTIES FOLDER "Galaxy")
endfunction()

# Add modules
#add_subdirectory(Sources/Controllers)
add_subdirectory(Sources/GalaxySimulator)
add_subdirectory(Sources/Model)
add_subdirectory(Sources/Render)
add_subdirectory(Sources/Simulation)
add_subdirectory(Sources/UI)
add_subdirectory(Sources/Utils)
