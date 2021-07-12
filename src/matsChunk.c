#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "matsChunk.h"

void printMatsChunk(matsChunk_t m)
{
	int i;

	printf("MATS magic:         %.4s\n", m.magic);
	printf("MATS size:          %u\n", m.size);
	printf("MATS num mats:      %u\n", m.nMats);
	printf("MATS materials:\n");
	printf("\n");

	for(i = 0; i < m.nMats; ++i)
	{
		printf("\tMATERIAL %d:\n", i);
		printMaterial(m.mats[i]);
			printf("\n");
	}

	printf("MATS tex combiners:\n");
	printf("\n");

	for(i = 0; i < m.nTCom; ++i)
	{
		printf("\tTEXCOM %d:\n", i);
		printTexCombiner(m.tCom[i]);
		if(i != m.nTCom - 1)
			printf("\n");
	}

	return;
}

void printMaterial(material_t m)
{
	int i;

	printf("\tMATERIAL fragment lighting on:         %s\n", (m.fragLightOn) ? "true" : "false");
	printf("\tMATERIAL vertex light on:              %s\n", (m.vertLightOn) ? "true" : "false");
	printf("\tMATERIAL hemisphere light on:          %s\n", (m.hemiLightOn) ? "true" : "false");
	printf("\tMATERIAL face culling on:              %s\n", (m.faceCullOn) ? "true" : "false");
	printf("\tMATERIAL face culling mode:            %u\n", m.faceCullMode);
	printf("\tMATERIAL polygon offs on:              %s\n", (m.polyOffsOn) ? "true" : "false");
	printf("\tMATERIAL polygon offset:               %d\n", m.polyOffset);
	printf("\tMATERIAL depth offset:                 %8.7f\n", m.depthOffset);
	printf("\tMATERIAL texture mappers used:         %u\n", m.tMapUsed);
	printf("\tMATERIAL texture coords used:          %u\n", m.tCoordUsed);
	printf("\tMATERIAL textures:\n");
	for(i = 0; i < 3; ++i)
	{
		printMatTex(m.textures[i]);
		if(i != 2)
			printf("\n");
	}
	printf("\tMATERIAL texture coordinates:\n");
	for(i = 0; i < 3; ++i)
	{
		printTexCoords(m.tCoords[i]);
		if(i != 2)
			printf("\n");
	}
	printf("\tMATERIAL emissive color:               ");
	printRGBA(m.emissiveCol);
	printf("\tMATERIAL ambient color:                ");
	printRGBA(m.ambientCol);
	printf("\tMATERIAL diffuse color:                ");
	printRGBA(m.diffuseCol);
	printf("\tMATERIAL specular 0 color:             ");
	printRGBA(m.specCol[0]);
	printf("\tMATERIAL specular 1 color:             ");
	printRGBA(m.specCol[1]);
	printf("\tMATERIAL constant 0 color:             ");
	printRGBA(m.constCol[0]);
	printf("\tMATERIAL constant 1 color:             ");
	printRGBA(m.constCol[1]);
	printf("\tMATERIAL constant 2 color:             ");
	printRGBA(m.constCol[2]);
	printf("\tMATERIAL constant 3 color:             ");
	printRGBA(m.constCol[3]);
	printf("\tMATERIAL constant 4 color:             ");
	printRGBA(m.constCol[4]);
	printf("\tMATERIAL constant 5 color:             ");
	printRGBA(m.constCol[5]);
	printf("\tMATERIAL buffer color:                 %3.2f, %3.2f, %3.2f, %3.2f\n", m.bufColor[0], m.bufColor[1], m.bufColor[2], m.bufColor[3]);
	printf("\tMATERIAL bump texture:                 %u\n", m.bumpTex);
	printf("\tMATERIAL bump mode:                    %u\n", m.bumpMode);
	printf("\tMATERIAL is bump renormalise:          %s\n", (m.isBumpRenorm) ? "true" : "false");
	printf("\tMATERIAL layer config:                 %u\n", m.layerConfig);
	printf("\tMATERIAL fresnel selector:             %u\n", m.fresnelSel);
	printf("\tMATERIAL is clamp highlight:           %s\n", (m.isClampHlight) ? "true" : "false");
	printf("\tMATERIAL distribution 0 on:            %s\n", (m.dist0On) ? "true" : "false");
	printf("\tMATERIAL distribution 1 on:            %s\n", (m.dist1On) ? "true" : "false");
	printf("\tMATERIAL geometric factor 0 on:        %s\n", (m.geoFactor0On) ? "true" : "false");
	printf("\tMATERIAL geometric factor 1 on:        %s\n", (m.geoFactor1On) ? "true" : "false");
	printf("\tMATERIAL reflection on:                %s\n", (m.reflectionOn) ? "true" : "false");
	printf("\tMATERIAL reflectance R sampler off?:   %s (%u)\n", (m.reflRSamplOff) ? "true" : "false", m.reflRSamplOff);
	printf("\tMATERIAL reflectance R sampler index:  %u\n", m.reflRSamplInd);
	printf("\tMATERIAL reflectance R sampler input:  %u\n", m.reflRSamplInp);
	printf("\tMATERIAL reflectance R sampler scale:  %3.2f\n", m.reflRSamplScale);
	printf("\tMATERIAL reflectance G sampler off?:   %s (%u)\n", (m.reflGSamplOff) ? "true" : "false", m.reflGSamplOff);
	printf("\tMATERIAL reflectance G sampler index:  %u\n", m.reflGSamplInd);
	printf("\tMATERIAL reflectance G sampler input:  %u\n", m.reflGSamplInp);
	printf("\tMATERIAL reflectance G sampler scale:  %3.2f\n", m.reflGSamplScale);
	printf("\tMATERIAL reflectance B sampler off?:   %s (%u)\n", (m.reflBSamplOff) ? "fasle" : "false", m.reflBSamplOff);
	printf("\tMATERIAL reflectance B sampler index:  %u\n", m.reflBSamplInd);
	printf("\tMATERIAL reflectance B sampler input:  %u\n", m.reflBSamplInp);
	printf("\tMATERIAL reflectance B sampler scale:  %3.2f\n", m.reflBSamplScale);
	printf("\tMATERIAL distribution 0 sampler off?:  %s (%u)\n", (m.dist0SamplOff) ? "true" : "false", m.dist0SamplOff);
	printf("\tMATERIAL distribution 0 sampler index: %u\n", m.dist0SamplInd);
	printf("\tMATERIAL distribution 0 sampler input: %u\n", m.dist0SamplInp);
	printf("\tMATERIAL distribution 0 sampler scale: %3.2f\n", m.dist0SamplScale);
	printf("\tMATERIAL distribution 1 sampler off?:  %s (%u)\n", (m.dist1SamplOff) ? "true" : "false", m.dist1SamplOff);
	printf("\tMATERIAL distribution 1 sampler index: %u\n", m.dist1SamplInd);
	printf("\tMATERIAL distribution 1 sampler input: %u\n", m.dist1SamplInp);
	printf("\tMATERIAL distribution 1 sampler scale: %3.2f\n", m.dist1SamplScale);
	printf("\tMATERIAL fresnel 1 sampler off?:       %s (%u)\n", (m.fres1SamplOff) ? "true" : "false", m.fres1SamplOff);
	printf("\tMATERIAL fresnel 1 sampler index:      %u\n", m.fres1SamplInd);
	printf("\tMATERIAL fresnel 1 sampler input:      %u\n", m.fres1SamplInp);
	printf("\tMATERIAL fresnel 1 sampler scale:      %3.2f\n", m.fres1SamplScale);
	printf("\tMATERIAL used texture env stages:      %u\n", m.nTexCombiners);
	printf("\tMATERIAL texture env stages indices:   %d, %d, %d, %d, %d, %d\n", m.texCInd[0], m.texCInd[1], m.texCInd[2], m.texCInd[3], m.texCInd[4], m.texCInd[5]);
	printf("\tMATERIAL alpha test on:                %s\n", (m.alphaTestOn) ? "true" : "false");
	printf("\tMATERIAL alpha reference value:        %u\n", m.alphaRefVal);
	printf("\tMATERIAL alpha func:                   0x%04x\n", m.alphaFunc);
	printf("\tMATERIAL depth test on:                %s\n", (m.depthTestOn) ? "true" : "false");
	printf("\tMATERIAL depth write on:               %s\n", (m.depthWriteOn) ? "true" : "false");
	printf("\tMATERIAL depth test func:              0x%04x\n", m.depthTestFunc);
	printf("\tMATERIAL blend enabled:                %s\n", (m.blendEnabled) ? "true" : "false");
	printf("\tMATERIAL blend source factor RGB:      0x%04x\n", m.blendSrcFactRGB);
	printf("\tMATERIAL blend dest factor RGB:        0x%04x\n", m.blendDstFactRGB);
	printf("\tMATERIAL blend equation alpha:         0x%04x\n", m.blendFuncRGB);
	printf("\tMATERIAL blend source factor alpha:    0x%04x\n", m.blendSrcFactAlp);
	printf("\tMATERIAL blend dest factor alpha:      0x%04x\n", m.blendDstFactAlp);
	printf("\tMATERIAL blend equation alpha:         0x%04x\n", m.blendFuncAlp);
	printf("\tMATERIAL blend color:                  %3.2f, %3.2f, %3.2f, %3.2f\n", m.blendColor[0], m.blendColor[1], m.blendColor[2], m.blendColor[3]);

	return;
}

