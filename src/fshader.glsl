#version 460 compatibility

struct mat_t
{
	bool hasTex0;
	bool hasTex1;
	bool hasTex2;
	bool hasFCol;

	int  aFunc;
	float aRef;

	sampler2D tex0;
	sampler2D tex1;
	sampler2D tex2;

	vec4 ambiC;
	vec4 diffC;
	vec4 specC;
	vec4 emisC;

	float depth;
};

struct direcLight_t
{
	vec3 dir;
	vec3 ambi;
	vec3 diff;
	vec3 spec;
};

in vec3 fPos;
in vec3 fNrm;
in vec4 fCol;
in vec2 fTex0;
in vec2 fTex1;
in vec2 fTex2;

out vec4 FragColor;

uniform mat_t mat;
uniform direcLight_t dLight;
uniform vec3 camPos;

vec3 calcDLight(direcLight_t l, vec3 vd)
{
	vec3 ldir, hdir, rv;
	float diff, spec;

	/* Calculate direction vectors */
	ldir = normalize(-l.dir);
	hdir = normalize(ldir + vd);

	/* Calculate diffuse and specular */
	diff = max(dot(fNrm, ldir), 0.0f);
	//spec = pow(max(dot(fNrm, hdir), 0.0f), 32.0f);
	spec = 0.0f;

	/* Calculate light vector */
	rv  = l.ambi * mat.ambiC.rgb;
	rv += l.diff * diff * mat.diffC.rgb;
	rv += l.spec * spec * mat.specC.rgb;
	//rv += mat.emisC.rgb;
	return(rv);
}

void main()
{
	vec3 vdir, light;
	vec4 fragcolor;

	/* If the alpha function is NEVER, discard immediately */
	if(mat.aFunc == 0x0200)
		discard;

	/* Handle the textures poorly for now */
	/* Need to update texture coords */
	fragcolor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	fragcolor = vec4(1.0f, 0.1f, 0.5f, 1.0f);
//	if(mat.hasTex0)
//		fragcolor *= texture(mat.tex0, fTex0);
//	if(mat.hasTex1)
//		fragcolor *= texture(mat.tex1, fTex0);
//	if(mat.hasTex2)
//		fragcolor *= texture(mat.tex2, fTex0);
//	if(mat.hasFCol)
//		fragcolor *= fCol;

	/* Handle the alpha. Reverse what it says */
	switch(mat.aFunc)
	{
		case 0x0201:
			if(fragcolor.a >= mat.aRef)
				discard;
		case 0x0202:
			if(fragcolor.a != mat.aRef)
				discard;
		case 0x0203:
			if(fragcolor.a > mat.aRef)
				discard;
		case 0x0204:
			if(fragcolor.a <= mat.aRef)
				discard;
		case 0x0205:
			if(fragcolor.a == mat.aRef)
				discard;
		case 0x0206:
			if(fragcolor.a < mat.aRef)
				discard;
	}

	/* Calculate lighting */
	vdir = normalize(camPos - fPos);
	light = calcDLight(dLight, vdir);
//	FragColor = fragcolor * vec4(light, 1.0f);

	FragColor = fragcolor;
	gl_FragDepth = gl_FragCoord.z + mat.depth;
};
