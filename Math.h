#ifndef RR_MATH
#define RR_MATH

#define PI 3.14159265359f

#include <math.h>

#include "BasicTypes.h"

struct Interval
{
	f32 min;
	f32 max;
};

struct integerPoint
{
	int x;
	int y;
};

static Interval InvertedInfinityInterval()
{
	Interval ret;
	ret.max = MINFLOAT;
	ret.min = MAXFLOAT;
	return ret;
}



static int Round(float f)
{
	return (int)(f - 0.5f);
}

static float Max(float a, float b)
{
	return _mm_cvtss_f32(_mm_max_ss(_mm_set_ss(b), _mm_set_ss(a)));
}

static float Min(float a, float b)
{
	return _mm_cvtss_f32(_mm_min_ss(_mm_set_ss(b), _mm_set_ss(a)));
}

static f32 Clamp(f32 val, f32 min, f32 max)
{
	return Min(Max(val, min), max);
}

static float MapRangeToRangeCapped(float f, float minIn, float maxIn, float minOut, float maxOut)
{
	if (maxIn == minIn)
	{
		return 0.0f;
	}
	return (maxOut - minOut) / (maxIn - minIn) * (Clamp(f, minIn, maxIn) - minIn) + minOut;
}

static float Cos(float a)
{
	return cosf(a);
}

static float Sin(float a)
{
	return sinf(a);
}

static f32 Sqrt(f32 f)
{
	return _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(f)));
}

static f32 FastInvSqrt(f32 f)
{
	return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(f)));
}

static f32 Square(f32 f)
{
	return (f*f);
}

static u32 Square(u32 f)
{
	return (f*f);
}

static float Abs(float a)
{
	return (a > 0) * a - (a < 0) * a;
}
static u32 AbsDiff(u32 a, u32 b)
{
	if (a > b)
	{
		return (a - b);
	}
	else
	{
		return (b - a);
	}
}
static u32 Min(u32 u1, u32 u2)
{
	return (u32)_mm_cvtsi128_si32(_mm_min_epi32(_mm_set1_epi32(u1), _mm_set1_epi32(u2)));
}

static u32 Max(u32 u1, u32 u2)
{
	return (u32)_mm_cvtsi128_si32(_mm_max_epi32(_mm_set1_epi32(u1), _mm_set1_epi32(u2)));
}

static float Norm(v2 a)
{
	return Sqrt(a.x * a.x + a.y * a.y);
}

static v2 Normalize(v2 a)
{
	float norm = Norm(a);
	if (norm != 0.0f)
	{
		return  a / norm;
	}
	else
	{
		return V2();
	}
}

static float Dot(v2 a, v2 b)
{
	return a.x * b.x + a.y * b.y;
}

static v2 Project(v2 a, v2 b)
{
	return V2(
		Dot(a, b) / ((float)pow(b.x, 2) + (float)pow(b.y, 2))*b.x,
		Dot(a, b) / ((float)pow(b.x, 2) + (float)pow(b.y, 2))*b.y
	);
}

static float QuadNorm(v2 a)
{
	return (powf(a.x, 2) + powf(a.y, 2));
}

static float BoxNorm(v2 a)
{
	return fmaxf(fabsf(a.x), fabsf(a.y));
}

static float BoxDist(v2 a, v2 b)
{
	return BoxNorm(a - b);
}

static float Dist(v2 a, v2 b)
{
	return Norm(a - b);
}

static float QuadDist(v2 a, v2 b)
{
	return QuadNorm(a - b);
}

static float AngleBetween(v2 a, v2 b)
{
	return (float)(atan2(b.y, b.x) - atan2(a.y, a.x));
}

static v2 PerpendicularVector(v2 a) //rotates 90° to the left
{
	return V2(-a.y, a.x);
}

static v2 RotateAroundOrigin(v2 vec, float angle)
{
	v2 temp = V2(vec.x, vec.y);

	vec.x = temp.x * cosf(angle) - temp.y *sinf(angle);
	vec.y = temp.y * cosf(angle) + temp.x *sinf(angle);

	return vec;
}


static v2 RotateAround(v2 origin, float angle, v2 v)
{
	v2 ret;

	ret.x = (v.x - origin.x)*cosf(angle) - (v.y - origin.y) * sinf(angle) + origin.x;
	ret.y = (v.y - origin.y)*cosf(angle) + (v.x - origin.x) * sinf(angle) + origin.y;

	return ret;
}

static v2 LerpVector2(v2 p1, float f1, v2 p2, float f2, float t)
{
	//Assert(f1 != f2);
	return (p1 + (t - f1) / (f2 - f1) * p2);
}

static v2 Min(v2 a, v2 b)
{
	return V2(Min(a.x, b.x), Min(a.y, b.y));
}

static v2 Max(v2 a, v2 b)
{
	return V2(Max(a.x, b.x), Max(a.y, b.y));
}


static float Dot(v3 a, v3 b)
{
	return
		a.x * b.x +
		a.y * b.y +
		a.z * b.z;
}

static float BoxNorm(v3 a)
{
	return Max(a.x, Max(a.x, a.y));
}

static float Norm(v3 a)
{
	return Sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
}

static v3 Normalize(v3 a)
{
	float norm = Norm(a);
	if (norm)
	{
		return a * (1.0f / norm);
	}
	else
	{
		return v3();
	}

}

