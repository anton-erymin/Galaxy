add_subdirectory(Core)

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
ConfigureGalaxyModule(Galaxy "${GALAXY_SOURCES}")
target_link_options(Galaxy PRIVATE /SUBSYSTEM:WINDOWS)
target_link_libraries(Galaxy PRIVATE Galaxy.Core Nucleus.Launch)
