#Testing
enable_testing()

#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/archivoprueba.txt
#        ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

add_executable(
        Tests
        test.cpp
)


#https://google.github.io/googletest/quickstart-cmake.html
target_link_libraries(
        Tests
        gtest_main
        OpenCL::OpenCL
)

target_include_directories(Tests PUBLIC 
     ${CMAKE_SOURCE_DIR}/include
    )

include(GoogleTest)
gtest_discover_tests(helpTests)