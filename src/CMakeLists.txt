



add_library(Camera
    Camera.cpp
)
target_include_directories(Camera PUBLIC 
     ${CMAKE_SOURCE_DIR}/include
    )
#Assimp
#find_package(assimp REQUIRED)
#
#add_library(SceneLoader
#    SceneLoader.cpp
#)
#target_link_libraries(SceneLoader
#    Private assimp::assimp
#)

add_executable(ocl_glfw
    main.cpp
)

target_include_directories(ocl_glfw PRIVATE
    ${OpenCL_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src/kernels
    ${CMAKE_SOURCE_DIR}/src/kernels/include
)

target_link_libraries(ocl_glfw
    OpenGL::GL
    OpenCL::OpenCL
    glfw
    GLEW::GLEW
    Camera
)

#file(COPY ${CMAKE_SOURCE_DIR}/src/raytracing_kernel.cl
#     DESTINATION ${CMAKE_BINARY_DIR}/src)

file(COPY ${CMAKE_SOURCE_DIR}/src/shaders/fragment.glsl
     DESTINATION ${CMAKE_BINARY_DIR}/src/shaders)
file(COPY ${CMAKE_SOURCE_DIR}/src/shaders/vertex.glsl
     DESTINATION ${CMAKE_BINARY_DIR}/src/shaders)


add_custom_target(copy_kernels ALL
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/src/kernels/raytracing_kernel.cl
        ${CMAKE_BINARY_DIR}/src/kernels/raytracing_kernel.cl
)

add_custom_target(copy_kernels_include ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/src/kernels
        ${CMAKE_BINARY_DIR}/src/include
)

add_dependencies(ocl_glfw copy_kernels)


