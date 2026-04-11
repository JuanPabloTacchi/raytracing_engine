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



int WIDTH = 1920;
int HEIGHT = 1080;

std::vector<cl_float3> cumulative_pixels(WIDTH * HEIGHT);
bool moved = false;
int max_samples = 200;
int cumulated_time = 1;

//---------------------------
// IDEA
//
//la idea es crear una textura que ocupe toda la pantalla y luego aplicar a cada pixel
//de estos el color calculado segun el motor de raytracing
//
// -----------------------------------------


void loadScenebeta()
{
    //piso 
    cl_float4 p1 = {0, 0, 0, 0};
    triangle_points.push_back(p1);
    cl_float4 p2 = {15, 0, 0, 0};
    triangle_points.push_back(p2);
    cl_float4 p3 = {0, 0, 15, 0};
    triangle_points.push_back(p3);
    //objeto
    cl_float4 p4 = {5, 5, 0, 0};
    triangle_points.push_back(p4);
    cl_float4 p5 = {5, 5, 5, 0};
    triangle_points.push_back(p5);
    cl_float4 p6 = {5, 10, 0, 0};
    triangle_points.push_back(p6);


    triangle_indexes.insert(triangle_indexes.end(), {0,1,2,3,4,5});

    Object o1;
    HittableObj ho1;
    ho1.Material = EMISSIVE;
    ho1.rgb = {1,1,1};
    o1.hitObj = ho1;
    o1.n_triangles = 1;
    o1.i_0 = 0;
    o1.i_f = 2;

    Object o2;
    HittableObj ho2;
    ho2.Material = DIFUSSE;
    ho2.rgb = {0,1,0};
    o2.n_triangles = 1;
    o2.hitObj = ho2;
    o2.i_0 = 3;
    o2.i_f = 5;

    objects.push_back(o1);
    objects.push_back(o2);
    return;
}


