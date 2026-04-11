#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_TARGET_OPENCL_VERSION 200
#define CL_HPP_MINIMUM_OPENCL_VERSION 120

#include <iostream>
#include <vector>
#include <string>
#include "gtest/gtest.h"
#include "Vec.h"
#include "Vec_gpu.h"
#include "object.h"
#include "ray.h"
#include "raytracing.h"
#include "triangles.h"
#include <CL/opencl.hpp>
#include <CL/cl_gl.h>

using namespace std;



class TestSuit : public ::testing::Test {
protected:
    std::vector<Object> objects;
    std::vector<Vec3> points;
    std::vector<int> triangles;

    void SetUp() override {
    
    // floor
    Vec3 p0 = Vec3(0, 0, 0);
    Vec3 p1 = Vec3(10, 0, 0);
    Vec3 p2 = Vec3(10, 0, 10);
    Vec3 p3 = Vec3(0, 0, 10);

    // ceiling
    Vec3 p4 = Vec3(0, 10, 0);
    Vec3 p5 = Vec3(10, 10, 0);
    Vec3 p6 = Vec3(10, 10, 10);
    Vec3 p7 = Vec3(0, 10, 10);

    // light
    Vec3 l0 = Vec3(3, 9.9, 3);
    Vec3 l1 = Vec3(7, 9.9, 3);
    Vec3 l2 = Vec3(7, 9.9, 7);
    Vec3 l3 = Vec3(3, 9.9, 7);

   
    points.insert(points.end(), {
        p0,p1,p2,p3,
        p4,p5,p6,p7,
        l0,l1,l2,l3
    });

    // ===== small box =====
    Vec3 b0 = Vec3(2,0,6);
    Vec3 b1 = Vec3(4,0,6);
    Vec3 b2 = Vec3(4,0,8);
    Vec3 b3 = Vec3(2,0,8);

    Vec3 b4 = Vec3(2,3,6);
    Vec3 b5 = Vec3(4,3,6);
    Vec3 b6 = Vec3(4,3,8);
    Vec3 b7 = Vec3(2,3,8);

    // ===== big box =====
    Vec3 t0 = Vec3(6,0,2);
    Vec3 t1 = Vec3(8,0,2);
    Vec3 t2 = Vec3(8,0,5);
    Vec3 t3 = Vec3(6,0,5);

    Vec3 t4 = Vec3(6,7,2);
    Vec3 t5 = Vec3(8,7,2);
    Vec3 t6 = Vec3(8,7,5);
    Vec3 t7 = Vec3(6,7,5);

    // push
    points.insert(points.end(), {
        b0,b1,b2,b3,b4,b5,b6,b7,
        t0,t1,t2,t3,t4,t5,t6,t7
    });

    // ======== triangles ========

    // floor
    triangles.insert(triangles.end(), {0,2,1, 0,3,2});

    // ceiling
    triangles.insert(triangles.end(), {4,5,6});
    triangles.insert(triangles.end(), {4,6,7});

    // back wall
    triangles.insert(triangles.end(), {3,6,2});
    triangles.insert(triangles.end(), {3,7,6});

    // left wall (red)
    triangles.insert(triangles.end(), {0,7,3});
    triangles.insert(triangles.end(), {0,4,7});

    // right wall (green)
    triangles.insert(triangles.end(), {1,2,6});
    triangles.insert(triangles.end(), {1,6,5});

    // light 
    triangles.insert(triangles.end(), {8,10,9});
    triangles.insert(triangles.end(), {8,11,10});

    triangles.insert(triangles.end(), {
    // small box
    12,13,14,  12,14,15,

    // (normal +Y)
    16,18,17,  16,19,18,

    //  -Z
    12,17,13,  12,16,17,

    // +Z
    15,14,18,  15,18,19,

    // -X
    12,15,19,  12,19,16,

    // +X
    13,17,18,  13,18,14,

    // big box
    // (normal -Y)
    20,21,22,  20,22,23,

    //  (normal +Y)
    24,26,25,  24,27,26,

    // -Z
    20,25,21,  20,24,25,

    // +Z
    23,22,26,  23,26,27,

    // -X
    20,23,27,  20,27,24,

    // +X
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
    light.hitObj = hoLight;
    hoLight.Material = EMISSIVE;
    hoLight.rgb = {1,1,1};
    light.hitObj = hoLight;

    //small wall
    Object box1;
    HittableObj hoBox1;
    box1.n_triangles = 12;
    box1.i_0 = 36;
    box1.i_f = 71;
    hoBox1.Material = DIFUSSE;
    hoBox1.rgb = {1,1,1};
    box1.hitObj = hoBox1;

    //big wall
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
    }
};





TEST_F(TestSuit, GetFirstHiT) { //Estos test de constructor tambien testean el operador []
    
    Object returned_object;
    Triangle returned_triangle;
    Vec3 returned_point;

    Ray test_ray1;
    test_ray1.bounces = 0;
    Vec3 test_point1 = Vec3(-11.6, 2.86401, 4.375);
    test_ray1.o = test_point1;
    test_ray1.d = Vec3(1,0.0001,0);

    bool res = getfirtsHit(&returned_object, &returned_triangle, &returned_point, test_ray1, objects.size(), objects.data(), triangles.data(), points.data());
    Vec3 expected_p0 = Vec3(10, 0, 0);
    Vec3 expected_p1 = Vec3(10, 0, 10);
    Vec3 expected_p2 = Vec3(10, 10, 10);
    std::cout << returned_triangle.A << std::endl;
    ASSERT_EQ(res, true) << "getfirstHit is considering a ray from itself, test 1";
    //ASSERT_EQ(expected_p0, returned_triangle.A) << "getfirstHit is considering a ray from itself, test 1";
    //ASSERT_EQ(expected_p1, returned_triangle.B) << "getfirstHit is considering a ray from itself, test 1";
    //ASSERT_EQ(expected_p2, returned_triangle.C) << "getfirstHit is considering a ray from itself, test 1";

    
 

}

TEST_F(TestSuit, GetFirstHiT2) { 
    
    Object returned_object;
    Triangle returned_triangle;
    Vec3 returned_point;

    //test if it collides with the back wall
    Ray test_ray1;
    test_ray1.bounces = 0;
    
    Vec3 test_point1 = Vec3(8.22762, 1.98438, 3.08194);
    test_ray1.o = test_point1;
    test_ray1.d = Vec3(0,0,1);

    bool res = getfirtsHit(&returned_object, &returned_triangle, &returned_point, test_ray1, objects.size(), objects.data(), triangles.data(), points.data());
    Vec3 expected_p0 = Vec3(10, 0, 0);
    Vec3 expected_p1 = Vec3(10, 0, 10);
    Vec3 expected_p2 = Vec3(10, 10, 10);
    std::cout << returned_triangle.A << std::endl;
    Object expected_obj;
    HittableObj expected_hoObj;
    expected_obj.n_triangles = 2;
    expected_obj.i_0 = 12;
    expected_obj.i_f = 17;
    expected_hoObj.Material = DIFUSSE;
    cl_float3 rgb_expected = {0.8,0.8,0.8};
    expected_hoObj.rgb = rgb_expected;
    expected_obj.hitObj = expected_hoObj;
    ASSERT_EQ(res, true) << "getfirstHit2 error";
    ASSERT_EQ(returned_object.hitObj.Material, DIFUSSE) << "getfirstHit2 error";
    ASSERT_EQ(returned_object.hitObj.rgb.x, rgb_expected.x) << "getfirstHit2 error";
    ASSERT_EQ(returned_object.hitObj.rgb.y, rgb_expected.y) << "getfirstHit2 error";
    ASSERT_EQ(returned_object.hitObj.rgb.z, rgb_expected.z) << "getfirstHit2 error";
    ASSERT_EQ(returned_object.i_0, 12) << "getfirstHit2 error";
    ASSERT_EQ(returned_object.i_f, 17) << "getfirstHit2 error";
    ASSERT_EQ(returned_object.n_triangles, 2) << "getfirstHit2 error";
   

    
 

}


int main(int argc, char **argv) {
    
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}