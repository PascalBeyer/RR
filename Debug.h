#ifndef RR_DEBUG
#define RR_DEBUG

// todo make this good
#define DEBUG 1

#define TimedBlock TimedBlock_ ___timed##__FILE__##__LINE__ = TimedBlock_(__FILE__, __LINE__, __FUNCTION__, __COUNTER__)

#define MAX_DEBUG_EVENT_COUNT (65536)

//todo make debug stuff thread local?
enum DebugEvenType
{
	DebugEvent_BeginTimedBlock,
	DebugEvent_EndTimedBlock
};

// todo: thread stuff
struct DebugEvent //todo : make this more of a tagged union?
{
	u32 cycles;
	u32 hitCount;
	u16 debugRecordIndex;
	u16 type;
};

struct DebugBlockInfo
{
	char *function;
	char *file;
	u32 line;
	u32 color;
};

struct SplittBlock;
struct DebugFrameList;

enum TweekerType
{
	Tweeker_EndOfBuffer,
	Tweeker_b32,
	Tweeker_u32,
	Tweeker_f32,
	Tweeker_v2,
	Tweeker_v3,
	Tweeker_v4,
};

struct Tweeker //todo stupid way for now, if this is to slow hash table.
{
	TweekerType type;
	String name;
	union // could make this not a union, but its okay like this we can have an array and maybe hash easier later
	{
		u32 u;
		f32 f;
		v2 vec2;
		v3 vec3;
		v4 vec4;
		b32 b;
	};
};

#define Tweekable(name, type) type name = *(type *)MaybeAddTweekerReturnValue(#name, Tweeker_##type);
static void *MaybeAddTweekerReturnValue(char *_name, TweekerType type);

struct Arena;

struct TweekerDynamicArray
{
	Tweeker *data;
	Arena *arena;
	u32 amount;
	u32 capacity;

	Tweeker operator[] (u32 i)
	{
		Assert(i < amount);
		return data[i];
	}
};

static Tweeker *ArrayAdd(TweekerDynamicArray *arr, Tweeker t) //todo leaking, when we have dynamic allocators, use those
{
	if (arr->amount + 1 < arr->capacity)
	{
		(arr->data)[arr->amount++] = t;
		return arr->data + (arr->amount - 1);
	}
	
	u32 newCapacity = 2 * arr->capacity + 1;

	Tweeker *newData = PushArray(arr->arena, Tweeker, 2 * arr->capacity + 1);
	memcpy(newData, arr->data, arr->capacity * sizeof(Tweeker));
	arr->capacity = 2 * arr->capacity + 1;
	arr->data = newData;
	arr->data[arr->amount++] = t;

	return (arr->data + (arr->amount - 1));
}

static TweekerDynamicArray CreateDynamicArray(Arena *arena, u32 capacity = 8)
{
	TweekerDynamicArray ret;
	ret.data = PushArray(arena, Tweeker, capacity);
	ret.arena = arena;
	ret.amount = 0;
	ret.capacity = capacity;
	return ret;
}

struct DebugState
{
	DebugEvent events[2][MAX_DEBUG_EVENT_COUNT]; // reseting double buffer

	Arena *arena;

	TweekerDynamicArray tweekers;
	
	u32 eventIndex;
	u32 eventArrayIndex;
	u32 lastFramesEventIndex;

	b32 paused;

	DebugFrameList *frameHead; // inserting into Tail, so tail is the newest
	DebugFrameList *frameTail; // todo : make this an array again?
	DebugFrameList *frameFree;
};


static DebugState globalDebugState;
extern u32 const DebugRecordsAmount;
extern DebugBlockInfo debugInfoArray[];

inline void ResetDebugState()
{
	globalDebugState.lastFramesEventIndex = globalDebugState.eventIndex;
	globalDebugState.eventIndex = 0;
	globalDebugState.eventArrayIndex = (globalDebugState.eventArrayIndex + 1) % 2;
}

inline void RecordDebugEvent(u32 index, DebugEvenType type, u32 hitCounter)
{
	DebugEvent *event = globalDebugState.events[globalDebugState.eventArrayIndex] + globalDebugState.eventIndex;
	++globalDebugState.eventIndex;
	Assert(globalDebugState.eventIndex < MAX_DEBUG_EVENT_COUNT);
	event->cycles = (u32)__rdtsc();
	event->debugRecordIndex = index;
	event->type = type;
	event->hitCount = hitCounter;
}

struct TimedBlock_
{
	u64 startCycleCount;
	u32 hitCounter;
	u16 id;
	
	TimedBlock_(char * fileName, int lineNumber, char *function, u32 count, u32 hitCounter = 1)
	{
		DebugBlockInfo *record = debugInfoArray + count;
		record->function = function;
		record->file = fileName;
		record->line = lineNumber;
		id = count;
		this->hitCounter = hitCounter;
		RecordDebugEvent(id, DebugEvent_BeginTimedBlock, hitCounter);
	}

	~TimedBlock_()
	{
		RecordDebugEvent(id, DebugEvent_EndTimedBlock, hitCounter);
	}

};


#endif
