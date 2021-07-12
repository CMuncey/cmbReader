#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "cmbTextures.h"
#include "cmbConstants.h"
#include "texChunk.h"

#include <string.h>

int8_t swizzleLUT[64] =
{
	 0,  1,  8,  9,  2,  3, 10, 11,
	16, 17, 24, 25, 18, 19, 26, 27,
	 4,  5, 12, 13,  6,  7, 14, 15,
	20, 21, 28, 29, 22, 23, 30, 31,
	32, 33, 40, 41, 34, 35, 42, 43,
	48, 49, 56, 57, 50, 51, 58, 59,
	36, 37, 44, 45, 38, 39, 46, 47,
	52, 53, 60, 61, 54, 55, 62, 63
};

/* Change the order around to make indexing easier */
int16_t etc1LUT[8][4] =
{
	{ 2,    8,  -2,  -8},
	{ 5,   17,  -5, -17},
	{ 9,   29,  -9, -29},
	{ 13,  42, -13, -42},
	{ 18,  60, -18, -60},
	{ 24,  80, -24, -80},
	{ 33, 106, -33,-106},
	{ 47, 183, -47,-183}
};

uint32_t getFmtSize(uint32_t f)
{
	switch(f)
	{
		case RGBA8:
			return(4);
		case RGB8:
			return(3);
		case RGBA4:
		case RGBA5551:
		case RGB565:
			return(2);
		case ETC1:
		case ETC1A4:
		case LA4:
		case LA8:
		case A8:
		case L8:
		case L4:
			return(1);
	}

	return(1);
}

void getColor(const uint8_t* pix, texRGBA8_t* t, uint32_t fmt)
{
	uint32_t t32;
	uint16_t t16;
	uint8_t t8;

	switch(fmt)
	{
		case RGBA8:
			t32 = *(uint32_t*)(pix);
			t->R = (t32 >> 24) & 0xFF;
			t->G = (t32 >> 16) & 0xFF;
			t->B = (t32 >>  8) & 0xFF;
			t->A = t32 & 0xFF;
			break;

		case RGB8:
			t32 = *(uint32_t*)(pix);
			t->R = (t32 >> 24) & 0xFF;
			t->G = (t32 >> 16) & 0xFF;
			t->B = (t32 >>  8) & 0xFF;
			t->A = 0xFF;
			break;

		case RGBA5551:
			t16 = *(uint16_t*)(pix);
			t->R = exp5to8((t16 >>  1) & 0x1F);
			t->G = exp5to8((t16 >>  6) & 0x1F);
			t->B = exp5to8((t16 >> 11) & 0x1F);
			t->A = (t16 & 0x01) * 0xFF;
			break;

		case RGB565:
			t16 = *(uint16_t*)(pix);
			t->R = exp5to8(t16 & 0x1F);
			t->G = exp6to8((t16 >>  5) & 0x3F);
			t->B = exp5to8((t16 >> 11) & 0x1F);
			t->A = 0xFF;
			break;

		case RGBA4:
			t16 = *(uint16_t*)(pix);
			t->R = exp4to8((t16 >>  4) & 0x0F);
			t->G = exp4to8((t16 >>  8) & 0x0F);
			t->B = exp4to8((t16 >> 12) & 0x0F);
			t->A = exp4to8((t16 & 0xF));
			break;

		/* might be wrong? */
		case LA8:
			t16 = *(uint16_t*)(pix);
			t->R = t->G = t->B = (t16 & 0xFF);
			t->A = ((t16 >> 8) & 0xFF);
			break;

		/* unused? */
		case LA4:
			t8  = *pix;
			t->R = t->G = t->B = ((t8 >> 4) | (t8 & 0xF0));
			t->A = ((t8 << 4) | (t8 & 0x0F));
			break;

		case L8:
			t8  = *pix;
			t->R = t->G = t->B = t->A = t8;
			break;

		case A8:
			t8  = *pix;
			t->R = t->G = t->B = 0xFF;
			t->A = t8;
			break;

		/* unimplemented */
		/* Need to rework, need src[offset / 2] */
		case L4:
			t8 = *pix;
			t->R = t->G = t->B = t->A = 0xFF;
			break;
	}
}

int clamp(int z, int x, int y)
{
	if(z < x)
		return(x);
	if(z > y)
		return(y);
	return(z);
}

uint8_t exp4to8(uint8_t n)
{
	/* Replicate the low order bits into the high order */
	return((n << 4) | n);
}

uint8_t exp5to8(uint8_t n)
{
	/* Bits 4-2 go to 2-0, 4-0 go to 7-3 */
	return((n << 3) | (n >> 2));
}

uint8_t exp6to8(uint8_t n)
{
	return((n << 2) | (n >> 4));
}

