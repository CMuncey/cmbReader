#ifndef CMBMODEL_H
#define CMBMODEL_H

#include <string>
#include "glm/glm.hpp"
#include "matsChunk.h"
#include "cmbConstants.h"
#include "cmbShader.h"
#include "cmb.h"
using namespace std;

struct cmbMaterial_t
{
	int8_t hasTex0;
	int8_t hasTex1;
	int8_t hasTex2;
	int8_t hasFCol;

	int32_t aFunc;
	float    aRef;

	uint32_t tex0;
	uint32_t tex1;
	uint32_t tex2;

	float  nrmScl;
	float  colScl;
	float tex0Scl;
	float tex1Scl;
	float tex2Scl;

	glm::vec4 ambiC;
	glm::vec4 diffC;
	glm::vec4 spc0C;
//	glm::vec4 spc1C;
	glm::vec4 emisC;
	glm::vec4 buffC;
	glm::vec4 constC[6];

	int nTCom;
	texCombiner_t tCom[6];

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
	float    depth;

	int cullMode;
};

struct cmbMesh_t
{
	unsigned int nInd, VAO, VBOs[8];
	unsigned int       EBO, TEXs[3];
	cmbMaterial_t               mat;
};

struct cmbModel_t
{
	int meshNum;
	int nMeshes, nBones;
	cmbMesh_t*   meshes;
	glm::mat4*    bones;
};

/* Model functions */
int8_t makeCmbModel(cmbModel_t*, const cmb_t*);
glm::mat4* makeBones(const cmb_t*);
void drawCmbModel(const cmbModel_t*, cmbShader_t* s);
void delCmbModel(cmbModel_t*);

/* Mesh functions */
uint8_t getDTSize(picaDataType p);
void makeCmbMesh(cmbMesh_t*, int, const cmb_t*, const glm::mat4*);
void drawCmbMesh(const cmbMesh_t*, cmbShader_t* s);
void delCmbMesh(cmbMesh_t*);

#endif
