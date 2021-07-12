#include <stdio.h>
#include <stdlib.h>
#include "sklmChunk.h"

void printSklmChunk(sklmChunk_t s)
{
	printf("SKLM magic:        %.4s\n", s.magic);
	printf("SKLM size:         %u\n", s.size);
	printf("SKLM MSHS chunk:\n");
	printMshsChunk(*s.mshsC);
	printf("\n");
	printf("SKLM SHP chunk:\n");
	printShpChunk(*s.shpC);

	return;
}

void printMshsChunk(mshsChunk_t m)
{
	int i;

	printf("\tMSHS magic:             %.4s\n", m.magic);
	printf("\tMSHS size:              %u\n", m.size);
	printf("\tMSHS number of meshses: %u\n", m.nMeshes);
	printf("\tMSHS id count:          %u\n", m.idCount);
	printf("\n");

	for(i = 0; i < m.nMeshes; ++i)
	{
		printf("\tMesh %d:\n", i);
		printMesh(m.meshes[i]);
		if(i != m.nMeshes - 1)
			printf("\n");
	}

	return;
}

void printMesh(mesh_t m)
{
	printf("\t\tMESH id:         %u\n", m.id);
	printf("\t\tMESH sepd index: %u\n", m.sepdInd);
	printf("\t\tMESH mats index: %u\n", m.matsInd);
}

void printShpChunk(shpChunk_t s)
{
	int i;

	printf("\tSHP magic:        %.4s\n", s.magic);
	printf("\tSHP size:         %u\n", s.size);
	printf("\tSHP num SEPDs:    %u\n", s.nSEPDs);
	printf("\tSHP flags:        0x%08x\n", s.flags);
	printf("\tSHP SEPD chunks:\n");
	printf("\n");

	for(i = 0; i < s.nSEPDs; ++i)
	{
		printf("\tSEPD %d:\n", i);
		printSepdChunk(s.sepdC[i]);
		if(i != s.nSEPDs - 1)
			printf("\n");
	}

	return;
}

void printSepdChunk(sepdChunk_t s)
{
	int i;

	printf("\t\t\tSEPD magic:        %.4s\n", s.magic);
	printf("\t\t\tSEPD size:         %u\n", s.size);
	printf("\t\t\tSEPD num PRMS:     %u\n", s.nPRMS);
	printf("\t\t\tSEPD flags1:       0x%04x\n", s.flags1);
	printf("\t\t\tSEPD center:       %3.2f, %3.2f %3.2f\n", s.center[0], s.center[1], s.center[2]);
	printf("\t\t\tSEPD Pos offset:   %3.2f, %3.2f %3.2f\n", s.posOff[0], s.posOff[1], s.posOff[2]);
	if(s.flags1 & SEPD_HAS_POSITIONS)
	{
		printf("\t\t\tSEPD positions:\n");
		printVertList(s.positions);
	}
	if(s.flags1 & SEPD_HAS_NORMALS)
	{
		printf("\t\t\tSEPD normals:\n");
		printVertList(s.normals);
	}
	if(s.flags1 & SEPD_HAS_COLORS)
	{
		printf("\t\t\tSEPD vert colors:\n");
		printVertList(s.vertColors);
	}
	if(s.flags1 & SEPD_HAS_UV0)
	{
		printf("\t\t\tSEPD tex 0 coords:\n");
		printVertList(s.tex0Coords);
	}
	if(s.flags1 & SEPD_HAS_UV1)
	{
		printf("\t\t\tSEPD tex 1 coords:\n");
		printVertList(s.tex1Coords);
	}
	if(s.flags1 & SEPD_HAS_UV2)
	{
		printf("\t\t\tSEPD tex 2 coords:\n");
		printVertList(s.tex2Coords);
	}
	if(s.flags1 & SEPD_HAS_INDICES)
	{
		printf("\t\t\tSEPD bone indices:\n");
		printVertList(s.boneIndices);
	}
	if(s.flags1 & SEPD_HAS_WEIGHTS)
	{
		printf("\t\t\tSEPD bone weights:\n");
		printVertList(s.boneWeights);
	}
	printf("\t\t\tSEPD bone dim:     %u\n", s.boneDimension);
	printf("\t\t\tSEPD flags2:       0x%04x\n", s.flags2);
	printf("\t\t\tSPED PRMS chunks:\n");
	printf("\n");

	for(i = 0; i < s.nPRMS; ++i)
	{
		printf("\t\t\tPRMS %d:\n", i);
		printPrmsChunk(s.prmsC[i]);
		if(i != s.nPRMS - 1)
			printf("\n");
	}

	return;
}

void printVertList(vertList_t v)
{
	printf("\t\t\t\tVLIST offset:    0x%08x\n", v.offset);
	printf("\t\t\t\tVLIST scale:     %8.7f\n", v.scale);
	printf("\t\t\t\tVLIST data type: 0x%04x\n", v.datType);
	printf("\t\t\t\tVLIST mode:      0x%04x\n", v.mode);
	printf("\t\t\t\tVLIST constant:  %3.2f, %3.2f, %3.2f, %3.2f\n", v.constant[0], v.constant[1], v.constant[2], v.constant[3]); 

	return;
}

void printPrmsChunk(prmsChunk_t p)
{
	int i;

	printf("\t\t\t\tPRMS magic:            %.4s\n", p.magic);
	printf("\t\t\t\tPRMS size:             %u\n", p.size);
	printf("\t\t\t\tPRMS num prm:          %u\n", p.nPRM);
	printf("\t\t\t\tPRMS skinning mode:    %u\n", p.skinMode);
	printf("\t\t\t\tPRMS num bone indices: %u\n", p.nBoneInd);
	printf("\t\t\t\tPRMS bone ind offset:  0x%08x\n", p.bIndOffs);
	printf("\t\t\t\tPRMS PRM chunk:\n");
	printPrmChunk(*p.prmC);
	printf("\t\t\t\tPRMS bone indices:");
	for(i = 0; i < p.nBoneInd; ++i)
		printf(" %u", p.bInds[i]);
	printf("\n");
}

void printPrmChunk(prmChunk_t p)
{
	printf("\t\t\t\t\tPRM magic:          %.4s\n", p.magic);
	printf("\t\t\t\t\tPRM size:           %u\n", p.size);
	printf("\t\t\t\t\tPRM is visible:     %u\n", p.visible);
	printf("\t\t\t\t\tPRM primitive mode: %u\n", p.primMode);
	printf("\t\t\t\t\tPRM data type:      0x%04x\n", p.datType);
	printf("\t\t\t\t\tPRM num indices:    %u\n", p.nVertInd);
	printf("\t\t\t\t\tPRM first index:    %u\n", p.firstInd);

}
