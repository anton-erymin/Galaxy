set(GALAXY_SOURCES
   GalaxyEngine.cpp
   GalaxyEngine.h
   GalaxyConfiguration.h
   GalaxyUI.cpp
   GalaxyUI.h
   OrbitController.cpp
   OrbitController.h
   Main.cpp)

add_executable(Galaxy ${GALAXY_SOURCES})

add_subdirectory(Core)

target_link_libraries(Galaxy PRIVATE Galaxy.Core Nucleus.Engine)
