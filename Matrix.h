#ifndef RR_MATRIX
#define RR_MATRIX

#include "Vector3.h"

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

	v3 R;

	R.x = P.x*A.a[0][0] + P.y*A.a[0][1] + P.z*A.a[0][2] + Pw*A.a[0][3];
	R.y = P.x*A.a[1][0] + P.y*A.a[1][1] + P.z*A.a[1][2] + Pw*A.a[1][3];
	R.z = P.x*A.a[2][0] + P.y*A.a[2][1] + P.z*A.a[2][2] + Pw*A.a[2][3];

	return(R);
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
		{ { c,-s, 0, 0 },
		{ s, c, 0, 0 },
		{ 0, 0, 1, 0 },
		{ 0, 0, 0, 1 } },
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
		{ { a * c, 0,		0,		0 },
		{ 0,	-b * c, 0,		0 },
		{ 0,	0,		d,		e },
		{ 0,	0,    1.0f,		0 }
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
			{ 2.0f / width, 0.0f, 0.0f, 0.0f },
			{ 0.0f, -2.0f / height, 0.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f, -1.0f }, //we are in a -1 to +1 cube, so -1 is the exact screen position
			{ 0.0f, 0.0f, 0.0f, 1.0f } }
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
			ret.a[i][j] = inv[4*i + j] * invDet;
		}
	}
		

	return ret;

}



#endif // !RR_Matrix

