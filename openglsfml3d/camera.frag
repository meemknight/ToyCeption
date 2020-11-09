#version 400

in vec4 vertexcolor;
out vec4 outcolor;

void unblend(inout vec3 color, float strenght);

subroutine vec3 applyEffect(vec3 color);
subroutine uniform applyEffect u_effect;

subroutine (applyEffect)
vec3 p_unblend(vec3 color)
{
	unblend(color,10);
	return color;
}


subroutine (applyEffect)
vec3 p_black(vec3 color)
{
	float c = (color.r + color.g + color.b)/3.f;
	return vec3(c,c,c);
}

subroutine (applyEffect)
vec3 p_normal(vec3 color)
{
	return color;
}

subroutine (applyEffect)
vec3 p_inverse(vec3 color)
{
	return vec3(1 - color.r, 1-color.g, 1-color.b);
}


void main()
{
	outcolor = vertexcolor;
	outcolor.rgb = u_effect(outcolor.rgb);
}

void unblend(inout vec3 color, float strenght)
{
	color.rgb *= strenght;
	color.rgb = floor(color.rgb);
	color.rgb /= strenght;
}
