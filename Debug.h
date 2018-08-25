#ifndef RR_DEBUG
#define RR_DEBUG


#define BEGIN_TIMED_BLOCK(ID) u64 startCycleCount##ID = __rdtsc();
#define BEGIN_TIMED_BLOCK_COUNTED(ID) u64 startCycleCountCounted##ID = __rdtsc();
#define END_TIMED_BLOCK(ID) debugCounters[DebugCycleCounter_##ID].cycleCount += __rdtsc() - startCycleCount##ID; ++debugCounters[DebugCycleCounter_##ID].hit;
#define END_TIMED_BLOCK_COUNTED(ID, count) debugCounters[DebugCycleCounter_##ID].cycleCount += (__rdtsc() - startCycleCountCounted##ID) ; debugCounters[DebugCycleCounter_##ID].hit += count;

#define DEBUG_MODE 1

#if 0
#define DEBUG_ON_OFF(arg) \
	static bool arg = DEBUG_MODE; \
	static bool debugSwitchInit##arg = false;\
	static Button *debugSwitchButton##arg = NULL;\
	if(!debugSwitchInit##arg)\
	{\
		debugSwitchButton##arg = new Button(Shape::Rectangle(10.0f + 125.0f + 250.0f * debugButtons->size, 40.0f, 250.0f, 40.0f), #arg, BoolToggle, ButtonEventStub, ButtonEventStub, &arg); \
		debugSwitchButton##arg->defaultColor = v4(0.7f, 1.0f, 0.0f, 0.0f);\
		debugSwitchButton##arg->hoverColor = v4(0.7f, 0.5f, 0.5f, 0.0f);\
		debugButtons->PushBack(debugSwitchButton##arg);\
		debugSwitchInit##arg = true;\
	}
#endif	

#define DEBUG_ON_OFF(arg, initialValue) \
	static bool arg = initialValue; \
	static bool debugSwitchInit##arg = false;\
	if(!debugSwitchInit##arg)\
	{\
		Button debugSwitchButton##arg = Button(V2(10.0f + 125.0f + 250.0f * debugButtons->size, 40.0f), 250.0f, 40.0f, CreateString(#arg), ButtomToggleBool, ButtonEventStub, ButtonEventStub, &arg); \
		debugSwitchButton##arg.defaultColor = V4(0.7f, 1.0f, 0.0f, 0.0f);\
		debugSwitchButton##arg.hoverColor = V4(0.7f, 0.5f, 0.5f, 0.0f);\
		debugButtons->PushBack(debugSwitchButton##arg);\
		debugSwitchInit##arg = true;\
	}\
	if(arg)


struct timed_block
{
	u64 startCycleCount;
	u32 id;

	timed_block(char * fileName, int lineNumber, char *function)
	{

	}

	~timed_block()
	{

	}

};

enum
{
	DebugCycleCounter_PixelTime,
	DebugCycleCounter_DrawRectangleSlowly,
	DebugCycleCounter_DrawBitmap,
	DebugCycleCounter_GameUpdateAndRender,
	DebugCycleCounter_SwitchBuffers,
	DebugCycleCounter_Frame,
	DebugCycleCounter_Count,
};


typedef struct DebugCycleCounter
{
	u64 cycleCount;
	u32 hit;
}DebugCycleCounter;

extern DebugCycleCounter debugCounters[DebugCycleCounter_Count];
#define MAX_DEBUG_BUTTON_COUNT 100


u32 GetDebugCounterIndex(char *fileName, int lineNumber, char *function, float camZ)
{
	return 0;
}


#endif
