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
        Vec3 p1 = Vec3(0, 0, 0);
        points.push_back(p1);
        Vec3 p2 = Vec3(15, 0, 0);
        points.push_back(p2);
        Vec3 p3 = Vec3(0, 0, 15);
        points.push_back(p3);
        //objeto
        Vec3 p4 = Vec3(5, 0, 0);
        points.push_back(p4);
        Vec3 p5 = Vec3(5, 0, 5);
        points.push_back(p5);
        Vec3 p6 = Vec3(5, 5, 0);
        points.push_back(p6);
        triangles.insert(triangles.end(), {0,1,2,3,4,5});
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

    }
};





TEST_F(TestSuit, GetFirstHiT) { //Estos test de constructor tambien testean el operador []
    
    Object returned_object;
    Triangle returned_triangle;
    Vec3 returned_point;

    Ray test_ray1;
    test_ray1.bounces = 0;
    Vec3 test_point1 = Vec3(5,1,0.5);
    test_ray1.o = test_point1;
    test_ray1.d = Vec3(1,0,0);

    bool res = getfirtsHit(&returned_object, &returned_triangle, &returned_point, test_ray1, objects.size(), objects.data(), triangles.data(), points.data());

    ASSERT_EQ(res, false) << "getfirstHit is considering a ray from itself, test 1";

    Ray test_ray2;
    test_ray2.bounces = 0;
    Vec3 test_point2 = Vec3(3,1,0.5);
    test_ray2.o = test_point2;
    test_ray2.d = Vec3(1,0,0);
    Vec3 expected_collision_point = Vec3(5,1,0.5);
    bool res2 = getfirtsHit(&returned_object, &returned_triangle, &returned_point, test_ray2, objects.size(), objects.data(), triangles.data(), points.data());

    ASSERT_EQ(res2, true) << "getfirstHit didnt collided when it should, test 2";
    ASSERT_EQ(returned_point, expected_collision_point) << "getfirsthit is not returning the expected point";
 

}



int main(int argc, char **argv) {
    
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}