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

#define Internal 1
#include "BasicTypes.h"

#define ArrayCount(a) (sizeof(a)/sizeof(*a))
#define MegaBytes(a) (1024 * KiloBytes(a))
#define KiloBytes(a) (1024 * (a))
#define OffsetOf(type, Member) (umm)&(((type *)0)->Member)
#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#define Die Assert(false)


struct ColoredVertex;
struct RenderCommands
{
	u32 maxBufferSize;
	u32 pushBufferSize;
	u8 *pushBufferBase;
	float inGameWidth;
	u32 width;
	u32 height;

	u32 MaxVertexCount;
	u32 vertexCount;
	ColoredVertex *vertexArray;
};

struct ImageBuffer
{
	void *memory;
	int height;
	int width;
	int pitch;
	int bytesPerPixel;
};

struct Key
{
	bool isDown;
	bool pressedThisFrame;
};

struct KeybordInput
{
	static const unsigned int amountOfKeys = 28;
	union
	{
		Key input[amountOfKeys];
		struct
		{
			Key a; Key b; Key c; Key d; Key e;
			Key f; Key g; Key h; Key i; Key j; 
			Key k; Key l; Key m; Key n; Key o;
			Key p; Key q; Key r; Key s; Key t;
			Key u; Key v; Key w; Key x; Key y;
			Key z;
			Key space;
			Key shift;
		};
		
	};
};

struct MouseInput
{
	long x;
	long y;
	bool leftButtonDown;
	bool rightButtonDown;
	bool leftButtonPressedThisFrame;
	bool rightButtonPressedThisFrame;
	f32 expectedTimePerFrame;

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