#include <stdlib.h>
#include <stdio.h>
#include "texChunk.h"

void printTexChunk(texChunk_t t)
{
	int i;

	printf("TEX magic:        %.4s\n", t.magic);
	printf("TEX size:         %u\n", t.size);
	printf("TEX num textures: %u\n", t.nTex);
	printf("\n");

	for(i = 0; i < t.nTex; ++i)
	{
		printf("Texture %d:\n", i);
		printTexture(t.tex[i]);
		if(i != t.nTex - 1)
			printf("\n");
	}

	return;
}

void printTexture(texture_t t)
{
	printf("Texture name:         %.16s\n", t.name);
	printf("Texture size:         %u\n", t.size);
	printf("Texture mipmap count: %u\n", t.nMipmaps);
	printf("Texture isETC1:       %d\n", t.isETC1);
	printf("Texture isCubemap:    %d\n", t.isCubemap);
	printf("Texture width:        %u\n", t.width);
	printf("Texture height:       %u\n", t.height);
	printf("Texture color format: 0x%04x\n", t.colFmt);
	printf("Texture data type:    0x%04x\n", t.datType);
	printf("Texture data offset:  %u\n", t.offset);

	return;
}

//typedef struct
//{
//	uint16_t r : 5;
//	uint16_t g : 6;
//	uint16_t b : 5;
//} rgb565_t;
//
//TODO move to textures.h/c
///* Just assume rgb565 for now */
//void dumpTexture(cmb_t c, texture_t t, const char* fn)
//{
//	rgb565_t rgb565;
//	uint32_t offs, i, j;
//	uint8_t* img;
//	FILE* f;
//
//	printf("Dumping texture %.16s\n", t.name);
//
//	img = malloc(t.width * t.height * 3);
//	offs = c.tDatPtr + t.offset;
//
//	for(i = j = 0; i < t.size; i += 2, j += 3)
//	{
//		rgb565 = *(rgb565_t*)(d + offs + i);
//		img[j] = rgb565.b;
//		img[j + 1] = rgb565.g;
//		img[j + 2] = rgb565.r;
//	}
//
//	/* Hacky bmp stuff */
//	f = fopen(fn, "wb");
//	if(f == NULL)
//		printf("bad file\n");
//	j = *(short*)("BM");
//	fwrite(&j, 2, 1, f);
//
//	/* Size */
//	j = (t.width * t.height * 3) + 54;
//	fwrite(&j, 4, 1, f);
//
//	/* reserved1, reserved2 */
//	j = 0;
//	fwrite(&j, 4, 1, f);
//
//	/* Pixel data start */
//	j = 54;
//	fwrite(&j, 4, 1, f);
//
//	/* Info header size, width, height */
//	j = 40;
//	fwrite(&j, 4, 1, f);
//	j = t.width;
//	fwrite(&j, 4, 1, f);
//	j = t.height;
//	fwrite(&j, 4, 1, f);
//
//	/* Planes, bitcount, compression/img size */
//	j = 1;
//	fwrite(&j, 2, 1, f);
//	j = 24;
//	fwrite(&j, 2, 1, f);
//	j = 0;
//	fwrite(&j, 4, 1, f);
//	fwrite(&j, 4, 1, f);
//
//	/* ppms and color stuff */
//	j = 32;
//	fwrite(&j, 4, 1, f);
//	fwrite(&j, 4, 1, f);
//	j = 0;
//	fwrite(&j, 4, 1, f);
//	fwrite(&j, 4, 1, f);
//
//	/* Actual data */
//	fwrite(img, 3, t.width * t.height, f);
//	fclose(f);
//}
