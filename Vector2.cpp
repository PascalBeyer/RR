#include "Vector2.h"

#include <math.h>
v2 V2()
{
	v2 ret;
	ret.x = 0;
	ret.y = 0;
	return ret;
}

v2 V2(int i)
{
	v2 ret;
	ret.x = (float)i;
	ret.y = (float)i;
	return ret;
}
v2 V2(u32 n)
{
	v2 ret;
	ret.x = (float)n;
	ret.y = (float)n;
	return ret;
}
v2 V2(float f)
{
	v2 ret;
	ret.x = f;
	ret.y = f;
	return ret;
}

v2 V2(float x, float y)
{
	v2 ret;
	ret.x = x;
	ret.y = y;
	return ret;
}
v2 V2(float x, int y)
{
	v2 ret;
	ret.x = x;
	ret.y = (float)y;
	return ret;
}
v2 V2(int x, float y)
{
	v2 ret;
	ret.x = (float)x;
	ret.y = y;
	return ret;
}
v2 V2(int x, int y)
{
	v2 ret;
	ret.x = (float)x;
	ret.y = (float)y;
	return ret;
}
v2 V2(u32 x, u32 y)
{
	v2 ret;
	ret.x = (float)x;
	ret.y = (float)y;
	return ret;
}
v2 V2(float x, u32 y)
{
	v2 ret;
	ret.x = x;
	ret.y = (float)y;
	return ret;
}
v2 V2(u32 x, float y)
{
	v2 ret;
	ret.x = (float)x;
	ret.y = y;
	return ret;
}
v2 operator+(v2 a, v2 b) 
{
	v2 ret;
	ret.x = a.x + b.x;
	ret.y = a.y + b.y;
	return ret;
}
v2& operator+=(v2 &a, v2 b)
{
	a = a + b;

	return a;
}
v2 operator-(v2 a, v2 b) 
{
	v2 ret;
	ret.x = a.x - b.x;
	ret.y = a.y - b.y;
	return ret;
}
v2& operator-=(v2 &a, v2 b)
{
	a = a - b;
	return a;
}
/*v2 operator*( v2 other)
{
	v2 ret;
	ret.x = x * other.x;
	ret.y = y * other.y;
	return ret;
}
v2 operator*=( v2 other)
{
	x *= other.x;
	y *= other.y;
	return *this;
}*/

v2 operator*(v2 a, float f) 
{
	v2 ret;
	ret.x = a.x * f;
	ret.y = a.y * f;
	return ret;
}
v2 operator*(float f, v2 a)
{
	v2 ret;
	ret.x = a.x * f;
	ret.y = a.y * f;
	return ret;
}
v2& operator*=(v2& a, float f)
{
	a = a * f;

	return a;
}

v2 operator/(v2 a, float f) 
{
	v2 ret;
	ret.x = a.x / f;
	ret.y = a.y / f;
	return ret;
}
v2& operator/=(v2 &a, float f)
{
	a = a / f;
	return a;
}
bool operator==(v2 a, v2 b)
{
	return (
		a.x == b.x && 
		a.y == b.y
		);
}
bool operator!=(v2 a, v2 b)
{
	return !(a == b);
}

v2 Normalize(v2 a)
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

float Dot(v2 a,  v2 b)
{
	return a.x * b.x + a.y * b.y;
}
v2 Project( v2 a,  v2 b)
{
	return V2(
		Dot(a, b) / ((float)pow(b.x, 2) + (float)pow(b.y, 2))*b.x, 
		Dot(a, b) / ((float)pow(b.x, 2) + (float)pow(b.y, 2))*b.y
	);
}
float Norm(v2 a)
{
	return sqrtf(powf(a.x, 2) + powf(a.y, 2));
}
float QuadNorm(v2 a)
{
	return (powf(a.x, 2) + powf(a.y, 2));
}
float BoxNorm(v2 a)
{
	return fmaxf(fabsf(a.x), fabsf(a.y));
}
float BoxDist(v2 a,  v2 b)
{
	return BoxNorm(a-b);
}
float Dist( v2 a,  v2 b)
{
	return Norm(a - b);
}
float QuadDist( v2 a,  v2 b)
{
	return QuadNorm(a - b);
}
float AngleBetween( v2 a,  v2 b)
{
	return (float)(atan2(b.y, b.x) - atan2(a.y, a.x));
}
v2 PerpendicularVector( v2 a) //rotates 90° to the left
{
	return V2(-a.y, a.x);
}

v2 RotateAroundOrigin(v2 vec, float angle)
{
	v2 temp = V2(vec.x, vec.y);

	vec.x = temp.x * cosf(angle) - temp.y *sinf(angle);
	vec.y = temp.y * cosf(angle) + temp.x *sinf(angle);

	return vec;
}


v2 RotateAround(v2 origin, float angle, v2 v)
{
	v2 ret;

	ret.x = (v.x - origin.x)*cosf(angle) - (v.y - origin.y) * sinf(angle) + origin.x;
	ret.y = (v.y - origin.y)*cosf(angle) + (v.x - origin.x) * sinf(angle) + origin.y;

	return ret;
}