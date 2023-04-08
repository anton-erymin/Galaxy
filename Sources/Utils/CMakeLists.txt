set(UTILS_SOURCES
	MathUtils.cpp
	MathUtils.h)
	
add_library(Galaxy.Utils ${UTILS_SOURCES})
ConfigureGalaxyModule(Galaxy.Utils "${UTILS_SOURCES}")
target_link_libraries(Galaxy.Utils PRIVATE Nucleus.Core)
