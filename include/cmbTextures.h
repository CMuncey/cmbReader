#ifndef CMBTEXTURES_H
#define CMBTEXTURES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "texChunk.h"

/* Look-up table for the texture swizzle */
/* And the color modifiers for ETC1 */
extern int8_t swizzleLUT[64];
extern int16_t etc1LUT[8][4];

typedef struct
{
	uint8_t R;
	uint8_t G;
	uint8_t B;
	uint8_t A;
} texRGBA8_t;

typedef struct
{
	uint8_t R;
	uint8_t G;
	uint8_t B;
} texRGB8_t;

/* Debug bmp header stuff */
/* Don't pad this one */
#pragma pack(push, 1)
typedef struct
{
	char    magic[2];
	uint32_t    size;
	uint16_t    res1;
	uint16_t    res2;
	uint32_t pixOffs;
	uint32_t  h2size;
	int32_t    width;
	int32_t   height;
	uint16_t cPlanes;
	uint16_t bitPPix;
	uint32_t compMet;
	uint32_t imgSize;
	int32_t     ppmX;
	int32_t     ppmY;
	uint32_t cPallet;
	uint32_t   cImpt;
} bmpHeaders_t;
#pragma pack(pop)

uint32_t getFmtSize(uint32_t);
void getColor(const uint8_t*, texRGBA8_t*, uint32_t);
int clamp(int, int, int);
uint8_t exp4to8(uint8_t);
uint8_t exp5to8(uint8_t);
uint8_t exp6to8(uint8_t);
int8_t signed3b(int32_t);
void etc1GetCols(uint8_t*, const int16_t*, const texRGB8_t*);
void etc1Alpha(uint8_t*, uint64_t, uint32_t);
void etc1Color(uint8_t*, uint64_t, uint32_t);
void etc1Decompress(const uint8_t*, uint8_t*, const texture_t*);
void decodeImg(const uint8_t*, uint8_t*, const texture_t*);
void dumpBMP(const uint8_t*, const texture_t*);

#ifdef __cplusplus
}
#endif

#endif
