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

add_executable(testing.${tail}.x main.cpp)
target_link_libraries(testing.${tail}.x pthread boost_system boost_regex)
