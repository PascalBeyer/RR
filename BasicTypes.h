#ifndef RR_BASICTYPES
#define RR_BASICTYPES

#define MAXFLOAT 3.402823466e+38F
#define MINFLOAT -MAXFLOAT
#define MAXF32 3.402823466e+38F
#define MINF32 -MAXFLOAT
#define F32MAX MAXF32
#define F32MIN MINF32
#define MAXU32 ((u32)-1)

#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t	s32;
typedef int64_t	s64;
typedef s32 bool32;
typedef float f32;
typedef double f64;
typedef uintptr_t uintptr;

typedef uintptr_t umm;
typedef intptr_t smm;

struct v2
{
	float x;
	float y;
};

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

union v4
{
	struct
	{
		float a;
		float r;
		float g;
		float b;
	};

	struct
	{
		float a;
		v3 rgb;
	};
	struct
	{
		float w;
		float x;
		float y;
		float z;
	};
};

static v2 V2()
{
	v2 ret;
	ret.x = 0;
	ret.y = 0;
	return ret;
}

static v2 V2(int i)
{
	v2 ret;
	ret.x = (float)i;
	ret.y = (float)i;
	return ret;
}
static v2 V2(u32 n)
{
	v2 ret;
	ret.x = (float)n;
	ret.y = (float)n;
	return ret;
}
static v2 V2(float f)
{
	v2 ret;
	ret.x = f;
	ret.y = f;
	return ret;
}

static v2 V2(float x, float y)
{
	v2 ret;
	ret.x = x;
	ret.y = y;
	return ret;
}
static v2 V2(float x, int y)
{
	v2 ret;
	ret.x = x;
	ret.y = (float)y;
	return ret;
}
static v2 V2(int x, float y)
{
	v2 ret;
	ret.x = (float)x;
	ret.y = y;
	return ret;
}
static v2 V2(int x, int y)
{
	v2 ret;
	ret.x = (float)x;
	ret.y = (float)y;
	return ret;
}
static v2 V2(u32 x, u32 y)
{
	v2 ret;
	ret.x = (float)x;
	ret.y = (float)y;
	return ret;
}
static v2 V2(float x, u32 y)
{
	v2 ret;
	ret.x = x;
	ret.y = (float)y;
	return ret;
}
static v2 V2(u32 x, float y)
{
	v2 ret;
	ret.x = (float)x;
	ret.y = y;
	return ret;
}
static v2 operator+(v2 a, v2 b)
{
	v2 ret;
	ret.x = a.x + b.x;
	ret.y = a.y + b.y;
	return ret;
}
static v2& operator+=(v2 &a, v2 b)
{
	a = a + b;

	return a;
}
static v2 operator-(v2 a, v2 b)
{
	v2 ret;
	ret.x = a.x - b.x;
	ret.y = a.y - b.y;
	return ret;
}
static v2& operator-=(v2 &a, v2 b)
{
	a = a - b;
	return a;
}

static v2 operator*(v2 a, float f)
{
	v2 ret;
	ret.x = a.x * f;
	ret.y = a.y * f;
	return ret;
}
static v2 operator*(float f, v2 a)
{
	v2 ret;
	ret.x = a.x * f;
	ret.y = a.y * f;
	return ret;
}
static v2& operator*=(v2& a, float f)
{
	a = a * f;

	return a;
}

static v2 operator/(v2 a, float f)
{
	v2 ret;
	ret.x = a.x / f;
	ret.y = a.y / f;
	return ret;
}

static v2& operator/=(v2 &a, float f)
{
	a = a / f;
	return a;
}

static bool operator==(v2 a, v2 b)
{
	return (
		a.x == b.x &&
		a.y == b.y
		);
}

static bool operator!=(v2 a, v2 b)
{
	return !(a == b);
}

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

static v4 V4()
{
	v4 ret;

	ret.a = 0;
	ret.r = 0;
	ret.g = 0;
	ret.b = 0;
	return ret;
}

static v4 V4(float a, float x, float y, float z)
{
	v4 ret;
	ret.a = a;
	ret.r = x;
	ret.g = y;
	ret.b = z;
	return ret;
}
static v4 V4(float a, v3 rgb)
{
	v4 ret;
	ret.a = a;
	ret.rgb = rgb;
	return ret;
}

static v4 operator+(v4 a, v4 b)
{
	v4 ret;
	ret.r = a.r + b.r;
	ret.g = a.g + b.g;
	ret.a = a.a + b.a;
	ret.b = a.b + b.b;
	return ret;
}

static v4& operator+=(v4 &a, v4 b)
{
	a = a + b;
	return a;
}

static v4 operator-(v4 a, v4 b)
{
	v4 ret;
	ret.a = a.a - b.a;
	ret.r = a.r - b.r;
	ret.g = a.g - b.g;
	ret.b = a.b - b.b;
	return ret;
}

static v4& operator-=(v4 &a, v4 b)
{
	a = a - b;

	return a;
}

static v4 operator*(float f, v4 other)
{
	v4 ret;
	ret.a = f*other.a;
	ret.r = f*other.r;
	ret.g = f*other.g;
	ret.b = f*other.b;
	return ret;
}

static v4 operator*(v4 a, float f)
{
	v4 ret;
	ret.a = a.a * f;
	ret.r = a.r * f;
	ret.g = a.g * f;
	ret.b = a.b * f;
	return ret;
}

static v4& operator*=(v4 &a, float f)
{
	a = a * f;
	return a;
}

static v4 operator*(v4 a, v4 b)
{
	v4 ret;
	ret.a = a.a * b.a;
	ret.r = a.r * b.r;
	ret.g = a.g * b.g;
	ret.b = a.b * b.b;
	return ret;
}

static v4& operator*=(v4 &a, v4 b)
{
	a = a*b;
	return a;
}

static v4 operator/(v4 a, float f)
{
	v4 ret;
	ret.a = a.a / f;
	ret.r = a.r / f;
	ret.g = a.g / f;
	ret.b = a.b / f;
	return ret;
}

static v4& operator/= (v4 &a, float f)
{
	a = a / f;
	return a;
}

static bool operator==(v4 a, v4 b)
{
	return (
		a.a == b.a &&
		a.r == b.r &&
		a.g == b.g &&
		a.b == b.b
		);
}

static bool operator!=(v4 a, v4 b)
{
	return !(a == b);
}


#endif
