#ifndef CMBCHUNK_H
#define CMBCHUNK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct
{
	char     magic[4];
	uint32_t     size;
	uint32_t  version;
	uint32_t   unused;
	char     name[16];
	uint32_t nIndices;
	uint32_t  sklCPtr;
	uint32_t matsCPtr;
	uint32_t  texCPtr;
	uint32_t sklmCPtr;
	uint32_t lutsCPtr;
	uint32_t vatrCPtr;
	uint32_t  vIndPtr;
	uint32_t  tDatPtr;
} cmbChunk_t;

/* Debug printouts */
void printCmbChunk(cmbChunk_t);

#ifdef __cplusplus
}
#endif

#endif
