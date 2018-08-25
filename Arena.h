#ifndef RR_Arena
#define RR_Arena

#include "BasicTypes.h"

struct Arena
{
	u8 *base;
	u8 *current;
	u32 capacity;
};

extern Arena *constantArena;
extern Arena *frameArena;
extern Arena *workingArena;

#define PushStruct(arena, type) (type *)PushStruct_(arena, sizeof(type))
#define PushZeroStruct(arena, type) (type *)PushZeroStruct_(arena, sizeof(type))
#define PushArray(arena, type, size) (type *)PushStruct_(arena, size * sizeof(type))
#define PushZeroArray(arena, type, size) (type *)PushZeroStruct_(arena, size * sizeof(type))


static u8 *PushStruct_(Arena *arena, u32 sizeOfType)
{
	Assert(arena->current - arena->base + sizeOfType < arena->capacity);
	u8* ret = arena->current;
	arena->current += sizeOfType;
	return ret;
}
static u8 *PushZeroStruct_(Arena *arena, u32 sizeOfType)
{
	Assert(arena->current - arena->base + sizeOfType < arena->capacity);
	u8* ret = arena->current;
	for (u8*it = ret; (it - ret) < sizeOfType; it++)
	{
		*it = 0;
	}
	arena->current += sizeOfType;
	return ret;
}

static void Clear(Arena *arena)
{
	arena->current = arena->base;
}

static Arena *InitArena(void *memory, u32 capacity)
{
	u8 *it = (u8 *)memory;
	Arena *ret = (Arena *)memory;
	ret->base = it + sizeof(Arena);
	ret->capacity = capacity;
	ret->current = ret->base;

	return ret;
}
#endif // !RR_Arena

