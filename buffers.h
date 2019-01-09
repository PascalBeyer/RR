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

// todo these have to be here, because I use em before Array.h, this is not great
DefineArray(bool);
DefineArray(u32);


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


#endif 