#pragma once
#include "HittableObj.h"

typedef struct 
{
    /* data */
    int i_0; //initial triangle index
    int i_f; //last triangle index
    int n_triangles;
    HittableObj hitObj; //material info
    
} Object;

