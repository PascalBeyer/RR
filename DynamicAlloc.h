
// apperantly having a space after \ ruines the define.
#define DefineDynamicArray(type)											   	\
struct type##DynamicArray													  	\
{																				  \
	type *data;																	\
   BuddyAllocator *alloc;                                                         \
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
	type *newData = DynamicAlloc(arr->alloc, type, newCapacity);							\
	memcpy(newData, arr->data, arr->capacity * sizeof(type));					\
	arr->capacity = newCapacity;												\
	DynamicFree(arr->alloc, arr->data);														\
	arr->data = newData;														\
	arr->data[arr->amount++] = t;												\
   \
	return (arr->amount - 1);													\
}																				\
\
static type##DynamicArray type##CreateDynamicArray(BuddyAllocator *alloc, u32 capacity = 8)			\
{																				\
	type##DynamicArray ret;														\
	ret.data = DynamicAlloc(alloc, type, capacity);									\
   ret.alloc = alloc; \
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
	type *newData = DynamicAlloc(arr->alloc, type, capacity);								\
	memcpy(newData, arr->data, arr->capacity * sizeof(type));					\
	arr->capacity = capacity;													\
	DynamicFree(arr->alloc, arr->data);														\
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



#define DynamicAlloc(alloc, type, size) (type *)BuddyAlloc(alloc, (size) * sizeof(type))
#define DynamicAllocArray(alloc, type, size) {(type *)BuddyAlloc(alloc, (size) * sizeof(type)), (size)}
#define DynamicFree(alloc, mem) BuddyFree(alloc, mem)

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

static BuddyAllocator *globalAlloc;

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

DefineDynamicArray(u32Ptr);
DefineDynamicArray(u32);



DefineDynamicArray(v3Ptr);


DefineDynamicArray(u8);

DefineDynamicArray(u8Ptr);


DefineDynamicArray(u16Ptr);

DefineDynamicArray(v2);
DefineDynamicArray(v3);
DefineDynamicArray(v4);

DefineArray(m4x4);
DefineArray(m4x4Array);
