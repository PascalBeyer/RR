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

static bool IsPowerOfTwo(u32 value)
{
	u8 highestBit = (u8)BitwiseScanReverse(value);
   
	return (value == (1u << highestBit));
};


//todo maybe make alloc optional by using this va_args macro stuff... have to understand that at somepoint anyway.
#define DynamicAlloc(type, size) (type *)BuddyAlloc(alloc, (size) * sizeof(type))
#define DynamicAllocArray(type, size) {(type *)BuddyAlloc(alloc, (size) * sizeof(type)), (size)}
#define DynamicFree(mem) BuddyFree(alloc, mem)

enum BuddyBlockNodeState
{
	Buddy_Unused,
	Buddy_Used,
	Buddy_SplitNode
};

DefineArray(BuddyBlockNodeState);
struct BuddyAllocator
{
	u32 size; // power of two smaller then 4 gigs.
	u32 logMinimumBlockSize; // see above
	u8 *base;
	u32 treeDepth;
	BuddyBlockNodeStateArray implicitBinaryTree;
	// true  - there is something filling this block for the size size / 2^depth
	// false - you have to go in further to decide.
   
	// Todo : make this actually use only 1 bit, per element 
	// WARNING: Bits "below" the first true, default to false, so we do not need to recurse everytime we insert or free.
};

static u32 IntegerLogarithm(u32 value)
{
	return BitwiseScanReverse(value);
}

static BuddyAllocator CreateBuddyAllocator(Arena *arena, u32 size, u32 minimumBlockSize)
{
	Assert(size);
	Assert(arena);
	Assert(size > minimumBlockSize);
	BuddyAllocator ret;
	u8 highestBit = (u8)BitwiseScanReverse(size);
	Assert(size == (1u << highestBit)); // is power of two
	u8 logMinimumBlockSize = (u8)BitwiseScanReverse(minimumBlockSize);
	Assert(minimumBlockSize == (1u << logMinimumBlockSize));
	u32 amountOfBlocks = size >> logMinimumBlockSize;
   
	ret.logMinimumBlockSize = logMinimumBlockSize;
	ret.treeDepth = IntegerLogarithm(amountOfBlocks);
	// a tree with 2^n leafs needs 2^(n+ 1) - 1 vertecies by geometric sum
	ret.implicitBinaryTree = PushZeroArray(arena, BuddyBlockNodeState, 2 * amountOfBlocks - 1); 
	ret.size = size;
	ret.base = PushData(arena, u8, size);
	return ret;
};

static void *BuddyAlloc(BuddyAllocator *alloc, u32 size)
{
	TimedBlock;
	Assert(size);// maybe we should allow this and return NULL.
	Assert(alloc);
	Assert(size < alloc->size); 
	u32 logDesiredSize = Max(IntegerLogarithm(size - 1) + 1, alloc->logMinimumBlockSize);
	u32 desiredSize = (1 << logDesiredSize);
	//Note : we have to move down the Tree, even tho we know that the "right" sized blocks are at alloc->implicitBoolArray[intoIndexSpace], because we don't want to recurse all the way down the tree, everytime we allocate something.
   
	u32 intoIndexSpace = (desiredSize >> alloc->logMinimumBlockSize); 
   
	void *ret = NULL;
	// todo maybe make this stackless.
	DeferRestore(frameArena);
	u32Array stack = PushZeroArray(frameArena, u32, 1);
	while (stack.amount)
	{
		u32 currentIndex = stack[stack.amount - 1]; stack.amount--;
		u32 depth = IntegerLogarithm(currentIndex); //BinaryTreeDepthFromIndex(currentIndex);
		auto node = alloc->implicitBinaryTree[currentIndex];
		u32 nodeSize = (alloc->size >> depth);
		if (node == Buddy_Used) continue;
		if (desiredSize > nodeSize) continue;
      
		if (node == Buddy_SplitNode)
		{
			//push left and right neighboor
			u32 relativeIndex = currentIndex - (1 << depth) + 1; // this is the index in the "row" in the binary tree.
			u32 childrenIndex = ((1 << (depth + 1)) - 1) + relativeIndex * 2;
			BuildStaticArray(frameArena, stack, childrenIndex);
			BuildStaticArray(frameArena, stack, childrenIndex + 1);
			continue;
		}
		
		Assert(node == Buddy_Unused);
      
		if (desiredSize == nodeSize)
		{
			// we are done
			u32 relativeIndex = currentIndex - (1 << depth) + 1;
			u32 coDepth = alloc->treeDepth - depth;
			u32 offset = (relativeIndex << (coDepth + alloc->logMinimumBlockSize));
			Assert(offset < alloc->size);
			alloc->implicitBinaryTree[currentIndex] = Buddy_Used;
			return (alloc->base + offset);
		}
      
		Assert(desiredSize < nodeSize)
         // push left and right neighboor
         u32 relativeIndex = currentIndex - (1 << depth) + 1; // this is the index in the "row" in the binary tree.
		u32 childrenIndex = ((1 << (depth + 1)) - 1) + relativeIndex * 2;
      
		alloc->implicitBinaryTree[currentIndex] = Buddy_SplitNode;
		BuildStaticArray(frameArena, stack, childrenIndex);
		BuildStaticArray(frameArena, stack, childrenIndex + 1);
	};
	Assert(ret);
   
	return ret;
}

