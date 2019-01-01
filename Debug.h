#ifndef RR_DEBUG
#define RR_DEBUG

// todo make this good
#define DEBUG 1


#define DEBUG_AMOUNT_OF_DEBUG_FRAMES 30

struct FrameTimeInfo
{
	u32 cycles;
	u32 hitCount;
};

DefineArray(FrameTimeInfo);
struct DebugFrame
{
	FrameTimeInfoArray times; // this is parrallel to DebugRecord, i.e. exectly one for each timed function.
};


enum TweekerType
{
	Tweeker_Invalid,
	Tweeker_b32,
	Tweeker_u32,
	Tweeker_f32,
	Tweeker_v2,
	Tweeker_v3,
	Tweeker_v4,

	Tweeker_v3i,
	Tweeker_EulerAngle,
	Tweeker_String,
	Tweeker_EntityType,
};

union TweekerValue
{
	u32 u;
	f32 f;
	v2 vec2;
	v3 vec3;
	v4 vec4;
	b32 b;
};

static TweekerValue CreateTweekerValue()
{
	return {};
}
static TweekerValue CreateTweekerValue(u32 u)
{
	TweekerValue ret = {};
	ret.u = u;
	return ret;
}
static TweekerValue CreateTweekerValue(f32 f)
{
	TweekerValue ret = {};
	ret.f = f;
	return ret;
}
static TweekerValue CreateTweekerValue(v2 vec2)
{
	TweekerValue ret = {};
	ret.vec2 = vec2;
	return ret;
}
static TweekerValue CreateTweekerValue(v3 vec3)
{
	TweekerValue ret = {};
	ret.vec3 = vec3;
	return ret;
}
static TweekerValue CreateTweekerValue(v4 vec4)
{
	TweekerValue ret = {};
	ret.vec4 = vec4;
	return ret;
}
static TweekerValue CreateTweekerValue(b32 b)
{
	TweekerValue ret = {};
	ret.b = b;
	return ret;
}


struct Tweeker //todo stupid way for now, if this is to slow hash table. @scope speedup, part3 minute 30
{
	TweekerType type;
	String name; // not sure, we could make these char* as they do not change after compile time... and its more speedy
	String function;

	union // could make this not a union, but its okay like this we can have an array and maybe hash easier later
	{
		TweekerValue value;
		u32 u;
		f32 f;
		v2 vec2;
		v3 vec3;
		v4 vec4;
		b32 b;
	};
};

static Tweeker CreateTweeker(TweekerType type, char *name, TweekerValue value)
{
	Tweeker ret;
	//ret.function;
	ret.name = CreateString(name);
	ret.type = type;
	ret.value = value;

	return ret;
}


#define Tweekable1(type, name, initalValue) type name = *(type *)MaybeAddTweekerReturnValue(#name, Tweeker_##type, __FUNCTION__, CreateTweekerValue(initalValue));
#define Tweekable2(type, name) type name = *(type *)MaybeAddTweekerReturnValue(#name, Tweeker_##type, __FUNCTION__);


#define Tweekable(...) Expand(GET_MACRO3(__VA_ARGS__, Tweekable1, Tweekable2, Die)(__VA_ARGS__))

static void *MaybeAddTweekerReturnValue(char *_name, TweekerType type, char *function);
static void *MaybeAddTweekerReturnValue(char *_name, TweekerType type, char *function, TweekerValue value);

DefineDynamicArray(Tweeker);



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
	b32 paused;
	b32 firstFrame;
	b32 drawDebug;

	u32 eventIndex;
	u32 amountOfEventsLastFrame;
	f32 lastFrameTime;
	DebugEvent events[DEBUG_AMOUNT_OF_DEBUG_FRAMES][MAX_DEBUG_EVENT_COUNT]; // reseting double buffer
	Arena *arena;

	File *tweekerFile;
	TweekerDynamicArray tweekers;
	
	DebugUIElementArray uiElements;

	DebugFrame debugFrames[DEBUG_AMOUNT_OF_DEBUG_FRAMES];
	u32 rollingFrameIndex;
};

static DebugState globalDebugState;

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
	if (globalDebugState.paused)
	{
		globalDebugState.eventIndex = 0;
		return;
	}
	globalDebugState.amountOfEventsLastFrame = globalDebugState.eventIndex;
	globalDebugState.eventIndex = 0;
	globalDebugState.rollingFrameIndex = (globalDebugState.rollingFrameIndex + 1) % DEBUG_AMOUNT_OF_DEBUG_FRAMES;

}


inline void RecordDebugEvent(u32 index, DebugEvenType type, u32 hitCounter)
{
	DebugEvent *event = &globalDebugState.events[globalDebugState.rollingFrameIndex][globalDebugState.eventIndex];
	++globalDebugState.eventIndex;
	Assert(globalDebugState.eventIndex < MAX_DEBUG_EVENT_COUNT);
	event->cycles = (u32)__rdtsc();
	event->debugRecordIndex = index;
	event->type = type;
	event->hitMultiplicity = hitCounter;
}

#endif
