#include <stdio.h>
#include <stdlib.h>
#include "lutsChunk.h"

void printLutsChunk(lutsChunk_t l)
{
	printf("LUTS magic:            %.4s\n", l.magic);
	printf("LUTS size:             %u\n", l.size);
	printf("LUTS luts set number?: %u\n", l.lutSetNum);

	return;
}
