#ifndef RR_VECTOR4
#define RR_VECTOR4

#include "Vector3.h"

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


v4 V4();
v4 V4(f32 a, v3 rgb);
v4 V4(float a, float r, float g, float b);


v4 operator+ (v4 a, v4 b) ;
v4& operator+= (v4 &a, v4 b);
v4 operator- (v4 a, v4 b) ;
v4& operator-= (v4 &a, v4 b);
v4 operator*(float f, v4 a);
v4 operator*(v4 a, float f);
v4& operator*=(v4 &a, float f);
v4 operator*(v4 a, v4 b);
v4& operator*=(v4 &a, v4 b);
v4 operator/(v4 a, float f);
v4& operator/=(v4 &a, float f);


bool operator==(v4 a, v4 b);
bool operator!=(v4 a, v4 b);

v4 Normalize(v4 a);
v4 Lerp(v4 a, float t,  v4 b);
float Dot(v4 a,  v4 b);
float Norm(v4 a);
float QuadDist(v4 a,  v4 b);
float Dist(v4 a,  v4 b);
float QuadNorm( v4 a);
v4 Unpack4x8(u32 *pack);
u32 Pack4x8(v4 pack);

#endif