void printMatTex(matTex_t m)
{
	printf("\t\tMATTEX texture index:            %d\n", m.texIndex);
	printf("\t\tMATTEX minification filter:      0x%04x\n", m.MinFilter);
	printf("\t\tMATTEX magnification filter:     0x%04x\n", m.MagFilter);
	printf("\t\tMATTEX wrap S mode:              0x%04x\n", m.WrapS);
	printf("\t\tMATTEX wrap T mode:              0x%04x\n", m.WrapT);
	printf("\t\tMATTEX min level of detail bias: %3.2f\n", m.minLODBias);
	printf("\t\tMATTEX level of detail bias:     %3.2f\n", m.LODBias);
	printf("\t\tMATTEX border color:             ");
	printRGBA(m.borderColor);

	return;
}

void printTexCoords(texCoords_t t)
{
	printf("\t\tTEXCOORDS matrix mode:      0x%02x\n", t.matrixMode);
	printf("\t\tTEXCOORDS reference camera: 0x%02x\n", t.refCam);
	printf("\t\tTEXCOORDS mapping mehtod:   0x%02x\n", t.mapMehtod);
	printf("\t\tTEXCOORDS coordinate index: 0x%02x\n", t.CoordIndex);
	printf("\t\tTEXCOORDS scale:            %3.2f, %3.2f\n", t.scale[0], t.scale[1]);
	printf("\t\tTEXCOORDS rotation:         %3.2f\n", t.rotation);
	printf("\t\tTEXCOORDS translation:      %3.2f, %3.2f\n", t.trans[0], t.trans[1]);

	return;
}

