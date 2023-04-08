set(LAUNCHER_SOURCES
	GalaxyEngine.cpp
	GalaxyEngine.h
	Main.cpp)
	
add_executable(Galaxy ${LAUNCHER_SOURCES})
ConfigureGalaxyModule(Galaxy "${LAUNCHER_SOURCES}")
target_link_options(Galaxy PRIVATE /SUBSYSTEM:WINDOWS)
target_link_libraries(Galaxy PRIVATE Nucleus.Launch Galaxy.GalaxySimulator)
