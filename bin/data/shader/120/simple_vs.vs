#version 120

attribute vec3 position;
attribute vec4 color;
attribute vec3 normal;
attribute vec2 texcoord;

varying vec4 out_color;
varying vec3 out_normal;
varying vec2 out_texcoord;

void main()
{
	vec3 pos = position;
    gl_Position = vec4(pos, 1.0);
    out_color = color;
    out_normal = normal;
    out_texcoord = texcoord;
}
