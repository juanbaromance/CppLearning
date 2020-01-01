set(ppc no)

if( ppc )
    set(CMAKE_C_COMPILER   ppc-gcc)
    set(CMAKE_CXX_COMPILER ppc-g++)
    set(tail ppc )
    set(CMAKE_CXX_FLAGS "-Os -ggdb")
else()
    set(tail local)
    set(CMAKE_CXX_FLAGS "-ggdb")
endif()

set(CMAKE_EXE_LINKER_FLAGS  "-lpthread -fPIC")

add_executable(testing.${tail}.x regular_polymorphism.cpp shapes_drawer.cpp)
