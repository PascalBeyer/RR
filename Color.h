#ifndef RR_COLOR
#define RR_COLOR

#include "BasicTypes.h"


static u32 ColorspaceToggle(u32 color)

{
	u32 a = (color >> 24 & 0xFF);
	u32 rOrb = (color >> 16) & 0xFF;
	u32 g = (color >> 8) & 0xFF;
	u32 bOrr = (color >> 0) & 0xFF;

	u32 ret = (a << 24) | (bOrr << 16) | (g << 8) | (rOrb << 0);

}

static u32 RGBAfromHEX(u32 pack)
{
	u32 a = 0xFF;
	u32 r = (pack >> 16) & 0xFF;
	u32 g = (pack >> 8) & 0xFF;
	u32 b = (pack >> 0) & 0xFF;

	u32 ret = (a << 24) | (b << 16) | (g << 8) | (r << 0);

	return ret;
}

static v4 ColorspaceToggle(v4 color)
{
	v4 ret;
	ret.a = color.a;
	ret.r = color.b;
	ret.b = color.r;
	ret.g = color.g;
	return ret;
}


static v3 ColorspaceToggle(v3 color)
{
	v3 ret;
	ret.r = color.b;
	ret.b = color.r;
	ret.g = color.g;
	return ret;
}


#endif // !RR_COLOR