int8_t signed3b(int32_t n)
{
	/* Sign extend the lower 3 bits */
	return((n << 29) >> 29);
}

void etc1GetCols(uint8_t* c, const int16_t* iTab, const texRGB8_t* t)
{
	int i, r, g, b;

	r = t->R, g = t->G, b = t->B;
	for(i = 0; i < 4; ++i)
	{
		c[(i * 3) + 0] = clamp(r + iTab[i], 0, 255);
		c[(i * 3) + 1] = clamp(g + iTab[i], 0, 255);
		c[(i * 3) + 2] = clamp(b + iTab[i], 0, 255);
	}
}

void etc1Alpha(uint8_t* dst, uint64_t a, uint32_t w)
{
	int32_t x, y, a1, a2, dInd;

	a1 = (a >> 32) & 0xFFFFFFFF;
	a2 = a & 0xFFFFFFFF;

	/* Given the rest of the dumb format, I assume half of each pixel's */
	/* alpha value comes from a1, and the other half from a2 */
	for(x = 0; x < 2; ++x)
		for(y = 0; y < 4; ++y, a2 >>= 4)
			dst[(((y * w) + x) << 2) + 3] = exp4to8(a2 & 0x0F);

	for(x = 2; x < 4; ++x)
		for(y = 0; y < 4; ++y, a1 >>= 4)
			dst[(((y * w) + x) << 2) + 3] = exp4to8(a1 & 0x0F);
}

void etc1Color(uint8_t* dst, uint64_t c, uint32_t w)
{
	const int16_t* iTab1;
	const int16_t* iTab2;
	uint32_t c1, c2, i, x, y, dInd;
	uint8_t cols1[12], cols2[12];
	uint8_t t1, diff, flip, lsb, msb, colInd, blk;
	int8_t t2;
	const uint8_t* cols;
	texRGB8_t base1, base2;

	/* c1 is bits 63-32, c2 is bits 31-0 */
	c1 = (c >> 32) & 0xFFFFFFFF;
	c2 = c & 0xFFFFFFFF;

	/* differential bit is 33, flip is 32 */
	diff = (c1 & 0x02) != 0;
	flip = (c1 & 0x01) != 0;

	/* Table entry 1 index is bits 39-37 */
	/* Table entry 2 index is bits 36-34 */
	iTab1 = etc1LUT[(c1 >> 5) & 0x07];
	iTab2 = etc1LUT[(c1 >> 2) & 0x07];

	if(diff)
	{
		/* Differential mode */
		/* X1 = 5 bits, X2 = next 3 bits signed */
		t1 = (c1 >> 27) & 0x1F;
		t2 = signed3b((c1 >> 24) & 0x07);
		base1.R = exp5to8(t1);
		base2.R = exp5to8(t1 + t2);

		t1 = (c1 >> 19) & 0x1F;
		t2 = signed3b((c1 >> 16) & 0x07);
		base1.G = exp5to8(t1);
		base2.G = exp5to8(t1 + t2);

		t1 = (c1 >> 11) & 0x1F;
		t2 = signed3b((c1 >> 8) & 0x07);
		base1.B = exp5to8(t1);
		base2.B = exp5to8(t1 + t2);
	}
	else
	{
		/* Individual mode */
		/* R1 = 63-60, G1 = 55-52, B1 = 47-44 */
		/* R2 = 59-56, G2 = 51-48, B2 = 43-40 */
		base1.R = exp4to8((c1 >> 28) & 0x0F);
		base2.R = exp4to8((c1 >> 24) & 0x0F);
		base1.G = exp4to8((c1 >> 20) & 0x0F);
		base2.G = exp4to8((c1 >> 16) & 0x0F);
		base1.B = exp4to8((c1 >> 12) & 0x0F);
		base2.B = exp4to8((c1 >>  8) & 0x0F);
	}

	/* Calculate all color offsets, and pick the right one later */
	etc1GetCols(cols1, iTab1, &base1);
	etc1GetCols(cols2, iTab2, &base2);

	/* Loop to set the data in dst */
	/* This looks accurate */
	for(i = 0; i < 16; ++i)
	{
		/* Figure out which modified color to use */
		lsb = (c2 >> i) & 0x01;
		msb = (c2 >> (16 + i)) & 0x01;
		colInd = (msb << 1) | lsb;

		/* y = row number, x = column number */
		/* dInd = ((row * numCols) + col) * bytes per pixel */
		x = i >> 2;
		y = i & 0x03;
		dInd = ((y * w) + x) << 2;

		/* Flip = 0, 2 tall blocks */
		/* Flip = 1, 2 fat blocks */
		/* Figure out which block we're in */
		/* to use the correct subblock color  */
		blk = (flip) ? (y & 0x02) : (x & 0x02);
		cols = (blk) ? cols2 : cols1;

		/* Index into the correct color location */
		/* ColInd is basically which table to use */
		dst[dInd + 0] = cols[(colInd * 3) + 0];
		dst[dInd + 1] = cols[(colInd * 3) + 1];
		dst[dInd + 2] = cols[(colInd * 3) + 2];

	}
}

