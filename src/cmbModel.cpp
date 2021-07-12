#include <string>
#include <stdint.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "cmbConstants.h"
#include "cmbTextures.h"
#include "sklmChunk.h"
#include "cmbModel.h"
#include "cmbShader.h"
#include "matsChunk.h"
#include "cmb.h"
using namespace std;

int8_t makeCmbModel(cmbModel_t* m, const cmb_t* c)
{
	int i, n;

	if(m == NULL)
	{
		fprintf(stderr, "Error: Loading unallocated model\n");
		return(1);
	}

	if(c == NULL)
	{
		fprintf(stderr, "Error: Loading model with unallocated CMB\n");
		return(1);
	}

	m->meshNum = -1;
	m->nMeshes = n = c->sklmC->mshsC->nMeshes;
	m->meshes = (cmbMesh_t*)malloc(sizeof(cmbMesh_t) * n);

	m->bones = makeBones(c);

	for(i = 0; i < n; ++i)
		makeCmbMesh(&(m->meshes[i]), i, c, m->bones);

	return(0);
}

glm::mat4* makeBones(const cmb_t* c)
{
	float sX, sY, sZ, cX, cY, cZ;
	float mat[16];
	int i, j, nBones;
	glm::mat4* bones;
	glm::vec3 scale;
	bone_t* bone;

	nBones = c->sklC->nBones;
	bones = (glm::mat4*)malloc(sizeof(glm::mat4) * nBones);

	for(i = 0; i < nBones; ++i)
	{
		bone = &(c->sklC->bones[i]);
		sX = sin(bone->rot[0]), cX = cos(bone->rot[0]);
		sY = sin(bone->rot[1]), cY = cos(bone->rot[1]);
		sZ = sin(bone->rot[2]), cZ = cos(bone->rot[2]);
		scale = glm::vec3(bone->scale[0], bone->scale[1], bone->scale[2]);

		/* Row 1 */
		mat[0] = scale.x * (cY * cZ);
		mat[1] = scale.x * (cY * sZ);
		mat[2] = scale.x * (-sY);
		mat[3] = 0.0f;

		/* Row 2 */
		mat[4] = scale.y * ((sX * sY * cZ) - (cX * sZ));
		mat[5] = scale.y * ((sX * sZ * sY) + (cX * cZ));
		mat[6] = scale.y * (sX * cY);
		mat[7] = 0.0f;

		/* Row 3 */
		mat[8]  = scale.z * ((cX * sY * cZ) + (sX * sZ));
		mat[9]  = scale.z * ((cX * sY * sZ) - (sX * cZ));
		mat[10] = scale.z * (cX * cY);
		mat[11] = 0.0f;

		/* Row 4 */
		mat[12] = bone->trans[0];
		mat[13] = bone->trans[1];
		mat[14] = bone->trans[2];
		mat[15] = 1.0f;

		bones[i] = glm::make_mat4(mat);

		/* Multiply parent by child, in that order */
		if(bone->pID != -1)
		{
			if(bone->pID > bone->id)
				printf("Bone came before parent\n");
			bones[i] = bones[bone->pID] * bones[i];
		}
	}

	return(bones);
}

void drawCmbModel(const cmbModel_t* m, cmbShader_t* s)
{
	uint32_t i;

	s->use();

	if(m->meshNum > -1)
		drawCmbMesh(&(m->meshes[m->meshNum]), s);
	else
		for(i = 0; i < m->nMeshes; ++i)
			drawCmbMesh(&(m->meshes[i]), s);
}

void delCmbModel(cmbModel_t* m)
{
	unsigned int i;

	for(i = 0; i < m->nMeshes; ++i)
		delCmbMesh(&(m->meshes[i]));
}

uint8_t getDTSize(picaDataType p)
{
	switch(p)
	{
		case signed8Bit:
		case unsigned8Bit:
			return(1);
		case signed16Bit:
		case unsigned16Bit:
			return(2);
		case signed32Bit:
		case unsigned32Bit:
		case float32Bit:
			return(4);
	}

	return(0);
}

