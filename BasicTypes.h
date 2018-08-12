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

#endif
