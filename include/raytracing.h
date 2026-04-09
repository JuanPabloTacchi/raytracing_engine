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
    int infinity_value = 1000000;
    float t = infinity_value;
    //recursion for checking which object is first hitted
    for(int i = 0; i < object_count ; i++ ){
        Object o_i = objects[i];
        int n_triangles = o_i.n_triangles;
        for(int j = 0;  j < n_triangles; j++){
            Triangle new_triangle;
            int new_triangle_index =  triangles[o_i.i_0+j*3];
            new_triangle.A = points[new_triangle_index+0];
            new_triangle.B = points[new_triangle_index+1];
            new_triangle.C = points[new_triangle_index+2];
            std::cout << new_triangle.A << std::endl;
            Vec3 normal = triangle_get_normal(new_triangle);
            float t_i = plane_triangle_intersect(r,new_triangle,normal);
            
            if (t_i > 0 && t_i < t){
                //check if point is inside triangle if not its discarded
                Vec3 point_in_triangle_plane = ray_col_point(r,t_i);
                bool is_inside_variable = is_inside(point_in_triangle_plane,new_triangle,normal);
                if(is_inside_variable){
                    *o = objects[i];
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