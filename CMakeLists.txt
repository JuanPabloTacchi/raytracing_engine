cmake_minimum_required(VERSION 3.20)

project(Raytracing_engine)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
#opencl
find_package(OpenCL REQUIRED)

#opengl
find_package(OpenGL REQUIRED)

#glew
find_package(GLEW REQUIRED)


#glfw
find_package(glfw3 CONFIG REQUIRED)

add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(extern)
add_subdirectory(test)





