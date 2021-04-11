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

add_library(Core STATIC ${CORE_SOURCES})

target_include_directories(Core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(Core PRIVATE Nucleus)