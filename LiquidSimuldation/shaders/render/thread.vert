#version 430 core
#include "../common.glsl"

layout(std430, binding = 0) buffer ParticlesInput
{
	Particle particles[];
};
layout(std430, binding = 1) buffer PairsInput
{
	PairData pairs[];
};

layout(std140, binding = 0) uniform ConfigInput { Config config; };
layout(location = 1) uniform mat3 view_matrix;
layout(location = 2) uniform vec4 color;

out vec4 vertex_color;

void main()
{
	int particle_id = gl_VertexID == 0 ? pairs[gl_InstanceID].first : pairs[gl_InstanceID].second;
	vec2 pos = particles[particle_id].position;
	gl_Position = vec4(view_matrix * vec3(pos, 1), 1);
	float factor = pairs[gl_InstanceID].proximity_coefficient;
	vertex_color = mix(color / 2, color, factor);
}