#pragma once
#include <CL/opencl.hpp>

class Vec3 {
public:
    float x, y, z;

    Vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

    Vec3() : x(0), y(0), z(0) {}

    // Sobrecarga del operador *
    Vec3 operator*(float scalar) const {
        return Vec3(x * scalar, y * scalar, z * scalar);
    }

    // Sobrecarga del operador *
    Vec3 operator*(int scalar) const {
        return Vec3(x * scalar, y * scalar, z * scalar);
    }

    // Sobrecarga del operador +
    Vec3 operator+(Vec3 vec) const {
        return Vec3(x + vec.x, y + vec.y, z + vec.z);
    }
     // Sobrecarga del operador -
    Vec3 operator-(Vec3 vec) const {
        return Vec3(x - vec.x, y - vec.y, z - vec.z);
    }
    // Sobrecarga del operador /
    Vec3 operator/(float scalar) const {
        return Vec3(x / scalar, y / scalar, z / scalar);
    }

    static Vec3 cross(const Vec3& a, const Vec3& b) {
        return Vec3(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        );
    }

    cl_float4 toCLF4(){
        cl_float4 f4 = {x, y, z, 0};
        return f4;
    }

};

// 👇 inline porque está en header
inline std::ostream& operator<<(std::ostream& os, const Vec3& v) {
    os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    return os;
}