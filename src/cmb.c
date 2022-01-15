#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "cmb.h"
#include "cmbChunk.h"
#include "sklChunk.h"
#include "matsChunk.h"
#include "texChunk.h"
#include "sklmChunk.h"
#include "lutsChunk.h"
#include "vatrChunk.h"

int8_t readCmb(cmb_t* c, const char* fn)
{
	FILE* f;
	int size;

	f = fopen(fn, "rb");
	if(f == NULL)
	{
		perror(fn);
		return(1);
	}

	/* Init everything to null for easy deletion later */
	/* if something goes wrong when reading it */
	c->data = NULL;
	c->cmbC = NULL;
	c->sklC = NULL;
	c->matsC = NULL;
	c->texC = NULL;
	c->sklmC = NULL;
	c->lutsC = NULL;
	c->vatrC = NULL;
	c->vatrD = NULL;
	c->vIndDat = NULL;
	c->texDat = NULL;

	/* Read the entire file into memory because it's small */
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	fseek(f, 0, SEEK_SET);
	c->data = malloc(size);
	fread(c->data, 1, size, f);
	fclose(f);

	/* Extract all of the chunks */
	if(readCmbChunk(c) != 0)
		return(2);
	if(readSklChunk(c) != 0)
		return(3);
	if(readMatsChunk(c) != 0)
		return(4);
	if(readTexChunk(c) != 0)
		return(5);
	if(readSklmChunk(c) != 0)
		return(6);
	if(readLutsChunk(c) != 0)
		return(7);
	if(readVatrChunk(c) != 0)
		return(8);

	/* Extract vertex index data block */
	size = c->cmbC->nIndices * sizeof(uint16_t);
	c->vIndDat = malloc(size);
	memcpy(c->vIndDat, c->data + c->cmbC->vIndPtr, size);

	/* Extract the texture data block */
	size = c->cmbC->size - c->cmbC->tDatPtr;
	c->texDat = malloc(size);
	memcpy(c->texDat, c->data + c->cmbC->tDatPtr, size);

	/* Don't need the data anymore, it's in the other stuff */
	/* Set it to null for easy deletion */
	free(c->data);
	c->data = NULL;

	return(0);
}

int8_t readCmbChunk(cmb_t* c)
{
	int offs;
	uint8_t* data;
	cmbChunk_t* cmbC;

	/* Allocate space */
	c->cmbC = malloc(sizeof(cmbChunk_t));
	cmbC = c->cmbC;
	data = c->data;
	offs = 0x00;

	/* Copy and check the magic number */
	/* Could just memcpy the whole thing, but that will make MM much harder */
	/* Plus I can add stuff to the structs this way and it doesn't break */
	memcpy(cmbC->magic, data + offs, sizeof(cmbChunk_t)), offs += 0x04;
	if(*(int*)cmbC->magic != *(int*)("cmb "))
	{
		fprintf(stderr, "Error: CMB chunk magic number does not match\n");
		fprintf(stderr, "\"cmb \" vs \"%.4s\"\n", cmbC->magic);
		return(1);
	}

	/* Read the rest of the data if the magic was good */
	cmbC->size = *(uint32_t*)(data + offs), offs += 0x04;
	cmbC->version = *(uint32_t*)(data + offs), offs += 0x04;
	cmbC->unused = *(uint32_t*)(data + offs), offs += 0x04;
	memcpy(cmbC->name, data + offs, 16), offs += 0x10;
	cmbC->nIndices = *(uint32_t*)(data + offs), offs += 0x04;
	cmbC->sklCPtr = *(uint32_t*)(data + offs), offs += 0x04;
	cmbC->matsCPtr = *(uint32_t*)(data + offs), offs += 0x04;
	cmbC->texCPtr = *(uint32_t*)(data + offs), offs += 0x04;
	cmbC->sklmCPtr = *(uint32_t*)(data + offs), offs += 0x04;
	cmbC->lutsCPtr = *(uint32_t*)(data + offs), offs += 0x04;
	cmbC->vatrCPtr = *(uint32_t*)(data + offs), offs += 0x04;
	cmbC->vIndPtr = *(uint32_t*)(data + offs), offs += 0x04;
	cmbC->tDatPtr = *(uint32_t*)(data + offs), offs += 0x04;

	return(0);
}

