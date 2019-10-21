set(ppc no)

if( ppc )
    set(CMAKE_C_COMPILER   ppc-gcc)
    set(CMAKE_CXX_COMPILER ppc-g++)
    set(tail ppc )
    set(CMAKE_CXX_FLAGS "-O3 -g")
else()
    set(tail local)
    set(CMAKE_CXX_FLAGS "-O3 -g -std=c++1z")
endif()

add_executable(timer.${tail}.x timer.cpp)

set(dependencies boost_system )

if( ppc )


else()

# set_target_properties(timer.x PROPERTIES COMPILE_FLAGS "-save-temps")

endif()

target_link_libraries(timer.${tail}.x pthread ${dependencies} )



