#ifndef LUTSCHUNK_H
#define LUTSCHUNK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct
{
	char      magic[4];
	uint32_t      size;
	uint32_t lutSetNum;
	uint32_t      unk1;
} lutsChunk_t;

/* Debug printouts */
void printLutsChunk(lutsChunk_t);

#ifdef __cplusplus
}
#endif

#endif
