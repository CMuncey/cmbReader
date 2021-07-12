#ifndef VATRCHUNK_H
#define VATRCHUNK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct
{
	float x;
	float y;
	float z;
} float3_t;

typedef struct
{
	float x;
	float y;
} float2_t;

typedef struct
{
	uint32_t   size;
	uint32_t offset;
} vertListDat_t;

typedef struct
{
	char            magic[4];
	uint32_t            size;
	uint32_t          maxInd;
	vertListDat_t  positions;
	vertListDat_t    normals;
	vertListDat_t     colors;
	vertListDat_t tex0Coords;
	vertListDat_t tex1Coords;
	vertListDat_t tex2Coords;
	vertListDat_t    boneInd;
	vertListDat_t boneWeight;
} vatrChunk_t;

typedef struct
{
	uint8_t*  positions;
	uint8_t*    normals;
	uint8_t*     colors;
	uint8_t* tex0Coords;
	uint8_t* tex1Coords;
	uint8_t* tex2Coords;
	uint8_t*    boneInd;
	uint8_t* boneWeight;
} vatrData_t;

/* Debug printouts */
void printVatrChunk(vatrChunk_t);
void printVertListDat(vertListDat_t);

//void printSomeVertexData(const uint8_t*, cmbChunk_t, vatrChunk_t);

#ifdef __cplusplus
}
#endif

#endif