void etc1Decompress(const uint8_t* src, uint8_t* dst, const texture_t* tex)
{
	uint64_t col, alph;
	uint32_t w, h, x, x2, y, y2, sOffs, dOffs;
	uint8_t hasA;

	hasA = (tex->colFmt == ETC1A4);
	w = tex->width;
	h = tex->height;

	/* Iterate over each 4x4 chunk in each 8x8 chunk */
	for(y2 = sOffs = 0; y2 < h; y2 += 8)
	{
		for(x2 = 0; x2 < w; x2 += 8)
		{
			for(y = 0; y < 8; y += 4)
			{
				for(x = 0; x < 8; x += 4)
				{
					dOffs = (((y2 + y) * w) + (x2 + x)) << 2;
					alph = 0xFFFFFFFFFFFFFFFF;

					if(hasA)
						alph = *(uint64_t*)(src + sOffs), sOffs += 8;
					col = *(uint64_t*)(src + sOffs), sOffs += 8;

					etc1Alpha(dst + dOffs, alph, w);
					etc1Color(dst + dOffs, col, w);
				}
			}
		}
	}

	/* Redo this later, etc1 stuff is v-flipped */
	//int x2, y2, h = t->height, w = t->width;
	for(x = 0, y = (h - 1); x < h >> 1; ++x, --y)
	{
		for(x2 = 0; x2 < w; ++x2)
		{
			y2 = ((uint32_t*)(dst))[(x * w) + x2];
			((uint32_t*)(dst))[(x * w) + x2] = ((uint32_t*)(dst))[(y * w) + x2];
			((uint32_t*)(dst))[(y * w) + x2] = y2;
		}
	}
}

void decodeImg(const uint8_t* src, uint8_t* dst, const texture_t* t)
{
	uint32_t x, ax, y, ay, px, fmt, inc, dInd, sInd;
	texRGBA8_t tex;

	if(t->isETC1)
	{
		etc1Decompress(src, dst, t);
		return;
	}

	fmt  = (t->datType << 16) | t->colFmt;
	inc  = getFmtSize(fmt);
	sInd = 0;

	for(ay = 0; ay < t->height; ay += 8)
	{
		for(ax = 0; ax < t->width; ax += 8)
		{
			for(px = 0; px < 64; ++px)
			{
				x = swizzleLUT[px] & 0x7;
				y = (swizzleLUT[px] - x) >> 3;

				dInd = (ax + x + ((t->height - 1 - (ay + y)) * t->width)) * 4;
				getColor(&(src[sInd]), &tex, fmt);
				dst[dInd + 0] = tex.B;
				dst[dInd + 1] = tex.G;
				dst[dInd + 2] = tex.R;
				dst[dInd + 3] = tex.A;

				sInd += inc;
			}
		}
	}

	return;
}

void dumpBMP(const uint8_t* texDat, const texture_t* t)
{
	bmpHeaders_t h;
	texRGBA8_t tmp;
	uint8_t* img;
	int i, size;
	char fn[21];
	FILE* f;

	strncpy(fn, t->name, 16);
	strcat(fn, ".bmp");
	f = fopen(fn, "wb");
	if(f == NULL)
	{
		perror(fn);
		return;
	}

	h.magic[0] = 'B', h.magic[1] = 'M';
	h.size = sizeof(bmpHeaders_t) + (t->width * t->height * 4);
	h.res1 = h.res2 = 0;
	h.pixOffs = sizeof(bmpHeaders_t);
	h.h2size = 40;
	h.width = t->width;
	h.height = t->height;
	h.cPlanes = 1;
	h.bitPPix = 32;
	h.compMet = 0;
	h.imgSize = h.size - sizeof(bmpHeaders_t);
	h.ppmX = 1000;
	h.ppmY = 1000;
	h.cPallet = 0;
	h.cImpt = 0;

	img = malloc(h.imgSize);
	decodeImg(texDat + t->offset, img, t);

	/* BMP format expects BGRA */
	for(i = 0, size = h.imgSize / 4; i < size; ++i)
	{
		tmp = ((texRGBA8_t*)(img))[i];
		img[(i << 2) + 0] = tmp.B;
		img[(i << 2) + 1] = tmp.G;
		img[(i << 2) + 2] = tmp.R;
		img[(i << 2) + 3] = tmp.A;
	}

	fwrite(&h, sizeof(bmpHeaders_t), 1, f);
	fwrite(img, h.imgSize, 1, f);
	fclose(f);

	return;
}
