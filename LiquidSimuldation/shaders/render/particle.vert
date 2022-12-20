#version 430 core
#include "../common.glsl"

layout(location = 0) in vec2 pos;

layout(std430, binding = 0) buffer ParticlesInput
{
	Particle particles[];
};
layout(std430, binding = 4) buffer ParticleIndexesInput
{
	int particle_indexes[];
};

layout(std140, binding = 1) uniform ConfigInput { Config config; };
layout(location = 0) uniform mat3 view_matrix;
layout(location = 2) uniform vec4 color;

out float radius;
out vec2 vertex_pos;
out vec4 vertex_color;

void main()
{
	int id = particle_indexes[gl_InstanceID];
	radius = particles[id].radius;
	vertex_pos = pos * particles[id].radius;
	vec2 position = pos * particles[id].radius + particles[id].position;
	gl_Position = vec4(view_matrix * vec3(position, 1.0), 1.0);
	vertex_color = vec4(color.xyz, 1.0);
}