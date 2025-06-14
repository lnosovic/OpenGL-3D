#version 330 core

in vec4 chCol;
in vec2 TexCoord;
out vec4 outCol;
uniform vec3 pomCol;
uniform sampler2D uTex;
void main()
{
	vec4 texColor = texture(uTex, TexCoord);
	vec4 Color =vec4(chCol.r*pomCol.r,chCol.g*pomCol.g,chCol.b*pomCol.b,1.0);
	outCol = Color*texColor;
}