project(SMLforDoor)
set(artefact main)

set(tail local)
set(CMAKE_CXX_FLAGS "-O3 -std=c++17")

add_executable(${artefact}.${tail}.x main.cpp)
include_directories( ./ )
set(dependencies boost_system )

target_link_libraries(${artefact}.${tail}.x pthread fmt ${dependencies} )



