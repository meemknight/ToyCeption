#version 400

in layout(location=0) vec2 position;
in layout(location=1) vec3 color;

out vec4 vertexcolor;

void main()
{
	gl_Position = vec4(position, 0, 1);
	vertexcolor = vec4(color,1);
}