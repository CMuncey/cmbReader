#version 460 core

struct texCom
{
	int  comRGB;
	int  comAlp;
	int  sclRGB;
	int  sclAlp;
	int  bufRGB;
	int  bufAlp;
	int src0RGB;
	int src1RGB;
	int src2RGB;
	int  op0RGB;
	int  op1RGB;
	int  op2RGB;
	int src0Alp;
	int src1Alp;
	int src2Alp;
	int  op0Alp;
	int  op1Alp;
	int  op2Alp;
	int   cCInd;
};

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

	vec4     ambiC;
	vec4     diffC;
	vec4     spc0C;
	vec4     spc1C;
	vec4     emisC;
	vec4     buffC;
	vec4 constC[6];

	int      nTCom;
	texCom tCom[6];

	float depth;
};

in vec3 fPos;
in vec3 fNrm;
in vec4 fCol;
in vec2 fTex0;
in vec2 fTex1;
in vec2 fTex2;

out vec4 FragColor;

uniform mat_t mat;
uniform vec3 camPos;

vec4 in0, in1, in2;
vec4 outCol;
vec4 outBuf;
vec4 constCol;

vec4 tComSrc(int src)
{
	switch(src)
	{
		case 0x6210:
		case 0x6211:
		case 0x8577:
			return(fCol);
		case 0x84C0:
			return(texture(mat.tex0, fTex0));
		case 0x84C1:
			return(texture(mat.tex1, fTex1));
		case 0x84C2:
			return(texture(mat.tex2, fTex2));
		case 0x84C3:
			return(vec4(1.0f));
		case 0x8576:
			return(constCol);
		case 0x8578:
			return(outCol);
		case 0x8579:
			return(outBuf);
	}
}

vec4 tComOp(int src, int op)
{
	vec4 v;

	v = tComSrc(src);
	switch(op)
	{
		case 0x0300:
			return(v);
		case 0x0301:
			return(1.0f - v.rgba);
		case 0x0302:
			return(v.aaaa);
		case 0x0303:
			return(1.0f - v.aaaa);
		case 0x8580:
			return(v.rrrr);
		case 0x8581:
			return(v.gggg);
		case 0x8582:
			return(v.bbbb);
		case 0x8583:
			return(1.0f - v.rrrr);
		case 0x8584:
			return(1.0f - v.gggg);
		case 0x8585:
			return(1.0f - v.bbbb);
	}
}

vec4 tComCmb(int cmb)
{
	switch(cmb)
	{
		case 0x0104:
			return(in0 + in1);
		case 0x1E01:
			return(in0);
		case 0x2100:
			return(in0 * in1);
		case 0x6401:
			return((in0 * in1) + in2);
		case 0x6402:
			return((in0 + in1) * in2);
		case 0x84E7:
			return(in0 - in1);
		case 0x8574:
			return((in0 + in1) - 0.5f);
		case 0x8575:
			return(mix(in0, in1, in2));
		case 0x86AE:
			return(vec4(vec3(4.0f * (dot(in0 - 0.5f, in1 - 0.5f))), 1.0f));
		case 0x86AF:
			return(vec4(4.0f * (dot(in0 - 0.5f, in1 - 0.5f))));
	}
}

vec4 tComScl(int cmb, int scl)
{
	vec4 v;

	v = tComCmb(cmb);
	switch(scl)
	{
		case 1:
			return(v);
		case 2:
			return(v * 2.0f);
		case 4:
			return(v * 4.0f);
	}
}

vec4 tComBuf(int buf)
{
	switch(buf)
	{
		case 0x8578:
			return(outCol);
		case 0x8579:
			return(outBuf);
		default:
			return(outBuf);
	}
}

void main()
{
	vec4 fragcolor, v1, v2;
	texCom tc;
	int i;

	// Try to do this stuff in mesh setup if possible
	// This is a lot for every fragment :(
	// Need to work on the entire texture at once though
	// because this uses texture coordinates
	// Otherwise figure out how to pack it for less
	// "send to gpu" calls

	// If the alpha function is NEVER, discard immediately
	if(mat.aFunc == 0x0200)
		discard;

	// Set the initial output buffer to the buffer color
	outBuf = clamp(mat.buffC, vec4(0.0f), vec4(1.0f));

	// For each texture combiner, do the texture combining
	for(i = 0; i < mat.nTCom; i++)
	{
		tc = mat.tCom[i];
		constCol = mat.constC[tc.cCInd];

		v1 = tComOp(tc.src0RGB, tc.op0RGB);
		v2 = tComOp(tc.src0Alp, tc.op0Alp);
		in0 = vec4(v1.rgb, v2.a);

		v1 = tComOp(tc.src1RGB, tc.op1RGB);
		v2 = tComOp(tc.src1Alp, tc.op1Alp);
		in1 = vec4(v1.rgb, v2.a);

		v1 = tComOp(tc.src2RGB, tc.op2RGB);
		v2 = tComOp(tc.src2Alp, tc.op2Alp);
		in2 = vec4(v1.rgb, v2.a);

		v1 = tComScl(tc.comRGB, tc.sclRGB);
		v2 = tComScl(tc.comAlp, tc.sclAlp);
		outCol = vec4(v1.rgb, v2.a);

		v1 = tComBuf(tc.bufRGB);
		v2 = tComBuf(tc.bufRGB);
		outBuf = vec4(v1.rgb, v2.a);
	}

	fragcolor = outCol;

	// Handle the alpha. Reverse what it says
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

	FragColor = fragcolor;
	gl_FragDepth = gl_FragCoord.z + mat.depth;
};