void printTexCombiner(texCombiner_t t)
{
	printf("\tTEXCOMBINER combine rgb:        0x%04x\n", t.combineRGB);
	printf("\tTEXCOMBINER combine alpha:      0x%04x\n", t.combineAlpha);
	printf("\tTEXCOMBINER scale rgb:          0x%04x\n", t.scaleRGB);
	printf("\tTEXCOMBINER scale alpha:        0x%04x\n", t.scaleAlpha);
	printf("\tTEXCOMBINER buffer input rgb:   0x%04x\n", t.bufInpRGB);
	printf("\tTEXCOMBINER buffer input alpha: 0x%04x\n", t.bufInpAlpha);
	printf("\tTEXCOMBINER source rgb:         0x%04x, 0x%04x, 0x%04x\n", t.srcRGB[0], t.srcRGB[1], t.srcRGB[2]);
	printf("\tTEXCOMBINER op rgb:             0x%04x, 0x%04x, 0x%04x\n", t.opRGB[0], t.opRGB[1], t.opRGB[2]);
	printf("\tTEXCOMBINER source alpha:       0x%04x, 0x%04x, 0x%04x\n", t.srcAlpha[0], t.srcAlpha[1], t.srcAlpha[2]);
	printf("\tTEXCOMBINER op alpha:           0x%04x, 0x%04x, 0x%04x\n", t.opAlpha[0], t.opAlpha[1], t.opAlpha[2]);
	printf("\tTEXCOMBINER constant index:     0x%08x\n", t.constInd);
	return;
}

void printRGBA(rgba_t c)
{
	printf("%u, %u, %u, %u\n", c.R, c.G, c.B, c.A);

	return;
}
