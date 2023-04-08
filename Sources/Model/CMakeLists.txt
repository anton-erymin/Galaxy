set(MODEL_SOURCES
	Constants.h
	Galaxy.cpp
	Galaxy.h
	Particle.cpp
	Particle.h
	SphericalModel.cpp
	SphericalModel.h
	Universe.cpp
	Universe.h)
	
add_library(Galaxy.Model STATIC ${MODEL_SOURCES})
ConfigureGalaxyModule(Galaxy.Model "${MODEL_SOURCES}")
target_link_libraries(Galaxy.Model PRIVATE Nucleus.Core Galaxy.Utils)