int8_t readSklChunk(cmb_t* c)
{
	int offs, i;
	uint8_t* data;
	sklChunk_t* sklC;
	bone_t* bone;

	/* Set everything up */
	c->sklC = malloc(sizeof(sklChunk_t));
	offs = c->cmbC->sklCPtr;
	sklC = c->sklC;
	data = c->data;
	sklC->bones = NULL;

	/* Copy and check the magic number */
	memcpy(sklC->magic, data + offs, 4), offs += 0x04;
	if(*(int*)sklC->magic != *(int*)("skl "))
	{
		fprintf(stderr, "Error: SKL chunk magic number does not match\n");
		fprintf(stderr, "\"skl \" vs \"%.4s\"\n", sklC->magic);
		return(1);
	}

	sklC->size = *(uint32_t*)(data + offs), offs += 0x04;
	sklC->nBones = *(uint32_t*)(data + offs), offs += 0x04;
	sklC->unk1 = *(uint32_t*)(data + offs), offs += 0x04;
	sklC->bones = malloc(sizeof(bone_t) * sklC->nBones);

	/* Read all of the bones */
	for(i = 0; i < sklC->nBones; ++i)
	{
		bone = &(sklC->bones[i]);
		bone->id = *(uint8_t*)(data + offs), offs += 0x01;
		bone->unk1 = *(uint8_t*)(data + offs), offs += 0x01;
		bone->pID = *(uint16_t*)(data + offs), offs += 0x02;
		memcpy(bone->scale, data + offs, 12), offs += 0x0C;
		memcpy(bone->rot, data + offs, 12), offs += 0x0C;
		memcpy(bone->trans, data + offs, 12), offs += 0x0C;
	}

	return(0);
}

