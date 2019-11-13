set(sources
    interpreter.cpp
    interpreter.hpp
    main.cpp
    program.hpp
)

set( PROJECT_SOURCE_DIR ../ )
set( CMAKE_CXX_FLAGS "-O0 -g -std=c++1z")
add_executable(lang_vm ${sources})
target_include_directories(lang_vm
    PRIVATE
    ${PROJECT_SOURCE_DIR}/shared
)
