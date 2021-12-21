#include <string>
#include <stdint.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "cmbConstants.h"
#include "cmbTextures.h"
#include "sklmChunk.h"
#include "cmbModel.hpp"
#include "cmbShader.hpp"
#include "cmb.h"
using namespace std;

/* Lots of room for improvement because many meshes re-use */
/* materials and sepd chunks, so the data from each of those */
/* can be read once and re-used in each mesh */

/* Also need to speed up cmb reading by only reading the useful stuff */
/* That will need to be afterwards though so I know what's useful */

/* TODO */
/* Read all textures in model */
/* Render params still need to be per mesh? */
/* Actually I think they're per material, so it can probably go with the textures/mats */

/*
	Model:
		All Shaders
		All SEPDs:
			position data
			normal data
			color data
			tex0, 1, 2 data
			bone index data
			bone scale data
			PRMS
				??? bone stuff
		All Textures

	Mesh:
		Ref to shader
		Ref to SEPD + params
		Ref to MAT + params
		Render parameters
*/

int8_t makeCmbModel(cmbModel_t* m, const cmb_t* c)
{
	uint32_t i;

	/* Make sure the model has somewhere to go */
	if(m == NULL)
	{
		fprintf(stderr, "Error: Loading unallocated model\n");
		return(1);
	}

	/* Kinda pointless, no check to make sure it's actually valid */
	if(c == NULL)
	{
		fprintf(stderr, "Error: Loading model with unallocated CMB\n");
		return(1);
	}

	/* Get the number of meshes */
	m->meshNum = -1;
	m->nMeshes = c->sklmC->mshsC->nMeshes;
	m->meshes = (cmbMesh_t*)malloc(sizeof(cmbMesh_t) * m->nMeshes);

	/* Make a shader for every material in the cmb */
	m->nShaders = c->matsC->nMats;
	m->shaders = (cmbShader_t*)malloc(sizeof(cmbShader_t) * m->nShaders);
	for(i = 0; i < m->nShaders; ++i)
		m->shaders[i] = cmbShader_t(c, i);

	/* Copy over all of the SEPD stuff from cmb */
	m->nSEPDs = c->sklmC->shpC->nSEPDs;
	m->sepds = (modelSEPD_t*)malloc(sizeof(modelSEPD_t) * m->nSEPDs);
	for(i = 0; i < m->nSEPDs; ++i)
		readSEPD(&(m->sepds[i]), i, c);

	/* Copy all of the textures into GPU memory */
	/* Shouldn't cause issues since textures are pretty small */
	m->nTex = c->texC->nTex;
	m->textures = (uint8_t**)malloc(sizeof(uint8_t*) * m->nTex);
	for(i = 0; i < m->nTex; ++i)
		m->textures[i] = makeTexture(i, c);

	/* Make render/material parameters for all of the mats */
	m->nMats = c->matsC->nMats;
	m->mats = (modelMAT_t*)malloc(sizeof(modelMAT_t) * m->nMats);
	for(i = 0; i < m->nMats; ++i)
		makeMat(&(m->mats[i]), i, c, m);

	/* Make the bone matrices (unused atm) */
	//m->bones = makeBones(c);

	for(i = 0; i < m->nMeshes; ++i)
		makeCmbMesh(&(m->meshes[i]), i, c, m);

	return(0);
}

uint8_t* makeTexture(int ind, const cmb_t* c)
{
	uint8_t* t;
	texture_t* tex;
	uint32_t w, h;
	GLenum dtype;

	/* Read the texture into normal memory */
	/* We'll copy it into GPU memory when making materials */
	tex = &(c->texC->tex[ind]);
	w = tex->width, h = tex->height;
	t = (uint8_t*)malloc(w * h * 4);
	decodeImg(c->texDat + tex->offset, t, tex);
	return(t);
}