static void BuddyFree(BuddyAllocator *buddy, void *_mem)
{
	TimedBlock;
	u8 *mem = (u8 *)_mem;
	Assert(buddy->base + buddy->size > mem);
	u32 offset = (u32)(mem - buddy->base);
	u32 blockSize = (1 << buddy->logMinimumBlockSize);
	Assert(!(offset & (blockSize - 1))); // offset is aligned to blockSize
	u32 amountOfBlocks = buddy->size >> buddy->logMinimumBlockSize;
   
	u32 blockIndex = offset >> buddy->logMinimumBlockSize;
	u32 depth = buddy->treeDepth;
	u32 relIndex = blockIndex;
	u32 currentIndex = blockIndex + (amountOfBlocks - 1);
	auto node = buddy->implicitBinaryTree[currentIndex];
	while (node == Buddy_Unused) // all things below are Buddy_Unused.
	{
		// go to parent
		relIndex >>= 1; 
		currentIndex = (1 << --depth) - 1 + relIndex;
		node = buddy->implicitBinaryTree[currentIndex];
	}
   
	u32 otherChildIndex = ((currentIndex - 1) ^ 1) + 1; // flippes the last bit,  -1 / 1 because we have an even number of leafes, but an uneven Number of nodes
	auto otherChild = buddy->implicitBinaryTree[otherChildIndex];
	do {
		buddy->implicitBinaryTree[currentIndex] = Buddy_Unused;
      
		otherChildIndex = ((currentIndex - 1) ^ 1) + 1;
      
		otherChild = buddy->implicitBinaryTree[otherChildIndex];
		relIndex >>= 1;
		currentIndex = (1 << --depth) - 1 + relIndex;
		node = buddy->implicitBinaryTree[currentIndex];
		Assert(node == Buddy_SplitNode); // todo : right now we do not treverse up to set Buddy_Used, maybe we should.
	} while (depth && otherChild == Buddy_Unused);
   
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

#define DynamicAllocatorRange 8
#define DynamicAllocatorMinimum 8

struct PooledFixedSizeAllocator
{
	MemoryPage *pools[DynamicAllocatorRange];
	ReferanceCountedArena *smallStuffArena;
	Arena *arena;
};

static BuddyAllocator *alloc;


// gives Pages between 2^DynamicAllocatorMinimum to 2^(DynamicAllocatorMinimum) * DynamicAllocatorRange
static void *PooledFixedSizeAlloc_(PooledFixedSizeAllocator *fsa, u32 size) 
{
	Assert(fsa);
	u32 shrinked = (size >> DynamicAllocatorMinimum);
	Assert(shrinked < DynamicAllocatorRange);
   
	if (shrinked == 0) // bundle small stuff
	{
		auto arena = fsa->smallStuffArena;
      
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
		u32 pageSize = (1 << DynamicAllocatorMinimum); // an n-page is ((n+1) << DynamicAllocatorMinimum) bytes long
		if (fsa->pools[0])
		{
			mem = (u8 *)fsa->pools[0];
			fsa->pools[0] = fsa->pools[0]->next;
		}
		else
		{
			Assert(pageSize >= actualSize);
			mem = PushData(fsa->arena, u8, pageSize);
		}
      
		fsa->smallStuffArena = (ReferanceCountedArena *)mem;
		arena = fsa->smallStuffArena;
		
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
   
	if (fsa->pools[shrinked])
	{
		u8 *mem = (u8 *)fsa->pools[shrinked];
		*mem++ = (u8)shrinked;
		fsa->pools[shrinked] = fsa->pools[shrinked]->next;
		return  (void *)mem;
	}
	
	// add new page
	{
		u32 pageSize = ((shrinked + 1) << DynamicAllocatorMinimum);
		Assert(pageSize > size); // we need one extra
		u8 *mem = PushData(fsa->arena, u8, pageSize);
		*mem++ = (u8)shrinked;
		return (void *)mem;
	}
}

static void PooledFixedSizeFree(PooledFixedSizeAllocator *fsa, void *mem)
{
	Assert(mem);
	u8 *data = (u8 *)mem;
	data--;
	Assert(*data < DynamicAllocatorRange);
	u8 index = *data;
   
	if (index == 0)
	{
		data -= 2;
		u16 distToBase = *(u16 *)data;
		ReferanceCountedArena *arr = (ReferanceCountedArena *)(data - distToBase);
		if ((--arr->allocatedItems) == 0)
		{
			MemoryPage *toInsert = (MemoryPage *)data;
			toInsert->next = fsa->pools[0];
			fsa->pools[0] = toInsert;
			return;
		}
      
		return;
	}
   
	MemoryPage *toInsert = (MemoryPage *)data;
	toInsert->next = fsa->pools[index];
	fsa->pools[index] = toInsert;
}

#undef DynamicAllocatorRange
#undef DynamicAllocatorMinimum

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

