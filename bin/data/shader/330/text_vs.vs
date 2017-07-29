#version 330 core
layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texcoord;

out vec2 out_texcoord;

void main()
{
	vec2 pos = position;
    gl_Position = vec4(pos, 1.0,1.0);
    out_texcoord = texcoord;
}