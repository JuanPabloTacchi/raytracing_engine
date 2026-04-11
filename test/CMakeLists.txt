#Testing
enable_testing()

#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/archivoprueba.txt
#        ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

add_executable(
        Tests
        test.cpp
)

add_executable(
        Tests_cornell_box
        test_cornell_box.cpp
)


#https://google.github.io/googletest/quickstart-cmake.html
target_link_libraries(
        Tests
        gtest_main
        OpenCL::OpenCL
)

target_link_libraries(
        Tests_cornell_box
        gtest_main
        OpenCL::OpenCL
)

target_include_directories(Tests PUBLIC 
     ${CMAKE_SOURCE_DIR}/include
)

target_include_directories(Tests_cornell_box PUBLIC 
     ${CMAKE_SOURCE_DIR}/include
)

include(GoogleTest)
gtest_discover_tests(Tests)
gtest_discover_tests(Tests_cornell_box)