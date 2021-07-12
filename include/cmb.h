#ifndef CMB_H
#define CMB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "cmbChunk.h"
#include "sklChunk.h"
#include "matsChunk.h"
#include "texChunk.h"
#include "sklmChunk.h"
#include "lutsChunk.h"
#include "vatrChunk.h"

typedef struct
{
	uint8_t*       data;
	cmbChunk_t*    cmbC;
	sklChunk_t*    sklC;
	matsChunk_t*  matsC;
	texChunk_t*    texC;
	sklmChunk_t*  sklmC;
	lutsChunk_t*  lutsC;
	vatrChunk_t*  vatrC;
	vatrData_t*   vatrD;
	uint8_t*    vIndDat;
	uint8_t*     texDat;
} cmb_t;

/* Read functions, readCmb is the important one */
int8_t readCmb(cmb_t*, const char*);
int8_t readCmbChunk(cmb_t*);
int8_t readSklChunk(cmb_t*);
int8_t readMatsChunk(cmb_t*);
int8_t readTexChunk(cmb_t*);
int8_t readSklmChunk(cmb_t*);
int8_t readLutsChunk(cmb_t*);
int8_t readVatrChunk(cmb_t*);

void delCmb(cmb_t);

/* Debug printouts */
void printCmb(cmb_t);

#ifdef __cplusplus
}
#endif

#endif
