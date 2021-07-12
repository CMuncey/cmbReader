#version 460 core

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNrm;
layout (location = 2) in vec4 vCol;
layout (location = 3) in vec2 vTex0;
layout (location = 4) in vec2 vTex1;
layout (location = 5) in vec2 vTex2;
layout (location = 6) in vec4 vBoneWeights;
layout (location = 7) in vec4 vBoneInds;

out vec3 fPos;
out vec3 fNrm;
out vec4 fCol;
out vec2 fTex0;
out vec2 fTex1;
out vec2 fTex2;

/* Transform matrices */
uniform mat3 norm;
uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

/* Scaling factors */
uniform float colScl;
uniform float tex0Scl;
uniform float tex1Scl;
uniform float tex2Scl;
uniform bool hasFCol;

void main()
{
	if(hasFCol)
		fCol = vCol * colScl;
	else
		fCol = vec4(1.0f) * colScl;
	fTex0 = vTex0 * tex0Scl;
	fTex1 = vTex1 * tex1Scl;
	fTex2 = vTex2 * tex2Scl;
	fNrm = normalize(norm * vNrm);
	fPos = vec3(model * vec4(vPos, 1.0f));
	gl_Position = proj * view * vec4(fPos, 1.0f);
}
