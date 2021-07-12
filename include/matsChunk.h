#ifndef MATSCHUNK_H
#define MATSCHUNK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct
{
	uint8_t R;
	uint8_t G;
	uint8_t B;
	uint8_t A;
} rgba_t;

typedef struct
{
	int16_t   texIndex;
	int16_t    padding;
	uint16_t MinFilter;
	uint16_t MagFilter;
	uint16_t     WrapS;
	uint16_t     WrapT;
	float   minLODBias;
	float      LODBias;
	rgba_t borderColor;
} matTex_t;

typedef struct
{
	uint8_t matrixMode;
	uint8_t     refCam;
	uint8_t  mapMehtod;
	uint8_t CoordIndex;
	float     scale[2];
	float     rotation;
	float     trans[2];
} texCoords_t;

typedef struct
{
	uint16_t   combineRGB;
	uint16_t combineAlpha;
	uint16_t     scaleRGB;
	uint16_t   scaleAlpha;
	uint16_t    bufInpRGB;
	uint16_t  bufInpAlpha;
	uint16_t    srcRGB[3];
	uint16_t     opRGB[3];
	uint16_t  srcAlpha[3];
	uint16_t   opAlpha[3];
	uint32_t     constInd;
} texCombiner_t;

typedef struct
{
	uint8_t      fragLightOn;
	uint8_t      vertLightOn;
	uint8_t      hemiLightOn;
	uint8_t       faceCullOn;
	uint8_t     faceCullMode;
	uint8_t       polyOffsOn;
	uint8_t             unk1;
	int8_t        polyOffset;
	float        depthOffset;
	uint32_t        tMapUsed;
	uint32_t      tCoordUsed;
	matTex_t     textures[3];
	texCoords_t   tCoords[3];
	rgba_t       emissiveCol;
	rgba_t        ambientCol;
	rgba_t        diffuseCol;
	rgba_t        specCol[2];
	rgba_t       constCol[6];
	float        bufColor[4];
	uint16_t         bumpTex;
	uint16_t        bumpMode;
	uint16_t    isBumpRenorm;
	uint16_t        padding1;
	uint16_t     layerConfig;
	uint16_t        padding2;
	uint16_t      fresnelSel;
	uint8_t    isClampHlight;
	uint8_t          dist0On;
	uint8_t          dist1On;
	uint8_t     geoFactor0On;
	uint8_t     geoFactor1On;
	uint8_t     reflectionOn;
	uint8_t    reflRSamplOff; /* IsAbs ??? */
	uint8_t    reflRSamplInd;
	uint16_t   reflRSamplInp;
	float    reflRSamplScale;
	uint8_t    reflGSamplOff; /* IsAbs ??? */
	uint8_t    reflGSamplInd;
	uint16_t   reflGSamplInp;
	float    reflGSamplScale;
	uint8_t    reflBSamplOff; /* IsAbs ??? */
	uint8_t    reflBSamplInd;
	uint16_t   reflBSamplInp;
	float    reflBSamplScale;
	uint8_t    dist0SamplOff; /* IsAbs ??? */
	uint8_t    dist0SamplInd;
	uint16_t   dist0SamplInp;
	float    dist0SamplScale;
	uint8_t    dist1SamplOff; /* IsAbs ??? */
	uint8_t    dist1SamplInd;
	uint16_t   dist1SamplInp;
	float    dist1SamplScale;
	uint8_t    fres1SamplOff; /* IsAbs ??? */
	uint8_t    fres1SamplInd;
	uint16_t   fres1SamplInp;
	float    fres1SamplScale;
	uint32_t   nTexCombiners;
	int16_t       texCInd[6];
	uint8_t      alphaTestOn;
	uint8_t      alphaRefVal;
	uint16_t       alphaFunc;
	uint8_t      depthTestOn;
	uint8_t     depthWriteOn;
	uint16_t   depthTestFunc;
	uint8_t     blendEnabled;
	uint8_t             unk2;
	uint16_t            unk3;
	uint16_t blendSrcFactRGB;
	uint16_t blendDstFactRGB;
	uint16_t    blendFuncRGB;
	uint16_t            unk4;
	uint16_t blendSrcFactAlp;
	uint16_t blendDstFactAlp;
	uint16_t    blendFuncAlp;
	uint16_t            unk5;
	float      blendColor[4];
} material_t;

typedef struct
{
	char       magic[4];
	uint32_t       size;
	uint32_t      nMats;
	uint32_t      nTCom;
	material_t*    mats;
	texCombiner_t* tCom;
} matsChunk_t;

/* Debug printouts */
void printMatsChunk(matsChunk_t);
void printMaterial(material_t);
void printMatTex(matTex_t);
void printTexCoords(texCoords_t);
void printTexCombiner(texCombiner_t);
void printRGBA(rgba_t);

#ifdef __cplusplus
}
#endif

#endif