void makeMat(modelMAT_t* mm, int ind, const cmb_t* c, const cmbModel_t* m)
{
	uint32_t i, w, h;
	uint32_t texInfo;
	material_t* mat;
	matTex_t* mTex;
	texture_t* tex;
	GLenum dtype;
	rgba_t col;
	const float* bcolor;

	/* Get the material from the cmb */
	mat = &(c->matsC->mats[ind]);

	/* Create space for the textures on the GPU */
	glGenTextures(3, mm->TEXs);

	/* Load every texture in the material */
	for(i = 0; i < mat->tMapUsed; ++i)
	{
		mTex = &(mat->textures[i]);
		bcolor = (float*)(&mTex->borderColor);
		tex = &(c->texC->tex[mTex->texIndex]);
		w = tex->width, h = tex->height;

		dtype = GL_TEXTURE_2D;
		glBindTexture(dtype, mm->TEXs[i]);
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
		glTexImage2D(dtype, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, m->textures[mTex->texIndex]);
		glGenerateMipmap(dtype);
	}

	/* Set the blending info stuff */
	mm->rP.blendingOn = mat->blendEnabled;
	if(mat->blendEnabled)
	{
		mm->rP.bSrcRGB = mat->blendSrcFactRGB;
		mm->rP.bDstRGB = mat->blendDstFactRGB;
		mm->rP.bFuncRGB = mat->blendFuncRGB;
		mm->rP.bSrcAlp = mat->blendSrcFactAlp;
		mm->rP.bDstAlp = mat->blendDstFactAlp;
		mm->rP.bFuncAlp = mat->blendFuncAlp;
		memcpy(mm->rP.bColor, mat->blendColor, 16);
	}

	/* Set the depth function, default to GL_LESS */
	if(mat->depthTestOn)
		mm->rP.depthFunc = mat->depthTestFunc;
	else
		mm->rP.depthFunc = GL_LESS;

	/* Set the face culling stuff */
	switch(mat->faceCullMode)
	{
		case 0:
			mm->rP.cullMode = GL_FRONT_AND_BACK;
			break;
		case 1:
			mm->rP.cullMode = GL_BACK;
			break;
		case 2:
			mm->rP.cullMode = GL_FRONT;
			break;
		case 3:
		default:
			mm->rP.cullMode = GL_NONE;
			break;
	}

	/* Set the material parameter stuff */
	for(i = 0; i < 6; ++i)
	{
		col = mat->constCol[i];
		mm->mP.constCol[i] = glm::vec4(col.R, col.G, col.B, col.A) / 255.0f;
	}
	for(i = 0; i < 3; ++i)
	{
		/* I think this is used for the scrolling textures? */
		texInfo = ((uint32_t)(mat->tCoords[i].mapMethod) << 16);
		texInfo |= mat->tCoords[i].CoordIndex;
		mm->mP.texInfo[i] = texInfo;
	}
	mm->mP.depth = mat->depthOffset;
}

