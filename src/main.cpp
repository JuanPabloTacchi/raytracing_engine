#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_TARGET_OPENCL_VERSION 200
#define CL_HPP_MINIMUM_OPENCL_VERSION 120

#include <cstddef>
#include <cstdlib>
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <random>
#include "Camera.h"
#include "Vec.h"
#include "Vec_gpu.h"
#include "object.h"


#include <vector>

#include <CL/opencl.hpp>
#include <CL/cl_gl.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <windows.h>


void enable_cuda_build_cache(bool enable)
{
#ifdef _MSC_VER
    if (enable)
        _putenv("CUDA_CACHE_DISABLE=0");
    else
        _putenv("CUDA_CACHE_DISABLE=1");
#else // GCC
    if (enable)
        putenv("CUDA_CACHE_DISABLE=0");
    else
        putenv("CUDA_CACHE_DISABLE=1");
#endif
}


cl::Program prog;
cl::CommandQueue queue;
cl::Context context;
cl::BufferGL clVBOs;
cl::ImageGL clImage;

GLuint vbo;
GLuint texture;
GLuint shaderProgram;
GLuint fullscreenVAO;

Camera camera;
float speed = 0.1;


std::vector<Object> objects;
std::vector<cl_float4> triangle_points;
std::vector<int> triangle_indexes;


int WIDTH = 600;
int HEIGHT = 800;


//---------------------------
// IDEA
//
//la idea es crear una textura que ocupe toda la pantalla y luego aplicar a cada pixel
//de estos el color calculado segun el motor de raytracing
//
// -----------------------------------------


void loadScene()
{
    //piso 
    cl_float4 p1 = {0, 0, 0, 0};
    triangle_points.push_back(p1);
    cl_float4 p2 = {15, 0, 0, 0};
    triangle_points.push_back(p2);
    cl_float4 p3 = {0, 0, 15, 0};
    triangle_points.push_back(p3);
    //objeto
    cl_float4 p4 = {5, 0, 0, 0};
    triangle_points.push_back(p4);
    cl_float4 p5 = {5, 0, 5, 0};
    triangle_points.push_back(p5);
    cl_float4 p6 = {5, 5, 0, 0};
    triangle_points.push_back(p6);


    triangle_indexes.insert(triangle_indexes.end(), {1,2,3,4,5,6});

    Object o1;
    HittableObj ho1;
    ho1.Material = EMISSIVE;
    o1.hitObj = ho1;
    o1.n_triangles = 1;
    o1.i_0 = 0;
    o1.i_f = 2;

    Object o2;
    HittableObj ho2;
    ho2.Material = DIFUSSE;
    o2.n_triangles = 1;
    o2.hitObj = ho2;
    o2.i_0 = 3;
    o2.i_f = 5;

    objects.push_back(o1);
    objects.push_back(o2);
    return;
}



//inits all of opencl
bool OpenClKernelInit(cl::Device& outDevice) {
    
    queue = cl::CommandQueue(context, outDevice);
    std::stringstream sourceCode;

  
    std::ifstream sourceFile("kernels/raytracing_kernel.cl");
    sourceCode << sourceFile.rdbuf();
  

    try {
        prog = cl::Program(context, sourceCode.str());
        prog.build({outDevice});
    } catch (cl::Error&) {
        std::string log =
        prog.getBuildInfo<CL_PROGRAM_BUILD_LOG>(outDevice);
        std::cerr << "Build log:\n" << log << std::endl;
        throw;
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
                context = ctx;
                outDevice = device;
                return ctx;
            } catch (cl::Error&) {
                std::cout << "error loading context"
              << std::endl;
            }
        }
    }

    throw std::runtime_error("No OpenCL-OpenGL interop device found");
}

