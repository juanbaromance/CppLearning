cmake_minimum_required(VERSION 3.13)

set(ppc no)

if( ppc )
    set(CMAKE_C_COMPILER   ppc-gcc)
    set(CMAKE_CXX_COMPILER ppc-g++)
    set(tail ppc )
    set(CMAKE_CXX_FLAGS "-Os -ggdb")
    find_package(Boost COMPONENTS thread REQUIRED)
else()
    set(tail local)
    set(CMAKE_CXX_FLAGS "-ggdb -std=c++1z")
endif()

set( SOURCES FutureExecutorsThen.cpp mtq.hpp futures.hpp )
add_executable(testing.${tail}.x FutureExecutorsThen.cpp)
target_link_libraries(testing.${tail}.x pthread)
