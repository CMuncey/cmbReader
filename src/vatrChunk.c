#include <stdio.h>
#include <stdlib.h>
#include "vatrChunk.h"

void printVatrChunk(vatrChunk_t v)
{
	printf("VATR magic:             %.4s\n", v.magic);
	printf("VATR size:              %u\n", v.size);
	printf("VATR num vertices?:     %u\n", v.maxInd);
	printf("VATR position data:     ");
	printVertListDat(v.positions);
	printf("VATR normal data:       ");
	printVertListDat(v.normals);
	printf("VATR colors data:       ");
	printVertListDat(v.colors);
	printf("VATR tex 0 Coords data: ");
	printVertListDat(v.tex0Coords);
	printf("VATR tex 1 Coords data: ");
	printVertListDat(v.tex1Coords);
	printf("VATR tex 2 Coords data: ");
	printVertListDat(v.tex2Coords);
	printf("VATR bone index data:   ");
	printVertListDat(v.boneInd);
	printf("VATR bone weight data:  ");
	printVertListDat(v.boneWeight);

	return;
}

void printVertListDat(vertListDat_t v)
{
	printf("%u, 0x%08x\n", v.size, v.offset);
	return;
}

//void printSomeVertexData(const uint8_t* d, cmbChunk_t c, vatrChunk_t v)
//{
//	float* f;
//	int i, n, offs;
//
//	/* number of vec3s in the position data */
//	n = ((v.positions.size >> 2) / 3);
//	offs = c.vatrCPtr + v.positions.offset;
//
//	printf("Vertex positions:\n");
//	for(i = 0; i < n; ++i)
//	{
//		f = (float*)(d + offs + (i * 12));
//		printf("%3.2f, %3.2f, %3.2f\n", f[0], f[1], f[2]);
//	}
//}
