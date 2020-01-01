cmake_minimum_required(VERSION 3.5)

project(futures LANGUAGES CXX)
set(CMAKE_CXX_FLAGS "-ggdb -std=c++1z")
add_executable(futures.x futures.cpp )
target_link_libraries(futures.x pthread)
