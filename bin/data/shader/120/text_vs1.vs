#version 120
attribute vec2 position;
attribute vec2 texcoord;
attribute float channel;

varying vec2 out_texcoord;
flat varying float out_channel;

void main()
{
	vec2 pos = position;
    gl_Position = vec4(pos, 1.0,1.0);
    out_texcoord = texcoord;
    out_channel = channel;
}