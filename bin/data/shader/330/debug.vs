#version 330 core

layout (location = 0) in vec3 position;

void main()
{
	vec3 pos = position;
    gl_Position = vec4(pos, 1.0);
}