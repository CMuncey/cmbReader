#ifndef MATSCHUNK_H
#define MATSCHUNK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct
{
	char  magic[4];
	uint32_t  size;
	uint32_t nBBox;
	uint32_t  unk1;
	uint32_t  unk2;
	float     minX;
	float     minY;
	float     minZ;
	float     maxX;
	float     maxY;
	float     maxZ;
	int32_t   unk3;
	int32_t   unk4;
	uint32_t  unk5;
} qtrsChunk_t;

#ifdef __cplusplus
}
#endif

#endif
