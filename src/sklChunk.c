#include <stdio.h>
#include <stdlib.h>
#include "sklChunk.h"

void printSklChunk(sklChunk_t s)
{
	int i;

	printf("SKL magic:     %.4s\n", s.magic);
	printf("SKL size:      %u\n", s.size);
	printf("SKL num bones: %u\n", s.nBones);
	printf("SKL bones:\n");
	for(i = 0; i < s.nBones; ++i)
	{
		printBone(s.bones[i]);
		if(i != s.nBones - 1)
			printf("\n");
	}

	return;
}

void printBone(bone_t b)
{
	printf("\tBONE id:          %u\n", b.id);
	printf("\tBONE parent id:   %d\n", b.pID);
	printf("\tBONE scale:       % 8.7f, % 8.7f, % 8.7f\n", b.scale[0], b.scale[1], b.scale[2]);
	printf("\tBONE rotation:    % 8.7f, % 8.7f, % 8.7f\n", b.rot[0], b.rot[1], b.rot[2]);
	printf("\tBONE translation: % 8.7f, % 8.7f, % 8.7f\n", b.trans[0], b.trans[1], b.trans[2]);

	return;
}
