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

target_link_libraries(Galaxy Nucleus)
