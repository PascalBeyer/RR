#ifndef RR_VECTOR2
#define RR_VECTOR2
#include "BasicTypes.h"

struct v2
{
	float x;
	float y;
};

v2 operator* (float f, v2 v);

v2 RotateAround(v2 origin, float angle, v2 v);
v2 V2();
v2 V2(int);
v2 V2(u32);
v2 V2(float);
v2 V2(float x, float y);
v2 V2(float x, int y);
v2 V2(int x, float y);
v2 V2(int x, int y);
v2 V2(u32 x, u32 y);
v2 V2(float x, u32 y);
v2 V2(u32 x, float y);

v2 operator+ (v2 a, v2 b);
v2& operator+= (v2& a, v2 b);
v2 operator- (v2 a, v2 b);
v2& operator-= (v2& a, v2 b);
//v2 operator*(v2 other);
//v2 operator*=(v2 other);

v2 operator/(v2 a, float f);
v2& operator/=(v2 &a, float f);
v2 operator*(v2 a, float f);
v2 operator*(float f, v2 a);
v2& operator*=(v2 &a, float f);
bool operator==(v2 a, v2 b);
bool operator!=(v2 a, v2 b);

v2 Normalize(v2 ad);

float Dot(v2 a, v2 b);
v2 Project(v2 a, v2 b);
float Norm(v2 a);
float BoxNorm(v2 a);
float BoxDist(v2 a, v2 b);
float QuadDist(v2 a, v2 b);
float Dist(v2 a, v2 b);
float QuadNorm(v2 a);
float AngleBetween(v2 a, v2 b);
v2 PerpendicularVector(v2 a);
static v2 LerpVector2(v2 v1, float f1, v2 v2, float f2, float t);
v2 LerpVector2(v2 v1, float f1, v2 v2, float f2, float t)
{
	//Assert(f1 != f2);
	return (v1 + (t - f1) / (f2 - f1) * v2);
}


v2 RotateAroundOrigin(v2 a, float angle);

#endif