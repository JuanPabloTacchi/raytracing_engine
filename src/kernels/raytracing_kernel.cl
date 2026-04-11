#include "include/raytracing.clh"
#include "include/random.clh"

Ray getRay(float3 o, float3 d){
    Ray r;
    r.o = o;
    r.d = d;
    r.bounces = 0;
    return r;
}

uint hash(uint x) {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}

uint tea_hash(uint val0, uint val1) {
    uint v0 = val0;
    uint v1 = val1;
    uint s0 = 0;
    for (uint n = 0; n < 16; n++) {
        s0 += 0x9e3779b9;
        v0 += ((v1 << 4) + 0xa341316c) ^ (v1 + s0) ^ ((v1 >> 5) + 0xc8013ea4);
        v1 += ((v0 << 4) + 0xad90777d) ^ (v0 + s0) ^ ((v0 >> 5) + 0x7e95761e);
    }
    return v0;
}

uint pcg_hash(uint input) {
    uint state = input * 747796405u + 2891336453u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
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
    uint cpu_seed,
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
    
    
    Ray r = getRay(o,o-cameraCenter);


    int pixel_index = x +  y * height;
    //uint seed = (uint)(pixel_index + time * 128);
    

    //uint3 seed = (uint3)(cpu_seed, get_global_id(0), get_global_id(0) + cpu_seed);
    //seed.x = hash(cpu_seed + get_global_id(0));
    //seed.y = hash(seed.x);
    //seed.z = hash(seed.y);
    // Dentro del kernel:
    uint2 state;
    uint gid = get_global_id(0);
    state.x = pixel_index ^ cpu_seed; 
    state.x = (state.x ^ 61) ^ (state.x >> 16); // Mezcla rápida
    state.x *= 9;
    state.x = state.x ^ (state.x >> 4);
    state.x *= 0x27d4eb2d;
    volatile float3 rgb = (float3)(0,0,0);

    

    int n_samples = 10;
    for(int i = 0; i < n_samples; i++){
        rgb = rgb + raytrace(r, objects, objects_count, points, points_count, triangle_index, triangles_count, state);
    }
    rgb = rgb/(float)n_samples;

    
    
    float4 color;
   
    color = (float4)(rgb, 1.0f); // negro

    write_imagef(img, (int2)(x, y), color);
}

