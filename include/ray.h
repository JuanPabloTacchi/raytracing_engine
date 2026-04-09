#pragma once
#include "Vec.h"

typedef struct 
{
    /* data */
    Vec3 o; //origin
    Vec3 d; // direction
    int bounces;//amount of bounces it had
} Ray;

inline Vec3 ray_col_point(Ray r,float t){
    return r.o + t*r.d; 
}
