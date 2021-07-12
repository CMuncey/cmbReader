#include <stdio.h>
#include <stdlib.h>
#include "cmbChunk.h"

void printCmbChunk(cmbChunk_t c)
{
	printf("Magic:               %.4s\n", c.magic);
	printf("Size:                %u\n", c.size);
	printf("Version:             %u\n", c.version);
	printf("Name:                %.16s\n", c.name);
	printf("Number of indices:   %u\n", c.nIndices);
	printf("SKL Chunk pointer:   0x%08x\n", c.sklCPtr);
	printf("MATS Chunk pointer:  0x%08x\n", c.matsCPtr);
	printf("TEX Chunk pointer:   0x%08x\n", c.texCPtr);
	printf("SKLM Chunk pointer:  0x%08x\n", c.sklmCPtr);
	printf("LUTS Chunk pointer:  0x%08x\n", c.lutsCPtr);
	printf("VATR Chunk pointer:  0x%08x\n", c.vatrCPtr);
	printf("Vertex Ind pointer:  0x%08x\n", c.vIndPtr);
	printf("Texture Dat pointer: 0x%08x\n", c.tDatPtr);

	return;
}
