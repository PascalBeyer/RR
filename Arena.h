#ifndef RR_Arena
#define RR_Arena
#include "buffers.h"


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


u8 *PushStruct_(Arena *arena, u32 sizeOfStruct);
u8 *PushZeroStruct_(Arena *arena, u32 sizeOfStruct);
void Clear(Arena *arena);
Arena *InitArena(void *memory, u32 capacity);


#endif // !RR_Arena

