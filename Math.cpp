#include "Math.h"
#include "SIMD.h"
int Round(float f)
{
	return (int)(f - 1.0f / 2.0f);
}

float CapToRange(float in, float capMin, float capMax)
{
	if (in < capMin)
	{
		return capMin;
	}
	if (in > capMax)
	{
		return capMax;
	}
	return in;
}
float MapRangeToRangeCapped(float f, float minIn, float maxIn, float minOut, float maxOut)
{
	if (maxIn == minIn)
	{
		return 0.0f;
	}
	return (maxOut - minOut) / (maxIn - minIn) * (CapToRange(f, minIn, maxIn) - minIn) + minOut;
}

