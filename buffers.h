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

#define ArrayCount(a) (sizeof(a)/sizeof(*a))
#define MegaBytes(a) (1024 * KiloBytes(a))
#define KiloBytes(a) (1024 * (a))
#define OffsetOf(type, Member) (umm)&(((type *)0)->Member)
#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#define Die Assert(false)


struct ColoredVertex
{
	v3 pos;
	u32 color;
};

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

struct Key
{
	bool isDown;
	bool pressedThisFrame;
};

enum KeyEnum
{
	Key_a, Key_b, Key_c, Key_d, Key_e,
	Key_f, Key_g, Key_h, Key_i, Key_j,
	Key_k, Key_l, Key_m, Key_n, Key_o,
	Key_p, Key_q, Key_r, Key_s, Key_t,
	Key_u, Key_v, Key_w, Key_x, Key_y,
	Key_z,
	Key_space,
	Key_shift,

	Key_count
};


struct KeybordInput
{
	static const unsigned int amountOfKeys = 28;
	union
	{
		Key input[Key_count];
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