#version 430 core

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

struct Particle
{
    vec2 position;
    vec2 velocity;
};

layout(std430, binding = 1) buffer ParticlesInput
{
    Particle particles[];
};

layout(location = 0) uniform float t;

void main() {
    uint id = gl_GlobalInvocationID.x;
    particles[id].position.x = id;
    particles[id].velocity.y = id;

    vec4 value = vec4(0.0, 0.0, 0.0, 1.0);
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
    float speed = 100;
    float width = 1000;

    value.x = mod(float(texelCoord.x) + t * speed, width) / (gl_NumWorkGroups.x);
    value.y = float(texelCoord.y) / (gl_NumWorkGroups.y);
    //value.x = vertex[0];
    //value.y = vertex[0];
    //value.z = vertex[0];
    //vertex[0] = t;
}