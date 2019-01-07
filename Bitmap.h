#ifndef RR_BITMAP
#define RR_BITMAP

#include "File.h"

#pragma pack(push, 1)
struct BitmapFileHeader
{
	u16 bfType;
	u32 bfSize;
	u16 bfReserved1;
	u16 bfReserved2;
	u32 bfOffBits;
   
	//infoheader
	u32 biSize;
	i32 biWidth;
	i32 biHeight;
	u16 biPlanes;
	u16 biBitCount;
	u32 compression;
	u32 sizeImage;
	i32 biXPerlsPerMeter;
	i32 biYPerlsPerMeter;
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

static u32 RegisterWrapingTexture(u32 width, u32 height, u32 *pixels);
static void UpdateWrapingTexture(Bitmap bitmap);

static u32 *GetPixel(Bitmap bitmap, u32 x, u32 y)
{
	Assert(x < bitmap.width && y < bitmap.height);
	u32 shift = y * bitmap.width + x;
	u32 *pixP = bitmap.pixels + shift;
	return pixP;
}


static Bitmap CreateBitmap(u32* pixels, u32 width, u32 height)
{
	Bitmap ret;
	ret.pixels = pixels;
	ret.width = width;
	ret.height = height;
	ret.textureHandle = RegisterWrapingTexture(width, height, pixels);
	return ret;
}

static Bitmap CreateBitmap(char* fileName, bool wrapping = false)
{
	Bitmap ret = {};
	//TODO: maybe check if its actually a bmp
	File tempFile = LoadFile(fileName);
	void *memPointer = tempFile.memory;
	if (!memPointer) return ret;
	ret.textureHandle = NULL;
	BitmapFileHeader *header = (BitmapFileHeader *)memPointer;
   
	ret.width = header->biWidth;
	ret.height = header->biHeight;
   
	u8 *bitMemPointer = (u8 *)memPointer;
	bitMemPointer += header->bfOffBits;
	ret.pixels = (u32 *)bitMemPointer;
   
	//switching masks
	u32 redMask = header->redMask;
	u32 blueMask = header->blueMask;
	u32 greenMask = header->greenMask;
	u32 alphaMask = ~(redMask | blueMask | greenMask);
   
	u32 redShift = BitwiseScanForward(redMask);
	u32 blueShift = BitwiseScanForward(blueMask);
	u32 greenShift = BitwiseScanForward(greenMask);
	u32 alphaShift = BitwiseScanForward(alphaMask);
   
	u32 *tempPixels = ret.pixels;
	for (i32 x = 0; x < header->biWidth; x++)
	{
		for (i32 y = 0; y < header->biHeight; y++)
		{
			u32 c = *tempPixels;
         
			u32 R = ((c >> redShift) & 0xFF);
			u32 G = ((c >> greenShift) & 0xFF);
			u32 B = ((c >> blueShift) & 0xFF);
			u32 A = ((c >> alphaShift) & 0xFF);
			float an = (A / 255.0f);
         
			R = (u32)((float)R * an);
			G = (u32)((float)G * an);
			B = (u32)((float)B * an);
			R = (u32)((float)R * an);
         
			*tempPixels++ = ((A << 24) | (R << 16) | (G << 8) | (B << 0));
		}
	}
   
	ret.textureHandle = RegisterWrapingTexture(ret.width, ret.height, ret.pixels);
	return ret;
}

static Bitmap CreateBitmap(String fileName)
{
	return CreateBitmap(ToNullTerminated(fileName));
}


#endif
