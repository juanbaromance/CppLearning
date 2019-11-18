set(ppc no)
set(artefact main)
if( ppc )
    set(CMAKE_C_COMPILER   ppc-gcc)
    set(CMAKE_CXX_COMPILER ppc-g++)
    set(tail ppc )
    set(CMAKE_CXX_FLAGS "-O3 -g")
else()
    set(tail local)
    set(CMAKE_CXX_FLAGS "-O3 -std=c++17")
endif()

add_executable(${artefact}.${tail}.x main.cpp)
include_directories( ./ )
set(dependencies boost_system )

if( ppc )

else()

# set_target_properties(${artefact}.${tail}.x PROPERTIES COMPILE_FLAGS "-save-temps")

endif()

target_link_libraries(${artefact}.${tail}.x pthread fmt ${dependencies} )



