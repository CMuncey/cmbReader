#ifndef SKLCHUNK_H
#define SKLCHUNK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct
{
	uint8_t     id;
	uint8_t   unk1;
	int16_t    pID;
	float scale[3];
	float   rot[3];
	float trans[3];
} bone_t;

typedef struct
{
	char   magic[4];
	uint32_t   size;
	uint32_t nBones;
	uint32_t   unk1;
	bone_t*   bones;
} sklChunk_t;

/* Debug printouts */
void printSklChunk(sklChunk_t);
void printBone(bone_t);

#ifdef __cplusplus
}
#endif

#endif
