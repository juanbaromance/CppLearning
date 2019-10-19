set(tail local)
set(CMAKE_CXX_FLAGS "-ggdb -std=c++1z")


set(artefact testing.${tail}.x)
add_executable( ${artefact} variadics.cpp)
target_link_libraries( ${artefact} pthread)