void loadScene()
{
    // ======== Points ========

    // floor
    cl_float4 p0 = {0, 0, 0, 0};
    cl_float4 p1 = {10, 0, 0, 0};
    cl_float4 p2 = {10, 0, 10, 0};
    cl_float4 p3 = {0, 0, 10, 0};

    // ceil
    cl_float4 p4 = {0, 10, 0, 0};
    cl_float4 p5 = {10, 10, 0, 0};
    cl_float4 p6 = {10, 10, 10, 0};
    cl_float4 p7 = {0, 10, 10, 0};

    // light 
    cl_float4 l0 = {3, 9.9, 3, 0};
    cl_float4 l1 = {7, 9.9, 3, 0};
    cl_float4 l2 = {7, 9.9, 7, 0};
    cl_float4 l3 = {3, 9.9, 7, 0};

    
    triangle_points.insert(triangle_points.end(), {
        p0,p1,p2,p3,
        p4,p5,p6,p7,
        l0,l1,l2,l3
    });

    // ===== small box =====
    cl_float4 b0 = {2,0,6,0};
    cl_float4 b1 = {4,0,6,0};
    cl_float4 b2 = {4,0,8,0};
    cl_float4 b3 = {2,0,8,0};

    cl_float4 b4 = {2,3,6,0};
    cl_float4 b5 = {4,3,6,0};
    cl_float4 b6 = {4,3,8,0};
    cl_float4 b7 = {2,3,8,0};

    // ===== big box =====
    cl_float4 t0 = {6,0,2,0};
    cl_float4 t1 = {8,0,2,0};
    cl_float4 t2 = {8,0,5,0};
    cl_float4 t3 = {6,0,5,0};

    cl_float4 t4 = {6,7,2,0};
    cl_float4 t5 = {8,7,2,0};
    cl_float4 t6 = {8,7,5,0};
    cl_float4 t7 = {6,7,5,0};

    triangle_points.insert(triangle_points.end(), {
        b0,b1,b2,b3,b4,b5,b6,b7,
        t0,t1,t2,t3,t4,t5,t6,t7
    });

    // ======== triangles ========

    // floor
    triangle_indexes.insert(triangle_indexes.end(), {0,2,1, 0,3,2});

    // ceiling
    triangle_indexes.insert(triangle_indexes.end(), {4,5,6});
    triangle_indexes.insert(triangle_indexes.end(), {4,6,7});

    // back wall 
    triangle_indexes.insert(triangle_indexes.end(), {3,6,2});
    triangle_indexes.insert(triangle_indexes.end(), {3,7,6});

    // left wall (red one)
    triangle_indexes.insert(triangle_indexes.end(), {0,7,3});
    triangle_indexes.insert(triangle_indexes.end(), {0,4,7});

    // right wall (green one)
    triangle_indexes.insert(triangle_indexes.end(), {1,2,6});
    triangle_indexes.insert(triangle_indexes.end(), {1,6,5});

    // light 
    triangle_indexes.insert(triangle_indexes.end(), {8,10,9});
    triangle_indexes.insert(triangle_indexes.end(), {8,11,10});

    triangle_indexes.insert(triangle_indexes.end(), {
    // small box
    12,13,14,  12,14,15,

    //  (normal +Y)
    16,18,17,  16,19,18,

    //  -Z
    12,17,13,  12,16,17,

    //  +Z
    15,14,18,  15,18,19,

    //  -X
    12,15,19,  12,19,16,

    //  +X
    13,17,18,  13,18,14,

    // big box
    //  (normal -Y)
    20,21,22,  20,22,23,

    //  (normal +Y)
    24,26,25,  24,27,26,

    //  -Z
    20,25,21,  20,24,25,

    //  +Z
    23,22,26,  23,26,27,

    //  -X
    20,23,27,  20,27,24,

    //  +X
    21,25,26,  21,26,22
    });

    // ======== objects ========

    // floor (white)
    Object floor;
    HittableObj hoFloor;
    floor.n_triangles = 2;
    floor.i_0 = 0;
    floor.i_f = 5;
    hoFloor.Material = DIFUSSE;
    hoFloor.rgb = {0.8,0.8,0.8};
    floor.hitObj = hoFloor;

    // ceiling (white)
    Object ceiling;
    HittableObj hoCeil;
    ceiling.n_triangles = 2;
    ceiling.i_0 = 6;
    ceiling.i_f = 11;
    hoCeil.Material = DIFUSSE;
    hoCeil.rgb = {0.8,0.8,0.8};
    ceiling.hitObj = hoCeil;

    // back wall (white)
    Object back;
    HittableObj hoBack;
    back.n_triangles = 2;
    back.i_0 = 12;
    back.i_f = 17;
    hoBack.Material = DIFUSSE;
    hoBack.rgb = {0.8,0.8,0.8};
    back.hitObj = hoBack;

    // left wall (red)
    Object left;
    HittableObj hoLeft;
    left.n_triangles = 2;
    left.i_0 = 18;
    left.i_f = 23;
    hoLeft.Material = DIFUSSE;
    hoLeft.rgb = {1,0,0};
    left.hitObj = hoLeft;

    // right wall (green)
    Object right;
    HittableObj hoRight;
    right.n_triangles = 2;
    right.i_0 = 24;
    right.i_f = 29;
    hoRight.Material = DIFUSSE;
    hoRight.rgb = {0,1,0};
    right.hitObj = hoRight;

    // light
    Object light;
    HittableObj hoLight;
    light.n_triangles = 2;
    light.i_0 = 30;
    light.i_f = 35;
    hoLight.Material = EMISSIVE;
    hoLight.rgb = {1.0f, 0.95f, 0.8f};
    light.hitObj = hoLight;

    //small box
    Object box1;
    HittableObj hoBox1;
    box1.n_triangles = 12;
    box1.i_0 = 36;
    box1.i_f = 71;
    hoBox1.Material = DIFUSSE;
    hoBox1.rgb = {1,1,1};
    box1.hitObj = hoBox1;

    //big box
    Object box2;
    HittableObj hoBox2;
    box2.n_triangles = 12;
    box2.i_0 = 72;
    box2.i_f = 107;
    hoBox2.Material = DIFUSSE;
    hoBox2.rgb = {1,1,1};
    box2.hitObj = hoBox2;
    

    objects.push_back(floor);
    objects.push_back(ceiling);
    objects.push_back(back);
    objects.push_back(left);
    objects.push_back(right);
    objects.push_back(light);
    objects.push_back(box1);
    objects.push_back(box2);
    
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
    moved = false;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
        newPos = camera.getPosition() + camera.getDirection()*speed;
        camera.Move_Camera(newPos, camera.getDirection());
        moved = true;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
        newPos = camera.getPosition() - camera.getDirection()*speed;
        camera.Move_Camera(newPos, camera.getDirection());
        moved = true;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
        newPos = camera.getPosition() + camera.getLeft()*speed;
        camera.Move_Camera(newPos, camera.getDirection());
        moved = true;
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
        newPos = camera.getPosition() + camera.getRight()*speed;
        camera.Move_Camera(newPos, camera.getDirection());
        moved = true;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS){ //rotate left
        Vec3 newDir = (camera.getDirection() + camera.getLeft() * sinf(speed/2));
        camera.Move_Camera(camera.getPosition(), newDir);
        moved = true;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS){ //rotate left
        Vec3 newDir = (camera.getDirection() + camera.getRight() * sinf(speed/2));
        camera.Move_Camera(camera.getPosition(), newDir);
        moved = true;
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){ //rotate left
        Vec3 newDir = (camera.getDirection() + camera.getUp() * sinf(speed/2));
        camera.Move_Camera(camera.getPosition(), newDir);
        moved = true;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){ //rotate left
        Vec3 newDir = (camera.getDirection() + camera.getDown() * sinf(speed/2));
        camera.Move_Camera(camera.getPosition(), newDir);
        moved = true;
    }
    //newPos = camera.getPosition() + Vec3(1,0,1);
    

}


