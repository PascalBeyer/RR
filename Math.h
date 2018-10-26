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
static u32 Abs(i32 a)
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
static f32 Dist(f32 a, f32 b)
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
static i32 Max(i32 u1, i32 u2)
{
	return _mm_cvtsi128_si32(_mm_max_epi32(_mm_set1_epi32(u1), _mm_set1_epi32(u2)));
}

static u32 SaveSubstract(u32 from, u32 b)
{
	return (from > b) ? (from - b) : 0;
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
	if (f1 == f2) return v1;
	return (v1 + (t - f1) / (f2 - f1) * (v2 - v1));
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


struct Quaternion
{
	union
	{
		struct
		{
			f32 w, x, y, z;
		};
		struct
		{
			f32 r;
			v3 v;
		};
	};
};

static Quaternion QuaternionFromAngleAxis(f32 angle, v3 rotationAxis)
{
	v3 normalized = Normalize(rotationAxis);
	Quaternion ret;
	ret.r = Cos(angle / 2.0f);
	ret.v = Sin(angle / 2.0f) * normalized;
	return ret;
}

static Quaternion operator*(Quaternion a, Quaternion b)
{
	Quaternion ret;
	ret.r = a.r * b.r - Dot(a.v, b.v);
	ret.v = a.r * b.v + b.r * a.v + CrossProduct(a.v, b.v);
	return ret;
}
static Quaternion operator+(Quaternion a, Quaternion b)
{
	Quaternion ret;
	ret.w = a.w + b.w;
	ret.x = a.x + b.x;
	ret.y = a.y + b.y;
	ret.z = a.z + b.z;
	return ret;
}

static Quaternion operator*(Quaternion a, f32 b)
{
	Quaternion ret;
	ret.w = a.w * b;
	ret.x = a.x * b;
	ret.y = a.y * b;
	ret.z = a.z * b;
	return ret;
}

static Quaternion operator*(f32 b, Quaternion a)
{
	Quaternion ret;
	ret.w = a.w * b;
	ret.x = a.x * b;
	ret.y = a.y * b;
	ret.z = a.z * b;
	return ret;
}

static Quaternion operator/(Quaternion a, f32 b)
{
	Quaternion ret;
	ret.w = a.w / b;
	ret.x = a.x / b;
	ret.y = a.y / b;
	ret.z = a.z / b;
	return ret;
}

static Quaternion operator/(f32 b, Quaternion a)
{
	Quaternion ret;
	ret.w = a.w / b;
	ret.x = a.x / b;
	ret.y = a.y / b;
	ret.z = a.z / b;
	return ret;
}

static f32 Norm(Quaternion a)
{
	return Sqrt(a.w * a.w + a.x * a.x + a.y * a.y + a.z * a.z);
}

static Quaternion Conjugate(Quaternion a)
{
	Quaternion ret;
	ret.w = a.w;
	ret.x = -a.x;
	ret.y = -a.y;
	ret.z = -a.z;
	return ret;
}

static Quaternion Inverse(Quaternion a)
{
	Quaternion ret = Conjugate(a);
	f32 norm = Norm(a);
	ret = ret / (norm * norm);
	return ret;
}

struct m4x4
{
	float a[4][4];
};


static m4x4 operator*(m4x4 A, m4x4 B)
{
	m4x4 R = {};

	for (int r = 0; r <= 3; ++r)
	{
		for (int c = 0; c <= 3; ++c)
		{
			for (int i = 0; i <= 3; ++i)
			{
				R.a[r][c] += A.a[r][i] * B.a[i][c];
			}
		}
	}
	return R;
}

static v3 Transform(m4x4 A, v3 P, float Pw)
{
	v3 r;

	r.x = P.x*A.a[0][0] + P.y*A.a[0][1] + P.z*A.a[0][2] + Pw*A.a[0][3];
	r.y = P.x*A.a[1][0] + P.y*A.a[1][1] + P.z*A.a[1][2] + Pw*A.a[1][3];
	r.z = P.x*A.a[2][0] + P.y*A.a[2][1] + P.z*A.a[2][2] + Pw*A.a[2][3];
	f32 w = P.x*A.a[3][0] + P.y*A.a[3][1] + P.z*A.a[3][2] + Pw*A.a[3][3];

	r /= w;

	return r;
}

static v3 operator*(m4x4 A, v3 P)
{
	v3 R = Transform(A, P, 1.0f);
	return(R);
}

static m4x4 Identity(void)
{
	m4x4 R =
	{
		{ { 1, 0, 0, 0 },
		{ 0, 1, 0, 0 },
		{ 0, 0, 1, 0 },
		{ 0, 0, 0, 1 } },
	};

	return(R);
}


static m4x4 Translation(v3 a)
{
	m4x4 R =
	{
		{ { 1, 0, 0, a.x },
		{ 0, 1, 0, a.y },
		{ 0, 0, 1, a.z },
		{ 0, 0, 0, 1 } },
	};

	return(R);
}

static m4x4 XRotation(float Angle)
{
	float c = cosf(Angle);
	float s = sinf(Angle);

	m4x4 R =
	{
		{ { 1, 0, 0, 0 },
		{ 0, c,-s, 0 },
		{ 0, s, c, 0 },
		{ 0, 0, 0, 1 } },
	};

	return(R);
}

static m4x4 YRotation(float Angle)
{
	float c = cosf(Angle);
	float s = sinf(Angle);

	m4x4 R =
	{
		{ { c, 0, s, 0 },
		{ 0, 1, 0, 0 },
		{ -s, 0, c, 0 },
		{ 0, 0, 0, 1 } },
	};

	return(R);
}

static m4x4 ZRotation(float Angle)
{
	float c = cosf(Angle);
	float s = sinf(Angle);

	m4x4 R =
	{
		{
			{ c,-s, 0, 0 },
			{ s, c, 0, 0 },
			{ 0, 0, 1, 0 },
			{ 0, 0, 0, 1 }
		},
	};

	return(R);
}

static m4x4 Transpose(m4x4 A)
{
	m4x4 R;

	for (int j = 0; j <= 3; ++j)
	{
		for (int i = 0; i <= 3; ++i)
		{
			R.a[j][i] = A.a[i][j];
		}
	}

	return(R);
}

static m4x4 Projection(float aspectWidthOverHeight, float focalLength)
{
	float a = 1.0f;
	float b = aspectWidthOverHeight;
	float c = focalLength;

	float nearClipPlaneDist = 0.1f;
	float farClipPlaneDist = 100.0f;

	float d = 2.0f / (nearClipPlaneDist - farClipPlaneDist);
	float e = (nearClipPlaneDist + farClipPlaneDist) / (nearClipPlaneDist - farClipPlaneDist);

	m4x4 R =
	{
		{
			{ a * c,	 0,			0,			0 },
			{ 0,		-b * c,		0,			0 },
			{ 0,		0,			d,			e },
			{ 0,		0,			1.0f,		0 }
		}
	};

	return(R);
}

static m4x4 Columns3x3(v3 X, v3 Y, v3 Z)
{
	m4x4 R =
	{
		{ { X.x, Y.x, Z.x, 0 },
		{ X.y, Y.y, Z.y, 0 },
		{ X.z, Y.z, Z.z, 0 },
		{ 0,   0,   0, 1 } }
	};

	return(R);
}

static m4x4 Rows3x3(v3 X, v3 Y, v3 Z)
{
	m4x4 R =
	{
		{ { X.x, X.y, X.z, 0 },
		{ Y.x, Y.y, Y.z, 0 },
		{ Z.x, Z.y, Z.z, 0 },
		{ 0,   0,   0, 1 } }
	};

	return(R);
}

static m4x4 Translate(m4x4 A, v3 T)
{
	m4x4 R = A;

	R.a[0][3] += T.x;
	R.a[1][3] += T.y;
	R.a[2][3] += T.z;

	return(R);
}

static v3 GetColumn(m4x4 A, u32 C)
{
	v3 R = { A.a[0][C], A.a[1][C], A.a[2][C] };
	return(R);
}

static v3 GetRow(m4x4 A, u32 R)
{
	v3 Result = { A.a[R][0], A.a[R][1], A.a[R][2] };
	return(Result);
}

static m4x4 CameraTransform(v3 X, v3 Y, v3 Z, v3 P)
{
	m4x4 R = Rows3x3(X, Y, Z);
	R = Translate(R, -(R*P));
	return(R);
}

static m4x4 Orthogonal(float width, float height)
{
	m4x4 R =
	{
		{
			{ 2.0f / width,			0.0f,			0.0f, 0.0f },
			{ 0.0f,					-2.0f / height, 0.0f, 0.0f },
			{ 0.0f,					0.0f,			1.0f, -1.0f }, //we are in a -1 to +1 cube, so -1 is the exact screen position
			{ 0.0f,					0.0f,			0.0f, 1.0f }
		}
	};

	return(R);
}

static m4x4 InvOrId(m4x4 mat)
{

	f32 *m = &mat.a[0][0];

	f32 inv[16], det;

	inv[0] = m[5] * m[10] * m[15] -
		m[5] * m[11] * m[14] -
		m[9] * m[6] * m[15] +
		m[9] * m[7] * m[14] +
		m[13] * m[6] * m[11] -
		m[13] * m[7] * m[10];

	inv[4] = -m[4] * m[10] * m[15] +
		m[4] * m[11] * m[14] +
		m[8] * m[6] * m[15] -
		m[8] * m[7] * m[14] -
		m[12] * m[6] * m[11] +
		m[12] * m[7] * m[10];

	inv[8] = m[4] * m[9] * m[15] -
		m[4] * m[11] * m[13] -
		m[8] * m[5] * m[15] +
		m[8] * m[7] * m[13] +
		m[12] * m[5] * m[11] -
		m[12] * m[7] * m[9];

	inv[12] = -m[4] * m[9] * m[14] +
		m[4] * m[10] * m[13] +
		m[8] * m[5] * m[14] -
		m[8] * m[6] * m[13] -
		m[12] * m[5] * m[10] +
		m[12] * m[6] * m[9];

	inv[1] = -m[1] * m[10] * m[15] +
		m[1] * m[11] * m[14] +
		m[9] * m[2] * m[15] -
		m[9] * m[3] * m[14] -
		m[13] * m[2] * m[11] +
		m[13] * m[3] * m[10];

	inv[5] = m[0] * m[10] * m[15] -
		m[0] * m[11] * m[14] -
		m[8] * m[2] * m[15] +
		m[8] * m[3] * m[14] +
		m[12] * m[2] * m[11] -
		m[12] * m[3] * m[10];

	inv[9] = -m[0] * m[9] * m[15] +
		m[0] * m[11] * m[13] +
		m[8] * m[1] * m[15] -
		m[8] * m[3] * m[13] -
		m[12] * m[1] * m[11] +
		m[12] * m[3] * m[9];

	inv[13] = m[0] * m[9] * m[14] -
		m[0] * m[10] * m[13] -
		m[8] * m[1] * m[14] +
		m[8] * m[2] * m[13] +
		m[12] * m[1] * m[10] -
		m[12] * m[2] * m[9];

	inv[2] = m[1] * m[6] * m[15] -
		m[1] * m[7] * m[14] -
		m[5] * m[2] * m[15] +
		m[5] * m[3] * m[14] +
		m[13] * m[2] * m[7] -
		m[13] * m[3] * m[6];

	inv[6] = -m[0] * m[6] * m[15] +
		m[0] * m[7] * m[14] +
		m[4] * m[2] * m[15] -
		m[4] * m[3] * m[14] -
		m[12] * m[2] * m[7] +
		m[12] * m[3] * m[6];

	inv[10] = m[0] * m[5] * m[15] -
		m[0] * m[7] * m[13] -
		m[4] * m[1] * m[15] +
		m[4] * m[3] * m[13] +
		m[12] * m[1] * m[7] -
		m[12] * m[3] * m[5];

	inv[14] = -m[0] * m[5] * m[14] +
		m[0] * m[6] * m[13] +
		m[4] * m[1] * m[14] -
		m[4] * m[2] * m[13] -
		m[12] * m[1] * m[6] +
		m[12] * m[2] * m[5];

	inv[3] = -m[1] * m[6] * m[11] +
		m[1] * m[7] * m[10] +
		m[5] * m[2] * m[11] -
		m[5] * m[3] * m[10] -
		m[9] * m[2] * m[7] +
		m[9] * m[3] * m[6];

	inv[7] = m[0] * m[6] * m[11] -
		m[0] * m[7] * m[10] -
		m[4] * m[2] * m[11] +
		m[4] * m[3] * m[10] +
		m[8] * m[2] * m[7] -
		m[8] * m[3] * m[6];

	inv[11] = -m[0] * m[5] * m[11] +
		m[0] * m[7] * m[9] +
		m[4] * m[1] * m[11] -
		m[4] * m[3] * m[9] -
		m[8] * m[1] * m[7] +
		m[8] * m[3] * m[5];

	inv[15] = m[0] * m[5] * m[10] -
		m[0] * m[6] * m[9] -
		m[4] * m[1] * m[10] +
		m[4] * m[2] * m[9] +
		m[8] * m[1] * m[6] -
		m[8] * m[2] * m[5];

	det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

	if (det == 0)
	{
		return Identity();
	}


	f32 invDet = 1.0f / det;

	m4x4 ret;

	for (u32 i = 0; i < 4; i++)
	{
		for (u32 j = 0; j < 4; j++)
		{
			ret.a[i][j] = inv[4 * i + j] * invDet;
		}
	}


	return ret;

}


static m4x4 QuaternionToMatrix(Quaternion a)
{

	f32 a2 = a.w * a.w, b2 = a.x * a.x, c2 = a.y * a.y, d2 = a.z * a.z;
	f32 ab = a.w * a.x, ac = a.w * a.y, ad = a.w * a.z;
	f32 bc = a.x * a.y, bd = a.x * a.z;
	f32 cd = a.y * a.z;
	
	f32 a11 = a2 + b2 - c2 - d2;
	f32 a12 = 2 * (bc - ad);
	f32 a13 = 2 * (bd + ac);

	f32 a21 = 2 * (bc + ad);
	f32 a22 = a2 - b2 + c2 - d2;
	f32 a23 = 2 * (cd - ab);

	f32 a31 = 2 * (bd - ac);
	f32 a32 = 2 * (cd + ab);
	f32 a33 = a2 - b2 - c2 + d2;

	//f32 a11 = a.x * a.x + a.y * a.y + a.w * a.w + a.z * a.z;
	//f32 a12 = 2.0f * (a.x * a.y - a.w * a.z);
	//f32 a13 = 2.0f * (a.w * a.y - a.x * a.z);
	//
	//f32 a21 = 2.0f * (a.x * a.y + a.w * a.z);
	//f32 a22 = a.w * a.w - a.x * a.x + a.y * a.y - a.z * a.z;
	//f32 a23 = 2.0f * (-a.w * a.x + a.y * a.z);
	//
	//f32 a31 = 2.0f * (-a.w * a.y + a.x * a.z);
	//f32 a32 = 2.0f * (a.w * a.y + a.x * a.z);
	//f32 a33 = a.w * a.w - a.x * a.x - a.y * a.y + a.z * a.z;

	m4x4 ret = 
	{
		{
			{ a11, a12, a13, 0.0f },
			{ a21, a22, a23, 0.0f },
			{ a31, a32, a33, 0.0f },
			{ 0.0f, 0.0f, 0.0f, 1.0f },
		}
	};

	return ret;
}

#endif

