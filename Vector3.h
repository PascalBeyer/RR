#ifndef RR_VECTOR3
#define RR_VECTOR3
#include "Vector2.h"
#include "Math.h"

union v3
{
	struct
	{
		float x, y, z;
	};
	struct
	{
		float r, g, b;
	};
	struct
	{
		v2 xy;
		float z;
	};
	struct
	{		
		float x;
		v2 yz;
	};	
	float v[3];
};

static v3 V3()
{
	v3 ret;
	ret.x = 0;
	ret.y = 0;
	ret.z = 0;
	return ret;
}
static v3 V3(float _x)
{
	v3 ret;
	ret.x = _x;
	ret.y = _x;
	ret.z = _x;
	return ret;
}

static v3 V3(float _x, float _y, float _z)
{
	v3 ret;
	ret.x = _x;
	ret.y = _y;
	ret.z = _z; return ret;
	return ret;
}
static v3 V3(int _x, int _y, int _z)
{
	v3 ret;
	ret.x = (float)_x;
	ret.y = (float)_y;
	ret.z = (float)_z; return ret;
}
static v3 V3(float _x, float _y, int _z)
{
	v3 ret;
	ret.x = _x;
	ret.y = _y;
	ret.z = (float)_z; return ret;
}
static v3 V3(float _x, int _y, float _z)
{
	v3 ret;
	ret.x = _x;
	ret.y = (float)_y;
	ret.z = _z; return ret;
}
static v3 V3(int _x, float _y, float _z)
{
	v3 ret;
	ret.x = (float)_x;
	ret.y = _y;
	ret.z = _z; return ret;
}

static v3 V3(int _x, float _y, int _z)
{
	v3 ret;
	ret.x = (float)_x;
	ret.y = _y;
	ret.z = (float)_z; return ret;
}
static v3 V3(float _x, int _y, int _z)
{
	v3 ret;
	ret.x = _x;
	ret.y = (float)_y;
	ret.z = (float)_z; return ret;
}
static v3 V3(int _x, int _y, float _z)
{
	v3 ret;
	ret.x = (float)_x;
	ret.y = (float)_y;
	ret.z = _z; return ret;
}
static v3 V3(u32 _x, u32 _y, u32 _z)
{
	v3 ret;
	ret.x = (float)_x;
	ret.y = (float)_y;
	ret.z = (float)_z; return ret;
}
static v3 V3(float _x, float _y, u32 _z)
{
	v3 ret;
	ret.x = _x;
	ret.y = _y;
	ret.z = (float)_z; return ret;
}
static v3 V3(float _x, u32 _y, float _z)
{
	v3 ret;
	ret.x = _x;
	ret.y = (float)_y;
	ret.z = _z; return ret;
}
static v3 V3(u32 _x, float _y, float _z)
{
	v3 ret;
	ret.x = (float)_x;
	ret.y = _y;
	ret.z = _z; return ret;
}
static v3 V3(u32 _x, float _y, u32 _z)
{
	v3 ret;
	ret.x = (float)_x;
	ret.y = (float)_y;
	ret.z = (float)_z; return ret;
}
static v3 V3(float _x, u32 _y, u32 _z)
{
	v3 ret;
	ret.x = (float)_x;
	ret.y = (float)_y;
	ret.z = (float)_z; return ret;
}
static v3 V3(u32 _x, u32 _y, float _z)
{
	v3 ret;
	ret.x = (float)_x;
	ret.y = (float)_y;
	ret.z = (float)_z; return ret;
}
static v3 V3(int _x, int _y, u32 _z)
{
	v3 ret;
	ret.x = (float)_x;
	ret.y = (float)_y;
	ret.z = (float)_z; return ret;
}
static v3 V3(int _x, u32 _y, int _z)
{
	v3 ret;
	ret.x = (float)_x;
	ret.y = (float)_y;
	ret.z = (float)_z; return ret;
}
static v3 V3(u32 _x, int _y, int _z)
{
	v3 ret;
	ret.x = (float)_x;
	ret.y = (float)_y;
	ret.z = (float)_z; return ret;
}
static v3 V3(u32 _x, int _y, u32 _z)
{
	v3 ret;
	ret.x = (float)_x;
	ret.y = (float)_y;
	ret.z = (float)_z; return ret;
}
static v3 V3(int _x, u32 _y, u32 _z)
{
	v3 ret;
	ret.x = (float)_x;
	ret.y = (float)_y;
	ret.z = (float)_z;
	return ret;
}
static v3 V3(u32 _x, u32 _y, int _z)
{
	v3 ret;
	ret.x = (float)_x;
	ret.y = (float)_y;
	ret.z = (float)_z;
	return ret;
}
static v3 V3(v2 xy, float z)
{
	v3 ret;
	ret.x = xy.x;
	ret.y = xy.y;
	ret.z = z;
	return ret;
}
static v3 V3(float x, v2 yz)
{
	v3 ret;
	ret.x = x;
	ret.yz = yz;
	return ret;
}