void updateCL(uint32_t cpu_seed) {
    //para mi yo del pasado que no se acuerda que es 
    //cl::Memory es cualquier objeto que vive en memoria de opencl tipo buffer, imagen, buffers de opengl, etc
    try{
        std::vector<cl::Memory> objs = { clImage };

        queue.enqueueAcquireGLObjects(&objs);

        cl::Kernel kernel(prog, "main_kernel");

        if (moved){
            std::fill(cumulative_pixels.begin(), cumulative_pixels.end(), cl_float3{0.0f, 0.0f, 0.0f});
            cumulated_time = 1;
        }



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

        cl::Buffer buffer_cumulative_pixels(
            context,
            CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
            sizeof(cl_float3) * WIDTH * HEIGHT,
            cumulative_pixels.data()
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
        kernel.setArg(9, cpu_seed);
        kernel.setArg(10, camera.getU().toCLF4());
        kernel.setArg(11, camera.getV().toCLF4());
        kernel.setArg(12, camera.getDirection().toCLF4());
        kernel.setArg(13, camera.getViewportCenter().toCLF4());
        kernel.setArg(14, camera.getPosition().toCLF4());
        kernel.setArg(15, buffer_cumulative_pixels);
        kernel.setArg(16, cumulated_time);
        kernel.setArg(17, max_samples);

        queue.enqueueNDRangeKernel(
            kernel,
            cl::NullRange,
            cl::NDRange(WIDTH, HEIGHT),
            cl::NullRange
        );
         //reads sample acummulation
        queue.enqueueReadBuffer(buffer_cumulative_pixels,CL_TRUE, 0, sizeof(cl_float3)*WIDTH*HEIGHT, cumulative_pixels.data(), 0, NULL);
        cumulated_time++;
        if (cumulated_time > max_samples){
            cumulated_time = max_samples;
        }

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
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "OpenCL + OpenGL + GLFW", nullptr, nullptr);
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
    std::mt19937 gen(rd()); // Motor Mersenne Twister
    std::uniform_int_distribution<uint32_t> dis(0, 4294967295); 

    // -------------------------
    // Main loop
    // -------------------------
    while (!glfwWindowShouldClose(window)) {
        //std::cout << camera.getPosition();
        //std::cout << "main ptr: " << &camera << std::endl;
        //std::cout << "address: " << &camera << " pos: " << camera.getPosition() << std::endl;

        glClear(GL_COLOR_BUFFER_BIT);
        Movement(window);
        uint32_t random_value = dis(gen);
        //std::cout << camera.getViewportCenter() << std::endl;
        //std::cout << camera.getU() << std::endl;
        //std::cout << camera.getV() << std::endl;
        //std::cout << random_value << std::endl;
        //std::cout << cumulated_time << std::endl;
        
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