/* I probably need to redo this to account for bones, not sure */
void makeCmbMesh(cmbMesh_t* m, int meshNum, const cmb_t* c, const glm::mat4* bones)
{
	unsigned int i, j, k, w, h, size;
	const float* bcolor;
	uint32_t* inds;
	uint8_t* tmpPtr;
	int constCol[4];
	GLenum dtype;
	mesh_t* cmbMesh;
	sepdChunk_t* sepdC;
	prmsChunk_t* prmsC;
	prmChunk_t* prmC;
	material_t* mat;
	matTex_t* mTex;
	texture_t* tex;
	texCombiner_t* tc;
	rgba_t col;

	cmbMesh = &(c->sklmC->mshsC->meshes[meshNum]);
	sepdC = &(c->sklmC->shpC->sepdC[cmbMesh->sepdInd]);
	mat = &(c->matsC->mats[cmbMesh->matsInd]);

	/* Set up some material variables */
	m->mat.hasFCol = m->mat.hasTex0 = m->mat.hasTex1 = m->mat.hasTex2 = 0;
	m->mat.aFunc = mat->alphaFunc;
	m->mat.aRef = (float)mat->alphaRefVal / 255.0;
	if(m->mat.aRef < -1.0f)
		m->mat.aRef = -1.0f;

	// TODO
	/* Dumb texture combiner stuff */
	m->mat.nTCom = mat->nTexCombiners;
	for(i = 0; i < m->mat.nTCom; ++i)
		memcpy(&(m->mat.tCom[i]), &(c->matsC->tCom[mat->texCInd[i]]), sizeof(texCombiner_t));

	/* Make the various buffers */
	glGenVertexArrays(1, &(m->VAO));
	glGenBuffers(8, m->VBOs);
	glGenBuffers(1, &(m->EBO));
	glGenTextures(3, m->TEXs);

	/* Bind the meshes VAO to load stuff to it */
	glBindVertexArray(m->VAO);

	/* Get the position data, this should always be here */
	if(sepdC->flags1 & SEPD_HAS_POSITIONS)
	{
		/* Position data type should always be float */
		size = c->vatrC->positions.size - sepdC->positions.offset;
		tmpPtr = c->vatrD->positions + sepdC->positions.offset;

		glBindBuffer(GL_ARRAY_BUFFER, m->VBOs[0]);
		glBufferData(GL_ARRAY_BUFFER, size, tmpPtr, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glEnableVertexAttribArray(0);
	}

	/* Get the normal data if there is any */
	if(sepdC->flags1 & SEPD_HAS_NORMALS)
	{
		dtype = (GLenum)sepdC->normals.datType;
		m->mat.colScl = sepdC->vertColors.scale;

		/* Normal mode 0 = read array */
		/* Mode 1 = constant value */
		if(sepdC->normals.mode == 0)
		{
			size = c->vatrC->normals.size - sepdC->normals.offset;
			tmpPtr = c->vatrD->normals + sepdC->normals.offset;
		}
		else
		{
			size = getDTSize((picaDataType)dtype) * 3;
			tmpPtr = (uint8_t*)(sepdC->normals.constant);
		}

		glBindBuffer(GL_ARRAY_BUFFER, m->VBOs[1]);
		glBufferData(GL_ARRAY_BUFFER, size, tmpPtr, GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, dtype, GL_TRUE, 0, (void*)0);
		glEnableVertexAttribArray(1);
	}

	/* Get the color data */
	if(sepdC->flags1 & SEPD_HAS_COLORS)
	{
		dtype = (GLenum)sepdC->vertColors.datType;
		m->mat.colScl = sepdC->vertColors.scale;
		m->mat.hasFCol = 1;

		if(sepdC->vertColors.mode == 0)
		{
			size = c->vatrC->colors.size - sepdC->vertColors.offset;
			tmpPtr = c->vatrD->colors + sepdC->vertColors.offset;
		}
		else
		{
			size = getDTSize((picaDataType)dtype) * 4;
			tmpPtr = (uint8_t*)(sepdC->vertColors.constant);
		}

		glBindBuffer(GL_ARRAY_BUFFER, m->VBOs[2]);
		glBufferData(GL_ARRAY_BUFFER, size, tmpPtr, GL_STATIC_DRAW);
		glVertexAttribPointer(2, 4, dtype, GL_FALSE, 0, (void*)0);
		glEnableVertexAttribArray(2);
	}

	/* Get texture 0 coordinate data if there is any */
	if(sepdC->flags1 & SEPD_HAS_UV0)
	{
		dtype = (GLenum)sepdC->tex0Coords.datType;
		size = c->vatrC->tex0Coords.size - sepdC->tex0Coords.offset;
		tmpPtr = c->vatrD->tex0Coords + sepdC->tex0Coords.offset;
		m->mat.tex0Scl = sepdC->tex0Coords.scale;
		m->mat.tex1Scl = sepdC->tex0Coords.scale;
		m->mat.tex2Scl = sepdC->tex0Coords.scale;

		glBindBuffer(GL_ARRAY_BUFFER, m->VBOs[3]);
		glBufferData(GL_ARRAY_BUFFER, size, tmpPtr, GL_STATIC_DRAW);
		glVertexAttribPointer(3, 2, dtype, GL_FALSE, 0, (void*)0);
		glEnableVertexAttribArray(3);
	}

	/* Get the texture 1 data */
	/* If there is none, use tex0 stuff */
	if(sepdC->flags1 & SEPD_HAS_UV1)
	{
		dtype = (GLenum)sepdC->tex1Coords.datType;
		size = c->vatrC->tex1Coords.size - sepdC->tex1Coords.offset;
		tmpPtr = c->vatrD->tex1Coords + sepdC->tex1Coords.offset;
		m->mat.tex1Scl = sepdC->tex1Coords.scale;
	}

	glBindBuffer(GL_ARRAY_BUFFER, m->VBOs[4]);
	glBufferData(GL_ARRAY_BUFFER, size, tmpPtr, GL_STATIC_DRAW);
	glVertexAttribPointer(4, 2, dtype, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(4);

	/* Get the texture 2 data */
	/* If there is none, use tex0 stuff */
	/* TODO this will use tex1 stuff if there is a tex1 */
	/* not sure if that's fine or not */
	if(sepdC->flags1 & SEPD_HAS_UV2)
	{
		dtype = (GLenum)sepdC->tex2Coords.datType;
		size = c->vatrC->tex2Coords.size - sepdC->tex2Coords.offset;
		tmpPtr = c->vatrD->tex2Coords + sepdC->tex2Coords.offset;
		m->mat.tex2Scl = sepdC->tex2Coords.scale;
	}

	glBindBuffer(GL_ARRAY_BUFFER, m->VBOs[5]);
	glBufferData(GL_ARRAY_BUFFER, size, tmpPtr, GL_STATIC_DRAW);
	glVertexAttribPointer(5, 2, dtype, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(5);

	/* TODO Get the bone index data */
	//if(sepdC.flags1 & SEPD_HAS_INDICES)

	/* TODO Get the bone weight data */
	//if(sepdC.flags1 & SEPD_HAS_WEIGHTS)

	/* Find the total number of indices, allocate inds */
	/* This will almost definitely need to change for bones */
	for(i = m->nInd = 0; i < sepdC->nPRMS; ++i)
		m->nInd += sepdC->prmsC[i].prmC->nVertInd;
	inds = (uint32_t*)malloc(sizeof(uint32_t) * m->nInd);

	/* Convert the indices to uint32_t */
	/* Can probably get away with not doing this, just save type */
	for(i = j = 0; i < sepdC->nPRMS; ++i)
	{
		prmC = sepdC->prmsC[i].prmC;

		if(prmC->datType == unsigned8Bit)
			for(k = 0; k < prmC->nVertInd; ++k, ++j)
				inds[j] = (c->vIndDat + (prmC->firstInd))[k];
		else if(prmC->datType == unsigned16Bit)
			for(k = 0; k < prmC->nVertInd; ++k, ++j)
				inds[j] = ((uint16_t*)(c->vIndDat + (prmC->firstInd << 1)))[k];
		else
			for(k = 0; k < prmC->nVertInd; ++k, ++j)
				inds[j] = ((uint32_t*)(c->vIndDat + (prmC->firstInd << 2)))[k];
	}

	size = sizeof(uint32_t) * m->nInd;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, inds, GL_STATIC_DRAW);
	free(inds);

	switch(mat->tMapUsed)
	{
		case 3:
			m->mat.hasTex2 = 1;
		case 2:
			m->mat.hasTex1 = 1;
		case 1:
			m->mat.hasTex0 = 1;
	}

	/* Load the actual textures */
	for(i = 0; i < mat->tMapUsed; ++i)
	{
		mTex = &(mat->textures[i]);
		tex = &(c->texC->tex[mTex->texIndex]);
		bcolor = (float*)(&mTex->borderColor);
		w = tex->width, h = tex->height;
		tmpPtr = (uint8_t*)malloc(w * h * 4);
		decodeImg(c->texDat + tex->offset, tmpPtr, tex);

		dtype = GL_TEXTURE_2D;
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(dtype, m->TEXs[i]);
		glTexParameteri(dtype, GL_TEXTURE_WRAP_S, mTex->WrapS);
		glTexParameteri(dtype, GL_TEXTURE_WRAP_T, mTex->WrapT);

		/* Trilinear hacks from noclip, not sure if this really makes it look better */
		if(mTex->MinFilter == GL_LINEAR_MIPMAP_NEAREST)
			glTexParameteri(dtype, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		else
			glTexParameteri(dtype, GL_TEXTURE_MIN_FILTER, mTex->MinFilter);

		glTexParameteri(dtype, GL_TEXTURE_MAG_FILTER, mTex->MagFilter);
		glTexParameterfv(dtype, GL_TEXTURE_BORDER_COLOR, bcolor);

		/* Param 2 is mipmap levels, might need later */
		glTexImage2D(dtype, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tmpPtr);
		glGenerateMipmap(dtype);
		glActiveTexture(0);
		free(tmpPtr);
	}

	/* Blending stuff */
	m->mat.blendingOn = mat->blendEnabled;
	if(mat->blendEnabled)
	{
		m->mat.bSrcRGB = mat->blendSrcFactRGB;
		m->mat.bDstRGB = mat->blendDstFactRGB;
		m->mat.bFuncRGB = mat->blendFuncRGB;
		m->mat.bSrcAlp = mat->blendSrcFactAlp;
		m->mat.bDstAlp = mat->blendDstFactAlp;
		m->mat.bFuncAlp = mat->blendFuncAlp;
		memcpy(m->mat.bColor, mat->blendColor, 16);
	}

	/* Read the depth function */
	if(mat->depthTestOn)
		m->mat.depthFunc = mat->depthTestFunc;
	else
		m->mat.depthFunc = GL_LESS;
	m->mat.depth = mat->depthOffset;

	/* Face culling mode */
	switch(mat->faceCullMode)
	{
		case 0:
			m->mat.cullMode = GL_FRONT_AND_BACK;
			break;
		case 1:
			m->mat.cullMode = GL_BACK;
			break;
		case 2:
			m->mat.cullMode = GL_FRONT;
			break;
		case 3:
		default:
			m->mat.cullMode = GL_NONE;
			break;
	}

	/* TODO Get the material data */
	col = mat->ambientCol;
	m->mat.ambiC = glm::vec4(col.R, col.G, col.B, col.A) / 255.0f;
	col = mat->diffuseCol;
	m->mat.diffC = glm::vec4(col.R, col.G, col.B, col.A) / 255.0f;
	col = mat->specCol[0];
	m->mat.spc0C = glm::vec4(col.R, col.G, col.B, col.A) / 255.0f;
	col = mat->emissiveCol;
	m->mat.emisC = glm::vec4(col.R, col.G, col.B, col.A) / 255.0f;
	m->mat.buffC = *(glm::vec4*)(mat->bufColor);
	for(i = 0; i < 6; ++i)
	{
		col = mat->constCol[i];
//printf("mat->constCol[%d]:  %u, %u, %u, %u\n", i, col.R, col.G, col.B, col.A);
		m->mat.constC[i] = glm::vec4(col.R, col.G, col.B, col.A) / glm::vec4(255.0f);
//printf("m->mat->constC[%d]: %3.2f, %3.2f, %3.2f, %3.2f\n", i, m->mat.constC[i][0], m->mat.constC[i][1], m->mat.constC[i][2], m->mat.constC[i][3]);
	}

//printf("Mesh %d\n", meshNum);
//printf("nTCom: %d\n", m->mat.nTCom);
//printf("Const color %d: %3.2f, %3.2f, %3.2f, %3.2f\n", m->mat.tCom[0].constInd, m->mat.constC[m->mat.tCom[0].constInd][0], m->mat.constC[m->mat.tCom[0].constInd][1], m->mat.constC[m->mat.tCom[0].constInd][2], m->mat.constC[m->mat.tCom[0].constInd][3]);
//printf("Const color %d: %3.2f, %3.2f, %3.2f, %3.2f\n", m->mat.tCom[1].constInd, m->mat.constC[m->mat.tCom[1].constInd][0], m->mat.constC[m->mat.tCom[1].constInd][1], m->mat.constC[m->mat.tCom[1].constInd][2], m->mat.constC[m->mat.tCom[1].constInd][3]);

	m->mat.tex0 = 0;
	m->mat.tex1 = 1;
	m->mat.tex2 = 2;
/*
if(meshNum == 15)
{
for(i = 0; i < m->mat.nTCom; ++i)
{
printTexCombiner(m->mat.tCom[i]);
printf("\n");
}
printf("color scale: %8.7f\n", m->mat.colScl);
printf("Tex0Scale: %8.7f\n", m->mat.tex0Scl);
printf("Tex1Scale: %8.7f\n", m->mat.tex1Scl);
printf("Tex2Scale: %8.7f\n", m->mat.tex2Scl);
printf("Const color[0]: %8.7f, %8.7f, %8.7f, %8.7f\n", m->mat.constC[0].x, m->mat.constC[0].y, m->mat.constC[0].z, m->mat.constC[0].w);
printf("Const color[4]: %8.7f, %8.7f, %8.7f, %8.7f\n", m->mat.constC[4].x, m->mat.constC[4].y, m->mat.constC[4].z, m->mat.constC[4].w);
printf("Buffer color: %8.7f, %8.7f, %8.7f, %8.7f\n", m->mat.buffC.x, m->mat.buffC.y, m->mat.buffC.z, m->mat.buffC.w);
printf("has vertex color: %u\n", m->mat.hasFCol);
printf("alpha func (ref): 0x%04x (%8.7f)\n", m->mat.aFunc, m->mat.aRef);
printf("depth func: 0x%04x\n", m->mat.depthFunc);
printf("blending on: %d\n", m->mat.blendingOn);
printf("depth: %8.7f\n", m->mat.depth);
}
*/

	/* Unbind stuff */
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glActiveTexture(0);

	return;
}

int dumb_thing = 0;

void drawCmbMesh(const cmbMesh_t* m, cmbShader_t* s)
{
	int sC, dC, sA, dA;
	float bR, bB, bG, bA;
	int i;
	char str[32];

	s->use();

	/* Vertex shader stuff */
	s->set1f("colScl", m->mat.colScl);
	s->set1f("tex0Scl", m->mat.tex0Scl);
	s->set1f("tex1Scl", m->mat.tex1Scl);
	s->set1f("tex2Scl", m->mat.tex2Scl);
	s->set1i("hasFCol", m->mat.hasFCol);

	/* Fragment shader stuff */
	s->set1i("mat.hasTex0", m->mat.hasTex0);
	s->set1i("mat.hasTex1", m->mat.hasTex1);
	s->set1i("mat.hasTex2", m->mat.hasTex2);
	s->set1i("mat.hasFCol", m->mat.hasFCol);
	s->set1i("mat.aFunc", m->mat.aFunc);
	s->set1f("mat.aRef", m->mat.aRef);
	s->set1f("mat.depth", m->mat.depth);
	s->set1i("mat.tex0", m->mat.tex0);
	s->set1i("mat.tex1", m->mat.tex1);
	s->set1i("mat.tex2", m->mat.tex2);

	/* Unused */
	s->set4f("mat.ambiC", m->mat.ambiC);
	s->set4f("mat.diffC", m->mat.diffC);
	s->set4f("mat.specC", m->mat.spc0C);
	s->set4f("mat.emisC", m->mat.emisC);

	/* Texture environment stuff
	s->set1i("mat.nTCom", m->mat.nTCom);
	s->set4f("mat.buffC", m->mat.buffC);
	s->set4f("mat.constC[0]", m->mat.constC[0]);
	s->set4f("mat.constC[1]", m->mat.constC[1]);
	s->set4f("mat.constC[2]", m->mat.constC[2]);
	s->set4f("mat.constC[3]", m->mat.constC[3]);
	s->set4f("mat.constC[4]", m->mat.constC[4]);
	s->set4f("mat.constC[5]", m->mat.constC[5]);
	for(i = 0; i < m->mat.nTCom; ++i)
	{
		sprintf(str, "mat.tCom[%d].comRGB", i);
		s->set1i(str, m->mat.tCom[i].combineRGB);
		sprintf(str, "mat.tCom[%d].comAlp", i);
		s->set1i(str, m->mat.tCom[i].combineAlpha);
		sprintf(str, "mat.tCom[%d].sclRGB", i);
		s->set1i(str, m->mat.tCom[i].scaleRGB);
		sprintf(str, "mat.tCom[%d].sclAlp", i);
		s->set1i(str, m->mat.tCom[i].scaleAlpha);
		sprintf(str, "mat.tCom[%d].bufRGB", i);
		s->set1i(str, m->mat.tCom[i].bufInpRGB);
		sprintf(str, "mat.tCom[%d].bufAlp", i);
		s->set1i(str, m->mat.tCom[i].bufInpAlpha);
		sprintf(str, "mat.tCom[%d].src0RGB", i);
		s->set1i(str, m->mat.tCom[i].srcRGB[0]);
		sprintf(str, "mat.tCom[%d].src1RGB", i);
		s->set1i(str, m->mat.tCom[i].srcRGB[1]);
		sprintf(str, "mat.tCom[%d].src2RGB", i);
		s->set1i(str, m->mat.tCom[i].srcRGB[2]);
		sprintf(str, "mat.tCom[%d].op0RGB", i);
		s->set1i(str, m->mat.tCom[i].opRGB[0]);
		sprintf(str, "mat.tCom[%d].op1RGB", i);
		s->set1i(str, m->mat.tCom[i].opRGB[1]);
		sprintf(str, "mat.tCom[%d].op2RGB", i);
		s->set1i(str, m->mat.tCom[i].opRGB[2]);
		sprintf(str, "mat.tCom[%d].src0Alp", i);
		s->set1i(str, m->mat.tCom[i].srcAlpha[0]);
		sprintf(str, "mat.tCom[%d].src1Alp", i);
		s->set1i(str, m->mat.tCom[i].srcAlpha[1]);
		sprintf(str, "mat.tCom[%d].src2Alp", i);
		s->set1i(str, m->mat.tCom[i].srcAlpha[2]);
		sprintf(str, "mat.tCom[%d].op0Alp", i);
		s->set1i(str, m->mat.tCom[i].opAlpha[0]);
		sprintf(str, "mat.tCom[%d].op1Alp", i);
		s->set1i(str, m->mat.tCom[i].opAlpha[1]);
		sprintf(str, "mat.tCom[%d].op2Alp", i);
		s->set1i(str, m->mat.tCom[i].opAlpha[2]);
	}
*/
	/* Handle blending */
	if(m->mat.blendingOn)
	{
		sC = m->mat.bSrcRGB, dC = m->mat.bDstRGB;
		sA = m->mat.bSrcAlp, dA = m->mat.bDstAlp;
		bR = m->mat.bColor[0], bB = m->mat.bColor[1];
		bG = m->mat.bColor[2], bA = m->mat.bColor[3];

		glEnable(GL_BLEND);
		glBlendFuncSeparate(sC, dC, sA, dA);
		glBlendEquationSeparate(m->mat.bFuncRGB, m->mat.bFuncAlp);
		glBlendColor(bR, bB, bG, bA);
	}

	/* Update the depth function */
	glDepthFunc(m->mat.depthFunc);

	/* Update the culling mode */
	if(m->mat.cullMode != GL_NONE)
	{
		glEnable(GL_CULL_FACE);
		glCullFace(m->mat.cullMode);
	}

	/* Draw the mesh */
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m->TEXs[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m->TEXs[1]);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m->TEXs[2]);
	glBindVertexArray(m->VAO);
	glDrawElements(GL_TRIANGLES, m->nInd, GL_UNSIGNED_INT, 0);

	/* Clean up */
	glBindVertexArray(0);
	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
}

void delCmbMesh(cmbMesh_t* m)
{
	glDeleteVertexArrays(1, &(m->VAO));
	glDeleteBuffers(8, m->VBOs);
	glDeleteBuffers(1, &(m->EBO));
	glDeleteTextures(3, m->TEXs);
}


// TODO debug garbage

		/* Texture combiner stuff? */
/*
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_LOD_BIAS, mTex->LODBias);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, tc->combineRGB);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, tc->combineAlpha);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, tc->srcRGB[0]);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, tc->srcRGB[0]);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, tc->srcRGB[1]);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, tc->srcRGB[1]);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB, tc->srcRGB[2]);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB, tc->srcRGB[2]);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, tc->srcAlpha[0]);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, tc->srcAlpha[0]);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, tc->srcAlpha[1]);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, tc->srcAlpha[1]);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA, tc->srcAlpha[2]);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA, tc->srcAlpha[2]);
		glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, tc->scaleRGB);
		glTexEnvf(GL_TEXTURE_ENV, GL_ALPHA_SCALE, tc->scaleAlpha);
		glTexEnviv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, constCol);
*/



//	col = mat->specCol[1];
//	m->mat.spc1C = glm::vec4(col.R, col.G, col.B, col.A);
//	col = mat->constant0Col;
//	m->mat.constC[0] = glm::vec4(col.R, col.G, col.B, col.A);
//	col = mat->constant1Col;
//	m->mat.constC[1] = glm::vec4(col.R, col.G, col.B, col.A);
//	col = mat->constant2Col;
//	m->mat.constC[2] = glm::vec4(col.R, col.G, col.B, col.A);
//	col = mat->constant3Col;
//	m->mat.constC[3] = glm::vec4(col.R, col.G, col.B, col.A);
//	col = mat->constant4Col;
//	m->mat.constC[4] = glm::vec4(col.R, col.G, col.B, col.A);
//	col = mat->constant5Col;
//	m->mat.constC[5] = glm::vec4(col.R, col.G, col.B, col.A);