void Movement(GLFWwindow* window){
    Vec3 newPos;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
        newPos = camera.getPosition() + camera.getDirection()*speed;
        camera.Move_Camera(newPos, camera.getDirection());
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
        newPos = camera.getPosition() - camera.getDirection()*speed;
        camera.Move_Camera(newPos, camera.getDirection());
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
        newPos = camera.getPosition() + camera.getLeft()*speed;
        camera.Move_Camera(newPos, camera.getDirection());
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
        newPos = camera.getPosition() + camera.getRight()*speed;
        camera.Move_Camera(newPos, camera.getDirection());
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS){ //rotate left
        Vec3 newDir = (camera.getDirection() + camera.getLeft() * sinf(speed/2));
        camera.Move_Camera(camera.getPosition(), newDir);
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS){ //rotate left
        Vec3 newDir = (camera.getDirection() + camera.getRight() * sinf(speed/2));
        camera.Move_Camera(camera.getPosition(), newDir);
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){ //rotate left
        Vec3 newDir = (camera.getDirection() + camera.getUp() * sinf(speed/2));
        camera.Move_Camera(camera.getPosition(), newDir);
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){ //rotate left
        Vec3 newDir = (camera.getDirection() + camera.getDown() * sinf(speed/2));
        camera.Move_Camera(camera.getPosition(), newDir);
    }
    //newPos = camera.getPosition() + Vec3(1,0,1);
    

}


void updateCL(float time) {
    //para mi yo del pasado que no se acuerda que es 
    //cl::Memory es cualquier objeto que vive en memoria de opencl tipo buffer, imagen, buffers de opengl, etc
    try{
        std::vector<cl::Memory> objs = { clImage };

        queue.enqueueAcquireGLObjects(&objs);

        cl::Kernel kernel(prog, "main_kernel");



        //std::cout << "points: " << triangle_points.size() << std::endl;
        //std::cout << "objects: " << objects.size() << std::endl;
        //std::cout << "indexes: " << triangle_indexes.size() << std::endl;
    
        cl::Buffer buffer_points(
            context,
            CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
            sizeof(cl_float4) * triangle_points.size(),
            triangle_points.data()
        );

        cl::Buffer buffer_objects(
            context,
            CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
            sizeof(Object) * objects.size(),
            objects.data()
        );

        cl::Buffer buffer_indexes(
            context,
            CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
            sizeof(int) * triangle_indexes.size(),
            triangle_indexes.data()
        );

        kernel.setArg(0, clImage);
        kernel.setArg(1, WIDTH);
        kernel.setArg(2, HEIGHT);
        kernel.setArg(3, buffer_objects);
        kernel.setArg(4, (int)objects.size());
        kernel.setArg(5, buffer_points);
        kernel.setArg(6, (int)triangle_points.size());
        kernel.setArg(7, buffer_indexes);
        kernel.setArg(8, (int)triangle_indexes.size());
        kernel.setArg(9, time);
        kernel.setArg(10, camera.getU().toCLF4());
        kernel.setArg(11, camera.getV().toCLF4());
        kernel.setArg(12, camera.getDirection().toCLF4());
        kernel.setArg(13, camera.getViewportCenter().toCLF4());
        kernel.setArg(14, camera.getPosition().toCLF4());

        queue.enqueueNDRangeKernel(
            kernel,
            cl::NullRange,
            cl::NDRange(WIDTH, HEIGHT),
            cl::NullRange
        );

        queue.enqueueReleaseGLObjects(&objs);
        queue.finish();
    }
    catch (cl::Error& e) {
        std::cerr << "OpenCL Error: " << e.what()
                  << " (" << e.err() << ")" << std::endl;
        throw;
    }
}

