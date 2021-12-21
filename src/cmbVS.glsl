#version 460 core

/* Scaling factors from the sepd chunk - 32 bytes */
layout (std140, binding = 0) uniform sepdParams
{
	float   posScale;
	float colorScale;
	float  tex0Scale;
	float  tex1Scale;
	float  tex2Scale;
	float boneWScale;
	int      boneDim;
	bool   hasVColor;
};

/* Improtant stuff from the mats chunk - 308 bytes */
layout (std140, binding = 1) uniform matParams
{
	mat4   texMat[3];
	vec4 constCol[6];
	uvec4    texInfo;
	float      depth;
};

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNorm;
layout (location = 2) in vec4 vColor;
layout (location = 3) in vec2 vTex0Coord;
layout (location = 4) in vec2 vTex1Coord;
layout (location = 5) in vec2 vTex2Coord;
layout (location = 6) in vec4 vBoneWeights;
layout (location = 7) in vec4 vBoneInds;

out VARS
{
	vec3 pos;
	vec3 norm;
	vec4 color;
	vec2 tex0Coord;
	vec2 tex1Coord;
	vec2 tex2Coord;
} vout;

/* Transform matrices */
uniform mat4 proj;
uniform mat3 norm;
uniform mat4 model;
uniform mat4 view;

vec2 getTexSrc(uint texNum)
{
	switch(texNum)
	{
		case 0:
			return(vTex0Coord * tex0Scale);
		case 1:
			return(vTex1Coord * tex1Scale);
		case 2:
			return(vTex2Coord * tex2Scale);
		default:
			return(vec2(0.0f, 0.0f));
	}
}

vec2 getTexCoords(int tInd)
{
	uint mapMode, texNum;
	vec2 texSrc;

	mapMode = texInfo.x >> 16;
	texNum = texInfo.x & 0xffff;

	switch(mapMode)
	{
		case 0:
			/* No texture mapping */
			return(vec2(0.0f, 0.0f));
		case 1:
			/* Regular mapping */
			texSrc = getTexSrc(texNum);
			return((texMat[tInd] * vec4(texSrc, 0.0f, 1.0f)).xy);
		case 2:
			/* TODO Cube mapping */
			return(vec2(0.0f, 0.0f));
		case 3:
			/* Sphere mapping */
			texSrc = (vNorm.xy * 0.5) + 0.5;
			return((texMat[tInd] * vec4(texSrc, 0.0f, 1.0f)).xy);
		case 4:
			/* TODO Projection mapping */
			return(vec2(0.0f, 0.0f));
		default:
			return(vec2(0.0f, 0.0f));
	}
}

void main()
{
	/* TODO the bone matrix stuff */

	/* Fragment color */
	if(hasVColor)
		vout.color = vColor * colorScale;
	else
		vout.color = vec4(1.0f) * colorScale;

	/* Calculate the texture coordinates */
	vout.tex0Coord = vTex0Coord * tex0Scale;
	vout.tex1Coord = vTex1Coord * tex1Scale;
	vout.tex2Coord = vTex2Coord * tex2Scale;

	/* This one is more accurate maybe? but idk what all it needs */
	//vout.tex0Coord = getTexCoords(0);
	//vout.tex1Coord = getTexCoords(1);
	//vout.tex2Coord = getTexCoords(2);

	/* Calculate vertex position */
	vout.norm = normalize(norm * vNorm);
	vout.pos = vec3(model * vec4(vPos, 1.0f));
	gl_Position = proj * view * vec4(vout.pos, 1.0f);
}
