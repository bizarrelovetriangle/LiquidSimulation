#version 430 core

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

struct Particle
{
    vec2 position;
    vec2 velocity;
};

struct Particle2
{
    ivec2 gridPosition;

    vec2 position;
    vec2 velosity;
    vec2 acceleration;
    float radius;

    float density;
    float density_near;

    int index;

    vec4 _shared_data;
};

layout(std430, binding = 1) buffer ParticlesInput
{
    Particle2 particles[];
};

layout(location = 0) uniform float t;

void main() {
    uint id = gl_GlobalInvocationID.x;
    particles[id].velosity += particles[id].acceleration * t;
    particles[id].position += particles[id].velosity * t;
}