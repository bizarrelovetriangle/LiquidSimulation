#version 430 core

in float radius;
in vec2 vertex_pos;
in vec4 vertex_color;
out vec4 FragColor;

void main()
{
	if (length(vertex_pos) < radius) {
		FragColor = vertex_color;
	}
}