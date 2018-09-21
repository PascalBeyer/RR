#ifndef RR_FONTS
#define RR_FONTS

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#include "File.h"

struct CharInfo
{
	v2 minUV;
	v2 maxUV;
	f32 xAdvance;
	u32 width;
	u32 height;
};
typedef CharInfo CharData;

struct Font
{
	Bitmap bitmap;
	CharInfo *charData;
	u32 amountOfChars = 0;
	f32 charHeight;
};

static Font CreateFontFromSTB(u32 width, u32 height, u8 *pixels, u32 amountOfChars, f32 charHeight, stbtt_bakedchar *charData)
{
	Font ret;

	u32 *output = PushArray(constantArena, u32, width * height);
	u32 *out = output;
	u8 *inp = pixels;

	for (u32 h = 0; h < height; h++)
	{
		for (u32 w = 0; w < width; w++)
		{
			u8 source = *inp++;
			//u32 color = 0xFFFFFFFF;
			u32 color = source << 24 | source << 16 | source << 8 | source << 0;
			*out++ = color;
		}
	}

	ret.charData = PushArray(constantArena, CharData, amountOfChars);
	v2 scale = V2(1.0f / (f32)width, 1.0f / (f32)height);
	for (u32 i = 0; i < amountOfChars; i++)
	{
		stbtt_bakedchar *cur = charData + i;
		CharData data;

		data.xAdvance = cur->xadvance;
		data.minUV = scale * V2(cur->x0, cur->y0);
		data.maxUV = scale * V2(cur->x1, cur->y1);
		data.width = cur->x1 - cur->x0;
		data.height = cur->y1 - cur->y0;

		ret.charData[i] = data;
	}


	ret.bitmap = CreateBitmap(output, width, height);
	ret.amountOfChars = amountOfChars;
	ret.charHeight = charHeight;
	return ret;
}

static Font LoadFont(char *fileName)
{
	Clear(workingArena);

	File file = LoadFile(fileName);

	u32 width = 1024;
	u32 height = 1024;
	u8 *pixels = PushArray(workingArena, u8, width * height);
	
	u32 amountOfChars = 255;
	f32 charHeight = 64.0f;
	stbtt_bakedchar *charData = PushArray(workingArena, stbtt_bakedchar, amountOfChars);
	stbtt_BakeFontBitmap((u8 *)file.memory, 0, charHeight, pixels, width, height, 0, amountOfChars, charData );

	FreeFile(file);

	return CreateFontFromSTB(width, height, pixels, amountOfChars, charHeight, charData);
}

static f32 GetActualStringLength(String string, f32 fontSize, Font font)
{
	f32 ret = 0.0f;
	float fScale = fontSize / (f32)font.charHeight;

	for (u32 i = 0; i < string.length; i++)
	{
		if (string[i] < font.amountOfChars)
		{
			CharData data = font.charData[string[i]];

			f32 scaledWidth = fScale * (f32)data.width;
			float actualFloatWidth = data.xAdvance * fScale;
			ret += actualFloatWidth;

		}
		else
		{
			Assert(!"Not handled font symbol");
		}
	}

	return ret;
}

#endif