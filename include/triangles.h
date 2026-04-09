#pragma once
#include "ray.h"

typedef struct 
{
    /* data */
    Vec3 A; // A point
    Vec3 B; // B point
    Vec3 C; // C point 
    //all in ccw
} Triangle;

Vec3 triangle_get_normal(Triangle t){
    Vec3 n = Vec3::cross((t.B-t.A),(t.C-t.A));
    n = Vec3::normalize(n);
    return n;    
}

//calculates intersection between plane and a ray
// n normalized normal of the plame created by triangle t
float plane_triangle_intersect(Ray r, Triangle T, Vec3 n){
    //calculates equations of plane Ax+By+Cz=D because  n dot (x,y,z) = D
    //so n dot (o+td) = D
    // -> t = (D-n dot o)/(n dot d)
    
    if( fabs(Vec3::dot(n, r.d)) < 1e-6f){
        return -1;
    }

    //calculate D for equation 
    float D = Vec3::dot(n,T.A);

    float t = (D - Vec3::dot(n,r.o))/Vec3::dot(n,r.d);

    return t;
}

//check if the intersection of a ray is inside
// P intersection
// T triangle
// n normal or triangle
// u,v vectors from uv mapping of triangle
//TODO creo que esto asume que el triangulo sea rectangulo
bool is_inside(Vec3 P, Triangle T, Vec3 n){
    Vec3 u = T.B-T.A;
    Vec3 v = T.C-T.A; 
    n = Vec3::cross(u,v);
    Vec3 p = P - T.A;
    Vec3 w = n/Vec3::dot(n,n);
    float alpha = Vec3::dot(w,Vec3::cross(p,v));
    float beta = Vec3::dot(w,Vec3::cross(u,p));
    
    if(alpha > 0 && beta > 0 && alpha + beta < 1){
        return true;
    }
    return false;
}