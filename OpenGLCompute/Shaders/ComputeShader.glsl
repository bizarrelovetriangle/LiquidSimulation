#version 430 core

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(std430, binding = 10) buffer Vbuffer
{
    float vertex[];
};

// images 
layout(rgba32f, binding = 0) uniform image2D imgOutput;

// variables
layout(location = 0) uniform float t;                 /** Time */

void main() {
    vec4 value = vec4(0.0, 0.0, 0.0, 1.0);
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
    float speed = 100;
    // the width of the texture
    float width = 1000;

    value.x = mod(float(texelCoord.x) + t * speed, width) / (gl_NumWorkGroups.x);
    value.y = float(texelCoord.y) / (gl_NumWorkGroups.y);
    value.x = vertex[0];
    value.y = vertex[0];
    value.z = vertex[0];
    vertex[0] = 2;
    imageStore(imgOutput, texelCoord, value);
}