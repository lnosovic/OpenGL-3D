#version 330 core

in vec4 chCol;
in vec2 TexCoord;
out vec4 outCol;

uniform sampler2D uTex;
void main()
{
	vec4 texColor = texture(uTex, TexCoord);
	outCol = chCol*texColor;
}