void readSEPD(modelSEPD_t* s, int ind, const cmb_t* c)
{
	uint32_t i, j, k, size;
	sepdChunk_t* sepdC;
	prmChunk_t* prmC;
	uint8_t* tmpPtr;
	uint32_t* inds;
	GLenum dtype;
	
	sepdC = &(c->sklmC->shpC->sepdC[ind]);

	/* Set up default values for the sepd parameters */
	s->prms.posScale = s->prms.colorScale = s->prms.tex0Scale = 1.0f;
	s->prms.tex1Scale = s->prms.tex2Scale = s->prms.boneWScale = 1.0f;
	s->prms.boneDim = s->prms.hasVColor = 0;

	/* Make the various buffers */
	glGenVertexArrays(1, &(s->VAO));
	glGenBuffers(8, s->VBOs);
	glGenBuffers(1, &(s->EBO));
	glGenBuffers(2, s->UBOs);

	/* Bind the meshes VAO to load stuff to it */
	glBindVertexArray(s->VAO);

	/* Get the position data, this should always be here */
	if(sepdC->flags1 & SEPD_HAS_POSITIONS)
	{
		/* Position data type should always be float */
		size = c->vatrC->positions.size - sepdC->positions.offset;
		tmpPtr = c->vatrD->positions + sepdC->positions.offset;

		glBindBuffer(GL_ARRAY_BUFFER, s->VBOs[0]);
		glBufferData(GL_ARRAY_BUFFER, size, tmpPtr, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glEnableVertexAttribArray(0);
	}

	/* Get the normal data if there is any */
	if(sepdC->flags1 & SEPD_HAS_NORMALS)
	{
		dtype = (GLenum)sepdC->normals.datType;
		s->prms.posScale = sepdC->normals.scale;

		/* Mode 0 = read array */
		/* Mode 1 = constant value */
		if(sepdC->normals.mode == 0)
		{
			size = c->vatrC->normals.size - sepdC->normals.offset;
			tmpPtr = c->vatrD->normals + sepdC->normals.offset;
		}
		else
		{
			/* Somehow the indices work for this, I guess OpenGL */
			/* just clamps the index if it's out of bounds? */
			/* Just 3 normals, but possibly way more indices */
			size = getDTSize((picaDataType)dtype) * 3;
			tmpPtr = (uint8_t*)(sepdC->normals.constant);
		}

		glBindBuffer(GL_ARRAY_BUFFER, s->VBOs[1]);
		glBufferData(GL_ARRAY_BUFFER, size, tmpPtr, GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, dtype, GL_TRUE, 0, (void*)0);
		glEnableVertexAttribArray(1);
	}

	/* Get the color data */
	if(sepdC->flags1 & SEPD_HAS_COLORS)
	{
		dtype = (GLenum)sepdC->vertColors.datType;
		s->prms.colorScale = sepdC->vertColors.scale;
		s->prms.hasVColor = 1;

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

		glBindBuffer(GL_ARRAY_BUFFER, s->VBOs[2]);
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
		s->prms.tex0Scale = sepdC->tex0Coords.scale;
		s->prms.tex1Scale = sepdC->tex0Coords.scale;
		s->prms.tex2Scale = sepdC->tex0Coords.scale;

		glBindBuffer(GL_ARRAY_BUFFER, s->VBOs[3]);
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
		s->prms.tex1Scale = sepdC->tex1Coords.scale;
	}

	glBindBuffer(GL_ARRAY_BUFFER, s->VBOs[4]);
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
		s->prms.tex2Scale = sepdC->tex2Coords.scale;
	}

	glBindBuffer(GL_ARRAY_BUFFER, s->VBOs[5]);
	glBufferData(GL_ARRAY_BUFFER, size, tmpPtr, GL_STATIC_DRAW);
	glVertexAttribPointer(5, 2, dtype, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(5);

	/* TODO Get the bone index data */
	//if(sepdC->flags1 & SEPD_HAS_INDICES)

	/* TODO Get the bone weight data */
	//if(sepdC->flags1 & SEPD_HAS_WEIGHTS)

	/* Find the total number of indices, allocate inds */
	/* This will almost definitely need to change for bones */
	for(i = s->nInd = 0; i < sepdC->nPRMS; ++i)
		s->nInd += sepdC->prmsC[i].prmC->nVertInd;
	inds = (uint32_t*)malloc(sizeof(uint32_t) * s->nInd);

	/* Convert the indices to uint32_t */
	/* Can probably get away with not doing this, just save type */
	/* and give that to drawElements() later */
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

	size = sizeof(uint32_t) * s->nInd;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, inds, GL_STATIC_DRAW);
	free(inds);

	/* Unbind stuff */
	glBindVertexArray(0);
}

glm::mat4* makeBones(const cmb_t* c)
{
	float sX, sY, sZ, cX, cY, cZ;
	float mtx[16];
	int i, nBones;
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

		/* There may be a glm function(s) to do this? */
		/* Row 1 */
		mtx[0] = scale.x * (cY * cZ);
		mtx[1] = scale.x * (cY * sZ);
		mtx[2] = scale.x * (-sY);
		mtx[3] = 0.0f;

		/* Row 2 */
		mtx[4] = scale.y * ((sX * sY * cZ) - (cX * sZ));
		mtx[5] = scale.y * ((sX * sZ * sY) + (cX * cZ));
		mtx[6] = scale.y * (sX * cY);
		mtx[7] = 0.0f;

		/* Row 3 */
		mtx[8]  = scale.z * ((cX * sY * cZ) + (sX * sZ));
		mtx[9]  = scale.z * ((cX * sY * sZ) - (sX * cZ));
		mtx[10] = scale.z * (cX * cY);
		mtx[11] = 0.0f;

		/* Row 4 */
		mtx[12] = bone->trans[0];
		mtx[13] = bone->trans[1];
		mtx[14] = bone->trans[2];
		mtx[15] = 1.0f;

		bones[i] = glm::make_mat4(mtx);

		/* Multiply parent by child, in that order */
		/* Matrix multiplication isn't commutative */
		if(bone->pID != -1)
		{
			/* Sanity check, I don't think this ever happens */
			if(bone->pID > bone->id)
				printf("Bone %d came before parent %d\n", bone->id, bone->pID);
			bones[i] = bones[bone->pID] * bones[i];
		}
	}

	return(bones);
}

