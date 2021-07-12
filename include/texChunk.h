#ifndef TEXCHUNK_H
#define TEXCHUNK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct
{
	uint32_t  rightOffs;
	uint32_t   leftOffs;
	uint32_t bottomOffs;
	uint32_t    topOffs;
	uint32_t  frontOffs;
	uint32_t   backOffs;
} cubemap_t;

typedef struct
{
	uint32_t     size;
	uint16_t nMipmaps;
	uint8_t    isETC1;
	uint8_t isCubemap;
	uint16_t    width;
	uint16_t   height;
	uint16_t   colFmt;
	uint16_t  datType;
	uint32_t   offset;
	char     name[16];
	cubemap_t cubemap;
} texture_t;

typedef struct
{
	char  magic[4];
	uint32_t  size;
	uint32_t  nTex;
	texture_t* tex;
} texChunk_t;

/* Debug printouts */
void printTexChunk(texChunk_t);
void printTexture(texture_t);

#ifdef __cplusplus
}
#endif

#endif
