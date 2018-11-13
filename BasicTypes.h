#ifndef RR_BASICTYPES
#define RR_BASICTYPES

#define MAXFLOAT 3.402823466e+38F
#define MINFLOAT -MAXFLOAT
#define MAXF32 3.402823466e+38F
#define MINF32 -MAXFLOAT
#define F32MAX MAXF32
#define F32MIN MINF32
#define MAXU32 ((u32)-1)

#define DefineArray(type)		\
struct type##Array				\
{								\
	type *data;					\
	u32 amount;					\
	type& operator[](u32 i)		\
	{							\
		Assert(i < amount);		\
		return data[i];			\
	}							\
}; 


#define BeginArray(arena, type, name) type##Array name = PushArray(arena, type, 0);
#define EndArray(arena, type, name) name.amount = (u32)((type *)arena->current - name.data);


// apperantly having a space after \ ruines the define.
#define DefineDynamicArray(type)												\
struct type##DynamicArray														\
{																				\
	type *data;																	\
	u32 amount;																	\
	u32 capacity;																\
																				\
	type &operator[] (u32 i)													\
	{																			\
		Assert(i < amount);														\
		return data[i];															\
	}																			\
};																				\
static type *ArrayAdd(type##DynamicArray *arr, type t)							\
{																				\
	if (arr->amount + 1 < arr->capacity)										\
	{																			\
		(arr->data)[arr->amount++] = t;											\
		return arr->data + (arr->amount - 1);									\
	}																			\
																				\
	u32 newCapacity = 2 * arr->capacity + 1;									\
																				\
	type *newData = DynamicAlloc(type, 2 * arr->capacity + 1);			\
	memcpy(newData, arr->data, arr->capacity * sizeof(type));					\
	arr->capacity = 2 * arr->capacity + 1;										\
	DynamicFree(arr->data);												\
	arr->data = newData;														\
	arr->data[arr->amount++] = t;												\
																				\
	return (arr->data + (arr->amount - 1));										\
}																				\
																				\
static type##DynamicArray type##CreateDynamicArray(u32 capacity = 8)	\
{																				\
	type##DynamicArray ret;														\
	ret.data = DynamicAlloc(type, capacity);								\
	ret.amount = 0;																\
	ret.capacity = capacity;													\
	return ret;																	\
}																				\
static void Clear(type##DynamicArray *arr)										\
{																				\
	arr->amount = 0;															\
}																				\



#define BuildStaticArray(arena, arr, item) PushData(arena, u8, sizeof(item)); arr.data[arr.amount++] = item;

#define For(arr) for(auto it = arr.data; it < arr.data + arr.amount; it++)

#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t	i32;
typedef int64_t	i64;
typedef i32 b32;
typedef float f32;
typedef double f64;
typedef uintptr_t uintptr;

typedef uintptr_t umm;
typedef intptr_t smm;


struct v2i
{
	i32 x;
	i32 y;
};

struct v2
{
	f32 x;
	f32 y;
};

union v3
{
	struct
	{
		f32 x, y, z;
	};
	struct
	{
		f32 r, g, b;
	};
	struct
	{
		v2 xy;
		f32 z;
	};
	struct
	{
		f32 x;
		v2 yz;
	};
	f32 v[3];
};

union v4
{
	struct
	{
		f32 a;
		f32 r;
		f32 g;
		f32 b;
	};

	struct
	{
		f32 a;
		v3 rgb;
	};
	struct
	{
		union
		{
			struct
			{
				f32 x;
				f32 y;
				f32 z;
			};
			v3 xyz;
		};
		f32 w;
	};
};

static v2i V2i(v2 a)
{
	return { (i32)a.x, (i32)a.y };
}


static v2 V2(v2i a)
{
	return { (f32)a.x, (f32)a.y };
}

static v2 V2()
{
	v2 ret;
	ret.x = 0;
	ret.y = 0;
	return ret;
}

static v2 V2(f32 x, f32 y)
{
	v2 ret;
	ret.x = x;
	ret.y = y;
	return ret;
}
static v2 V2(f32 x, int y)
{
	v2 ret;
	ret.x = x;
	ret.y = (f32)y;
	return ret;
}
static v2 V2(int x, f32 y)
{
	v2 ret;
	ret.x = (f32)x;
	ret.y = y;
	return ret;
}
static v2 V2(int x, int y)
{
	v2 ret;
	ret.x = (f32)x;
	ret.y = (f32)y;
	return ret;
}
static v2 V2(u32 x, u32 y)
{
	v2 ret;
	ret.x = (f32)x;
	ret.y = (f32)y;
	return ret;
}
static v2 V2(f32 x, u32 y)
{
	v2 ret;
	ret.x = x;
	ret.y = (f32)y;
	return ret;
}
static v2 V2(u32 x, f32 y)
{
	v2 ret;
	ret.x = (f32)x;
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

static v2 operator*(v2 a, f32 f)
{
	v2 ret;
	ret.x = a.x * f;
	ret.y = a.y * f;
	return ret;
}
static v2 operator*(f32 f, v2 a)
{
	v2 ret;
	ret.x = a.x * f;
	ret.y = a.y * f;
	return ret;
}
static v2& operator*=(v2& a, f32 f)
{
	a = a * f;

	return a;
}

static v2 operator/(v2 a, f32 f)
{
	v2 ret;
	ret.x = a.x / f;
	ret.y = a.y / f;
	return ret;
}

static v2& operator/=(v2 &a, f32 f)
{
	a = a / f;
	return a;
}

static v2 operator*(v2 a, v2 b)
{
	v2 ret;
	ret.x = a.x * b.x;
	ret.y = a.y * b.y;
	return ret;
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


static v2i V2i()
{
	v2i ret;
	ret.x = 0;
	ret.y = 0;
	return ret;
}
static v2i V2i(i32 x, u32 y)
{
	v2i ret;
	ret.x = x;
	ret.y = y;
	return ret;
}
static v2i V2i(u32 x, u32 y)
{
	v2i ret;
	ret.x = (i32)x;
	ret.y = (i32)y;
	return ret;
}
static v2i operator+(v2i a, v2i b)
{
	v2i ret;
	ret.x = a.x + b.x;
	ret.y = a.y + b.y;
	return ret;
}
static v2i& operator+=(v2i &a, v2i b)
{
	a = a + b;

	return a;
}
static v2i operator-(v2i a, v2i b)
{
	v2i ret;
	ret.x = a.x - b.x;
	ret.y = a.y - b.y;
	return ret;
}
static v2i& operator-=(v2i &a, v2i b)
{
	a = a - b;
	return a;
}

static v2i operator*(v2i a, i32 f)
{
	v2i ret;
	ret.x = a.x * f;
	ret.y = a.y * f;
	return ret;
}
static v2i operator*(i32 f, v2i a)
{
	v2i ret;
	ret.x = a.x * f;
	ret.y = a.y * f;
	return ret;
}
static v2i& operator*=(v2i& a, i32 f)
{
	a = a * f;

	return a;
}

static v2i operator/(v2i a, i32 f)
{
	v2i ret;
	if (f == 0) return V2i();
	ret.x = a.x / f;
	ret.y = a.y / f;
	return ret;
}

static v2i& operator/=(v2i &a, i32 f)
{
	a = a / f;
	return a;
}

static v2i operator*(v2i a, v2i b)
{
	v2i ret;
	ret.x = a.x * b.x;
	ret.y = a.y * b.y;
	return ret;
}

static bool operator==(v2i a, v2i b)
{
	return (
		a.x == b.x &&
		a.y == b.y
		);
}

static bool operator!=(v2i a, v2i b)
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
static v3 V3(f32 _x)
{
	v3 ret;
	ret.x = _x;
	ret.y = _x;
	ret.z = _x;
	return ret;
}

static v3 V3(f32 _x, f32 _y, f32 _z)
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
	ret.x = (f32)_x;
	ret.y = (f32)_y;
	ret.z = (f32)_z; return ret;
}
static v3 V3(f32 _x, f32 _y, int _z)
{
	v3 ret;
	ret.x = _x;
	ret.y = _y;
	ret.z = (f32)_z; return ret;
}
static v3 V3(f32 _x, int _y, f32 _z)
{
	v3 ret;
	ret.x = _x;
	ret.y = (f32)_y;
	ret.z = _z; return ret;
}
static v3 V3(int _x, f32 _y, f32 _z)
{
	v3 ret;
	ret.x = (f32)_x;
	ret.y = _y;
	ret.z = _z; return ret;
}

static v3 V3(int _x, f32 _y, int _z)
{
	v3 ret;
	ret.x = (f32)_x;
	ret.y = _y;
	ret.z = (f32)_z; return ret;
}
static v3 V3(f32 _x, int _y, int _z)
{
	v3 ret;
	ret.x = _x;
	ret.y = (f32)_y;
	ret.z = (f32)_z; return ret;
}
static v3 V3(int _x, int _y, f32 _z)
{
	v3 ret;
	ret.x = (f32)_x;
	ret.y = (f32)_y;
	ret.z = _z; return ret;
}
static v3 V3(u32 _x, u32 _y, u32 _z)
{
	v3 ret;
	ret.x = (f32)_x;
	ret.y = (f32)_y;
	ret.z = (f32)_z; return ret;
}
static v3 V3(f32 _x, f32 _y, u32 _z)
{
	v3 ret;
	ret.x = _x;
	ret.y = _y;
	ret.z = (f32)_z; return ret;
}
static v3 V3(f32 _x, u32 _y, f32 _z)
{
	v3 ret;
	ret.x = _x;
	ret.y = (f32)_y;
	ret.z = _z; return ret;
}
static v3 V3(u32 _x, f32 _y, f32 _z)
{
	v3 ret;
	ret.x = (f32)_x;
	ret.y = _y;
	ret.z = _z; return ret;
}
static v3 V3(u32 _x, f32 _y, u32 _z)
{
	v3 ret;
	ret.x = (f32)_x;
	ret.y = (f32)_y;
	ret.z = (f32)_z; return ret;
}
static v3 V3(f32 _x, u32 _y, u32 _z)
{
	v3 ret;
	ret.x = (f32)_x;
	ret.y = (f32)_y;
	ret.z = (f32)_z; return ret;
}
static v3 V3(u32 _x, u32 _y, f32 _z)
{
	v3 ret;
	ret.x = (f32)_x;
	ret.y = (f32)_y;
	ret.z = (f32)_z; return ret;
}
static v3 V3(int _x, int _y, u32 _z)
{
	v3 ret;
	ret.x = (f32)_x;
	ret.y = (f32)_y;
	ret.z = (f32)_z; return ret;
}
static v3 V3(int _x, u32 _y, int _z)
{
	v3 ret;
	ret.x = (f32)_x;
	ret.y = (f32)_y;
	ret.z = (f32)_z; return ret;
}
static v3 V3(u32 _x, int _y, int _z)
{
	v3 ret;
	ret.x = (f32)_x;
	ret.y = (f32)_y;
	ret.z = (f32)_z; return ret;
}
static v3 V3(u32 _x, int _y, u32 _z)
{
	v3 ret;
	ret.x = (f32)_x;
	ret.y = (f32)_y;
	ret.z = (f32)_z; return ret;
}
static v3 V3(int _x, u32 _y, u32 _z)
{
	v3 ret;
	ret.x = (f32)_x;
	ret.y = (f32)_y;
	ret.z = (f32)_z;
	return ret;
}
static v3 V3(u32 _x, u32 _y, int _z)
{
	v3 ret;
	ret.x = (f32)_x;
	ret.y = (f32)_y;
	ret.z = (f32)_z;
	return ret;
}
static v3 V3(v2 xy, f32 z)
{
	v3 ret;
	ret.x = xy.x;
	ret.y = xy.y;
	ret.z = z;
	return ret;
}
static v3 V3(f32 x, v2 yz)
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
static v3 operator*(v3 a, f32 f)
{
	v3 ret;
	ret.x = a.x * f;
	ret.y = a.y * f;
	ret.z = a.z * f;
	return ret;
}
static v3& operator*=(v3& a, f32 f)
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

static v3 operator/(v3 a, f32 f)
{
	v3 ret;
	ret.x = a.x / f;
	ret.y = a.y / f;
	ret.z = a.z / f;
	return ret;
}

static v3& operator/=(v3& a, f32 f)
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

static v3 operator*(f32 scalar, v3 a)
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

static v4 V4(f32 a, f32 x, f32 y, f32 z)
{
	v4 ret;
	ret.a = a;
	ret.r = x;
	ret.g = y;
	ret.b = z;
	return ret;
}
static v4 V4(f32 a, v3 rgb)
{
	v4 ret;
	ret.a = a;
	ret.rgb = rgb;
	return ret;
}
static v4 V4(v3 xyz, f32 w)
{
	v4 ret;
	ret.w = w;
	ret.xyz = xyz;
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

static v4 operator*(f32 f, v4 other)
{
	v4 ret;
	ret.a = f*other.a;
	ret.r = f*other.r;
	ret.g = f*other.g;
	ret.b = f*other.b;
	return ret;
}

static v4 operator*(v4 a, f32 f)
{
	v4 ret;
	ret.a = a.a * f;
	ret.r = a.r * f;
	ret.g = a.g * f;
	ret.b = a.b * f;
	return ret;
}

static v4& operator*=(v4 &a, f32 f)
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

static v4 operator/(v4 a, f32 f)
{
	v4 ret;
	ret.a = a.a / f;
	ret.r = a.r / f;
	ret.g = a.g / f;
	ret.b = a.b / f;
	return ret;
}

static v4& operator/= (v4 &a, f32 f)
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
