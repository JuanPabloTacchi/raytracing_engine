#pragma once
#include <iostream>
#include <vector>
#include <string>
#include "gtest/gtest.h"
#include "Vec.h"
#include "object.h"
#include "ray.h"
#include "triangles.h"
#include <CL/opencl.hpp>
#include <CL/cl_gl.h>

using namespace std;

//Not exactly the same, but very close, only changes the fact that is not float3 but Vec3 (for th operations) and also points is vec3 and not vec4
bool getfirtsHit(Object* o, Triangle* triangle, Vec3* point, Ray r, int object_count, Object* objects, int* triangles, Vec3* points){
    float infinity_value = 1e30f;
    float t = infinity_value;
    //recursion for checking which object is first hitted
    for(int i = 0; i < object_count ; i++ ){
        Object o_i = objects[i];
        int n_triangles = o_i.n_triangles;
        for(int j = 0;  j < n_triangles; j++){
            Triangle new_triangle;
            int triangles_base_index = o_i.i_0 + j*3;

            int i0 = triangles[triangles_base_index];
            int i1 = triangles[triangles_base_index + 1];
            int i2 = triangles[triangles_base_index + 2];
            new_triangle.A = points[i0];
            new_triangle.B = points[i1];
            new_triangle.C = points[i2];
            std::cout << new_triangle.A << std::endl;
            Vec3 normal = triangle_get_normal(new_triangle);
            float t_i = plane_triangle_intersect(r,new_triangle,normal);
            
            if (t_i > 0 && t_i < t){
                //check if point is inside triangle if not its discarded
                Vec3 point_in_triangle_plane = ray_col_point(r,t_i);
                bool is_inside_variable = is_inside(point_in_triangle_plane,new_triangle,normal);
                if(is_inside_variable){
                    *o = objects[i];
                    o->i_0 = objects[i].i_0;
                    o->i_f = objects[i].i_f;
                    o->n_triangles = objects[i].n_triangles;
                    o->hitObj.Material = objects[i].hitObj.Material;
                    o->hitObj.rgb = objects[i].hitObj.rgb;
                    t = t_i;
                    *triangle = new_triangle;
                    *point = point_in_triangle_plane; 
                }

            }
        }
    }
    if (t == infinity_value){
        return false;
    }
    return true;

}