int8_t readMatsChunk(cmb_t* c)
{
	int offs, i, j;
	uint8_t* data;
	matsChunk_t* matsC;
	material_t* mat;
	texCoords_t* tcor;
	matTex_t* mtex;
	texCombiner_t* tcom;

	c->matsC = malloc(sizeof(matsChunk_t));
	offs = c->cmbC->matsCPtr;
	matsC = c->matsC;
	data = c->data;
	matsC->mats = NULL;
	matsC->tCom = NULL;

	/* Read the MATS Chunk */
	memcpy(matsC->magic, data + offs, 4), offs += 0x04;
	if(*(int*)matsC->magic != *(int*)("mats"))
	{
		fprintf(stderr, "Error: MATS chunk magic number does not match\n");
		fprintf(stderr, "\"mats\" vs \"%.4s\"\n", matsC->magic);
		return(1);
	}
	matsC->size = *(uint32_t*)(data + offs), offs += 0x04;
	matsC->nMats = *(uint32_t*)(data + offs), offs += 0x04;
	matsC->mats = malloc(sizeof(material_t) * matsC->nMats);

	/* Read all of the materials */
	for(i = matsC->nTCom = 0; i < matsC->nMats; ++i)
	{
		mat = &(matsC->mats[i]);
		mat->fragLightOn = *(uint8_t*)(data + offs), offs += 0x01;
		mat->vertLightOn = *(uint8_t*)(data + offs), offs += 0x01;
		mat->hemiLightOn = *(uint8_t*)(data + offs), offs += 0x01;
		mat->faceCullOn = *(uint8_t*)(data + offs), offs += 0x01;
		mat->faceCullMode = *(uint8_t*)(data + offs), offs += 0x01;
		mat->polyOffsOn = *(uint8_t*)(data + offs), offs += 0x01;
		mat->unk1 = *(uint8_t*)(data + offs), offs += 0x01;
		mat->polyOffset = *(int8_t*)(data + offs), offs += 0x01;
		if(mat->polyOffsOn)
			mat->depthOffset = (float)(mat->polyOffset) / 0x10000;
		else
			mat->depthOffset = 0.0f;
		mat->tMapUsed = *(uint32_t*)(data + offs), offs += 0x04;
		mat->tCoordUsed = *(uint32_t*)(data + offs), offs += 0x04;

		for(j = 0; j < 3; ++j)
		{
			mtex = &(mat->textures[j]);
			mtex->texIndex = *(int16_t*)(data + offs), offs += 0x02;
			mtex->padding = *(int16_t*)(data + offs), offs += 0x02;
			mtex->MinFilter = *(uint16_t*)(data + offs), offs += 0x02;
			mtex->MagFilter = *(uint16_t*)(data + offs), offs += 0x02;
			mtex->WrapS = *(uint16_t*)(data + offs), offs += 0x02;
			mtex->WrapT = *(uint16_t*)(data + offs), offs += 0x02;
			mtex->minLODBias = *(float*)(data + offs), offs += 0x04;
			mtex->LODBias = *(float*)(data + offs), offs += 0x04;
			mtex->borderColor = *(rgba_t*)(data + offs), offs += 0x04;
		}

		for(j = 0; j < 3; ++j)
		{
			tcor = &(mat->tCoords[j]);
			tcor->matrixMode = *(uint8_t*)(data + offs), offs += 0x01;
			tcor->refCam = *(uint8_t*)(data + offs), offs += 0x01;
			tcor->mapMethod = *(uint8_t*)(data + offs), offs += 0x01;
			tcor->CoordIndex = *(uint8_t*)(data + offs), offs += 0x01;
			memcpy(tcor->scale, data + offs, 8), offs += 0x08;
			tcor->rotation = *(float*)(data + offs), offs += 0x04;
			memcpy(tcor->trans, data + offs, 8), offs += 0x08;
		}

		mat->emissiveCol = *(rgba_t*)(data + offs), offs += 0x04;
		mat->ambientCol = *(rgba_t*)(data + offs), offs += 0x04;
		mat->diffuseCol = *(rgba_t*)(data + offs), offs += 0x04;
		mat->specCol[0] = *(rgba_t*)(data + offs), offs += 0x04;
		mat->specCol[1] = *(rgba_t*)(data + offs), offs += 0x04;
		mat->constCol[0] = *(rgba_t*)(data + offs), offs += 0x04;
		mat->constCol[1] = *(rgba_t*)(data + offs), offs += 0x04;
		mat->constCol[2] = *(rgba_t*)(data + offs), offs += 0x04;
		mat->constCol[3] = *(rgba_t*)(data + offs), offs += 0x04;
		mat->constCol[4] = *(rgba_t*)(data + offs), offs += 0x04;
		mat->constCol[5] = *(rgba_t*)(data + offs), offs += 0x04;
		memcpy(mat->bufColor, data + offs, 16), offs += 0x10;
		mat->bumpTex = *(uint16_t*)(data + offs), offs += 0x02;
		mat->bumpMode = *(uint16_t*)(data + offs), offs += 0x02;
		mat->isBumpRenorm = *(uint16_t*)(data + offs), offs += 0x02;
		mat->padding1 = *(uint16_t*)(data + offs), offs += 0x02;
		mat->layerConfig = *(uint16_t*)(data + offs), offs += 0x02;
		mat->padding2 = *(uint16_t*)(data + offs), offs += 0x02;
		mat->fresnelSel = *(uint16_t*)(data + offs), offs += 0x02;
		mat->isClampHlight = *(uint8_t*)(data + offs), offs += 0x01;
		mat->dist0On = *(uint8_t*)(data + offs), offs += 0x01;
		mat->dist1On = *(uint8_t*)(data + offs), offs += 0x01;
		mat->geoFactor0On = *(uint8_t*)(data + offs), offs += 0x01;
		mat->geoFactor1On = *(uint8_t*)(data + offs), offs += 0x01;
		mat->reflectionOn = *(uint8_t*)(data + offs), offs += 0x01;
		mat->reflRSamplOff = *(uint8_t*)(data + offs), offs += 0x01;
		mat->reflRSamplInd = *(uint8_t*)(data + offs), offs += 0x01;
		mat->reflRSamplInp = *(uint16_t*)(data + offs), offs += 0x02;
		mat->reflRSamplScale = *(float*)(data + offs), offs += 0x04;
		mat->reflGSamplOff = *(uint8_t*)(data + offs), offs += 0x01;
		mat->reflGSamplInd = *(uint8_t*)(data + offs), offs += 0x01;
		mat->reflGSamplInp = *(uint16_t*)(data + offs), offs += 0x02;
		mat->reflGSamplScale = *(float*)(data + offs), offs += 0x04;
		mat->reflBSamplOff = *(uint8_t*)(data + offs), offs += 0x01;
		mat->reflBSamplInd = *(uint8_t*)(data + offs), offs += 0x01;
		mat->reflBSamplInp = *(uint16_t*)(data + offs), offs += 0x02;
		mat->reflBSamplScale = *(float*)(data + offs), offs += 0x04;
		mat->dist0SamplOff = *(uint8_t*)(data + offs), offs += 0x01;
		mat->dist0SamplInd = *(uint8_t*)(data + offs), offs += 0x01;
		mat->dist0SamplInp = *(uint16_t*)(data + offs), offs += 0x02;
		mat->dist0SamplScale = *(float*)(data + offs), offs += 0x04;
		mat->dist1SamplOff = *(uint8_t*)(data + offs), offs += 0x01;
		mat->dist1SamplInd = *(uint8_t*)(data + offs), offs += 0x01;
		mat->dist1SamplInp = *(uint16_t*)(data + offs), offs += 0x02;
		mat->dist1SamplScale = *(float*)(data + offs), offs += 0x04;
		mat->fres1SamplOff = *(uint8_t*)(data + offs), offs += 0x01;
		mat->fres1SamplInd = *(uint8_t*)(data + offs), offs += 0x01;
		mat->fres1SamplInp = *(uint16_t*)(data + offs), offs += 0x02;
		mat->fres1SamplScale = *(float*)(data + offs), offs += 0x04;
		mat->nTexCombiners = *(uint32_t*)(data + offs), offs += 0x04;
		memcpy(mat->texCInd, data + offs, 12), offs += 0x0C;

		/* Find the total number of texture combiners */
		/* Dumb that this isn't in the MATS chunk to begin with */
		for(j = 0; j < mat->nTexCombiners; ++j)
			if(mat->texCInd[j] + 1 > matsC->nTCom)
				matsC->nTCom = mat->texCInd[j] + 1;

		mat->alphaTestOn = *(uint8_t*)(data + offs), offs += 0x01;
		mat->alphaRefVal = *(uint8_t*)(data + offs), offs += 0x01;
		mat->alphaFunc = *(uint16_t*)(data + offs), offs += 0x02;
		mat->depthTestOn = *(uint8_t*)(data + offs), offs += 0x01;
		mat->depthWriteOn = *(uint8_t*)(data + offs), offs += 0x01;
		mat->depthTestFunc = *(uint16_t*)(data + offs), offs += 0x02;
		mat->blendEnabled = *(uint8_t*)(data + offs), offs += 0x01;
		mat->unk1 = *(uint8_t*)(data + offs), offs += 0x01;
		mat->unk2 = *(uint16_t*)(data + offs), offs += 0x02;
		mat->blendSrcFactRGB = *(uint16_t*)(data + offs), offs += 0x02;
		mat->blendDstFactRGB = *(uint16_t*)(data + offs), offs += 0x02;
		mat->blendFuncRGB = *(uint16_t*)(data + offs), offs += 0x02;
		mat->unk3 = *(uint16_t*)(data + offs), offs += 0x02;
		mat->blendSrcFactAlp = *(uint16_t*)(data + offs), offs += 0x02;
		mat->blendDstFactAlp = *(uint16_t*)(data + offs), offs += 0x02;
		mat->blendFuncAlp = *(uint16_t*)(data + offs), offs += 0x02;
		mat->unk4 = *(uint16_t*)(data + offs), offs += 0x02;
		memcpy(mat->blendColor, data + offs, 16), offs += 0x10;
	}

	/* Read all of the texture combiner structs */
	matsC->tCom = malloc(sizeof(texCombiner_t) * matsC->nTCom);
	for(i = 0; i < matsC->nTCom; ++i)
	{
		tcom = &(matsC->tCom[i]);
		tcom->combineRGB = *(uint16_t*)(data + offs), offs += 0x02;
		tcom->combineAlpha = *(uint16_t*)(data + offs), offs += 0x02;
		tcom->scaleRGB = *(uint16_t*)(data + offs), offs += 0x02;
		tcom->scaleAlpha = *(uint16_t*)(data + offs), offs += 0x02;
		tcom->bufInpRGB = *(uint16_t*)(data + offs), offs += 0x02;
		tcom->bufInpAlpha = *(uint16_t*)(data + offs), offs += 0x02;
		memcpy(tcom->srcRGB, data + offs, 6), offs += 0x06;
		memcpy(tcom->opRGB, data + offs, 6), offs += 0x06;
		memcpy(tcom->srcAlpha, data + offs, 6), offs += 0x06;
		memcpy(tcom->opAlpha, data + offs, 6), offs += 0x06;
		tcom->constInd = *(uint32_t*)(data + offs), offs += 0x04;
	}

	return(0);

}

