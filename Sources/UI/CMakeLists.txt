set(UI_SOURCES
	MainWindow.cpp
	MainWindow.h)
	
add_library(Galaxy.UI STATIC ${UI_SOURCES})
ConfigureGalaxyModule(Galaxy.UI "${UI_SOURCES}")
target_link_libraries(Galaxy.UI PRIVATE Nucleus.Engine)
