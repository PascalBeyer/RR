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
	Tweeker_Invalid,
	Tweeker_b32,
	Tweeker_u32,
	Tweeker_f32,
	Tweeker_v2,
	Tweeker_v3,
	Tweeker_v4,
	Tweeker_String,

};

struct Tweeker //todo stupid way for now, if this is to slow hash table. @scope speedup, part3 minute 30
{
	TweekerType type;
	String name; // not sure, we could make these char* as they do not change after compile time... and its more speedy
	String function;

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

#define Tweekable(type, name) type name = *(type *)MaybeAddTweekerReturnValue(#name, Tweeker_##type, __FUNCTION__);
static void *MaybeAddTweekerReturnValue(char *_name, TweekerType type, char *function);

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

static Tweeker *ArrayAdd(TweekerDynamicArray *arr, Tweeker t)
{
	if (arr->amount + 1 < arr->capacity)
	{
		(arr->data)[arr->amount++] = t;
		return arr->data + (arr->amount - 1);
	}
	
	u32 newCapacity = 2 * arr->capacity + 1;

	Tweeker *newData = DynamicAlloc(Tweeker,  2 * arr->capacity + 1);
	memcpy(newData, arr->data, arr->capacity * sizeof(Tweeker));
	arr->capacity = 2 * arr->capacity + 1;
	DynamicFree(arr->data);
	arr->data = newData;
	arr->data[arr->amount++] = t;

	return (arr->data + (arr->amount - 1));
}

static TweekerDynamicArray CreateDynamicArray(Arena *arena, u32 capacity = 8)
{
	TweekerDynamicArray ret;
	ret.data = DynamicAlloc(Tweeker, capacity);
	ret.arena = arena;
	ret.amount = 0;
	ret.capacity = capacity;
	return ret;
}


struct File;

struct TweekerRenderList
{
	Tweeker *tweeker;
	TweekerRenderList *next;
};

struct DebugUITweekerFile
{
	Char *function; // we do pointer compare
	TweekerRenderList *first;
	DebugUITweekerFile *next;
};

struct DebugUIElement
{
	u16 type;
	u16 flag;
	union
	{
		DebugUITweekerFile *tweeker;
	}; // I was here
};

DefineArray(DebugUIElement);

struct DebugState
{
	DebugEvent events[2][MAX_DEBUG_EVENT_COUNT]; // reseting double buffer

	Arena *arena;

	File *tweekerFile;
	TweekerDynamicArray tweekers;
	
	u32 eventIndex;
	u32 eventArrayIndex;
	u32 lastFramesEventIndex;

	b32 paused;

	DebugUIElementArray uiElements;

	DebugFrameList *frameHead; // inserting into Tail, so tail is the newest
	DebugFrameList *frameTail; // todo : make this an array again?
	DebugFrameList *frameFree;
};


static DebugState globalDebugState;
extern u32 const DebugRecordsAmount;
extern DebugBlockInfo debugInfoArray[];

static Tweeker *GetTweeker(String name)
{
	For(globalDebugState.tweekers)
	{
		if (name == it->name)
		{
			return it;
		}
	}
	return NULL;
}

static void Tweek(Tweeker t)
{
	Tweeker *toAlter = GetTweeker(t.name);
	if (toAlter)
	{
		toAlter->vec4 = t.vec4;
	}
	else
	{
		Die; //ConsoleOutputError("Tried to tweek Tweeker %s, but could not find it!", t.name); dont have this here. it's fine
	}
}

static String TweekerToString(Tweeker t, Arena *arena = frameArena)
{
	switch (t.type)
	{
	case Tweeker_b32:
	{
		if (t.b)
		{
			return S("true", arena);
		}
		return S("false", arena);

	}break;
	case Tweeker_u32:
	{
		return UtoS(t.u, arena);
	}break;
	case Tweeker_f32:
	{
		return FtoS(t.f, arena);
	}break;

	case Tweeker_v2:
	{
		return V2toS(t.vec2, arena);
	}break;

	case Tweeker_v3:
	{
		return V3toS(t.vec3, arena);
	}break;
	case Tweeker_v4:
	{
		return V4toS(t.vec4, arena);
	}break;
	default:
		break;
	}
	return S("");
}


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