int8_t readTexChunk(cmb_t* c)
{
	int offs, i;
	uint8_t* data;
	texChunk_t* texC;
	texture_t* tex;
	cubemap_t* cub;

	c->texC = malloc(sizeof(texChunk_t));
	offs = c->cmbC->texCPtr;
	data = c->data;
	texC = c->texC;
	texC->tex = NULL;

	memcpy(texC, data + offs, 4), offs += 0x04;
	if(*(int*)texC->magic != *(int*)("tex "))
	{
		fprintf(stderr, "Error: TEX chunk magic number does not match\n");
		fprintf(stderr, "\"tex \" vs \"%.4s\"\n", texC->magic);
		return(1);
	}
	texC->size = *(uint32_t*)(data + offs), offs += 0x04;
	texC->nTex = *(uint32_t*)(data + offs), offs += 0x04;
	texC->tex = malloc(sizeof(texture_t) * texC->nTex);

	for(i = 0; i < texC->nTex; ++i)
	{
		tex = &(texC->tex[i]);
		tex->size = *(uint32_t*)(data + offs), offs += 0x04;
		tex->nMipmaps = *(uint16_t*)(data + offs), offs += 0x02;
		tex->isETC1 = *(uint8_t*)(data + offs), offs += 0x01;
		tex->isCubemap = *(uint8_t*)(data + offs), offs += 0x01;
		tex->width = *(uint16_t*)(data + offs), offs += 0x02;
		tex->height = *(uint16_t*)(data + offs), offs += 0x02;
		tex->colFmt = *(uint16_t*)(data + offs), offs += 0x02;
		tex->datType = *(uint16_t*)(data + offs), offs += 0x02;
		tex->offset = *(uint32_t*)(data + offs), offs += 0x04;
		memcpy(tex->name, data + offs, 16), offs += 0x10;
	}

	for(i = 0; i < texC->nTex; ++i)
		if(texC->tex[i].isCubemap)
			memcpy(&(texC->tex[i].cubemap), data + offs, 24), offs += 0x18;

	return(0);
}

