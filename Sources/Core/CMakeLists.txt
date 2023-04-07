set(CORE_SOURCES
   Constants.h
   Galaxy.cpp
   Galaxy.h
   Solver.cpp
   Solver.h
   SphericalModel.cpp
   SphericalModel.h
   BarnesHutTree.cpp
   BarnesHutTree.h
   Threading.cpp
   Threading.h
   MathUtils.cpp
   MathUtils.h)

add_library(Galaxy.Core STATIC ${CORE_SOURCES})
target_link_libraries(Galaxy.Core PRIVATE Nucleus.Core Nucleus.Engine)
