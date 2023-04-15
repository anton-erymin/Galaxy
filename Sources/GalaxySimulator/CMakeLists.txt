set(GALAXY_SIMULATOR_SOURCES
	GalaxyTypes.h
	GalaxySimulator.cpp
	GalaxySimulator.h)
	
add_library(Galaxy.GalaxySimulator STATIC ${GALAXY_SIMULATOR_SOURCES})
ConfigureGalaxyModule(Galaxy.GalaxySimulator "${GALAXY_SIMULATOR_SOURCES}")
target_link_libraries(Galaxy.GalaxySimulator PRIVATE
	Galaxy.Render
	Galaxy.UI
	Galaxy.Model
	Galaxy.Simulation
	Nucleus.Engine)
