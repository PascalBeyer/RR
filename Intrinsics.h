#ifndef RR_INTRINSICS
#define RR_INTRINSICS

#define IFloat __m128
#define IInt __m128i
#define IMulF(a,b) _mm_mul_ps(a,b)
#define IAddF(a,b) _mm_add_ps(a,b)
#define ISubF(a,b) _mm_sub_ps(a,b)
#define IConvertIntToF(a) _mm_cvtepi32_ps(a)
#define ISetInt1(a)	_mm_set1_epi32(a)
#define ISetF1(a) _mm_set1_ps(a)
#define ISetInt(a, b, c, d)	_mm_setr_epi32(a, b, c, d)
#define ISetF(a, b, c, d) _mm_setr_ps(a, b, c, d)
#define ITruncate(a) _mm_cvttps_epi32(a)
#define IRound(a) _mm_cvtps_epi32(a)
#define ILoadInt(a) _mm_loadu_si128((__m128i *)(a))
#define IShiftRight(a, shift) _mm_srli_epi32(a, shift)
#define IShiftLeft(a, shift) _mm_slli_epi32(a, shift)
#define IAndF(a, b) _mm_and_ps(a, b)
#define IAndNotF(a, b) _mm_andnot_ps(a, b)
#define IOrF(a,b) _mm_or_ps(a, b)
#define ICastFToInt(a) _mm_castps_si128(a)
#define IMinF(a, b) _mm_min_ps(a, b)
#define IMaxF(a, b) _mm_max_ps(a, b)
#define IAndInt(a, b) _mm_and_si128(a, b)
#define IAndNotInt(a, b) _mm_andnot_si128(a, b)
#define IOrInt(a, b) _mm_or_si128(a, b)
#define IStoreInt(a, b) _mm_storeu_si128((__m128i *)a, b)
#define IRootF(a) _mm_sqrt_ps(a)



#include "BasicTypes.h"
#include <math.h>

static float SquareRoot(float f)
{
	return sqrtf(f);
}


static u32 BitScanForward(u32 value)
{
	unsigned long result = 32;
#if COMPILER_MSVC
	if (!_BitScanForward(&result, value))
		result = 32;
#else


	for (int i = 0; i < 32; i++)
	{
		if (value & 1 << i)
		{
			result = i;
			break;
		}
	}
#endif
	return (u32)result;

}

#endif