static v3 FastNormalize(v3 a)
{
	return a * FastInvSqrt(a.x * a.x + a.y * a.y + a.z * a.z);
}

static v3 Lerp(v3 a, float t, v3 b)
{
	return a*(1.0f - t) + b*t;
}

static float Dist(v3 a, v3 b)
{
	return Norm(a - b);
}
static v3 i12(v2 a)
{
	return V3(a.x, a.y, 0);
}
static v3 i12(float ax, float ay)
{
	return V3(ax, ay, 0);
}

static v3 CrossProduct(v3 a, v3 b)
{
	return V3(
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	);
}

static float QuadNorm(v3 a)
{
	return ((a.x * a.x) + (a.y * a.y) + (a.z * a.z));
}
static float NormSquared(v3 a)
{
	return ((a.x * a.x) + (a.y * a.y) + (a.z * a.z));
}
static float QuadDist(v3 a, v3 b)
{
	return QuadNorm(a - b);
}


static v3 LerpVector3(v3 v1, v3 v2, float t)
{
	return t * v2 + (1.0f - t) * v1;
}
static v3 LerpVector3(v3 v1, float f1, v3 v2, float f2, float t)
{
	//Assert(f1 != f2);
	return (v1 + (t - f1) / (f2 - f1) * v2);
}

static v3 Unpack3x8(u32 *pack)
{
	float ignored = (f32)((*pack >> 24) & 0xFF) / 255.0f;
	float r = (f32)((*pack >> 16) & 0xFF) / 255.0f;
	float g = (f32)((*pack >> 8) & 0xFF) / 255.0f;
	float b = (f32)((*pack >> 0) & 0xFF) / 255.0f;
	return V3(r, g, b);
}

static v3 Unpack3x8(u32 pack)
{
	float ignored = (f32)((pack >> 24) & 0xFF) / 255.0f;
	float b = (f32)((pack >> 16) & 0xFF) / 255.0f;
	float g = (f32)((pack >> 8) & 0xFF) / 255.0f;
	float r = (f32)((pack >> 0) & 0xFF) / 255.0f;
	return V3(r, g, b);
}

static u32 Pack3x8(v3 pack)
{
	float a = (1.0f * 255.0f);
	float r = (pack.r * 255.0f);
	float g = (pack.g * 255.0f);
	float b = (pack.b * 255.0f);

	return (((u32)(a + 0.5f) << 24) |
		((u32)(b + 0.5f) << 16) |
		((u32)(g + 0.5f) << 8) |
		((u32)(r + 0.5f) << 0));

}
static u32 SavePack3x8(v3 pack)
{
	float a = (1.0f * 255.0f);
	float r = (Min(pack.r, 1.0f) *255.0f);
	float g = (Min(pack.g, 1.0f)* 255.0f);
	float b = (Min(pack.b, 1.0f)* 255.0f);

	return (((u32)(a + 0.5f) << 24) |
		((u32)(b + 0.5f) << 16) |
		((u32)(g + 0.5f) << 8) |
		((u32)(r + 0.5f) << 0));
}

static v3 Zinversion(v3 v)
{
	return V3(v.x, v.y, -v.z);
}

static float Norm(v4 a)
{
	return Sqrt(a.x * a.x + a.y * a.y + a.z * a.z + a.w * a.w);
}

static v4 Normalize(v4 a)
{
	float norm = Norm(a);
	if (norm)
	{
		return a * (1.0f / norm);
	}
	else
	{
		return V4();
	}
}

static v4 Lerp(v4 a, float t, v4 b)
{
	return a*(1.0f - t) + b*t;
}

static float Dot(v4 a, v4 b)
{
	return a.r * b.r + a.g * b.g;
}

static float QuadNorm(v4 a)
{
	return (powf(a.r, 2) + powf(a.g, 2) + powf(a.a, 2) + powf(a.b, 2));
}

static float Dist(v4 a, v4 b)
{
	return Norm(a - b);
}

static float QuadDist(v4 a, v4 b)
{
	return QuadNorm(a - b);
}

//RGBA
static v4 Unpack4x8(u32 *pack)
{
	float a = (f32)((*pack >> 24) & 0xFF) / 255.0f;
	float b = (f32)((*pack >> 16) & 0xFF) / 255.0f;
	float g = (f32)((*pack >> 8) & 0xFF) / 255.0f;
	float r = (f32)((*pack >> 0) & 0xFF) / 255.0f;

	return V4(a, r, g, b);
}

static v4 Unpack4x8(u32 pack)
{
	float a = (f32)((pack >> 24) & 0xFF) / 255.0f;
	float b = (f32)((pack >> 16) & 0xFF) / 255.0f;
	float g = (f32)((pack >> 8) & 0xFF) / 255.0f;
	float r = (f32)((pack >> 0) & 0xFF) / 255.0f;

	return V4(a, r, g, b);
}

static u32 Pack4x8(v4 pack)
{
	float a = (pack.a * 255.0f);
	float r = (pack.r * 255.0f);
	float g = (pack.g * 255.0f);
	float b = (pack.b * 255.0f);

	return (((u32)(a + 0.5f) << 24) |
		((u32)(b + 0.5f) << 16) |
		((u32)(g + 0.5f) << 8) |
		((u32)(r + 0.5f) << 0));

}


#endif

