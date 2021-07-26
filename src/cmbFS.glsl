#version 460 core

/* Improtant stuff from the mats chunk */
layout (std140, binding = 1) uniform matParams
{
	mat4   texMat[3];
	vec4 constCol[6];
	uvec4    texInfo;
	float      depth;
};

in VARS
{
	vec3 pos;
	vec3 norm;
	vec4 color;
	vec2 tex0Coord;
	vec2 tex1Coord;
	vec2 tex2Coord;
} fin;

out vec4 FragColor;

uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2D tex2;

vec4 fsFunc();

/* Temp depth lineariser thing for depth shader */
float near = 0.1f;
float far = 10.0f;
float linDepth(float d)
{
	float z = (d * 2.0f) - 1.0f;
	return((2.0f * near) / (far + near - (z * (far - near))));
}

void main()
{
	/* This is stupid, but it fixes the brightness issue on actors */
	/* It was hard to notice on scenes, but actors were totally washed out */
	FragColor = fsFunc() * 0.7f;
	gl_FragDepth = gl_FragCoord.z + depth;
}
