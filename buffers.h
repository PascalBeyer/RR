#ifndef RR_BUFFERS
#define RR_BUFFERS

#ifndef  COMPILER_MSVC
#define COMPILER_MSVC 0
#endif // ! COMPILER_MSVC

#ifndef  COMPILER_LLVM
#define COMPILER_LLVM 0
#endif // ! COMPILER_MSVC

#if !COMPILER_MSVC && !COMPILER_MSVC
#if _MSC_VER

#undef COMPILER_MSVC 
#define COMPILER_MSVC 1

#else
#undef COMPILER_LLVM 
#define COMPILER_LLVM 1
#endif // 
#endif


#if COMPILER_MSVC
#include <intrin.h>
#else
#define BEGIN_TIMED_BLOCK(ID)
#define END_TIMED_BLOCK(ID)
#endif

#define Assert(Expression) if(!(Expression)) {__debugbreak();}
#define InvalidDefaultCase \
default: \
{\
	Die;\
}break;

template <typename F>
struct saucy_defer {
	F f;
	saucy_defer(F f) : f(f) {}
	~saucy_defer() { f(); }
};

template <typename F>
saucy_defer<F> defer_func(F f) {
	return saucy_defer<F>(f);
}

#define DEFER_1(x, y) x##y
#define DEFER_2(x, y) DEFER_1(x, y)
#define DEFER_3(x)    DEFER_2(x, __LINE__)
#define defer(code)   auto DEFER_3(_defer_) = defer_func([&](){code;})

#define RR_Internal 1

struct ColoredVertex
{
	v3 pos;
	u32 color;
};

DefineArray(ColoredVertex);
DefineArray(bool);
DefineArray(u32);


struct TexturedVertex
{
	v3 pos;
	v2 uv;
	u32 color;
};

struct RenderCommands
{
	u32 maxBufferSize;
	u32 pushBufferSize;
	u8 *pushBufferBase;

	u32 width;
	u32 height;
	f32 aspectRatio;

};

struct ImageBuffer
{
	void *memory;
	int height;
	int width;
	int pitch;
	int bytesPerPixel;
};

struct SoundBuffer
{
	i16 toneVolume;
	int samplesPerSecond;
	int runningSampleIndex;
	int bytesPerSample;
	int secondaryBufferSize;
	int latencySampleCount;
	bool soundIsPlaying;
	bool soundIsValid;
	i16 *soundSamples;
	int sampleAmount;
};



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