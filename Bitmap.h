#ifndef RR_BITMAP
#define RR_BITMAP

#include "BasicTypes.h"
#include "buffers.h"


#pragma pack(push,1)
struct BitmapFileHeader
{
	u16 bfType;
	u32 bfSize;
	u16 bfReserved1;
	u16 bfReserved2;
	u32 bfOffBits;

	//infoheader
	u32 biSize;
	s32 biWidth;
	s32 biHeight;
	u16 biPlanes;
	u16 biBitCount;
	u32 compression;
	u32 sizeImage;
	s32 biXPerlsPerMeter;
	s32 biYPerlsPerMeter;
	u32 biClrUsed;
	u32 biCLrImpartant;

	u32 redMask;
	u32 greenMask;
	u32 blueMask;
};
#pragma pack(pop)


struct Bitmap
{
	u32 *pixels;
	u32 width;
	u32 height;
	u32 textureHandle;
	//BitmapFileHeader *header;
};


extern u32 (*AllocateGPUTexture)(u32 width, u32 height, u32 *pixels);

Bitmap CreateBitmap(u32* pixels, u32 width, u32 height);
Bitmap CreateBitmap(char *fileName);

void Free(Bitmap *bitmap);

static u32 *GetPixel(Bitmap bitmap, u32 x, u32 y)
{
	u32 shift = y * bitmap.width + x;
	u32 *pixP = bitmap.pixels + shift;
	return pixP;
}


#endif
