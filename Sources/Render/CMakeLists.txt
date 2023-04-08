set(RENDER_SOURCES
	GalaxyRenderer.cpp
	GalaxyRenderer.h)
	
add_library(Galaxy.Render ${RENDER_SOURCES})
ConfigureGalaxyModule(Galaxy.Render "${RENDER_SOURCES}")
#target_link_libraries(Galaxy PRIVATE Nucleus.Launch Galaxy.GalaxySimulator)
