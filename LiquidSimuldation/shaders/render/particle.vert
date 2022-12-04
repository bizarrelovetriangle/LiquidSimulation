#version 430 core
#include "../common.glsl"

layout(location = 0) in vec2 pos;

layout(std430, binding = 0) buffer ParticlesInput
{
	Particle particles[];
};

layout(location = 0) uniform mat3 view_matrix;
layout(location = 2) uniform vec4 color;

out float radius;
out vec2 vertex_pos;
out vec4 vertex_color;

void main()
{
	radius = particles[gl_InstanceID].radius;
	vertex_pos = pos * particles[gl_InstanceID].radius;
	vec2 position = pos * particles[gl_InstanceID].radius + particles[gl_InstanceID].position;
	gl_Position = vec4(view_matrix * vec3(position, 1.0), 1.0);
	vertex_color = vec4(color.xyz, 1.0);
}