/* TODO convert this to offsets */
int8_t readSklmChunk(cmb_t* c)
{
	sklmChunk_t* sklmC;
	mshsChunk_t* mshsC;
	shpChunk_t* shpC;
	sepdChunk_t* sepdC;
	prmsChunk_t* prmsC;
	prmChunk_t* prmC;
	uint32_t size, offs, i, j;
	uint32_t sklmOffs, mshsOffs, shpOffs;
	uint32_t sepdOffs, prmsOffs, prmOffs;

	/* Allocate and read the SKLM chunk */
	sklmOffs = c->cmbC->sklmCPtr;
	size = sizeof(sklmChunk_t);
	c->sklmC = malloc(size);
	sklmC = c->sklmC;
	memcpy(sklmC, c->data + sklmOffs, size);

	/* Set everything to null for easy deletion */
	/* if a magic number doesn't match */
	sklmC->mshsC = NULL;
	sklmC->shpC  = NULL;

	if(*(int*)sklmC->magic != *(int*)("sklm"))
	{
		fprintf(stderr, "Error: SKLM chunk magic number does not match\n");
		fprintf(stderr, "\"sklm\" vs \"%.4s\"\n", sklmC->magic);
		return(1);
	}

	/* Allocate and read the MSHS chunk and meshes */
	mshsOffs = sklmOffs;
	mshsOffs += *(uint32_t*)(c->data + sklmOffs + 0x08);
	size = sizeof(mshsChunk_t);
	sklmC->mshsC = malloc(size);
	mshsC = sklmC->mshsC;
	memcpy(mshsC, c->data + mshsOffs, size);
	mshsC->meshes = NULL;

	if(*(int*)mshsC->magic != *(int*)("mshs"))
	{
		fprintf(stderr, "Error: MSHS chunk magic number does not match\n");
		fprintf(stderr, "\"mshs\" vs \"%.4s\"\n", mshsC->magic);
		return(1);
	}

	offs = mshsOffs + 0x10;
	size = sizeof(mesh_t) * mshsC->nMeshes;
	mshsC->meshes = malloc(size);
	memcpy(mshsC->meshes, c->data + offs, size);

	/* Allocate and read the SHP chunk and other stuff */
	/* Start of SKLM chunk + value of offset array */
	shpOffs = sklmOffs;
	shpOffs += *(uint32_t*)(c->data + sklmOffs + 0x0C);
	size = sizeof(shpChunk_t);
	sklmC->shpC = malloc(size);
	shpC = sklmC->shpC;
	memcpy(shpC, c->data + shpOffs, size);
	shpC->sepdC = NULL;

	if(*(int*)shpC->magic != *(int*)("shp "))
	{
		fprintf(stderr, "Error: SHP chunk magic number does not match\n");
		fprintf(stderr, "\"shp \" vs \"%.4s\"\n", shpC->magic);
		return(1);
	}

	/* SEPD chunks in SHP chunk */
	size = sizeof(sepdChunk_t) * shpC->nSEPDs;
	shpC->sepdC = malloc(size);
	sepdC = shpC->sepdC;
	for(i = 0; i < shpC->nSEPDs; ++i)
		sepdC[i].prmsC = NULL;
	for(i = 0; i < shpC->nSEPDs; ++i)
	{
		/* Start of the SHP chunk + value of offset array */
		sepdOffs = shpOffs;
		sepdOffs += *(uint16_t*)(c->data + shpOffs + 0x10 + (i << 1));
		memcpy(&(sepdC[i]), c->data + sepdOffs, sizeof(sepdChunk_t));

		if(*(int*)sepdC[i].magic != *(int*)("sepd"))
		{
			fprintf(stderr, "Error: SEPD chunk %d magic number ", i);
			fprintf(stderr, "does not match\n\"sepd\" vs ");
			fprintf(stderr, "\"%.4s\"\n", sepdC[i].magic);
			return(1);
		}

		/* PRMS chunks in each SEPD chunk */
		size = sizeof(prmsChunk_t) * sepdC[i].nPRMS;
		sepdC[i].prmsC = malloc(size);
		prmsC = sepdC[i].prmsC;
		for(j = 0; j < sepdC[i].nPRMS; ++j)
			prmsC[j].prmC = NULL, prmsC[j].bInds = NULL;
		for(j = 0; j < sepdC[i].nPRMS; ++j)
		{
			/* Start of current SEPD chunk + value of offset array */
			prmsOffs = sepdOffs;
			prmsOffs += *(uint16_t*)(c->data + sepdOffs + 0x108 + (j << 1));
			memcpy(&(prmsC[j]), c->data + prmsOffs, sizeof(prmsChunk_t));

			if(*(int*)prmsC[j].magic != *(int*)("prms"))
			{
				fprintf(stderr, "Error: PRMS chunk %d, %d magic ", i, j);
				fprintf(stderr, "number does not match\n\"prms\" vs ");
				fprintf(stderr, "\"%.4s\"\n", prmsC[j].magic);
				return(1);
			}

			/* PRM chunk in PRMS chunk */
			/* Assuming 1 prm chunk per prms, seems to be the case */
			if(prmsC[j].nPRM != 1)
			{
				fprintf(stderr, "Warning: PRMS %d, %d ", i, j);
				fprintf(stderr, "nPRM: %d\n", prmsC[j].nPRM);
			}

			/* Start of current PRMS chunk + value of offset array */
			prmOffs = prmsOffs;
			prmOffs += *(uint32_t*)(c->data + prmsOffs + 0x14);
			size = sizeof(prmChunk_t);
			prmsC[j].prmC = malloc(size);
			prmC = prmsC[j].prmC;
			memcpy(prmC, c->data + prmOffs, size);

			if(*(int*)prmC->magic != *(int*)("prm "))
			{
				fprintf(stderr, "Error: PRM chunk %d, %d magic ", i, j);
				fprintf(stderr, "number does not match\n\"prm \" vs ");
				fprintf(stderr, "\"%.4s\"\n", prmC->magic);
				return(1);
			}

			/* Bone indices in PRMS chunk */
			offs = prmsOffs + 0x18;
			size = sizeof(uint16_t) * prmsC[j].nBoneInd;
			prmsC[j].bInds = malloc(size);
			memcpy(prmsC[j].bInds, c->data + offs, size);
		}
	}

	return(0);
}

