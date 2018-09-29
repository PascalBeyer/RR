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

#include "BasicTypes.h"


#if COMPILER_MSVC
#include <intrin.h>
#else
#define BEGIN_TIMED_BLOCK(ID)
#define END_TIMED_BLOCK(ID)
#endif

#define RR_Internal 1


struct ColoredVertex
{
	v3 pos;
	u32 color;
};

DefineArray(ColoredVertex);

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
	f32 focalLength;
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
	s16 toneVolume;
	int samplesPerSecond;
	int runningSampleIndex;
	int bytesPerSample;
	int secondaryBufferSize;
	int latencySampleCount;
	bool soundIsPlaying;
	bool soundIsValid;
	s16 *soundSamples;
	int sampleAmount;
};


#endif 