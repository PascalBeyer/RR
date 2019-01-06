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
};								\
\
static type* operator+(type##Array arr, u32 index)								\
{																				\
	Assert(index < arr.amount);													\
	return arr.data + index;													\
}																				\
static bool operator!(type##Array arr)											\
{																				\
	return (!arr.amount);														\
}																				\



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
static type* operator+(type##DynamicArray arr, u32 index)						\
{																				\
	Assert(index < arr.amount);													\
	return arr.data + index;													\
}																				\
\
static u32 ArrayAdd(type##DynamicArray *arr, type t)							\
{																				\
	if (arr->amount + 1 < arr->capacity)										\
	{																			\
		(arr->data)[arr->amount++] = t;											\
		return (arr->amount - 1);												\
	}																			\
   \
	u32 newCapacity = 2 * arr->capacity + 1;									\
   \
	type *newData = DynamicAlloc(type, newCapacity);							\
	memcpy(newData, arr->data, arr->capacity * sizeof(type));					\
	arr->capacity = newCapacity;												\
	DynamicFree(arr->data);														\
	arr->data = newData;														\
	arr->data[arr->amount++] = t;												\
   \
	return (arr->amount - 1);													\
}																				\
\
static type##DynamicArray type##CreateDynamicArray(u32 capacity = 8)			\
{																				\
	type##DynamicArray ret;														\
	ret.data = DynamicAlloc(type, capacity);									\
	ret.amount = 0;																\
	ret.capacity = capacity;													\
	return ret;																	\
}																				\
static void Clear(type##DynamicArray *arr)										\
{																				\
	arr->amount = 0;															\
}																				\
static void Reserve(type##DynamicArray *arr, u32 capacity)						\
{																				\
	if (capacity < arr->capacity)												\
	{																			\
		return;																	\
	}																			\
   \
	type *newData = DynamicAlloc(type, capacity);								\
	memcpy(newData, arr->data, arr->capacity * sizeof(type));					\
	arr->capacity = capacity;													\
	DynamicFree(arr->data);														\
	arr->data = newData;														\
	return;																		\
}																				\
\
static void UnorderedRemove(type##DynamicArray *arr, u32 index)					\
{																				\
	Assert(index < arr->amount);												\
	arr->data[index] = arr->data[--arr->amount];								\
}																				\
static bool operator!(type##DynamicArray arr)									\
{																				\
	return (!arr.amount);														\
}																				\


#define DefineDFArray(type)												\
struct type##DFArray														\
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
static type* operator+(type##DFArray arr, u32 index)						\
{																				\
	Assert(index < arr.amount);													\
	return arr.data + index;													\
}																				\
\
static u32 ArrayAdd(type##DFArray *arr, type t)									\
{																				\
	if (arr->amount + 1 < arr->capacity)										\
	{																			\
		(arr->data)[arr->amount++] = t;											\
		return (arr->amount - 1);												\
	}																			\
   \
	u32 newCapacity = 2 * arr->capacity + 1;									\
   \
	type *newData = PushData(frameArena, type, newCapacity);					\
	memcpy(newData, arr->data, arr->capacity * sizeof(type));					\
	arr->capacity = newCapacity;												\
	arr->data = newData;														\
	arr->data[arr->amount++] = t;												\
   \
	return (arr->amount - 1);													\
}																				\
\
static type##DFArray type##CreateDFArray(u32 capacity = 8)						\
{																				\
	type##DFArray ret;														\
	ret.data = PushData(frameArena, type, capacity);						\
	ret.amount = 0;																\
	ret.capacity = capacity;													\
	return ret;																	\
}																				\
static void Clear(type##DFArray *arr)											\
{																				\
	arr->amount = 0;															\
}																				\
\
static void UnorderedRemove(type##DFArray *arr, u32 index)						\
{																				\
	Assert(index < arr->amount);												\
	arr->data[index] = arr->data[--arr->amount];								\
}																				\
static bool operator!(type##DFArray arr)										\
{																				\
	return (!arr.amount);														\
}																				\



#define GET_MACRO3(_1,_2,_3, NAME,...) NAME
#define Expand(x) x

#define BuildStaticArray(arena, arr, item) PushData(arena, u8, sizeof(item)); arr.data[arr.amount++] = item;

#define ForArr(arr) for(auto it = (arr).data; it < (arr).data + (arr).amount; it++)
#define ForVarArr(it, arr) for(auto it = (arr).data; it < (arr).data + (arr).amount; it++)


#define For(...) Expand(GET_MACRO3(__VA_ARGS__, Ignored, ForVarArr, ForArr)(__VA_ARGS__))

#if 0
#define ForArrValue(arr) for (auto &it = *arr.data; &it < arr.data + arr.amount; it = (&it) + 1)
#define ForArrVarValue(it, arr) for (auto &it = *arr.data; &it < arr.data + arr.amount; &it = (&it) + 1)

#define ForRef(...) Expand(GET_MACRO3(__VA_ARGS__, Ignored, ForArrVarValue, ForArrValue)(__VA_ARGS__))
#endif


#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t	i32;
typedef int64_t	i64;
typedef int b32;
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

union v2
{
	struct
	{
		f32 x;
		f32 y;
	};
	struct
	{
		f32 u;
		f32 v;
	};
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
static v2 V2(f32 x, i32 y)
{
	v2 ret;
	ret.x = x;
	ret.y = (f32)y;
	return ret;
}
static v2 V2(i32 x, f32 y)
{
	v2 ret;
	ret.x = (f32)x;
	ret.y = y;
	return ret;
}
static v2 V2(i32 x, i32 y)
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


static v2i operator-(v2i a)
{
	return { -a.x, -a.y };
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
static v3 V3(i32 _x, i32 _y, i32 _z)
{
	v3 ret;
	ret.x = (f32)_x;
	ret.y = (f32)_y;
	ret.z = (f32)_z; return ret;
}
static v3 V3(f32 _x, f32 _y, i32 _z)
{
	v3 ret;
	ret.x = _x;
	ret.y = _y;
	ret.z = (f32)_z; return ret;
}
static v3 V3(f32 _x, i32 _y, f32 _z)
{
	v3 ret;
	ret.x = _x;
	ret.y = (f32)_y;
	ret.z = _z; return ret;
}
static v3 V3(i32 _x, f32 _y, f32 _z)
{
	v3 ret;
	ret.x = (f32)_x;
	ret.y = _y;
	ret.z = _z; return ret;
}

static v3 V3(i32 _x, f32 _y, i32 _z)
{
	v3 ret;
	ret.x = (f32)_x;
	ret.y = _y;
	ret.z = (f32)_z; return ret;
}
static v3 V3(f32 _x, i32 _y, i32 _z)
{
	v3 ret;
	ret.x = _x;
	ret.y = (f32)_y;
	ret.z = (f32)_z; return ret;
}
static v3 V3(i32 _x, i32 _y, f32 _z)
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
static v3 V3(i32 _x, i32 _y, u32 _z)
{
	v3 ret;
	ret.x = (f32)_x;
	ret.y = (f32)_y;
	ret.z = (f32)_z; return ret;
}
static v3 V3(i32 _x, u32 _y, i32 _z)
{
	v3 ret;
	ret.x = (f32)_x;
	ret.y = (f32)_y;
	ret.z = (f32)_z; return ret;
}
static v3 V3(u32 _x, i32 _y, i32 _z)
{
	v3 ret;
	ret.x = (f32)_x;
	ret.y = (f32)_y;
	ret.z = (f32)_z; return ret;
}
static v3 V3(u32 _x, i32 _y, u32 _z)
{
	v3 ret;
	ret.x = (f32)_x;
	ret.y = (f32)_y;
	ret.z = (f32)_z; return ret;
}
static v3 V3(i32 _x, u32 _y, u32 _z)
{
	v3 ret;
	ret.x = (f32)_x;
	ret.y = (f32)_y;
	ret.z = (f32)_z;
	return ret;
}
static v3 V3(u32 _x, u32 _y, i32 _z)
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

union v3i
{
	struct
	{
		i32 x, y, z;
	};
	struct
	{
		v2i xy;
		i32 z;
	};
	struct
	{
		i32 x;
		v2i yz;
	};
	i32 v[3];
};

static v3 V3(v3i a)
{
	return V3(a.x, a.y, a.z);
}

static v3i V3i()
{
	v3i ret;
	ret.x = 0;
	ret.y = 0;
	ret.z = 0;
	return ret;
}

static v3i V3i(i32 _x, i32 _y, i32 _z)
{
	v3i ret;
	ret.x = _x;
	ret.y = _y;
	ret.z = _z; 
	return ret;
}
static v3i V3i(u32 _x, u32 _y, u32 _z)
{
	v3i ret;
	ret.x = (i32)_x;
	ret.y = (i32)_y;
	ret.z = (i32)_z; 
	return ret;
}
static v3i V3i(i32 _x, i32 _y, u32 _z)
{
	v3i ret;
	ret.x = (i32)_x;
	ret.y = (i32)_y;
	ret.z = (i32)_z; 
	return ret;
}
static v3i V3i(i32 _x, u32 _y, i32 _z)
{
	v3i ret;
	ret.x = (i32)_x;
	ret.y = (i32)_y;
	ret.z = (i32)_z; return ret;
}
static v3i V3i(u32 _x, i32 _y, i32 _z)
{
	v3i ret;
	ret.x = (i32)_x;
	ret.y = (i32)_y;
	ret.z = (i32)_z; return ret;
}
static v3i V3i(u32 _x, i32 _y, u32 _z)
{
	v3i ret;
	ret.x = (i32)_x;
	ret.y = (i32)_y;
	ret.z = (i32)_z; return ret;
}
static v3i V3i(i32 _x, u32 _y, u32 _z)
{
	v3i ret;
	ret.x = (i32)_x;
	ret.y = (i32)_y;
	ret.z = (i32)_z;
	return ret;
}
static v3i V3i(u32 _x, u32 _y, i32 _z)
{
	v3i ret;
	ret.x = (i32)_x;
	ret.y = (i32)_y;
	ret.z = (i32)_z;
	return ret;
}
static v3i V3i(v2i xy, i32 z)
{
	v3i ret;
	ret.x = xy.x;
	ret.y = xy.y;
	ret.z = z;
	return ret;
}
static v3i V3i(v2i xy, u32 z)
{
	v3i ret;
	ret.xy = xy;
	ret.z = (i32)z;
	return ret;
}
static v3i V3i(i32 x, v2i yz)
{
	v3i ret;
	ret.x = x;
	ret.yz = yz;
	return ret;
}
static v3i V3i(u32 x, v2i yz)
{
	v3i ret;
	ret.x = (i32)x;
	ret.yz = yz;
	return ret;
}

static v3i operator+(v3i a, v3i b)
{
	v3i ret;
	ret.x = a.x + b.x;
	ret.y = a.y + b.y;
	ret.z = a.z + b.z;
	return ret;
}
static v3i& operator+=(v3i &a, v3i b)
{
	a = a + b;
	return a;
}
static v3i operator-(v3i a, v3i b)
{
	v3i ret;
	ret.x = a.x - b.x;
	ret.y = a.y - b.y;
	ret.z = a.z - b.z;
	return ret;
}
static v3i& operator-=(v3i &a, v3i b)
{
	a = a - b;
	return a;
}
static v3i operator*(v3i a, i32 f)
{
	v3i ret;
	ret.x = a.x * f;
	ret.y = a.y * f;
	ret.z = a.z * f;
	return ret;
}
static v3i& operator*=(v3i& a, i32 f)
{
	a = a * f;
	return a;
}
static v3i operator*(v3i a, v3i b)
{
	v3i ret;
	ret.x = a.x * b.x;
	ret.y = a.y * b.y;
	ret.z = a.z * b.z;
	return ret;
}
static v3i& operator*=(v3i &a, v3i b)
{
	a = a * b;
	return a;
}

static v3i operator/(v3i a, i32 f)
{
	v3i ret;
	ret.x = a.x / f;
	ret.y = a.y / f;
	ret.z = a.z / f;
	return ret;
}

static v3i& operator/=(v3i& a, i32 f)
{
	a = a / f;
	return a;
}

static bool operator==(v3i a, v3i b)
{
	return (
		a.x == b.x &&
		a.y == b.y &&
		a.z == b.z
		);
}

static bool operator!=(v3i a, v3i b)
{
	return !(a == b);
}

static v3i operator*(i32 scalar, v3i a)
{
	v3i ret;
	ret.x = scalar * a.x;
	ret.y = scalar * a.y;
	ret.z = scalar * a.z;
	return ret;
}
static v3i operator-(v3i a)
{
	return { -a.x, -a.y, -a.z };
}

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
				union
				{
					struct
					{
						f32 x;
						f32 y;					
					};
					v2 xy;
				};
				f32 z;
			};
			v3 xyz;
		};
		f32 w;
	};
};


static v4 V4()
{
	v4 ret;
   
	ret.a = 0;
	ret.r = 0;
	ret.g = 0;
	ret.b = 0;
	return ret;
}

static v4 V4(f32 a, f32 r, f32 g, f32 b)
{
	v4 ret;
	ret.a = a;
	ret.r = r;
	ret.g = g;
	ret.b = b;
	return ret;
}


static v4 V4(i32 a, i32 r, i32 g, i32 b)
{
	v4 ret;
	ret.a = (f32)a;
	ret.r = (f32)r;
	ret.g = (f32)g;
	ret.b = (f32)b;
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

static v4 operator/(v4 a, v4 b) // todo do we do this in a _safe_ way later?
{
	v4 ret;
	ret.a = a.a / b.a;
	ret.r = a.r / b.r;
	ret.g = a.g / b.g;
	ret.b = a.b / b.b;
#if 0
	ret.a = Min(a.a / b.a, 1.0f);
	ret.r = Min(a.r / b.r, 1.0f);
	ret.g = Min(a.g / b.g, 1.0f);
	ret.b = Min(a.b / b.b, 1.0f);
#endif
	return ret;
}

static v4& operator*=(v4 &a, v4 b)
{
	a = a*b;
	return a;
}


static v4& operator/=(v4 &a, v4 b)
{
	a = a/b;
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


union v4i
{
	struct
	{
		i32 a;
		i32 r;
		i32 g;
		i32 b;
	};
   
	struct
	{
		i32 a;
		v3i rgb;
	};
   
	struct
	{
		i32 x, y, z, w;
	};
   struct 
   {
      v2i xy;
      v2i zw;
   };
   
   struct 
   {
      i32 x;
      v2i yz;
      i32 w;
   };
   
   struct
   {
      v3i xyz;
      i32 w;
   };
   
	i32 v[4];
};

static v4 V4(v4i a)
{
	return V4(a.x, a.y, a.z, a.w);
}

static v4i V4i()
{
	v4i ret;
	ret.x = 0;
	ret.y = 0;
	ret.z = 0;
   ret.w = 0;
	return ret;
}

static v4i V4i(i32 _x, i32 _y, i32 _z, i32 _w)
{
	v4i ret;
	ret.x = _x;
	ret.y = _y;
	ret.z = _z; 
   ret.w = _w;
	return ret;
}
static v4i V4i(u32 _x, u32 _y, u32 _z, i32 _w)
{
	v4i ret;
	ret.x = (i32)_x;
	ret.y = (i32)_y;
	ret.z = (i32)_z; 
   ret.w = (i32)_w; 
	return ret;
}
static v4i V4i(i32 _x, i32 _y, u32 _z, i32 _w)
{
	v4i ret;
	ret.x = (i32)_x;
	ret.y = (i32)_y;
	ret.z = (i32)_z; 
   ret.w = (i32)_w; 
	return ret;
}
static v4i V4i(i32 _x, u32 _y, i32 _z, i32 _w)
{
	v4i ret;
	ret.x = (i32)_x;
	ret.y = (i32)_y;
	ret.z = (i32)_z; 
   ret.w = (i32)_w; 
   return ret;
}
static v4i V4i(u32 _x, i32 _y, i32 _z, i32 _w)
{
	v4i ret;
	ret.x = (i32)_x;
	ret.y = (i32)_y;
	ret.z = (i32)_z; 
   ret.w = (i32)_w; 
   return ret;
}
static v4i V4i(u32 _x, i32 _y, u32 _z, i32 _w)
{
	v4i ret;
	ret.x = (i32)_x;
	ret.y = (i32)_y;
	ret.z = (i32)_z; 
   ret.w = (i32)_w; 
   return ret;
}
static v4i V4i(i32 _x, u32 _y, u32 _z, i32 _w)
{
	v4i ret;
	ret.x = (i32)_x;
	ret.y = (i32)_y;
	ret.z = (i32)_z;
   ret.w = (i32)_w; 
	return ret;
}
static v4i V4i(u32 _x, u32 _y, i32 _z, i32 _w)
{
	v4i ret;
	ret.x = (i32)_x;
	ret.y = (i32)_y;
	ret.z = (i32)_z;
   ret.w = (i32)_w; 
	return ret;
}
static v4i V4i(v2i xy, i32 z, i32 _w)
{
	v4i ret;
	ret.x = xy.x;
	ret.y = xy.y;
	ret.z = z;
   ret.w = (i32)_w; 
	return ret;
}
static v4i V4i(v2i xy, u32 z, i32 _w)
{
	v4i ret;
	ret.xy = xy;
	ret.z = (i32)z;
   ret.w = (i32)_w; 
	return ret;
}
static v4i V4i(i32 x, v2i yz, i32 _w)
{
	v4i ret;
	ret.x = x;
	ret.yz = yz;
   ret.w = (i32)_w; 
	return ret;
}
static v4i V4i(u32 x, v2i yz, i32 _w)
{
	v4i ret;
	ret.x = (i32)x;
	ret.yz = yz;
   ret.w = (i32)_w; 
	return ret;
}

static v4i operator+(v4i a, v4i b)
{
	v4i ret;
	ret.x = a.x + b.x;
	ret.y = a.y + b.y;
	ret.z = a.z + b.z;
   ret.w = a.w + b.w;
	return ret;
}
static v4i& operator+=(v4i &a, v4i b)
{
	a = a + b;
	return a;
}
static v4i operator-(v4i a, v4i b)
{
	v4i ret;
	ret.x = a.x - b.x;
	ret.y = a.y - b.y;
	ret.z = a.z - b.z;
   ret.w = a.w - b.w;
	return ret;
}
static v4i& operator-=(v4i &a, v4i b)
{
	a = a - b;
	return a;
}
static v4i operator*(v4i a, i32 f)
{
	v4i ret;
	ret.x = a.x * f;
	ret.y = a.y * f;
	ret.z = a.z * f;
   ret.w = a.w * f;
	return ret;
}

static v4i operator*(i32 scalar, v4i a)
{
	v4i ret;
	ret.x = scalar * a.x;
	ret.y = scalar * a.y;
	ret.z = scalar * a.z;
   ret.w = scalar * a.w;
	return ret;
}
static v4i& operator*=(v4i& a, i32 f)
{
	a = a * f;
	return a;
}
static v4i operator*(v4i a, v4i b)
{
	v4i ret;
	ret.x = a.x * b.x;
	ret.y = a.y * b.y;
	ret.z = a.z * b.z;
   ret.w = a.w * b.w;
	return ret;
}
static v4i& operator*=(v4i &a, v4i b)
{
	a = a * b;
	return a;
}

static v4i operator/(v4i a, i32 f)
{
	v4i ret;
	ret.x = a.x / f;
	ret.y = a.y / f;
	ret.z = a.z / f;
   ret.w = a.w / f;
	return ret;
}

static v4i& operator/=(v4i& a, i32 f)
{
	a = a / f;
	return a;
}

static bool operator==(v4i a, v4i b)
{
	return (
		a.x == b.x &&
		a.y == b.y &&
		a.z == b.z &&
      a.w == b.w
		);
}

static bool operator!=(v4i a, v4i b)
{
	return !(a == b);
}

static v4i operator-(v4i a)
{
	return { -a.x, -a.y, -a.z, -a.w };
}


#endif
