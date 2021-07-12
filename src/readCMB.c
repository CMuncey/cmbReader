#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "cmb.h"
#include "sklmChunk.h"
#include "cmbConstants.h"
#include "cmbTextures.h"

/* https://wiki.cloudmodding.com/oot/3D:CMB_format */

void dumpTextures(const cmb_t*);
void getTexEnvInfo(const cmb_t*);
const char* tcLookUp1(uint16_t);
const char* tcLookUp2(uint16_t);

int main(int argc, char** argv)
{
	cmb_t cmb;
	int i, j, k;

	if(argc < 2)
	{
		fprintf(stderr, "Usage: %s filename.cmb\n", argv[0]);
		exit(1);
	}

	if((i = readCmb(&cmb, argv[1])) != 0)
		fprintf(stderr, "Error reading cmb file: %d\n", i);

	printCmb(cmb);
	printf("\n");
	getTexEnvInfo(&cmb);
	printf("\n");
	dumpTextures(&cmb);

	delCmb(cmb);

	return(0);
}

void dumpTextures(const cmb_t* cmb)
{
	for(int i = 0; i < cmb->texC->nTex; ++i)
	{
		texture_t t = cmb->texC->tex[i];
		printf("texture %d (%s) type: ", i, t.name);
		switch(((t.datType << 16) | t.colFmt))
		{
			case RGBA8:
				printf("RGBA8\n");
				break;
			case RGB8:
				printf("RGB8\n");
				break;
			case RGBA4:
				printf("RGBA4\n");
				break;
			case RGBA5551:
				printf("RGBA5551\n");
				break;
			case RGB565:
				printf("RGB565\n");
				break;
			case ETC1:
				printf("ETC1\n");
				break;
			case ETC1A4:
				printf("ETC1A4\n");
				break;
			case LA4:
				printf("LA4\n");
				break;
			case LA8:
				printf("LA8\n");
				break;
			case A8:
				printf("A8\n");
				break;
			case L8:
				printf("L8\n");
				break;
			case L4:
				printf("L4\n");
				break;
			default:
				printf("Unknown\n");
				break;
		}
		dumpBMP(cmb->texDat, &t);
	}
}

//Material X:
//	Tex Env Y:
//		Constant Color:
//		Buffer RGB:
//		Buffer Alpha:
//		RGB:
//			source0.????
//			source1.????
//			source2.????
//			RGB combine OP
//		Alpha:
//			source0.????
//			source1.????
//			source2.????
//			RGB combine OP
void getTexEnvInfo(const cmb_t* c)
{
	int i, j;
	material_t* m;
	texCombiner_t* tc;
	rgba_t col;

	for(i = 0; i < c->matsC->nMats; ++i)
	{
		m = &(c->matsC->mats[i]);
		printf("Material %d:\n", i);

		for(j = 0; j < m->nTexCombiners; ++j)
		{
			tc = &(c->matsC->tCom[m->texCInd[j]]);
			col = m->constCol[tc->constInd];

			printf("\tTexture Environment %d (%d):\n", j, m->texCInd[j]);
			printf("\t\tConstant Color: %u, %u, %u, %u\n", col.R, col.G, col.B, col.A);
			printf("\t\tBuffer RGB:   %s\n", tcLookUp2(tc->bufInpRGB));
			printf("\t\tBuffer Alpha: %s\n", tcLookUp2(tc->bufInpAlpha));
			printf("\t\tRGB:\n");
			printf("\t\t\tt_CmbIn0.rgb = %s%s\n", tcLookUp2(tc->srcRGB[0]), tcLookUp2(tc->opRGB[0]));
			printf("\t\t\tt_CmbIn1.rgb = %s%s\n", tcLookUp2(tc->srcRGB[1]), tcLookUp2(tc->opRGB[1]));
			printf("\t\t\tt_CmbIn2.rgb = %s%s\n", tcLookUp2(tc->srcRGB[2]), tcLookUp2(tc->opRGB[2]));
			printf("\t\tAlpha:\n");
			printf("\t\t\tt_CmbIn0.a = %s%s\n", tcLookUp2(tc->srcAlpha[0]), tcLookUp2(tc->opAlpha[0]));
			printf("\t\t\tt_CmbIn1.a = %s%s\n", tcLookUp2(tc->srcAlpha[1]), tcLookUp2(tc->opAlpha[1]));
			printf("\t\t\tt_CmbIn2.a = %s%s\n", tcLookUp2(tc->srcAlpha[2]), tcLookUp2(tc->opAlpha[2]));
			printf("\t\tt_CmbOut = vec4((%s).rgb, (%s).a)\n", tcLookUp2(tc->combineRGB), tcLookUp2(tc->combineAlpha));
			printf("\n");
		}
	}
}

