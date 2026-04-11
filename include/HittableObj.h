#pragma once
#include <CL/opencl.hpp>
#include <CL/cl_gl.h>


#define EMISSIVE 0
#define DIFUSSE 1

typedef struct 
{
    /* data */
    int Material; //material of the object
    cl_float3 rgb;

} HittableObj;

