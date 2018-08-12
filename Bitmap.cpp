#include "Bitmap.h"

#include "Intrinsics.h"
#include "File.h"



void *AllocateGPUTextureStub(u32 width, u32 height, u32 *pixels)
{
	return NULL;
}

//void *(*AllocateGPUTexture)(u32 width, u32 height, u32 *pixels) = &AllocateGPUTextureStub;

Bitmap CreateBitmap(u32* pixels, u32 width, u32 height)
{
	Bitmap ret;
	ret.pixels = pixels;
	ret.width = width;
	ret.height = height;
	ret.textureHandle = AllocateGPUTexture(width, height, pixels);
	return ret;
}

Bitmap CreateBitmap(char* fileName)
{
	Bitmap ret;
	//TODO: maybe check if its actually a bmp
	File tempFile = File(fileName);
	void * memPointer = tempFile.GetMemory();
	ret.textureHandle = NULL;
	BitmapFileHeader *header = (BitmapFileHeader *)memPointer;

	ret.width = header->biWidth;
	ret.height =header->biHeight;

	u8 *bitMemPointer = (u8*)memPointer;
	bitMemPointer+= header->bfOffBits;
	ret.pixels = (u32*)bitMemPointer;

	//switching masks
	u32 redMask = header->redMask;
	u32 blueMask = header->blueMask;
	u32 greenMask = header->greenMask;
	u32 alphaMask = ~(redMask | blueMask | greenMask);

	u32 redShift = BitScanForward(redMask);
	u32 blueShift = BitScanForward(blueMask);
	u32 greenShift = BitScanForward(greenMask);
	u32 alphaShift = BitScanForward(alphaMask);

	u32 *tempPixels = ret.pixels;
	for (s32 x = 0; x < header->biWidth; x++)
	{
		for (s32 y = 0; y < header->biHeight; y++)
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
	ret.textureHandle = AllocateGPUTexture(ret.width, ret.height, ret.pixels);
	return ret;
}


void Free(Bitmap *bitmap)
{
	Assert(!"stubbed");

	//u32 memorySize = header->biSize + height * width * sizeof(u32);
	//File file = File(header, memorySize);
	//file.Free();
	//bitmap->pixels = NULL;
	//width = 0;
	//height = 0;
}
