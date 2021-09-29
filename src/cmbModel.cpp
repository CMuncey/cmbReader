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

int8_t makeCmbModel(cmbModel_t* m, const cmb_t* c)
{
	uint32_t i;

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
	m->nMeshes = c->sklmC->mshsC->nMeshes;
	m->meshes = (cmbMesh_t*)malloc(sizeof(cmbMesh_t) * m->nMeshes);

	/* Make the bone matrices */
	m->bones = makeBones(c);

	for(i = 0; i < m->nMeshes; ++i)
		makeCmbMesh(&(m->meshes[i]), i, c, m);

	return(0);
}

glm::mat4* makeBones(const cmb_t* c)
{
	float sX, sY, sZ, cX, cY, cZ;
	float mat[16];
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

	for(i = 0; i < m->nMeshes; ++i)
		delCmbMesh(&(m->meshes[i]));
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
	unsigned int i, j, k, w, h, size;
	const float* bcolor;
	uint32_t texInfo;
	uint32_t* inds;
	uint8_t* tmpPtr;
	GLenum dtype;
	mesh_t* cmbMesh;
	sepdChunk_t* sepdC;
	prmChunk_t* prmC;
	material_t* mat;
	matTex_t* mTex;
	texture_t* tex;
	rgba_t col;

	cmbMesh = &(c->sklmC->mshsC->meshes[meshNum]);
	sepdC = &(c->sklmC->shpC->sepdC[cmbMesh->sepdInd]);
	mat = &(c->matsC->mats[cmbMesh->matsInd]);

	/* Compile the shader for the mesh */
	/* This has potential for repeats, find a way to do it per material */
	/* Keep them in model, but normal array won't work */
	/* Try double array? */
	m->shader = cmbShader_t(c, cmbMesh->matsInd);

	/* Initialise the SEPD parameters */
	m->sepdP.posScale = m->sepdP.colorScale = m->sepdP.tex0Scale = 1.0f;
	m->sepdP.tex1Scale = m->sepdP.tex2Scale = m->sepdP.boneWScale = 1.0f;
	m->sepdP.boneDim = 0;
	m->sepdP.hasVColor = 0;

	/* Link the various matrices */
	m->rendP.projMat = &(model->projMat);
	m->rendP.viewMat = &(model->viewMat);
	m->rendP.modelMat = &(model->modelMat);
	m->rendP.normMat = &(model->normMat);

	/* Make the various buffers */
	glGenVertexArrays(1, &(m->VAO));
	glGenBuffers(8, m->VBOs);
	glGenBuffers(1, &(m->EBO));
	glGenBuffers(2, m->UBOs);
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
		m->sepdP.posScale = sepdC->normals.scale;

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
		m->sepdP.colorScale = sepdC->vertColors.scale;
		m->sepdP.hasVColor = 1;

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
		m->sepdP.tex0Scale = sepdC->tex0Coords.scale;
		m->sepdP.tex1Scale = sepdC->tex0Coords.scale;
		m->sepdP.tex2Scale = sepdC->tex0Coords.scale;

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
		m->sepdP.tex1Scale = sepdC->tex1Coords.scale;
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
		m->sepdP.tex2Scale = sepdC->tex2Coords.scale;
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

	size = sizeof(uint32_t) * m->nInd;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, inds, GL_STATIC_DRAW);
	free(inds);

	/* Load the actual textures */
	/* TODO Should probably do this in the model instead */
	/* since this can use repeats, just like shaders */
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

	/* Set the blending info stuff */
	m->rendP.blendingOn = mat->blendEnabled;
	if(mat->blendEnabled)
	{
		m->rendP.bSrcRGB = mat->blendSrcFactRGB;
		m->rendP.bDstRGB = mat->blendDstFactRGB;
		m->rendP.bFuncRGB = mat->blendFuncRGB;
		m->rendP.bSrcAlp = mat->blendSrcFactAlp;
		m->rendP.bDstAlp = mat->blendDstFactAlp;
		m->rendP.bFuncAlp = mat->blendFuncAlp;
		memcpy(m->rendP.bColor, mat->blendColor, 16);
	}

	/* Set the depth function, default to GL_LESS */
	if(mat->depthTestOn)
		m->rendP.depthFunc = mat->depthTestFunc;
	else
		m->rendP.depthFunc = GL_LESS;

	/* Set the face culling stuff */
	switch(mat->faceCullMode)
	{
		case 0:
			m->rendP.cullMode = GL_FRONT_AND_BACK;
			break;
		case 1:
			m->rendP.cullMode = GL_BACK;
			break;
		case 2:
			m->rendP.cullMode = GL_FRONT;
			break;
		case 3:
		default:
			m->rendP.cullMode = GL_NONE;
			break;
	}

	/* Set the material parameter stuff */
	for(i = 0; i < 6; ++i)
	{
		col = mat->constCol[i];
		m->matP.constCol[i] = glm::vec4(col.R, col.G, col.B, col.A) / 255.0f;
	}
	for(i = 0; i < 3; ++i)
	{
		/* I think this is used for the scrolling textures? */
		texInfo = ((uint32_t)(mat->tCoords[i].mapMethod) << 16);
		texInfo |= mat->tCoords[i].CoordIndex;
		m->matP.texInfo[i] = texInfo;
	}
	m->matP.depth = mat->depthOffset;

