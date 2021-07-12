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
