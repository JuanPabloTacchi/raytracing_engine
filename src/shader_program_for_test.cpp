#include <cstddef>
#include <cstdlib>
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <random>


#include <vector>

#define CL_HPP_ENABLE_EXCEPTIONS
#include <CL/opencl.hpp>
#include <CL/cl_gl.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <windows.h>

cl::Program prog;
cl::CommandQueue queue;
cl::Context context;
cl::BufferGL clVBOs;
GLuint vbo;

//inits all of opencl
bool OpenClInit() {
    std::vector<cl::Platform> platforms;
    std::vector<cl::Device> devices;
    //se obtiene la plataforma y device
    cl::Platform::get(&platforms);
    for (auto& p : platforms) {
      p.getDevices(CL_DEVICE_TYPE_GPU, &devices);
      if (devices.size() > 0) break;
    }
    if (devices.size() == 0) {
      std::cerr << "Not GPU device found" << std::endl;
      return false;
    }

    std::cout << "GPU Used: " << devices.front().getInfo<CL_DEVICE_NAME>()
              << std::endl;

    context = cl::Context(devices.front());
    queue = cl::CommandQueue(context, devices.front());
    std::stringstream sourceCode;

  
    std::ifstream sourceFile("kernel.cl");
    sourceCode << sourceFile.rdbuf();
  

    try {
        prog = cl::Program(context, sourceCode.str());
        prog.build({devices.front()});
    } catch (cl::Error&) {
        std::cerr << prog.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices.front())
                  << std::endl;
        return false;
    }

    return true;
}

cl::Context CreateCLGLContext(cl::Device& outDevice) {
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    for (auto& platform : platforms) {
        std::vector<cl::Device> devices;
        platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);

        for (auto& device : devices) {
            //opengl uses another context than opencl, this line makes sure its using the same context
            cl_context_properties props[] = {
                CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
                CL_WGL_HDC_KHR,    (cl_context_properties)wglGetCurrentDC(),
                CL_CONTEXT_PLATFORM,
                (cl_context_properties)(platform()),
                0
            };

            try {
                cl::Context ctx(device, props);
                outDevice = device;
                return ctx;
            } catch (

            ) {
                std::cout << "error loading context"
              << std::endl;
            }
        }
    }

    throw std::runtime_error("No OpenCL-OpenGL interop device found");
}

void updateCL() {
    std::vector<cl::Memory> glObjects;
    glObjects.push_back(openglBufferVBO);

    queue.enqueueAcquireGLObjects(&glObjects);

    kernel.setArg(0, openglBufferVBO);
    queue.enqueueNDRangeKernel(
        kernel,
        cl::NullRange,
        cl::NDRange(NUM_VERTS),
        cl::NullRange
    );

    queue.enqueueReleaseGLObjects(&glObjects);
    queue.finish();
}

void renderGL(){
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(vao);
    glDrawArrays(GL_POINTS, 0, NUM_VERTS);

}

int main() {
    // -------------------------
    // GLFW / OpenGL
    // -------------------------
    if (!glfwInit()) {
        std::cerr << "GLFW init failed\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //creates window of 800x600
    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenCL + OpenGL + GLFW", nullptr, nullptr);
    if (!window) {
        std::cerr << "Window creation failed\n";
        glfwTerminate();
        return -1;
    }


    glfwMakeContextCurrent(window);

    // ---------------
    //glew init
    // ---------------
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW init failed\n";
        return -1;
    }


    std::cout << "OpenGL context created\n";

    // -------------------------
    // OpenCL
    // -------------------------

    //OpenClInit();


    // Creates vbo in opengl
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * 1024, nullptr, GL_DYNAMIC_DRAW);
    // OPencl takes the opengl VBO
    clVBOs = cl::BufferGL(context, CL_MEM_WRITE_ONLY, vbo);


    // -------------------------
    // Main loop
    // -------------------------
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        updateCL();
        renderGL();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    return 0;
}