const char* tcLookUp1(uint16_t n)
{
	switch(n)
	{
		case 0x1E01: return("REPLACE                  | (t_CmbIn0)");
		case 0x2100: return("MODULATE                 | (t_CmbIn0 * t_CmbIn1)");
		case 0x0104: return("ADD                      | (t_CmbIn0 + t_CmbIn1)");
		case 0x8574: return("ADD_SIGNED               | (t_CmbIn0 + tCmbIn1) - 0.5");
		case 0x8575: return("INTERPOLATE              | mix(t_CmbIn0, t_CmbIn1, t_CmbIn2)");
		case 0x84E7: return("SUBTRACT                 | (t_CmbIn0 - t_CmbIn1)");
		case 0x86AE: return("DOT3_RGB                 | vec4(vec3(4.0 * (dot(t_CmbIn0 - 0.5, t_CmbIn1 - 0.5))), 1.0)");
		case 0x86AF: return("DOT3_RGBA                | vec4(4.0 * (dot(t_CmbIn0 - 0.5, t_CmbIn1 - 0.5)))");
		case 0x6401: return("MULT_ADD                 | ((t_CmbIn0 * t_CmbIn1) + t_CmbIn2)");
		case 0x6402: return("ADD_MULT                 | ((t_CmbIn0 + t_CmbIn1) * t_CmbIn2)");
		case 0x84C0: return("TEXTURE0                 | tex0");
		case 0x84C1: return("TEXTURE1                 | tex1");
		case 0x84C2: return("TEXTURE2                 | tex2");
		case 0x84C3: return("TEXTURE3                 | unsupported");
		case 0x8576: return("CONSTANT                 | t_CmbConstant");
		case 0x8577: return("PRIMARY_COLOR            | v_Color");
		case 0x8578: return("PREVIOUS                 | t_CmbOut");
		case 0x8579: return("PREVIOUS_BUFFER          | t_CmbOutBuffer");
		case 0x6210: return("FRAGMENT_PRIMARY_COLOR   | v_Color");
		case 0x6211: return("FRAGMENT_SECONDARY_COLOR | v_Color");
		case 0x0300: return("SRC_COLOR                | rv.rgba");
		case 0x0301: return("ONE_MINUS_SRC_COLOR      | (1.0 - rv.rgba)");
		case 0x0302: return("SRC_ALPHA                | rv.aaaa");
		case 0x0303: return("ONE_MINUS_SRC_ALPHA      | (1.0 - rv.rgba)");
		case 0x8580: return("SRC_R                    | rv.rrrr");
		case 0x8581: return("SRC_G                    | rv.gggg");
		case 0x8582: return("SRC_B                    | rv.bbbb");
		case 0x8583: return("ONE_MINUS_SRC_R          | (1.0 - rv.rrrr)");
		case 0x8584: return("ONE_MINUS_SRC_G          | (1.0 - rv.gggg)");
		case 0x8585: return("ONE_MINUS_SRC_B          | (1.0 - rv.bbbb)");
		default: return("UNKNOWN");
	}
}

const char* tcLookUp2(uint16_t n)
{
	switch(n)
	{
		case 0x1E01: return("t_CmbIn0");
		case 0x2100: return("t_CmbIn0 * t_CmbIn1");
		case 0x0104: return("(t_CmbIn0 + t_CmbIn1)");
		case 0x8574: return("(t_CmbIn0 + tCmbIn1) - 0.5");
		case 0x8575: return("mix(t_CmbIn0, t_CmbIn1, t_CmbIn2)");
		case 0x84E7: return("(t_CmbIn0 - t_CmbIn1)");
		case 0x86AE: return("vec4(vec3(4.0 * (dot(t_CmbIn0 - 0.5, t_CmbIn1 - 0.5))), 1.0)");
		case 0x86AF: return("vec4(4.0 * (dot(t_CmbIn0 - 0.5, t_CmbIn1 - 0.5)))");
		case 0x6401: return("((t_CmbIn0 * t_CmbIn1) + t_CmbIn2)");
		case 0x6402: return("((t_CmbIn0 + t_CmbIn1) * t_CmbIn2)");
		case 0x84C0: return("tex0");
		case 0x84C1: return("tex1");
		case 0x84C2: return("tex2");
		case 0x84C3: return("unsupported");
		case 0x8576: return("t_CmbConstant");
		case 0x8577: return("v_Color");
		case 0x8578: return("t_CmbOut");
		case 0x8579: return("t_CmbOutBuffer");
		case 0x6210: return("v_Color");
		case 0x6211: return("v_Color");
		case 0x0300: return(".rgb");
		case 0x0301: return("(1.0 - .rgb)");
		case 0x0302: return(".a");
		case 0x0303: return("(1.0 - .rgb)");
		case 0x8580: return(".rrr");
		case 0x8581: return(".ggg");
		case 0x8582: return(".bbb");
		case 0x8583: return("(1.0 - .rrr)");
		case 0x8584: return("(1.0 - .ggg)");
		case 0x8585: return("(1.0 - .bbb)");
		default: return("UNKNOWN");
	}
}
