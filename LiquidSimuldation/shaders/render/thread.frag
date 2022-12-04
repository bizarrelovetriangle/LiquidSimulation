#version 430 core

in vec2 vertex_pos;
in vec4 vertex_color;
out vec4 FragColor;

void main()
{
	FragColor = vertex_color;
}