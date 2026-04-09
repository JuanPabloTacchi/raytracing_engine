#include "Vec.h"
#include "Camera.h"
#include <iostream>
#include <ostream>


Camera::Camera(int width, int height, Vec3 origin, Vec3 direction, Vec3 up){
   
    this->image_width = width;
    this->image_height = height;
    this->o = origin;
    this->d = direction;
    this->aspect_ratio = float(image_width)/float(image_height);
    this->viewport_height = 1; // por ahora seteo el viewport height como 1
    this->viewport_width = aspect_ratio*viewport_height;
    this->focal_lenght = 1; //por ahora lo mismo lo dejare como 1
    this->up = up;
    this->left = Vec3::cross(up,d);
    this->right = left*-1;

    Vec3 viewport_center = origin + direction*focal_lenght;
    Vec3 v_vp = (up*-1); // u but in space coordinates still (viewport), it should be normalized because up is normalized
    Vec3 u_vp = Vec3::cross(up,direction); //same thing but with v
    Vec3 corner_vec = u_vp*(-(float(viewport_width)/2)) + v_vp*(-float(viewport_height)/2); //vector that takes the vectors from center to the top left corner
    uv_center = corner_vec + viewport_center;
    u = (u_vp*viewport_width/image_width);
    v = (v_vp*viewport_height/image_height);


    //(1, -400, -299.999)
    //(1, -400.001, -300)
}

Camera::Camera() 
    : Camera(600,600,Vec3(0,0,0),Vec3(1,0,0),Vec3(0,1,0))
{}

//ni la uso
Ray Camera::shootRay(int u_x, int v_y){
    Ray ray;
    ray.o = u*u_x + v*v_y;
    ray.d = d;
    ray.bounces = 0;
    return ray;
}

void Camera::Move_Camera(Vec3 new_o, Vec3 new_d){

    this->o = new_o;
    this->d = new_d;
    this->left = Vec3::cross(up,d);
    this->right = left*-1;

    Vec3 viewport_center = o + d*this->focal_lenght;
    Vec3 v_vp = (up*-1); // u but in space coordinates still (viewport), it should be normalized because up is normalized
    Vec3 u_vp = Vec3::cross(up,d); //same thing but with v
    Vec3 corner_vec = u_vp*(-(float(viewport_width)/2)) + v_vp*(-float(viewport_height)/2); //vector that takes the vectors from center to the top left corner
    uv_center = corner_vec + viewport_center;
    u = (u_vp*viewport_width/image_width);
    v = (v_vp*viewport_height/image_height);
    
}


