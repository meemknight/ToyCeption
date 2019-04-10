#version 400

in vec4 vertexcolor;
out vec4 outcolor;

void unblend(inout vec4 color, float strenght);

void main()
{
	outcolor = vertexcolor;
	unblend(outcolor, 10);
}

void unblend(inout vec4 color, float strenght)
{
	color.rgb *= strenght;
	color.rgb = floor(color.rgb);
	color.rgb /= strenght;
}
