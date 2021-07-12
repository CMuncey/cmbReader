#ifndef SKLMCHUNK_H
#define SKLMCHUNK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define SEPD_HAS_POSITIONS 0x01
#define SEPD_HAS_NORMALS  0x02
#define SEPD_HAS_COLORS   0x04
#define SEPD_HAS_UV0      0x08
#define SEPD_HAS_UV1      0x10
#define SEPD_HAS_UV2      0X20
#define SEPD_HAS_INDICES  0x40
#define SEPD_HAS_WEIGHTS  0x80

typedef struct
{
	uint16_t sepdInd;
	uint8_t  matsInd;
	uint8_t       id;
} mesh_t;

typedef struct
{
	char    magic[4];
	uint32_t    size;
	uint32_t nMeshes;
	uint16_t    unk1;
	uint16_t idCount;
	mesh_t*   meshes;
} mshsChunk_t;

typedef struct
{
	uint32_t   offset;
	float       scale;
	uint16_t  datType;
	uint16_t     mode;
	float constant[4];
} vertList_t;

typedef struct
{
	char     magic[4];
	uint32_t     size;
	uint32_t  visible;
	uint32_t primMode;
	uint16_t  datType;
	uint16_t     unk1;
	uint16_t nVertInd;
	uint16_t firstInd;
} prmChunk_t;

typedef struct
{
	char     magic[4];
	uint32_t     size;
	uint32_t     nPRM;
	uint16_t skinMode;
	uint16_t nBoneInd;
	uint32_t bIndOffs;
	prmChunk_t*  prmC;
	uint16_t*   bInds;
} prmsChunk_t;

typedef struct
{
	char          magic[4];
	uint32_t          size;
	uint16_t         nPRMS;
	uint16_t        flags1;
	float        center[3];
	float        posOff[3];
	vertList_t   positions;
	vertList_t     normals;
	vertList_t  vertColors;
	vertList_t  tex0Coords;
	vertList_t  tex1Coords;
	vertList_t  tex2Coords;
	vertList_t boneIndices;
	vertList_t boneWeights;
	uint16_t boneDimension;
	uint16_t        flags2;
	prmsChunk_t*     prmsC;
} sepdChunk_t;

typedef struct
{
	char      magic[4];
	uint32_t      size;
	uint32_t    nSEPDs;
	uint32_t     flags;
	sepdChunk_t* sepdC;
} shpChunk_t;

typedef struct
{
	char      magic[4];
	uint32_t      size;
	mshsChunk_t* mshsC;
	shpChunk_t*   shpC;
} sklmChunk_t;

/* Debug printouts */
void printSklmChunk(sklmChunk_t);
void printMshsChunk(mshsChunk_t);
void printMesh(mesh_t);
void printShpChunk(shpChunk_t);
void printSepdChunk(sepdChunk_t);
void printPrmsChunk(prmsChunk_t);
void printPrmChunk(prmChunk_t);
void printVertList(vertList_t);

#ifdef __cplusplus
}
#endif

#endif
