set(ppc no)

if( ppc )
    set(CMAKE_C_COMPILER   ppc-gcc)
    set(CMAKE_CXX_COMPILER ppc-g++)
    set(tail ppc )
    set(CMAKE_CXX_FLAGS "-Os -ggdb")
else()
    set(tail local)
    set(CMAKE_CXX_FLAGS "-ggdb -std=c++1z")
endif()

add_executable(testing.${tail}.x mutex_condition.cpp)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lpthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
