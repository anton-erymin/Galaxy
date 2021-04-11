set(GALAXY_SOURCES
   GalaxyApplication.cpp
   GalaxyApplication.h
   GalaxyEngine.cpp
   GalaxyEngine.h
   GalaxyConfiguration.h
   GalaxyUI.cpp
   GalaxyUI.h
   OrbitController.cpp
   OrbitController.h
   ${GAME_DIR}/WinApplication.cpp
   ${GAME_DIR}/WinApplication.h
   Main.cpp)

add_executable(Galaxy ${GALAXY_SOURCES})

add_subdirectory(Core)

target_include_directories(Galaxy PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(Galaxy Nucleus Core)
