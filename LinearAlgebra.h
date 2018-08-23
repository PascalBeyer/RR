#ifndef RR_LINEARALGEBRA
#define RR_LINEARALGEBRA
#include "Matrix.h"

struct Vector2Basis
{
	v2 d1;
	v2 d2;
};

const Vector2Basis v2StdBasis = { V2(1,0) , V2(0,1) };

static Vector2Basis Scale(Vector2Basis basis, float other)
{
	Vector2Basis ret;
	ret.d1 = basis.d1 * other;
	ret.d2 = basis.d2 * other;
	return ret;
}


struct Vector3Basis
{
	v3 d1;
	v3 d2;
	v3 d3;
};

const Vector3Basis v3StdBasis = { V3(1,0,0) , V3(0, 1, 0), V3(0, 0, 1) };

static Vector3Basis Scale(Vector3Basis basis, float other) 
{
	Vector3Basis ret;
	ret.d1 = basis.d1 * other;
	ret.d2 = basis.d2 * other;
	ret.d3 = basis.d3 * other;
	return ret;
}
static Vector3Basis operator*(float f, Vector3Basis basis)
{
	return Scale(basis, f);
}

static Vector3Basis TransformBasis(Vector3Basis basis, m4x4 matrix)
{
	return { matrix * basis.d1, matrix * basis.d2, matrix * basis.d3 };
}

static m4x4 CamBasisToMat(Vector3Basis basis)
{
	return Columns3x3(basis.d1, basis.d2, basis.d3);
}


static Vector3Basis ZeroInclusion12(Vector2Basis dim2Basis)
{
	Vector3Basis ret;
	ret.d1 = { dim2Basis.d1.x, dim2Basis.d1.y, 0 };
	ret.d2 = { dim2Basis.d2.x, dim2Basis.d2.y, 0 };
	ret.d3 = V3(0, 0, 1);
	return ret;
}
static v3 ExpressInBasis(v3 point, Vector3Basis basis)
{
	float d1 = Dot(point, basis.d1);
	float d2 = Dot(point, basis.d2);
	float d3 = Dot(point, basis.d3);

	return V3(d1, d2, d3);
}

static v2 ProjectOntoRectangle(float distToRect, Vector3Basis rectBasis, v3 pointToProject)
{
	v3 relPoint = ExpressInBasis(pointToProject, rectBasis);

	float zScale = distToRect / relPoint.z;

	float projectedX = relPoint.x * zScale;
	float projectedY = relPoint.y * zScale;

	v2 ret = V2(projectedX, projectedY);
	return ret;
}

static v2 p12(v3 a)
{
	return V2(a.x, a.y);
}
static v2 p23(v3 a)
{
	return V2(a.y, a.z);
}


static bool PointInQuadraliteral(v2 p1, v2 p2, v2 p3, v2 p4, v2 pointToCheck);
static bool PointInCenteredRectangle(v2 pos, float width, float height, v2 pointToCheck);
static bool PointInRectangle(v2 pos, float width, float height, v2 pointToCheck);

bool PointInQuadraliteral(v2 p1, v2 p2, v2 p3, v2 p4, v2 pointToCheck)
{
	v2 p12 = p2 - p1;
	v2 pp12 = PerpendicularVector(p12);

	v2 p13 = p3 - p1;
	v2 pp13 = PerpendicularVector(p13);

	v2 p42 = p2 - p4;
	v2 pp42 = PerpendicularVector(p42);

	v2 p43 = p3 - p4;
	v2 pp43 = PerpendicularVector(p43);


	float s12 = Dot(pp12, pointToCheck - p1);
	float s13 = Dot(pp13, pointToCheck - p1);
	float s42 = Dot(pp42, pointToCheck - p4);
	float s43 = Dot(pp43, pointToCheck - p4);

	return (s12 >= 0.0f && s43 >= 0.0f && s13 <= 0.0f && s42 <= 0.0f);
}
bool PointInCenteredRectangle(v2 pos, float width, float height, v2 pointToCheck)
{
	pos -= 0.5f * V2(width, height);
	return ((pos.x <= pointToCheck.x) && (pointToCheck.x <= pos.x + width)) && ((pos.y <= pointToCheck.y) && (pointToCheck.y <= pos.y + height));
}
bool PointInRectangle(v2 pos, float width, float height, v2 pointToCheck)
{
	return ((pos.x <= pointToCheck.x) && (pointToCheck.x <= pos.x + width)) && ((pos.y <= pointToCheck.y) && (pointToCheck.y <= pos.y + height));
}


#endif
