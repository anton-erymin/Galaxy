set(GALAXY_SIMULATOR_SOURCES
	GalaxySimulator.cpp
	GalaxySimulator.h)
	
add_library(Galaxy.GalaxySimulator STATIC ${GALAXY_SIMULATOR_SOURCES})
ConfigureGalaxyModule(Galaxy.GalaxySimulator "${GALAXY_SIMULATOR_SOURCES}")
target_link_libraries(Galaxy.GalaxySimulator PUBLIC Galaxy.Model Nucleus.Engine)
