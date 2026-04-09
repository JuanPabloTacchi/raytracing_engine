#pragma once
#include "Vec.h"
#include "Ray.h"

class Camera {
private:
    // atributos internos
    int image_width;
    int image_height;
    Vec3 o; //origin
    Vec3 d; // direction normalized please
    Vec3 up; //up vector also normalized please
    Vec3 right;
    Vec3 left;
    float focal_lenght; //distance beetween viewport and camera center
    float viewport_width;
    float viewport_height;
    float aspect_ratio;
    Vec3 u; //vector para la derecha
    Vec3 v; //vector para abajo
    Vec3 uv_center;


public:
    // constructor
    Camera(int width, int height, Vec3 origin, Vec3 direction, Vec3 up);

    Camera();

    // métodos
    
    Ray shootRay(int u, int v);

    Vec3 getU(){
        return u;
    }
    Vec3 getV(){
        return v;
    }
    Vec3 getViewportCenter(){
        return uv_center;
    }
    Vec3 getDirection(){
        return d;
    }
    Vec3 getRight(){
        return right;
    }
    Vec3 getLeft(){
        return left;
    }
    Vec3 getUp(){
        return up;
    }
    Vec3 getDown(){
        return up*-1;
    }
    Vec3 getPosition(){
        return o;
    }
    void Move_Camera(Vec3 new_o, Vec3 new_d);
    // atributos públicos
    


};