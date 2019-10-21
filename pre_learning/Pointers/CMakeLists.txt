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

if( ppc )
    add_executable(constexpr.x constexpr.cpp)
else()
    # add_executable(constexpr.x constexpr.cpp)
    add_executable(testing.${tail}.x main.cpp)
    target_link_libraries(testing.${tail}.x pthread boost_system boost_regex)
endif()

#set_target_properties(constexpr.x PROPERTIES COMPILE_FLAGS "-save-temps")

