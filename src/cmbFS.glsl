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

void main()
{
	//FragColor = fsFunc() * vec4(fin.norm, 1.0f);
	FragColor = fsFunc();
	gl_FragDepth = gl_FragCoord.z + depth;
}

