add_executable (testing testing.cpp)
target_link_libraries (testing LINK_PUBLIC Observers)
target_include_directories (testing PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