static v3 operator+(v3 a, v3 b)
{
	v3 ret;
	ret.x = a.x + b.x;
	ret.y = a.y + b.y;
	ret.z = a.z + b.z;
	return ret;
}
static v3& operator+=(v3 &a, v3 b)
{
	a = a + b;
	return a;
}
static v3 operator-(v3 a, v3 b)
{
	v3 ret;
	ret.x = a.x - b.x;
	ret.y = a.y - b.y;
	ret.z = a.z - b.z;
	return ret;
}
static v3& operator-=(v3 &a, v3 b)
{
	a = a - b;
	return a;
}
static v3 operator*(v3 a, float f)
{
	v3 ret;
	ret.x = a.x * f;
	ret.y = a.y * f;
	ret.z = a.z * f;
	return ret;
}
static v3& operator*=(v3& a, float f)
{
	a = a * f;
	return a;
}
static v3 operator*(v3 a, v3 b)
{
	v3 ret;
	ret.x = a.x * b.x;
	ret.y = a.y * b.y;
	ret.z = a.z * b.z;
	return ret;
}
static v3& operator*=(v3 &a, v3 b)
{
	a = a + b;
	return a;
}

static v3 operator/(v3 a, float f)
{
	v3 ret;
	ret.x = a.x / f;
	ret.y = a.y / f;
	ret.z = a.z / f;
	return ret;
}
static v3& operator/=(v3& a, float f)
{
	a = a / f;
	return a;
}
static bool operator==(v3 a, v3 b)
{
	return (
		a.x == b.x &&
		a.y == b.y &&
		a.z == b.z
		);
}
static bool operator!=(v3 a, v3 b)
{
	return !(a == b);
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

static v3 operator*(float scalar, v3 a)
{
	v3 ret;
	ret.x = scalar * a.x;
	ret.y = scalar * a.y;
	ret.z = scalar * a.z;
	return ret;
}
static v3 operator-(v3 a)
{
	return { -a.x, -a.y, -a.z };
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
	float r = (f32)((pack >> 16) & 0xFF) / 255.0f;
	float g = (f32)((pack >> 8) & 0xFF) / 255.0f;
	float b = (f32)((pack >> 0) & 0xFF) / 255.0f;
	return V3(r, g, b);
}

static u32 Pack3x8(v3 pack)
{
	float a = (1.0f * 255.0f);
	float r = (pack.r * 255.0f);
	float g = (pack.g * 255.0f);
	float b = (pack.b * 255.0f);

	return (((u32)(a + 0.5f) << 24) |
		((u32)(r + 0.5f) << 16) |
		((u32)(g + 0.5f) << 8) |
		((u32)(b + 0.5f) << 0));

}
static u32 SavePack3x8(v3 pack)
{
	float a = (1.0f * 255.0f);
	float r = (Min(pack.r, 1.0f) *255.0f);
	float g = (Min(pack.g, 1.0f)* 255.0f);
	float b = (Min(pack.b, 1.0f)* 255.0f);

	return (((u32)(a + 0.5f) << 24) |
		((u32)(r + 0.5f) << 16) |
		((u32)(g + 0.5f) << 8) |
		((u32)(b + 0.5f) << 0));
}

static v3 Zinversion(v3 v)
{
	return V3(v.x, v.y, -v.z);
}

//v2 OrthorgonalProject(const v3& a, const v3 &b);
#endif