int8_t readLutsChunk(cmb_t* c)
{
	int offs;
	uint8_t* data;
	lutsChunk_t* lutsC;

	c->lutsC = malloc(sizeof(lutsChunk_t));
	offs = c->cmbC->lutsCPtr;
	lutsC = c->lutsC;
	data = c->data;

	memcpy(lutsC->magic, c->data + offs, 4), offs += 0x04;
	if(*(int*)lutsC->magic != *(int*)("luts"))
	{
		fprintf(stderr, "Error: LUTS chunk magic number does not match\n");
		fprintf(stderr, "\"luts\" vs \"%.4s\"\n", lutsC->magic);
		return(1);
	}

	lutsC->size = *(uint32_t*)(data + offs), offs += 0x04;
	lutsC->lutSetNum = *(uint32_t*)(data + offs), offs += 0x04;
	lutsC->unk1 = *(uint32_t*)(data + offs), offs += 0x04;

	return(0);
}

int8_t readVatrChunk(cmb_t* c)
{
	int size, offs, offs2;

	offs = c->cmbC->vatrCPtr;
	size = sizeof(vatrChunk_t);
	c->vatrC = malloc(size);
	memcpy(c->vatrC, c->data + offs, size);

	if(*(int*)c->vatrC->magic != *(int*)("vatr"))
	{
		fprintf(stderr, "Error: VATR chunk magic number does not match\n");
		fprintf(stderr, "\"vatr\" vs \"%.4s\"\n", c->vatrC->magic);
		return(1);
	}

	c->vatrD = malloc(sizeof(vatrData_t));
	c->vatrD->positions = NULL;
	c->vatrD->normals = NULL;
	c->vatrD->colors = NULL;
	c->vatrD->tex0Coords = NULL;
	c->vatrD->tex1Coords = NULL;
	c->vatrD->tex2Coords = NULL;
	c->vatrD->boneInd = NULL;
	c->vatrD->boneWeight = NULL;

	if((size = c->vatrC->positions.size) != 0)
	{
		offs2 = offs + c->vatrC->positions.offset;
		c->vatrD->positions = malloc(size);
		memcpy(c->vatrD->positions, c->data + offs2, size);
	}
	if((size = c->vatrC->normals.size) != 0)
	{
		offs2 = offs + c->vatrC->normals.offset;
		c->vatrD->normals = malloc(size);
		memcpy(c->vatrD->normals, c->data + offs2, size);
	}
	if((size = c->vatrC->colors.size) != 0)
	{
		offs2 = offs + c->vatrC->colors.offset;
		c->vatrD->colors = malloc(size);
		memcpy(c->vatrD->colors, c->data + offs2, size);
	}
	if((size = c->vatrC->tex0Coords.size) != 0)
	{
		offs2 = offs + c->vatrC->tex0Coords.offset;
		c->vatrD->tex0Coords = malloc(size);
		memcpy(c->vatrD->tex0Coords, c->data + offs2, size);
	}
	if((size = c->vatrC->tex1Coords.size) != 0)
	{
		offs2 = offs + c->vatrC->tex1Coords.offset;
		c->vatrD->tex1Coords = malloc(size);
		memcpy(c->vatrD->tex1Coords, c->data + offs2, size);
	}
	if((size = c->vatrC->tex2Coords.size) != 0)
	{
		offs2 = offs + c->vatrC->tex2Coords.offset;
		c->vatrD->tex2Coords = malloc(size);
		memcpy(c->vatrD->tex2Coords, c->data + offs2, size);
	}
	if((size = c->vatrC->boneInd.size) != 0)
	{
		offs2 = offs + c->vatrC->boneInd.offset;
		c->vatrD->boneInd = malloc(size);
		memcpy(c->vatrD->boneInd, c->data + offs2, size);
	}
	if((size = c->vatrC->boneWeight.size) != 0)
	{
		offs2 = offs + c->vatrC->boneWeight.offset;
		c->vatrD->boneWeight = malloc(size);
		memcpy(c->vatrD->boneWeight, c->data + offs2, size);
	}

	return(0);
}

