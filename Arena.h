#ifndef RR_Arena
#define RR_Arena

struct Arena
{
	u8 *base;
	u8 *current;
	u32 capacity;
};

static Arena *frameArena;

#define PushStruct(arena, type) (type *)PushStruct_(arena, sizeof(type))
#define PushZeroStruct(arena, type) (type *)PushZeroStruct_(arena, sizeof(type))
#define PushArray(arena, type, size) {(type *)PushStruct_(arena, (size) * sizeof(type)), (size)}
#define PushZeroArray(arena, type, size) {(type *)PushZeroStruct_(arena, (size) * sizeof(type)), (size)}
#define PushData(arena, type, size) (type *)PushStruct_(arena, (size) * sizeof(type))
#define PushZeroData(arena, type, size) (type *)PushZeroStruct_(arena, (size) * sizeof(type))
#define PopStruct(arena, type) *(type *)PopStruct_(arena, sizeof(type))

#define DeferRestore(arena) u8* saveCurrentArenaPtr = arena->current; defer(arena->current = saveCurrentArenaPtr);

static u8 *PopStruct_(Arena *arena, u32 sizeOfType)
{
	Assert((u32)(arena->current - arena->base) > sizeOfType);
	arena->current -= sizeOfType;
	return arena->current;
}

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

static Arena *PushArena(Arena *arena, u32 capacity)
{
	u32 size = capacity + sizeof(Arena);
	u8 *data = PushData(arena, u8, size);
	return InitArena(data, capacity);
}

// this leaks whenever buckets has to be resized. 
// this also leaks the rest of a bucket when we do not fit.
struct BucketBuffer
{
   Arena *arena;
   u16 bucketSize;
   u16 currentBucket;
   u16 currentOffset;
   u8PtrArray buckets;
};

static void *PushStruct_(BucketBuffer *buffer, u32 _size)
{
   Assert(_size < buffer->bucketSize);
   u16 size = (u16)_size;
   if(buffer->currentOffset + size < buffer->bucketSize)
   {
      u8 *ret = buffer->buckets[buffer->currentBucket] + buffer->currentOffset;
      buffer->currentOffset += size;
      return ret;
   }
   Assert(buffer->currentBucket < MAX_U16);
   
   u8 *newBucket = PushData(buffer->arena, u8, buffer->bucketSize);
   if(buffer->currentBucket + 1u >= buffer->buckets.amount)
   {
      u8PtrArray newArray = PushArray(buffer->arena, u8Ptr, buffer->buckets.amount * 2u);
      memcpy(buffer->buckets.data, newArray.data, buffer->buckets.amount * sizeof(u8Ptr));
      buffer->buckets = newArray;
   }
   
   buffer->buckets[buffer->currentBucket++] = newBucket;
   buffer->currentOffset = size;
   return newBucket;
}

static BucketBuffer CreateBucketBuffer(Arena *arena, u16 bucketSize)
{
   BucketBuffer ret;
   ret.arena = arena;
   ret.bucketSize = bucketSize;
   ret.currentBucket = 0;
   ret.currentOffset = 0;
   ret.buckets = PushArray(arena, u8Ptr, 10);
}

#endif // !RR_Arena

