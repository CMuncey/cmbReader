#ifndef CMBCONSTANTS_H
#define CMBCONSTANTS_H

enum picaDataType
{
	signed8Bit        = 0x1400,
	unsigned8Bit      = 0x1401,
	signed16Bit       = 0x1402,
	unsigned16Bit     = 0x1403,
	signed32Bit       = 0x1404,
	unsigned32Bit     = 0x1405,
	float32Bit        = 0x1406,
	UnsignedByte44DMP = 0x6760,
	Unsigned4BitsDMP  = 0x6761,
	UnsignedShort4444 = 0x8033,
	UnsignedShort5551 = 0x8034,
	UnsignedShort565  = 0x8363
};

enum picaTextureFormat
{
	RGBANativeDMP            = 0x6752,
	RGBNativeDMP             = 0x6754,
	AlphaNativeDMP           = 0x6756,
	LuminanceNativeDMP       = 0x6757,
	LuminanceAlphaNativeDMP  = 0x6758,
	ETC1RGB8NativeDMP        = 0x675A,
	ETC1AlphaRGB8A4NativeDMP = 0x675B
};

enum picaTexDatFmt
{
	ETC1     = 0x0000675A,
	ETC1A4   = 0x0000675B,
	RGBA8    = 0x14016752,
	RGB8     = 0x14016754,
	RGBA4    = 0x80336752,
	RGBA5551 = 0x80346752,
	RGB565   = 0x83636754,
	LA4      = 0x67606758,
	LA8      = 0x14016758,
	A8       = 0x14016756,
	L8       = 0x14016757,
	L4       = 0x67616757
};

enum tComResultOpDMP
{
	Add         = 0x0104,
	Replace     = 0x1E01,
	Modulate    = 0x2100,
	MulAdd      = 0x6401,
	AddMul      = 0x6402,
	Sub         = 0x84E7,
	AddSigned   = 0x8574,
	Interpolate = 0x8575,
	Dot3RGB     = 0x86AE,
	Dot3RGBA    = 0x86AF
};

enum tComSourceDMP
{
	FragPrimaryColor   = 0x6210,
	FragSecondaryColor = 0x6211,
	Texture0           = 0x84C0,
	Texture1           = 0x84C1,
	Texture2           = 0x84C2,
	Texture3           = 0x84C3,
	Constant           = 0x8576,
	PrimaryColor       = 0x8577,
	Prev               = 0x8578,
	PrevBuffer         = 0x8579
};

enum tComOpDMP
{
	SrcColor         = 0x0300,
	OneMinusSrcColor = 0x0301,
	SrcAlpha         = 0x0302,
	OneMinusSrcAlpha = 0x0303,
	SrcR             = 0x8580,
	SrcG             = 0x8581,
	SrcB             = 0x8582,
	OneMinusSrcR     = 0x8583,
	OnrMinusSrcG     = 0x8584,
	OneMinusSrcB     = 0x8585
};

#endif
