#version 120

attribute vec2 position;
attribute vec2 texcoord;

varying vec2 out_texcoord;

void main()
{
	vec2 pos = position;
    gl_Position = vec4(pos,1.f, 1.f);
    out_texcoord = texcoord;
}
