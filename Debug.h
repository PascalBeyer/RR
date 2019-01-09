#ifndef RR_DEBUG
#define RR_DEBUG

// todo make this good
#define DEBUG 1

#define TimedBlock TimedBlock_ ___timed##__FILE__##__LINE__ = TimedBlock_(__FILE__, __LINE__, __FUNCTION__, __COUNTER__)

#define MAX_DEBUG_EVENT_COUNT (655356)

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
	u32 hitMultiplicity;
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


extern u32 const debugRecordsAmount;
extern DebugBlockInfo debugInfoArray[];

inline void RecordDebugEvent(u32 index, DebugEvenType type, u32 hitCounter);

struct TimedBlock_
{
	u64 startCycleCount;
	u32 hitCounter;
	u16 id;
   
	TimedBlock_(char * fileName, int lineNumber, char *function, u16 count, u32 hitCounter = 1)
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

struct File;
struct DebugUITweekerFile;
struct Arena;

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
	b32 drawDebug;
   
	u32 eventIndex;
	u32 amountOfEventsLastFrame;
	f32 lastFrameTime;
	DebugEvent events[DEBUG_AMOUNT_OF_DEBUG_FRAMES][MAX_DEBUG_EVENT_COUNT]; // reseting double buffer
	Arena *arena;
   
	DebugUIElementArray uiElements;
   
	DebugFrame debugFrames[DEBUG_AMOUNT_OF_DEBUG_FRAMES];
	u32 rollingFrameIndex;
};

static DebugState globalDebugState;


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
	event->debugRecordIndex = (u16)index;
	event->type = (u16)type;
	event->hitMultiplicity = hitCounter;
}

#endif