//if(meshNum == 15)
//{
//printf("Tex0Scale: %8.7f\n", m->sepdP.tex0Scale);
//printf("Tex1Scale: %8.7f\n", m->sepdP.tex1Scale);
//printf("Tex2Scale: %8.7f\n", m->sepdP.tex2Scale);
//printf("Const color[0]: %8.7f, %8.7f, %8.7f, %8.7f\n", m->matP.constCol[0].x, m->matP.constCol[0].y, m->matP.constCol[0].z, m->matP.constCol[0].w);
//printf("Const color[4]: %8.7f, %8.7f, %8.7f, %8.7f\n", m->matP.constCol[4].x, m->matP.constCol[4].y, m->matP.constCol[4].z, m->matP.constCol[4].w);
//printf("color scale: %8.7f\n", m->sepdP.colorScale);
//printf("has vertex color: %u\n", m->sepdP.hasVColor);
//printf("depth func: 0x%04x\n", m->rendP.depthFunc);
//printf("blending on: %d\n", m->rendP.blendingOn);
//printf("depth: %8.7f\n", m->matP.depth);
//}

	/* Unbind stuff */
	glBindVertexArray(0);
	glActiveTexture(0);

	return;
}

void drawCmbMesh(cmbMesh_t* m)
{
	int sC, dC, sA, dA;
	float bR, bB, bG, bA;
	GLenum dtype;

	m->shader.use();
	glBindVertexArray(m->VAO);

	/* Update transform matrices */
	m->shader.set4mf("proj", *(m->rendP.projMat));
	m->shader.set4mf("view", *(m->rendP.viewMat));
	m->shader.set4mf("model", *(m->rendP.modelMat));
	m->shader.set3mf("norm", *(m->rendP.normMat));

	/* Update uniform buffers */
	/* 340 bytes per mesh per frame */
	/* Might be able to do this in model, many meshes share sepd/mats */
	/* that might mess up blending though, needing to draw transparent */
	/* things before solid things */
	dtype = GL_UNIFORM_BUFFER;
	glBindBufferBase(dtype, 0, m->UBOs[0]);
	glBufferData(dtype, sizeof(sepdParams_t), &m->sepdP, GL_DYNAMIC_DRAW);
	glBindBufferBase(dtype, 1, m->UBOs[1]);
	glBufferData(dtype, sizeof(matParams_t), &m->matP, GL_DYNAMIC_DRAW);
	glBindBuffer(dtype, 0);

	/* Handle blending */
	if(m->rendP.blendingOn)
	{
		sC = m->rendP.bSrcRGB, dC = m->rendP.bDstRGB;
		sA = m->rendP.bSrcAlp, dA = m->rendP.bDstAlp;
		bR = m->rendP.bColor[0], bB = m->rendP.bColor[1];
		bG = m->rendP.bColor[2], bA = m->rendP.bColor[3];

		glEnable(GL_BLEND);
		glBlendFuncSeparate(sC, dC, sA, dA);
		glBlendEquationSeparate(m->rendP.bFuncRGB, m->rendP.bFuncAlp);
		glBlendColor(bR, bB, bG, bA);
	}

	/* Update the depth function */
	glDepthFunc(m->rendP.depthFunc);

	/* Update the culling mode */
	if(m->rendP.cullMode != GL_NONE)
	{
		glEnable(GL_CULL_FACE);
		glCullFace(m->rendP.cullMode);
	}

	/* Load the textures */
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m->TEXs[0]);
	m->shader.set1i("tex0", 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m->TEXs[1]);
	m->shader.set1i("tex1", 1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, m->TEXs[2]);
	m->shader.set1i("tex2", 2);

	/* Draw the mesh */
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
	glDeleteBuffers(2, m->UBOs);
	glDeleteTextures(3, m->TEXs);
	m->shader.del();
}
