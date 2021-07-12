#include <vector>
#include <string>
#include <random>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "shader.h"
#include "mesh.h"
#include "cmb.h"
#include "sklmChunk.h"
#include "cmbConstants.h"
#include "cmbTextures.h"
using namespace std;

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

mMesh_t::mMesh_t(const cmb_t* c, int meshNum, vector<glm::mat4> bones)
{
	unsigned int i, j, k, w, h, size;
	const float* bcolor;
	uint32_t* inds;
	uint8_t* tmpPtr;
	GLenum dtype;
	mesh_t* cmbMesh;
	sepdChunk_t* sepdC;
	prmsChunk_t* prmsC;
	prmChunk_t* prmC;
	material_t* mat;
	matTex_t* mTex;
	texture_t* tex;
	rgba_t col;

	cmbMesh = &(c->sklmC->mshsC->meshes[meshNum]);
	sepdC = &(c->sklmC->shpC->sepdC[cmbMesh->sepdInd]);
	mat = &(c->matsC->mats[cmbMesh->matsInd]);

	hasFCol = hasTex0 = hasTex1 = hasTex2 = 0;

	/* Make the various buffers */
	glGenVertexArrays(1, &VAO);
	glGenBuffers(8, VBOs);
	glGenBuffers(1, &EBO);
	glGenTextures(3, TEXs);

	/* Bind the meshes VAO to load stuff to it */
	glBindVertexArray(VAO);

	/* Get the position data, this should always be here */
	if(sepdC->flags1 & SEPD_HAS_POSITIONS)
	{
		/* Position data type should always be float */
		size = c->vatrC->positions.size - sepdC->positions.offset;
		tmpPtr = c->vatrD->positions + sepdC->positions.offset;

		glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
		glBufferData(GL_ARRAY_BUFFER, size, tmpPtr, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glEnableVertexAttribArray(0);
	}

	/* Get the normal data if there is any */
	if(sepdC->flags1 & SEPD_HAS_NORMALS)
	{
		dtype = (GLenum)sepdC->normals.datType;

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

		glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
		glBufferData(GL_ARRAY_BUFFER, size, tmpPtr, GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, dtype, GL_TRUE, 0, (void*)0);
		glEnableVertexAttribArray(1);
	}

	/* TODO Get the color data */
	if(sepdC->flags1 & SEPD_HAS_COLORS)
	{
		dtype = (GLenum)sepdC->vertColors.datType;
		hasFCol = 1;

		/* Normal mode 0 = read array */
		/* Mode 1 = constant value */
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

		glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
		glBufferData(GL_ARRAY_BUFFER, size, tmpPtr, GL_STATIC_DRAW);
		glVertexAttribPointer(2, 4, dtype, GL_TRUE, 0, (void*)0);
		glEnableVertexAttribArray(2);	
	}

	/* Get texture 0 coordinate data if there is any */
	if(sepdC->flags1 & SEPD_HAS_UV0)
	{
		dtype = (GLenum)sepdC->tex0Coords.datType;
		size = c->vatrC->tex0Coords.size - sepdC->tex0Coords.offset;
		tmpPtr = c->vatrD->tex0Coords + sepdC->tex0Coords.offset;
		hasTex0 = 1;

		glBindBuffer(GL_ARRAY_BUFFER, VBOs[3]);
		glBufferData(GL_ARRAY_BUFFER, size, tmpPtr, GL_STATIC_DRAW);
		glVertexAttribPointer(3, 2, dtype, GL_TRUE, 0, (void*)0);
		glEnableVertexAttribArray(3);
	}

	/* TODO Get the texture 1 data */
	//if(sepdC.flags1 & SEPD_HAS_UV1)

	/* TODO Get the texture 2 data */
	//if(sepdC.flags1 & SEPD_HAS_UV2)

	/* TODO Get the bone index data */
	//if(sepdC.flags1 & SEPD_HAS_INDICES)

	/* TODO Get the bone weight data */
	//if(sepdC.flags1 & SEPD_HAS_WEIGHTS)

	/* Find the total number of indices, allocate inds */
	for(i = numInd = 0; i < sepdC->nPRMS; ++i)
		numInd += sepdC->prmsC[i].prmC->nVertInd;
	inds = (uint32_t*)malloc(sizeof(uint32_t) * numInd);

	/* Convert the indices to uint32_t */
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

	size = sizeof(uint32_t) * numInd;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, inds, GL_STATIC_DRAW);
	free(inds);

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
		glBindTexture(dtype, TEXs[i]);
		glTexParameteri(dtype, GL_TEXTURE_WRAP_S, mTex->WrapS);
		glTexParameteri(dtype, GL_TEXTURE_WRAP_T, mTex->WrapT);
		glTexParameteri(dtype, GL_TEXTURE_MIN_FILTER, mTex->MinFilter);
		glTexParameteri(dtype, GL_TEXTURE_MAG_FILTER, mTex->MagFilter);
		glTexParameterfv(dtype, GL_TEXTURE_BORDER_COLOR, bcolor);

		/* Param 2 is mipmap levels, might need later */
		glTexImage2D(dtype, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tmpPtr);
		glGenerateMipmap(dtype);
		free(tmpPtr);
	}

	/* TODO Get the material data */
	col = mat->ambientCol;
	material.ambiC = glm::vec3(col.R, col.B, col.G);
	col = mat->diffuseCol;
	material.diffC = glm::vec3(col.R, col.B, col.G);
	col = mat->specular0Col;
	material.specC = glm::vec3(col.R, col.B, col.G);
	col = mat->emissiveCol;
	material.emisC = glm::vec3(col.R, col.B, col.G);
	material.tex0 = 0;
	material.tex1 = 1;
	material.tex2 = 2;

	/* Unbind stuff */
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	return;
}

void mMesh_t::draw(mShader_t& s)
{
	s.use();
	s.set1i("hasFCol", hasFCol);
	s.set1i("hasTex0", hasTex0);
	s.set1i("hasTex1", hasTex1);
	s.set1i("hasTex2", hasTex2);
	s.set1i("material.tex0", material.tex0);
	s.set1i("material.tex1", material.tex1);
	s.set1i("material.tex2", material.tex2);
	s.set3f("material.ambiC", material.ambiC);
	s.set3f("material.diffC", material.diffC);
	s.set3f("material.specC", material.specC);
	s.set3f("material.emisC", material.emisC);

	/* Draw the mesh */
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TEXs[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, TEXs[1]);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, TEXs[2]);
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, numInd, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
}

void mMesh_t::del()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(8, VBOs);
	glDeleteBuffers(1, &EBO);
	glDeleteTextures(3, TEXs);
}
