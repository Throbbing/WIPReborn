#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec4 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 texcoord;

out vec4 out_color;
out vec3 out_normal;
out vec2 out_texcoord;

void main()
{
	vec3 pos = position;
    gl_Position = vec4(pos, 1.0);
    out_color = color; 
    out_normal = normal;
    out_texcoord = texcoord;
}