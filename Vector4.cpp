#include "Vector4.h"
#include "Math.h"
v4 V4()
{
	v4 ret;

	ret.a = 0;
	ret.r = 0;
	ret.g = 0;
	ret.b = 0;
	return ret;
}
v4 V4(float a, float x, float y, float z)
{
	v4 ret;
	ret.a = a;
	ret.r = x;
	ret.g = y;	
	ret.b = z;
	return ret;
}
v4 V4(float a, v3 rgb)
{
	v4 ret;
	ret.a = a;
	ret.rgb = rgb;
	return ret;
}

v4 operator+(v4 a, v4 b)
{
	v4 ret;
	ret.r = a.r + b.r;
	ret.g = a.g + b.g;
	ret.a = a.a + b.a;
	ret.b = a.b + b.b;
	return ret;
}
v4& operator+=(v4 &a, v4 b)
{
	a = a + b;
	return a;
}
v4 operator-(v4 a, v4 b)  
{
	v4 ret;
	ret.a = a.a - b.a;
	ret.r = a.r - b.r;
	ret.g = a.g - b.g;
	ret.b = a.b - b.b;
	return ret;
}
v4& operator-=(v4 &a, v4 b)
{
	a = a - b;

	return a;
}

v4 operator*(float f, v4 other)
{
	v4 ret;
	ret.a = f*other.a;
	ret.r = f*other.r;
	ret.g = f*other.g;
	ret.b = f*other.b;
	return ret;
}
v4 operator*(v4 a, float f)
{
	v4 ret;
	ret.a = a.a * f;
	ret.r = a.r * f;
	ret.g = a.g * f;
	ret.b = a.b * f;
	return ret;
}
v4& operator*=(v4 &a, float f)
{
	a = a * f;
	return a;
}


v4 operator*(v4 a, v4 b)
{
	v4 ret;
	ret.a = a.a * b.a;
	ret.r = a.r * b.r;
	ret.g = a.g * b.g;
	ret.b = a.b * b.b;
	return ret;
}
v4& operator*=(v4 &a, v4 b)
{
	a = a*b;
	return a;
}

v4 operator/(v4 a, float f)
{
	v4 ret;
	ret.a = a.a / f;
	ret.r = a.r / f;
	ret.g = a.g / f;
	ret.b = a.b / f;
	return ret;
}

v4& operator/= (v4 &a, float f)
{
	a = a / f;
	return a;
}

bool operator==(v4 a, v4 b)
{
	return (
		a.a == b.a &&
		a.r == b.r &&
		a.g == b.g &&
		a.b == b.b 
		);
}
bool operator!=(v4 a, v4 b)
{
	return !(a == b);
}

v4 Normalize(v4 a)
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
v4 Lerp(v4 a, float t, v4 b)
{
	return a*(1.0f - t) + b*t;
}
float Dot(v4 a, v4 b)
{
	return a.r * b.r + a.g * b.g;
}
float Norm(v4 a)
{
	return sqrtf(powf(a.r, 2) + powf(a.g, 2) + powf(a.a, 2) + powf(a.b, 2));
}
float QuadNorm(v4 a)
{
	return (powf(a.r, 2) + powf(a.g, 2) + powf(a.a, 2) + powf(a.b, 2));
}
float Dist(v4 a, v4 b)
{
	return Norm(a - b);
}
float QuadDist(v4 a, v4 b)
{
	return QuadNorm(a - b);
}

//todo get this bgra vs rgba stuff straight 
v4 Unpack4x8(u32 *pack)
{
	float a = (f32)((*pack >> 24) & 0xFF) / 255.0f;
	float b = (f32)((*pack >> 16) & 0xFF) / 255.0f;
	float g = (f32)((*pack >> 8) & 0xFF) / 255.0f;
	float r = (f32)((*pack >> 0) & 0xFF) / 255.0f;
	
	return V4(a, r, g, b);
}
u32 Pack4x8(v4 pack)
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
