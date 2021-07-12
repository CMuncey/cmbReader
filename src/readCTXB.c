#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "texChunk.h"
#include "cmbTextures.h"

/* CTXB header */
typedef struct
{
	char     magic[4];
	uint32_t     size;
	uint32_t    nTexC;
	uint32_t  unknown;
	uint32_t texCOffs;
	uint32_t texDOffs;
} ctxbChunk_t;

int main(int argc, char** argv)
{
	ctxbChunk_t* ctxbC;
	texChunk_t* texC;
	uint8_t* data;
	FILE* f;
	int i, j, k, l;
	char* fn;

	/* Check the arguments */
	if(argc < 2)
	{
		fprintf(stderr, "Usage: %s filename.zar\n", argv[0]);
		exit(1);
	}

	/* Open and check the file */
	f = fopen(argv[1], "rb");
	if(!f)
	{
		perror(argv[1]);
		exit(1);
	}

	/* Read the entire thing into memory */
	fseek(f, 0, SEEK_END);
	i = ftell(f);
	fseek(f, 0, SEEK_SET);
	data = malloc(i);
	fread(data, 1, i, f);
	fclose(f);

	/* Read the data into the chunks */
	ctxbC = (ctxbChunk_t*)(data);
	texC = readTexChunk(data, ctxbC->texCOffs, ctxbC->nTexC);

	/* Print out the ctxb chunk */
	printf("CTXB magic:                %.4s\n", ctxbC->magic);
	printf("CTXB size:                 %u\n", ctxbC->size);
	printf("CTXB Number of tex chunks: %u\n", ctxbC->nTexC);
	printf("CTXB Tex chunk offset:     0x%08x\n", ctxbC->texCOffs);
	printf("CTXB Tex data offset:      0x%08x\n", ctxbC->texDOffs);
	printf("\n");

	/* Get the filename for textures */
	l = strlen(argv[1]);
	fn = malloc(l + 8);
	strncpy(fn, argv[1], l);

	/* Print and dump textures */
	for(i = k = 0; i < ctxbC->nTexC; ++i)
	{
		printf("TexChunk %d:\n", i);
		printTexChunk(texC[i]);
		for(j = 0; j < texC[i].nTex; ++j)
		{
			fn[l] = '\0';
			sprintf(fn + l, "-%d.bmp", k);
			dumpBMP(data + ctxbC->texDOffs, &(texC[i].tex[j]), fn);
		}
	}

	return(0);
}
