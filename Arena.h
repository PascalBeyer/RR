#ifndef RR_Arena
#define RR_Arena

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
#define PushArray(arena, type, size) {(type *)PushStruct_(arena, (size) * sizeof(type)), (size)}
#define PushZeroArray(arena, type, size) {(type *)PushZeroStruct_(arena, (size) * sizeof(type)), (size)}
#define PushData(arena, type, size) (type *)PushStruct_(arena, (size) * sizeof(type))
#define PushZeroData(arena, type, size) (type *)PushZeroStruct_(arena, (size) * sizeof(type))

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
	//memset(arena->current, 0, sizeOfType); todo once we are rid of the libraries slot this in

	u8* it = ret;
	for (u32 i = 0; i < sizeOfType; i++)
	{
		*it++ = 0;
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

struct MemoryPage
{
	void *page;
	MemoryPage *next;
};

struct ReferanceCountedArena
{
	u8 *base;
	u8 *current;
	u32 capacity;
	u32 allocatedItems;
};


struct DynamicAllocator
{
	MemoryPage *pools[256];
	ReferanceCountedArena *smallStuffArena;
};

extern DynamicAllocator *alloc;

//todo maybe make alloc optional by using this va_args macro stuff... have to understand that at somepoint anyway.
#define DynamicAlloc(alloc, type, size) (type *)DynamicAlloc_(alloc, (size) * sizeof(type))

// right now allocate up to 8 megabytes, per instance. Allocates in the range of 2^15 = 32768 bytes to 8 megs right now.
// We do something, where small stuff gets put into a fixed Arena, that gets dynamically allocated as a pool of size 1.
static void *DynamicAlloc_(DynamicAllocator *alloc, u32 size) 
{
	Assert(alloc);
	u32 shrinked = (size >> 15);
	Assert(shrinked < 256);

	if (shrinked == 0) // bundle small stuff
	{
		auto arena = alloc->smallStuffArena;

		//				 size + shrinked   + distance to arena->base
		u32 actualSize = size + sizeof(u8) + sizeof(u16);

		if (arena && ((arena->current - arena->base + actualSize) <= arena->capacity)) 
		{
			u8 *ret = arena->current; 

			
			*(u16 *)ret++ = (u16)(arena->base - arena->current); // this happens before so the check is unified with the other pools
			*ret++ = 0;

			arena->current += actualSize;
			arena->allocatedItems++;
			return ret;
		}

		// todo: here might be a thing, where we have a problem with actual size vs size, so that something with very specific sizes could not be allocated

		u8 *mem;
		u32 pageSize = (1 << 15); // an n-page is ((n+1) << 15) bytes long
		if (alloc->pools[0])
		{
			mem = (u8 *)alloc->pools[0];
			alloc->pools[0] = alloc->pools[0]->next;
		}
		else
		{
			Assert(pageSize >= actualSize);
			mem = PushData(constantArena, u8, pageSize);
		}

		alloc->smallStuffArena = (ReferanceCountedArena *)mem;
		arena = alloc->smallStuffArena;
		
		arena->allocatedItems = 1;
		arena->base = mem;
		arena->capacity = pageSize;
		arena->current = arena->base + sizeof(ReferanceCountedArena);

		u8 *ret = arena->current;
		arena->current += actualSize;

		*(u16 *)ret++ = (u16)(arena->base - arena->current);
		*ret++ = 0;

		return ret;
	}

	Assert(shrinked);

	if (alloc->pools[shrinked])
	{
		u8 *mem = (u8 *)alloc->pools[shrinked];
		*mem++ = (u8)shrinked;
		alloc->pools[shrinked] = alloc->pools[shrinked]->next;
		return  (void *)mem;
	}
	
	// add new page
	{
		u32 pageSize = ((shrinked + 1) << 15);
		Assert(pageSize > size); // we need one extra
		u8 *mem = PushData(constantArena, u8, pageSize);
		*mem++ = (u8)shrinked;
		return (void *)mem;
	}
};

static void DynamicFree(DynamicAllocator *alloc, void *mem)
{
	Assert(mem);
	u8 *data = (u8 *)mem;
	data--;
	Assert(*data < 256);
	u8 index = *data;

	if (index == 0)
	{
		data -= 2;
		u16 distToBase = *(u16 *)data;
		ReferanceCountedArena *arr = (ReferanceCountedArena *)(data - distToBase);
		if ((--arr->allocatedItems) == 0)
		{
			MemoryPage *toInsert = (MemoryPage *)data;
			toInsert->next = alloc->pools[0];
			alloc->pools[0] = toInsert;
			return;
		}

		return;
	}

	MemoryPage *toInsert = (MemoryPage *)data;
	toInsert->next = alloc->pools[index];
	alloc->pools[index] = toInsert;
}

//todo  make own memcpy, fast one at that
#if 0
static void memcpy(void *source, void *dest, u32 numberOfBytes)
{
	u8 *destu8 = (u8 *)dest;
	u8 *sourceu8 = (u8 *)source;

	for (u32 i = 0; i < numberOfBytes; i++)
	{
		*destu8++ = *sourceu8++;
	}
}
#endif
#endif // !RR_Arena

