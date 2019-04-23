#version 400

layout(location=0) in vec2 position;
layout(location=1) in vec3 color;

out vec4 vertexcolor;

void main()
{
	gl_Position = vec4(position, 0, 1);
	vertexcolor = vec4(color,1);
}