#version 430 core
#include "../common.glsl"

layout(location = 0) uniform mat3 view_matrix;
layout(location = 2) uniform vec4 color;

out vec4 vertex_color;

void main()
{
	PairData thread = pairs[gl_InstanceID];
	int particle_id = gl_VertexID == 0 ? thread.first : thread.second;
	vec2 pos = particles[particle_id].position;
	gl_Position = vec4(view_matrix * vec3(pos, 1), 1);

	float dist = distance(particles[thread.first].position, particles[thread.second].position);

	float factor = (dist - thread.rest_length) / thread.rest_length * 1.5;
	vertex_color = mix(color / 10, color, abs(factor));
}