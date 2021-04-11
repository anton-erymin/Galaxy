cmake_minimum_required(VERSION 3.17)

project(Galaxy)

# Compile options.
if (WIN32)
    # For grouping projects in folders in the solution.
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    
    # Unreferenced local variables.
    add_compile_options(/wd4101)

    # 'argument': truncation
    add_compile_options(/wd4305)
    
    # C4297: function assumed not to throw an exception but does
    add_compile_options(/wd4297)
	
    add_compile_options(/wd4834)

    # Bullet header warnings.
    add_compile_options(/wd5033 /wd4267 /wd4244)

    add_compile_options(/W3 /WX /MP)
 
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    
    if (BILLIARDS_SERVER)
        #add_compile_options(/MD)
        add_compile_definitions(BT_MANAGED_CODE B3_MANAGED_CODE)
    else()
        add_compile_options(/MD)
    endif()    
endif()

# Set common variables.
set(CMAKE_CXX_STANDARD 17)
set(LIBS_DIR ${CMAKE_SOURCE_DIR}/Libs)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Bin)

# ----------------------------------------

# Libs directories.
# Add definitions.

if (BILLIARDS_RENDER)
    # Libs directories related to renderer.
    include_directories(SYSTEM
        ${LIBS_DIR}/stb
        ${LIBS_DIR}/tinyobjloader
        ${LIBS_DIR}/imgui)
    
    if (WIN32)
        include_directories(SYSTEM
            ${LIBS_DIR}/gl
            ${LIBS_DIR}/freetype/include
            ${LIBS_DIR}/imgui/examples)
    endif()
else()
    
    add_compile_definitions(NO_RENDER)
    
endif()

if (BILLIARDS_SOUND)
    include_directories(SYSTEM ${LIBS_DIR}/AudioFile)
    include_directories(SYSTEM ${LIBS_DIR}/openal-soft/include)
else()
    add_compile_definitions(NO_SOUND)
endif()

include_directories(SYSTEM ${LIBS_DIR}/Bullet/Source)

if (BILLIARDS_LIGHTWEIGHT)
    add_compile_definitions(LIGHTWEIGHT)
endif()

if (BILLIARDS_SERVER)
    add_compile_definitions(BUILD_SERVER)
endif()

# Add source include directories.
include_directories(
    Sources/Nucleus/Common
    Sources/Nucleus/Controllers
    Sources/Nucleus/Data
    Sources/Nucleus/Engine
    Sources/Nucleus/IO
    Sources/Nucleus/Math
    Sources/Nucleus/Systems
    Sources/Nucleus/Utils
    Sources/Rules/EightBall 
    Sources/Rules/Russian 
    Sources/Rules/Snooker 
    Sources/Game
    Sources/Robot)
    
if (BILLIARDS_RENDER)
    # Add Renderer source include directories.
    include_directories(
        Sources/Nucleus/Loaders
        Sources/Nucleus/Render
        Sources/Nucleus/Render/GL
        Sources/Nucleus/ShaderManager
        Sources/Nucleus/UI
        Sources/Shaders)
endif()

if (BILLIARDS_SOUND)
    # Add Renderer source include directories.
    include_directories(
        Sources/Nucleus/Sound
        Sources/Nucleus/Sound/AL)
endif()
    
add_subdirectory(Sources/Nucleus)
add_subdirectory(Sources/Rules/EightBall)
add_subdirectory(Sources/Rules/Russian)
add_subdirectory(Sources/Rules/Snooker)
add_subdirectory(Sources/Robot)
add_subdirectory(Sources/Game)

# Set precompiled header from Nucleus to all other targets.
target_precompile_headers(RussianRules REUSE_FROM Nucleus)
target_precompile_headers(EightBallRules REUSE_FROM Nucleus)
target_precompile_headers(SnookerRules REUSE_FROM Nucleus)
target_precompile_headers(Robot REUSE_FROM Nucleus)
target_precompile_headers(Game REUSE_FROM Nucleus)


if (BILLIARDS_SERVER)
	include_external_msproject(Module 
		"${CMAKE_SOURCE_DIR}/Sources/Module/Module.vcxproj"
		GUID "{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}"	
		Game)	
	include_external_msproject(Tracer "${CMAKE_SOURCE_DIR}/Sources/Tracer/Tracer.csproj" Module)
	
	set_target_properties(Module PROPERTIES
		MAP_IMPORTED_CONFIG_DEBUG Debug
		MAP_IMPORTED_CONFIG_MINSIZEREL Release
		MAP_IMPORTED_CONFIG_RELWITHDEBINFO Debug
		MAP_IMPORTED_CONFIG_RELEASE Release)
		
	set_target_properties(Tracer PROPERTIES
		MAP_IMPORTED_CONFIG_DEBUG Debug
		MAP_IMPORTED_CONFIG_MINSIZEREL Release
		MAP_IMPORTED_CONFIG_RELWITHDEBINFO Debug
		MAP_IMPORTED_CONFIG_RELEASE Release)
endif()
