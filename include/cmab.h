#ifndef CMAB_H
#define CMAB_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	uint32_t         type;
	uint32_t numKeyFrames;
	uint32_t      timeEnd;
	float        unknown1;
	uint32_t     unknown2;
} cmabTrack_t;

typedef struct
{
	char         magic[4];
	uint32_t     animType;
	uint32_t       matInd;
	uint32_t   channelInd;
	cmabTrack_t*   tracks;
} cmabMMAD_t;

typedef struct
{

} cmabTXPT_t;

typedef struct
{
	char      magic[4];
	uint32_t  numAnims;
	cmabMMAD_t*  mmads;
	cmabTXPT_t*  txpts;
} cmabMADS_t;

typedef struct
{
	char    magic[4];
	uint32_t numTabs;
	uint32_t  tabInd;
	char**  strTable;
} cmabStrTab_t;

typedef struct
{
	char       magic[4];
	uint32_t subVersion;
	uint32_t       size;
	uint32_t   unknown1;
	uint32_t  numChunks;
	uint32_t  chunkOffs;
	uint32_t strTabOffs;
	uint32_t texDatOffs;
	uint32_t  chunkType;
	uint32_t   duration;
	uint32_t   unknown2;
	uint32_t   unknown3;
	uint32_t   txptOffs;
} cmab_t;

#ifdef __cplusplus
}
#endif

#endif
