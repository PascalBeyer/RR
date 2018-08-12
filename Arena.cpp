#include "Arena.h"

u8 *PushStruct_(Arena *arena, u32 sizeOfType)
{
	Assert(arena->current - arena->base + sizeOfType < arena->capacity);
	u8* ret = arena->current;
	arena->current += sizeOfType;
	return ret;
}
u8 *PushZeroStruct_(Arena *arena, u32 sizeOfType)
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

void Clear(Arena *arena)
{
	arena->current = arena->base;
}

Arena *InitArena(void *memory, u32 capacity)
{
	u8 *it = (u8 *)memory;
	Arena *ret = (Arena *)memory;
	ret->base = it + sizeof(Arena);
	ret->capacity = capacity;
	ret->current = ret->base;

	return ret;
}