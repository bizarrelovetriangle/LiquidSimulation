#version 430 core
#include "common.glsl"

layout(location = 0) in vec2 pos;
out vec4 vertexColor;

layout(std430, binding = 0) buffer ParticlesInput
{
    Particle particles[];
};

layout(location = 0) uniform mat3 view_matrix;
layout(location = 2) uniform vec4 color;

void main()
{
    gl_Position = vec4(view_matrix * vec3(pos, 1.0), 1.0);
    vertexColor = vec4(color.xyz, 1.0);
}