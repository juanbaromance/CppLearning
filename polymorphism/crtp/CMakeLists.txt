SET(CMAKE_C_COMPILER   ppc-gcc)
SET(CMAKE_CXX_COMPILER ppc-g++)
set(CMAKE_CXX_FLAGS "-Os -ggdb")
set(CMAKE_EXE_LINKER_FLAGS  "-lpthread -fPIC")

add_executable(testing main.cpp)

# install(TARGETS testing RUNTIME)