void delCmb(cmb_t c)
{
	int i, j;

	/* Delete data if it still exists */
	if(c.data != NULL)
		free(c.data);

	/* Delete CMB chunk */
	if(c.cmbC != NULL)
		free(c.cmbC);

	/* Delete SKL chunk */
	if(c.sklC != NULL)
	{
		if(c.sklC->bones != NULL)
			free(c.sklC->bones);
		free(c.sklC);
	}

	/* Delete MATS chunk */
	if(c.matsC != NULL)
	{
		if(c.matsC->mats != NULL)
			free(c.matsC->mats);
		if(c.matsC->tCom != NULL)
			free(c.matsC->tCom);
		free(c.matsC);
	}

	/* Delete TEX chunk */
	if(c.texC != NULL)
	{
		if(c.texC->tex != NULL)
			free(c.texC->tex);
		free(c.texC);
	}

	/* Delete SKLM chunk */
	if(c.sklmC != NULL)
	{
		if(c.sklmC->mshsC != NULL)
		{
			if(c.sklmC->mshsC->meshes != NULL)
				free(c.sklmC->mshsC->meshes);
			free(c.sklmC->mshsC);
		}

		if(c.sklmC->shpC != NULL)
		{
			if(c.sklmC->shpC->sepdC != NULL)
			{
				for(i = 0; i < c.sklmC->shpC->nSEPDs; ++i)
				{
					if(c.sklmC->shpC->sepdC[i].prmsC != NULL)
					{
						for(j = 0; j < c.sklmC->shpC->sepdC[i].nPRMS; ++j)
						{
							if(c.sklmC->shpC->sepdC[i].prmsC[j].prmC != NULL)
								free(c.sklmC->shpC->sepdC[i].prmsC[j].prmC);
							if(c.sklmC->shpC->sepdC[i].prmsC[j].bInds != NULL)
								free(c.sklmC->shpC->sepdC[i].prmsC[j].bInds);
						}
						free(c.sklmC->shpC->sepdC[i].prmsC);
					}
				}
				free(c.sklmC->shpC->sepdC);
			}
			free(c.sklmC->shpC);
		}
		free(c.sklmC);
	}

	/* Delete LUTS chunk */
	if(c.lutsC != NULL)
		free(c.lutsC);

	/* Delete VATR chunk */
	if(c.vatrC != NULL)
		free(c.vatrC);

	/* Delete VATR data */
	if(c.vatrD != NULL)
	{
		if(c.vatrD->positions != NULL)
			free(c.vatrD->positions);
		if(c.vatrD->normals != NULL)
			free(c.vatrD->normals);
		if(c.vatrD->colors != NULL)
			free(c.vatrD->colors);
		if(c.vatrD->tex0Coords != NULL)
			free(c.vatrD->tex0Coords);
		if(c.vatrD->tex1Coords != NULL)
			free(c.vatrD->tex1Coords);
		if(c.vatrD->tex2Coords != NULL)
			free(c.vatrD->tex2Coords);
		if(c.vatrD->boneInd != NULL)
			free(c.vatrD->boneInd);
		if(c.vatrD->boneWeight != NULL)
			free(c.vatrD->boneWeight);
		free(c.vatrD);
	}

	/* Delete Vertex Index Data section */
	if(c.vIndDat != NULL)
		free(c.vIndDat);

	/* Delete Texture Data section */
	if(c.texDat != NULL)
		free(c.texDat);

	/* Do not free c, it may be on a stack */
	return;
}

void printCmb(cmb_t c)
{
	int i, j;

	printCmbChunk(*c.cmbC);
	printf("\n");
	printSklChunk(*c.sklC);
	printf("\n");
	printMatsChunk(*c.matsC);
	printf("\n");
	printTexChunk(*c.texC);
	printf("\n");
	printSklmChunk(*c.sklmC);
	printf("\n");
	printLutsChunk(*c.lutsC);
	printf("\n");
	printVatrChunk(*c.vatrC);
}