void drawCmbModel(const cmbModel_t* m)
{
	uint32_t i;

	/* Debug to print an individual mesh */
	if(m->meshNum > -1)
		drawCmbMesh(&(m->meshes[m->meshNum]));
	else
		for(i = 0; i < m->nMeshes; ++i)
			drawCmbMesh(&(m->meshes[i]));
}

void delCmbModel(cmbModel_t* m)
{
	unsigned int i;

	/* Shouldn't need to check these, they should exist */
	for(i = 0; i < m->nTex; ++i)
		free(m->textures[i]);
	free(m->textures);
	free(m->shaders);
	free(m->meshes);
	free(m->bones);
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
		default:
			return(0);
	}
}

/* I probably need to redo this to account for bones, not sure */
void makeCmbMesh(cmbMesh_t* m, int meshNum, const cmb_t* c, cmbModel_t* model)
{
	mesh_t* cmbMesh;

	cmbMesh = &(c->sklmC->mshsC->meshes[meshNum]);

	/* Link the shader */
	m->shader = &(model->shaders[cmbMesh->matsInd]);

	/* Link the various matrices */
	m->projMat = &(model->projMat);
	m->viewMat = &(model->viewMat);
	m->modelMat = &(model->modelMat);
	m->normMat = &(model->normMat);

	/* Link the SEPD for the mesh */
	m->sepd = &(model->sepds[cmbMesh->sepdInd]);

	/* Link the material for the mesh */
	m->mat = &(model->mats[cmbMesh->matsInd]);

	return;
}

void drawCmbMesh(cmbMesh_t* m)
{
	int sC, dC, sA, dA;
	float bR, bB, bG, bA;
	GLenum dtype;

	m->shader->use();
	glBindVertexArray(m->sepd->VAO);

	/* Update transform matrices */
	m->shader->set4mf("proj", *(m->projMat));
	m->shader->set4mf("view", *(m->viewMat));
	m->shader->set4mf("model", *(m->modelMat));
	m->shader->set3mf("norm", *(m->normMat));

	/* Update uniform buffers */
	/* 340 bytes per mesh per frame */
	/* Might be able to do this in model, many meshes share sepd/mats */
	/* that might mess up blending though, needing to draw transparent */
	/* things before solid things (not that I do that yet) */
	dtype = GL_UNIFORM_BUFFER;
	glBindBufferBase(dtype, 0, m->sepd->UBOs[0]);
	glBufferData(dtype, sizeof(sepdParams_t), &m->sepd->prms, GL_DYNAMIC_DRAW);
	glBindBufferBase(dtype, 1, m->sepd->UBOs[1]);
	glBufferData(dtype, sizeof(matParams_t), &m->mat->mP, GL_DYNAMIC_DRAW);
	glBindBuffer(dtype, 0);

	/* Handle blending */
	if(m->mat->rP.blendingOn)
	{
		sC = m->mat->rP.bSrcRGB, dC = m->mat->rP.bDstRGB;
		sA = m->mat->rP.bSrcAlp, dA = m->mat->rP.bDstAlp;
		bR = m->mat->rP.bColor[0], bB = m->mat->rP.bColor[1];
		bG = m->mat->rP.bColor[2], bA = m->mat->rP.bColor[3];

		glEnable(GL_BLEND);
		glBlendFuncSeparate(sC, dC, sA, dA);
		glBlendEquationSeparate(m->mat->rP.bFuncRGB, m->mat->rP.bFuncAlp);
		glBlendColor(bR, bB, bG, bA);
	}

	/* Update the depth function */
	glDepthFunc(m->mat->rP.depthFunc);

	/* Update the culling mode */
	if(m->mat->rP.cullMode != GL_NONE)
	{
		glEnable(GL_CULL_FACE);
		glCullFace(m->mat->rP.cullMode);
	}

	/* Load the textures */
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m->mat->TEXs[0]);
	m->shader->set1i("tex0", 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m->mat->TEXs[1]);
	m->shader->set1i("tex1", 1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m->mat->TEXs[2]);
	m->shader->set1i("tex2", 2);

	/* Draw the mesh */
	glDrawElements(GL_TRIANGLES, m->sepd->nInd, GL_UNSIGNED_INT, 0);

	/* Clean up */
	glBindVertexArray(0);
	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
}
