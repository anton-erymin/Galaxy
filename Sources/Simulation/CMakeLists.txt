set(SIMULATION_SOURCES
	BarnesHut/BarnesHutCPUTree.h
	BarnesHut/BarnesHutCPUTree.cpp
	BarnesHut/BarnesHutGPUTree.h
	BarnesHut/BarnesHutGPUTree.cpp
	Solvers/BarnesHutCPUSolver.cpp
	Solvers/BarnesHutCPUSolver.h
	Solvers/BarnesHutGPUSolver.cpp
	Solvers/BarnesHutGPUSolver.h
	Solvers/BruteforceCPUSolver.cpp
	Solvers/BruteforceCPUSolver.h
	Solvers/BruteforceGPUSolver.cpp
	Solvers/BruteforceGPUSolver.h
	Solvers/ISolver.h)
	
add_library(Galaxy.Simulation ${SIMULATION_SOURCES})
ConfigureGalaxyModule(Galaxy.Simulation "${SIMULATION_SOURCES}")
target_link_libraries(Galaxy.Simulation PRIVATE
	Galaxy.Model
	Galaxy.Utils
	Nucleus.Core)