//dibuja la textura en la ventana
void renderGL(){
    //limpia la ventana
    glClear(GL_COLOR_BUFFER_BIT);
    //utiliza el pipeline shaderProgram
    glUseProgram(shaderProgram);

    //asigna al shader la textura modificada por el raytracing
    

    glUniform1i(glGetUniformLocation(shaderProgram, "tex"), 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    //añade el VAO (que no hace nada pero es obligatorio)
    glBindVertexArray(fullscreenVAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);

}

int main() {

    enable_cuda_build_cache(false);


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



    // Creates texture in opengl
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    //define la textura (si lo sobrexplico cada cosa es porque no me acuerdo mucho)
    glTexImage2D(
        GL_TEXTURE_2D, //textura bidimensional
        0,              //mipmap 0 
        GL_RGBA32F,     // float por canal (ideal raytracing) 4 floats de 32 bits RGBA
        WIDTH,          //se explica solo
        HEIGHT,
        0,              //border
        GL_RGBA,        //el orden de los datos (RGBA)
        GL_FLOAT,       //cada canal del rgba es float
        nullptr         //datos iniciales
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    //crea el VAO (es solo para tener un dato y que se calcule)
    glGenVertexArrays(1, &fullscreenVAO);
    glBindVertexArray(fullscreenVAO);

    //Crea el shader program que sera usado para dibujar en pantalla
    std::stringstream vshader_stream; //lee vertex shader
    std::string vshader_string;
    std::ifstream sourceFilesvshader("shaders/vertex.glsl");
    vshader_stream << sourceFilesvshader.rdbuf();
    vshader_string = vshader_stream.str();
    //se crea el vertex shader
    GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
    const char* src = vshader_string.c_str();
    //se compila el vertex shader
    glShaderSource(vshader, 1, &src, nullptr);
    glCompileShader(vshader);


    GLint success;
    glGetShaderiv(vshader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        GLint logSize = 0;
        glGetShaderiv(vshader, GL_INFO_LOG_LENGTH, &logSize);

        std::string log(logSize, ' ');
        glGetShaderInfoLog(vshader, logSize, nullptr, log.data());

        std::cerr << "Vertex Shader Compilation Error:\n";
        std::cerr << log << std::endl;
    }
    else
    {
        std::cout << "Vertex Shader compiled successfully\n";
    }


    //lee el fragment shader
    std::stringstream fshader_stream;
    std::string fshader_string;
    std::ifstream sourceFilesfshader("shaders/fragment.glsl");
    fshader_stream << sourceFilesfshader.rdbuf();
    fshader_string = fshader_stream.str();
    //crea el fragment shader
    GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* srcf = fshader_string.c_str();
    //compila el fragment shader
    glShaderSource(fshader, 1, &srcf, nullptr);
    glCompileShader(fshader);
    //crea el programa/pipelin shaderProgram
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vshader);
    glAttachShader(shaderProgram, fshader);

    //link del pipeline de opengl
    glLinkProgram(shaderProgram);

    GLint linkSuccess;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linkSuccess);

    if (!linkSuccess)
    {
        GLint logSize = 0;
        glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &logSize);

        std::string log(logSize, ' ');
        glGetProgramInfoLog(shaderProgram, logSize, nullptr, log.data());

        std::cerr << "Shader Program Link Error:\n";
        std::cerr << log << std::endl;
    }
    else
    {
        std::cout << "Shader Program linked successfully\n";
    }


    

    //crea un device null que sera el compartido con opengl
    cl::Device device;

    // ------------------
    //  Pipeline Init
    // ------------------
    CreateCLGLContext(device); //esta funcion setea el device y context para que sea el mismo que opengl
    OpenClKernelInit(device);


    //------------------
    // OPENCL
    //
    // Me rendi de escribir en ingles obviamente me canse
    // OpenCl Envuelve la textura de OpenGl
    clImage = cl::ImageGL(
        context,
        CL_MEM_WRITE_ONLY,
        GL_TEXTURE_2D,
        0,
        texture
    );

    //-----------------
    // creates camera
    // -----------------

    Vec3 origin = Vec3(0,0,0);
    Vec3 direction = Vec3(1,0,0);
    Vec3 up = Vec3(0,1,0);
    camera = Camera(WIDTH, HEIGHT, origin, direction, up);

    //carga la escena
    loadScene();


    //Randomizer
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(0.0f, 500.0f);

    // -------------------------
    // Main loop
    // -------------------------
    while (!glfwWindowShouldClose(window)) {
        //std::cout << camera.getPosition();
        //std::cout << "main ptr: " << &camera << std::endl;
        //std::cout << "address: " << &camera << " pos: " << camera.getPosition() << std::endl;

        glClear(GL_COLOR_BUFFER_BIT);
        Movement(window);
        float random_value = dist(gen);
        //std::cout << camera.getViewportCenter() << std::endl;
        //std::cout << camera.getU() << std::endl;
        //std::cout << camera.getV() << std::endl;
        //std::cout << random_value << std::endl;
        //std::cout << camera.getV() << std::endl;
        updateCL(random_value);
        renderGL();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    //clReleaseCommandQueue(queue);
    //clReleaseContext(context);
    return 0;
}