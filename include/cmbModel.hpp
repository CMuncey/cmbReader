#ifndef CMBMODEL_H
#define CMBMODEL_H

#include <vector>
#include "glm/glm.hpp"

#include "cmb.h"
#include "matsChunk.h"
#include "cmbConstants.h"
#include "cmbShader.hpp"
using namespace std;

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
	glm::mat4*  projMat;
	glm::mat4*  viewMat;
	glm::mat4* modelMat;
	glm::mat3*  normMat;

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
};

struct cmbMesh_t
{
	unsigned int    nInd, VAO, VBOs[8];
	unsigned int EBO, UBOs[2], TEXs[3];

	cmbShader_t*  shader;
	sepdParams_t  sepdP;
	matParams_t    matP;
	renderParams_t rendP;
};

struct cmbModel_t
{
	int32_t  nMeshes, nBones;
	cmbMesh_t*        meshes;
	glm::mat4*         bones;

	glm::mat4  projMat;
	glm::mat4  viewMat;
	glm::mat4 modelMat;
	glm::mat3  normMat;

	cmbShader_t* shaders;
	sepdParams_t* sepdPs;
	matParams_t* matPs;

	/* Debug */
	int         meshNum;
};

/* Model functions */
int8_t makeCmbModel(cmbModel_t*, const cmb_t*);
glm::mat4* makeBones(const cmb_t*);
void drawCmbModel(const cmbModel_t*);
void delCmbModel(cmbModel_t*);

/* Mesh functions */
uint8_t getDTSize(picaDataType p);
void makeCmbMesh(cmbMesh_t*, int, const cmb_t*, cmbModel_t*);
void drawCmbMesh(cmbMesh_t*);
void delCmbMesh(cmbMesh_t*);

#endif
