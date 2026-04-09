#include <iostream>
#include <vector>
#include <string>
#include "gtest/gtest.h"
#include "Vec.h"
#include "Vec_gpu.h"
#include "object.h"
#include <CL/opencl.hpp>
#include <CL/cl_gl.h>

using namespace std;

bool getfirtsHit(Object* o, Triangle* triangle, float3* point, Ray r, int object_count, __global Object* objects, __global int* triangles, __global float4* points){
    int infinity_value = 1000000;
    float t = infinity_value;
    //recursion for checking which object is first hitted
    for(int i = 0; i < object_count ; i++ ){
        //Object o = objects[i];
        //*o = objects[i];
        Object o_i = objects[i];
        int n_triangles = o_i.n_triangles;
        for(int j = 0;  j < n_triangles; j++){
            Triangle new_triangle;
            int new_triangle_index =  triangles[o_i.i_0+j*3];
            new_triangle.A = points[new_triangle_index+0].xyz;
            new_triangle.B = points[new_triangle_index+1].xyz;
            new_triangle.C = points[new_triangle_index+2].xyz;
            float3 normal = triangle_get_normal(new_triangle);
            float t_i = plane_triangle_intersect(r,new_triangle,normal);
            
            if (t_i > 0 && t_i < t){
                //check if point is inside triangle if not its discarded
                float3 point_in_triangle_plane = ray_col_point(r,t_i);
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


TEST(getFirstHit, TestCorrectness) { //Estos test de constructor tambien testean el operador []
    Matrix m1 = Matrix(2);
    m1[0,1] = 1; //matriz 0 1

    ASSERT_EQ((m1[0,0]), 0) << "Matriz constructor1 no se inicializo bien";
    ASSERT_EQ((m1[0,1]), 1) << "Matriz constructor1 no se inicializo bien";

    Matrix m3 = Matrix(3); //Matriz  0 1 0
    m3[0,1] = 1;

    ASSERT_EQ((m3[0,0]), 0) << "Matriz constructor1 no se inicializo bien";
    ASSERT_EQ((m3[0,1]), 1) << "Matriz constructor1 no se inicializo bien";
    ASSERT_EQ((m3[0,2]), 0) << "Matriz constructor1 no se inicializo bien";

}



int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}