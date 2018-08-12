#ifndef RR_MATH
#define RR_MATH

#define PI 3.14159265359f


#include "buffers.h"
#include <math.h>

struct Interval
{
	f32 min;
	f32 max;
};

static Interval InvertedInfinityInterval()
{
	Interval ret;
	ret.max = MINFLOAT;
	ret.min = MAXFLOAT;
	return ret;
}


struct integerPoint
{
	int x;
	int y;
};

int Round(float f);

float Min(float a, float b);
float Max(float a, float b);


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

float MapRangeToRangeCapped(float f, float minIn, float maxIn, float minOut, float maxOut);
float CapToRange(float in, float capMin, float capMax);


#endif

