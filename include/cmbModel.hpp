#ifndef CMBMODEL_H
#define CMBMODEL_H

#include <vector>
#include "glm/glm.hpp"

#include "cmb.h"
#include "matsChunk.h"
#include "cmbConstants.h"
#include "cmbShader.hpp"
#include "sklmChunk.h"
using namespace std;

/* SKLM chunk has a MSHS chunk, which has some number of MESH chunks */
/* Each MESH chunk has SEPD index and a MATS index */
/* SEPD chunk contains actual data and things like scale/weights */
/* MATS chunk contains info about textures, draw settings */

struct sepdParams_t
{
	float   posScale;
	float colorScale;
	float  tex0Scale;
	float  tex1Scale;
	float  tex2Scale;
	float boneWScale;
	int      boneDim;
	int    hasVColor;
};

struct matParams_t
{
	glm::mat4   texMat[3];
	glm::vec4 constCol[6];
	uint32_t   texInfo[4];
	float           depth;
};

struct renderParams_t
{
	int  blendingOn;
	int     bSrcRGB;
	int     bDstRGB;
	int     bSrcAlp;
	int     bDstAlp;
	int    bFuncRGB;
	int    bFuncAlp;
	float bColor[4];

	int depthWrite;
	int  depthFunc;

	int cullMode;

	matParams_t matP;
};

struct modelSEPD_t
{
	uint32_t VAO, VBOs[8];
	uint32_t EBO, UBOs[2];

	uint32_t nInd;

	sepdParams_t prms;
};

struct modelMAT_t
{
	uint32_t TEXs[3];

	renderParams_t rP;
	matParams_t    mP;
};

struct cmbMesh_t
{
	glm::mat4*  projMat;
	glm::mat4*  viewMat;
	glm::mat4* modelMat;
	glm::mat3*  normMat;

	cmbShader_t* shader;
	modelSEPD_t*   sepd;
	modelMAT_t*     mat;
};

struct cmbModel_t
{
	int32_t nMeshes, nBones;
	int32_t    nTex, nSEPDs;
	int32_t nShaders, nMats;

	cmbShader_t*  shaders;
	uint8_t**    textures;
	modelMAT_t*      mats;
	modelSEPD_t*    sepds;
	glm::mat4*      bones;
	cmbMesh_t*     meshes;

	glm::mat4  projMat;
	glm::mat4  viewMat;
	glm::mat4 modelMat;
	glm::mat3  normMat;

	/* Debug */
	int         meshNum;
};

/* Model functions */
int8_t makeCmbModel(cmbModel_t*, const cmb_t*);
uint8_t* makeTexture(int, const cmb_t*);
void makeMat(modelMAT_t*, int, const cmb_t*,  const cmbModel_t*);
void readSEPD(modelSEPD_t*, int, const cmb_t*);
glm::mat4* makeBones(const cmb_t*);
void drawCmbModel(const cmbModel_t*);
void delCmbModel(cmbModel_t*);

/* Mesh functions */
uint8_t getDTSize(picaDataType p);
void makeCmbMesh(cmbMesh_t*, int, const cmb_t*, cmbModel_t*);
void drawCmbMesh(cmbMesh_t*);

#endif
