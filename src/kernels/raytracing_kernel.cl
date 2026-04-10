#include "include/raytracing.clh"
#include "include/random.clh"

Ray getRay(float3 o, float3 d){
    Ray r;
    r.o = o;
    r.d = d;
    r.bounces = 0;
    return r;
}

//el tema de los Vec entre cpu y gpu es algo que tendre k hacer algun refactor siosi
//osea idealmente usar solo float3 pero como no existe en cpu me tiene todo f
__kernel void main_kernel(
    write_only image2d_t img,
    int width,
    int height,
    __global Object* objects,
    int objects_count,
    __global float4* points,
    int points_count,
    __global int* triangle_index,
    int triangles_count,
    float time,
    float4 _u,
    float4 _v,
    float4 _d,
    float4 _viewportCenter,
    float4 _cameraCenter
) {
    //camera info
    float3 u = _u.xyz;
    float3 v = _v.xyz;
    float3 d = _d.xyz;
    float3 viewportCenter = _viewportCenter.xyz;
    float3 cameraCenter = _cameraCenter.xyz;

    //time = 2;
    int x = get_global_id(0);
    int y = get_global_id(1);

    if (x >= width || y >= height)
        return;
    
    float3 o = u*x + v*y + viewportCenter;
    
    //o = (float3)(0,1,1);
    //float3 o = viewportCenter*2;
    //parallel projection
    //Ray r = getRay(o,d);
    //perspective projection
    Ray r = getRay(o,o-cameraCenter);


    int pixel_index = x +  y * height;
    uint seed = (uint)(pixel_index + time * 128);


    volatile float3 rgb = (float3)(0,0,0);
    //rgb = (float3)(0,0,0);

    

    int n_samples = 15;
    for(int i = 0; i < n_samples; i++){
        rgb = rgb + raytrace(r, objects, objects_count, points, points_count, triangle_index, triangles_count, seed);

    }
    rgb = rgb/(float)n_samples;

    
    
    float4 color;
   
    color = (float4)(rgb, 1.0f); // negro

    write_imagef(img, (int2)(x, y), color);
}

//float x_proportion = ((float)x)/get_global_size(0);
    //float y_proportion = ((float)y)/get_global_size(1);
    //rgb = (float3)(0, 0, (v*-y).y);
    //rgb = (float3)(0, (u*-x).z, (v*-y).y);
    //rgb = (float3)(0, 1, 0);



__kernel void kernel_test(
    write_only image2d_t img,
    int width,
    int height,
    __global Object* objects,
    int objects_count,
    __global float4* points,
    int points_count,
    __global int* triangle_index,
    int triangles_count,
    float time,
    float4 _u,
    float4 _v,
    float4 _d
) {
    int x = get_global_id(0);
    int y = get_global_id(1);

    if (x >= width || y >= height)
        return;

    float4 color;

   
    color = (float4)(0.0f, 0.0f, 0.0f, 1.0f); // negro

    write_imagef(img, (int2)(x, y), color);
}



