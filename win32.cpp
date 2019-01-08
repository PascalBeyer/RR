#undef CreateFile
#undef PlaySound

#define WGLPROC(a) a = (a##_ *)wglGetProcAddress(#a);

#define ArrayCount(a) (sizeof(a)/sizeof(*a))
#define GigaBytes(a) (1024 * MegaBytes(a))
#define MegaBytes(a) (1024 * KiloBytes(a))
#define KiloBytes(a) (1024 * (a))
#define OffsetOf(type, Member) (umm)&(((type *)0)->Member)
#define Die Assert(false)

#include "Game.h"

#include <Windows.h>
#include <gl/gl.h>
#include <dsound.h>
#include <cstdio>


#undef CreateFile
#undef PlaySound

// note this is so when we compile shaders we can just stop.
static bool running;
#include "OpenGL.h"

struct windowDimension
{
	int height;
	int width;
};

//sound
#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);


// todo: remove all globals?
static bool globalGamePaused;
static ImageBuffer globalImageBuffer;
static LPDIRECTSOUNDBUFFER globalSoundBuffer;
static MouseInput globalMouseInput;
static BITMAPINFO globalInfo;

static WorkHandler workHandler;
static i64 globalPerformanceCountFrequency;
static HANDLE semaphoreHandle;
static HWND globalWindow;

static void FreeFile(File file) // todo make this take a pointer and clear it, just in case.
{
	void *memory = file.memory;
	if (memory)
	{
		DynamicFree(memory);
	}
}

//todo clean it up, such that there are not 3 LoadFile...
static File LoadFile(char *fileName)
{
	void *memory = 0;
	unsigned int size = 0;
	
	HANDLE fileHandle = CreateFileA(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fileHandle == INVALID_HANDLE_VALUE)
	{
		return { 0, NULL };
	}
   
	LARGE_INTEGER fileSize;
	if (GetFileSizeEx(fileHandle, &fileSize))
	{
		u32 fileSize32 = (u32)fileSize.QuadPart;
		memory = DynamicAlloc(u8, fileSize32);
		if (memory)
		{
			DWORD bytesRead;
			if (ReadFile(fileHandle, memory, fileSize32, &bytesRead, 0) && fileSize32 == bytesRead)
			{
				size = fileSize32;
			}
			else
			{
				Die;
				memory = 0;
			}
		}		
	}
   
	CloseHandle(fileHandle);
   
	File result;
	result.fileSize = size;
	result.memory = (u8 *)memory;
	return result;
}

static File LoadFile(char *fileName, void *dest, u32 destSize)
{
	void *memory = NULL;
	u32 size = 0;
   
	HANDLE fileHandle = CreateFileA(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fileHandle == INVALID_HANDLE_VALUE)
	{
		return { 0, NULL };
	}
   
	LARGE_INTEGER fileSize;
	if (GetFileSizeEx(fileHandle, &fileSize))
	{
		u32 fileSize32 = (u32)fileSize.QuadPart;
		
		if (fileSize32 <= destSize)
		{
			memory = dest;
         
			DWORD bytesRead;
			if (ReadFile(fileHandle, memory, fileSize32, &bytesRead, 0) && fileSize32 == bytesRead)
			{
				size = fileSize32;
			}
			else
			{
				Die;
				memory = 0;
			}
		}
	}
   
	CloseHandle(fileHandle);
   
	File result;
	result.fileSize = size;
	result.memory = (u8 *)memory;
	return result;
}

static File LoadFile(char *fileName, Arena *arena)
{
	void *memory = 0;
	unsigned int size = 0;
   
	HANDLE fileHandle = CreateFileA(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fileHandle != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER fileSize;
		if (GetFileSizeEx(fileHandle, &fileSize))
		{
			u32 fileSize32 = (u32)fileSize.QuadPart;
			memory = PushData(arena, u8, fileSize32);
			if (memory)
			{
				DWORD bytesRead;
				if (ReadFile(fileHandle, memory, fileSize32, &bytesRead, 0) && fileSize32 == bytesRead)
				{
					size = fileSize32;
				}
				else
				{
					Die;
					memory = 0;
				}
			}
         
		}
		CloseHandle(fileHandle);
	}
	File result;
	result.fileSize = size;
	result.memory = (u8 *)memory;
	return result;
}

static bool WriteEntireFile(char * fileName, File file) //zero terminated
{
	void * memory = file.memory;
	unsigned int size = file.fileSize;
   
	bool result = false;
   
	HANDLE fileHandle = CreateFileA(fileName, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, NULL, NULL);
	if (fileHandle != INVALID_HANDLE_VALUE)
	{
		DWORD bytesWritten;
		if (WriteFile(fileHandle, memory, size, &bytesWritten, 0) && (bytesWritten == size))
		{
			result = true;
		}
		else
		{
			//todo: logging
			Die;
			OutputDebugStringA("Error writing file.");
		}
      
		CloseHandle(fileHandle);
	}
	return result;
   
}

// note path ending in '/' filetype includes '.'. Strings get loaded into arenaForStrings
static StringArray FindAllFiles(char *path, char *fileType, Arena *arenaForStrings)
{
	char* searchString = FormatCString("%c**%c*", path, fileType);
   
	Arena *arena = PushArena(frameArena, 8000);
   
	BeginArray(arena, String, ret);
	WIN32_FIND_DATAA data = {};
	HANDLE handle = FindFirstFile(searchString, &data);
	if (handle != INVALID_HANDLE_VALUE)
	{
		*PushStruct(arena, String) = S(data.cFileName, arenaForStrings);
		
		while (FindNextFile(handle, &data))
		{
			*PushStruct(arena, String) = S(data.cFileName, arenaForStrings);
		}
	}
   
	EndArray(arena, String, ret);
   
	return ret;
}

static void initializeImageBuffer(BITMAPINFO *info, ImageBuffer *buffer, int width, int height)
{
   
	if (buffer->memory)
	{
		VirtualFree(buffer->memory, 0, MEM_RELEASE);
	}
	buffer->height = height;
	buffer->width = width;
   
	info->bmiHeader.biSize = sizeof(info->bmiHeader);
	info->bmiHeader.biWidth = buffer->width;
	info->bmiHeader.biHeight = -buffer->height;
	info->bmiHeader.biPlanes = 1;
	info->bmiHeader.biBitCount = 32;
	info->bmiHeader.biCompression = BI_RGB;
   
	buffer->bytesPerPixel = 4;
	// width and height have to be divisible by 4
	int rendererOverdraw = 8;
	int memorySize = width * height * buffer->bytesPerPixel + rendererOverdraw;
	buffer->memory = VirtualAlloc(0, memorySize, MEM_RESERVE| MEM_COMMIT, PAGE_READWRITE);
	buffer->memory = ((u8 *)buffer->memory + 4);
	buffer->pitch = width*buffer->bytesPerPixel;
	//MEM_COMMIT = System should actually pay attension to them
}

static windowDimension win32GetWindowDimension(HWND &window)
{
	windowDimension ret;
	RECT clientRect;
	GetClientRect(window, &clientRect);
   
	ret.height = clientRect.bottom - clientRect.top;
	ret.width = clientRect.right - clientRect.left;
   
	return ret;
}

static void displayImageBuffer(HDC deviceContext)//BITMAPINFO *info, HDC deviceContext, int windowWidth, int windowHeight, ImageBuffer *buffer)
{
#if 0
	StretchDIBits(deviceContext,
                 0, 0, buffer->width, buffer->height,
                 0, 0, buffer->width, buffer->height,
                 buffer->memory,
                 info,
                 DIB_RGB_COLORS, SRCCOPY);
#else
	/*glViewport(0, 0, windowWidth, windowHeight);
   
 glBindTexture(GL_TEXTURE_2D, globalTextureHandle);
 glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, buffer->width, buffer->height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, buffer->memory);
 
 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
 
 glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
 
 glEnable(GL_TEXTURE_2D);
 
 glClearColor(1.0f, 0.0f, 1.0f, 0.0f);
 glClear(GL_COLOR_BUFFER_BIT);
 
 glMatrixMode(GL_TEXTURE);
 glLoadIdentity();
 glMatrixMode(GL_MODELVIEW);
 glLoadIdentity();
 glMatrixMode(GL_PROJECTION);
 float a = 2.0f / (float)buffer->width;
 float b = 2.0f / (float)buffer->height;
 float projM[] =
 {
  a, 0, 0, 0,
  0, b, 0, 0,
  0, 0, 1, 0,
  -1, -1, 0, 1,
 };
 glLoadMatrixf(projM);
 
 v2 minP = v2();
 v2 maxP = v2(buffer->width, buffer->height);
 
 glBegin(GL_TRIANGLES);
 
 //Lower Triangle
 glTexCoord2f(0.0f, 1.0f);
 glVertex2f(minP.x, minP.y);
 
 glTexCoord2f(1.0f, 1.0f);
 glVertex2f(maxP.x, minP.y);
 
 glTexCoord2f(1.0f, 0.0f);
 glVertex2f(maxP.x, maxP.y);
 
 //upper Triangle
 glTexCoord2f(0.0f, 1.0f);
 glVertex2f(minP.x, minP.y);
 
 glTexCoord2f(0.0f, 0.0f);
 glVertex2f(minP.x, maxP.y);
 
 glTexCoord2f(1.0f, 0.0f);
 glVertex2f(maxP.x, maxP.y);
 
 glEnd();
 
 */
   
	//OpenGLDisplayImageBuffer(info, deviceContext, windowWidth, windowHeight, buffer);
   
	TimedBlock;
   
	SwapBuffers(deviceContext);
   
#endif
}

static void initializeSoundBuffer(HWND window, SoundBuffer *soundOutput)
{
   
	//soundOutput.hz = 256;
	soundOutput->toneVolume = 2000;
	soundOutput->samplesPerSecond = 48000;
	soundOutput->runningSampleIndex = 0;
	//soundOutput.squareWavePeriod = soundOutput.samplesPerSecond / soundOutput.hz;
	//soundOutput.halfSquareWavePeriod = (soundOutput.squareWavePeriod / 2);
	soundOutput->bytesPerSample = sizeof(i16) * 2;
	soundOutput->secondaryBufferSize = soundOutput->samplesPerSecond *soundOutput->bytesPerSample;
	soundOutput->soundIsPlaying = false;
	soundOutput->soundIsValid = false;
	soundOutput->latencySampleCount = soundOutput->samplesPerSecond / 10; //1/4 seconds of latency
	
   
	//load library
	HMODULE dSoundLibrary = LoadLibrary("dsound.dll");
	if (dSoundLibrary)
	{
		//NOTE: Get a DirectSoundobject
		LPDIRECTSOUND directSound;
		direct_sound_create *directSoundCreate = (direct_sound_create *)GetProcAddress(dSoundLibrary, "DirectSoundCreate");
		if (directSoundCreate && SUCCEEDED(directSoundCreate(0, &directSound, 0)))
		{
			WAVEFORMATEX waveFormat = {};
			waveFormat.wFormatTag = WAVE_FORMAT_PCM;
			waveFormat.nChannels = 2;
			waveFormat.nSamplesPerSec = soundOutput->samplesPerSecond;
			waveFormat.wBitsPerSample = 16;
			waveFormat.nBlockAlign = (waveFormat.nChannels * waveFormat.wBitsPerSample) / 8;
			waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
			waveFormat.cbSize = 0;
			//NOTE: "create" a primary buffer
         
			if (SUCCEEDED(directSound->SetCooperativeLevel(window, DSSCL_PRIORITY)))
			{
				//TODO :DSBVAPS _GLOBALFOCUS
				DSBUFFERDESC bufferDescription = {};
				bufferDescription.dwSize = sizeof(bufferDescription);
				bufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;
            
				LPDIRECTSOUNDBUFFER primaryBuffer;
				if (SUCCEEDED(directSound->CreateSoundBuffer(&bufferDescription, &primaryBuffer, 0)))
				{
					if (SUCCEEDED(primaryBuffer->SetFormat(&waveFormat)))
					{
						OutputDebugStringA("primary buffer created");
					}
					else
					{
						//TODO: diag
					}
				}
			}
			else
			{
				// TODO diag
			}
         
			//NOTE: "create" a secondary buffer
			DSBUFFERDESC bufferDescription = {};
			bufferDescription.dwSize = sizeof(bufferDescription);
			bufferDescription.dwBufferBytes = soundOutput->secondaryBufferSize;
			bufferDescription.dwFlags = 0;
			bufferDescription.lpwfxFormat = &waveFormat;
         
			if (SUCCEEDED(directSound->CreateSoundBuffer(&bufferDescription, &globalSoundBuffer, 0)))
			{
				OutputDebugStringA("secondary buffer created");
			}
			else
			{
            
			}
			//NOTE: Start it playing
		}
		else
		{
			//TODO diagnostic
		}
      
	}
}

static void win32FillSoundBuffer(SoundBuffer* soundOutput, DWORD byteToLock, DWORD bytesToWrite,
                                 i16 *samples)
{
	void* region1;
	DWORD region1Size;
	void* region2;
	DWORD region2Size;
   
	//s16 s16 s16...
	//[Left  Right] Left  Right
	// one Sample
	if (SUCCEEDED(globalSoundBuffer->Lock(byteToLock, bytesToWrite, &region1, &region1Size, &region2, &region2Size, 0)))
	{
		//TODO assert taht stuff is valid
		i16 *sampleOut = (i16*)region1;
		i16 *sourceSample = samples;
		DWORD Region1SampleCount = region1Size / soundOutput->bytesPerSample;
		for (DWORD sampleIndex = 0; sampleIndex < Region1SampleCount; ++sampleIndex)
		{
			*sampleOut++ = *sourceSample++;
			*sampleOut++ = *sourceSample++;
			soundOutput->runningSampleIndex++;
		}
		DWORD Region2SampleCount = region2Size / soundOutput->bytesPerSample;
		sampleOut = (i16*)region2;
		for (DWORD sampleIndex = 0; sampleIndex < Region2SampleCount; ++sampleIndex)
		{
			*sampleOut++ = *sourceSample++;
			*sampleOut++ = *sourceSample++;
			soundOutput->runningSampleIndex++;
		}
      
		globalSoundBuffer->Unlock(region1, region1Size, region2, region2Size);
	}
}

inline f32 win32GetSecondsElapsed(LARGE_INTEGER start, LARGE_INTEGER end)
{
	return (((f32)end.QuadPart - (f32)start.QuadPart) / (f32)globalPerformanceCountFrequency);
}

LRESULT CALLBACK win32MainWindowCallback(
HWND window,
UINT message,
WPARAM wParam,
LPARAM lParam)
{
	LRESULT result = 0;
   
	switch (message)
	{
      case WM_DESTROY:
      {
         running = false;
      }break;
      
      case WM_PAINT:
      {
         //OutputDebugStringA("WM_PAINT\n");
         
         PAINTSTRUCT paint;
         HDC deviceContext = BeginPaint(window, &paint);
         windowDimension dim;
         dim = win32GetWindowDimension(window);
         
         //displayImageBuffer(&globalInfo, deviceContext, dim.width, dim.height, &globalImageBuffer);
         
         EndPaint(window, &paint);
         
      }break;
      default:
      {
         result = DefWindowProc(window, message, wParam, lParam);
         break;
      }
	}
	return result;
}

#define WRITEBARRIER _WriteBarrier(); _mm_sfence();
#define READBARRIER _ReadBarrier()
struct WorkQueueEntry
{
	char *stringToPrint;
};
static u32 volatile entryCount;
static u32 volatile todo; // todo hui ui ui, what is this jank exactly?
WorkQueueEntry entries[256];

struct ThreadInfo
{
	u32 threadIndex;
	HANDLE semaphoreHandle;
};

static i64 AtomicIncrement(volatile i64 *toIncrement)
{
   return InterlockedIncrement((volatile long *)toIncrement);
}

static i64 AtomicCompareExchange(volatile i64 *dest, i64 expectedValueOfDest, i64 newValue)
{
   return InterlockedCompareExchange((volatile long *)dest, (long)expectedValueOfDest, (long)newValue);
}


static bool work(int threadIndex)
{
	bool worked = false;
	
	if (todo < entryCount)
	{
		int entryIndex = InterlockedIncrement(&todo) - 1;
		WorkQueueEntry *entry = entries + entryIndex++;
		char buffer[256];
		wsprintf(buffer, "Thread %u: %s \n", threadIndex, entry->stringToPrint);
		OutputDebugStringA(buffer);
		worked = true;
	}
	return worked;
	
}

static void WakeThreads()
{
	ReleaseSemaphore(semaphoreHandle, 1, 0);
}

DWORD WINAPI ThreadProc(LPVOID param)
{
	ThreadInfo *threadInfo = (ThreadInfo *)param;
   
	char *stringToOutput = (char *)param;
	for (;;)
	{
		if (WorkDone(&workHandler))
		{
			WaitForSingleObjectEx(threadInfo->semaphoreHandle, INFINITE, FALSE);
		}
		else
		{
			DoWork(&workHandler);
		}
	}
	return 0;
}

typedef BOOL WINAPI wgl_swap_inverval_ext(int interval);
typedef HGLRC WINAPI wgl_create_context_attribs_ARB(HDC hdC, HGLRC sharedContext, const int *attribList);


#define WGL_DRAW_TO_WINDOW_ARB                  0x2001
#define WGL_ACCELERATION_ARB                    0x2003
#define WGL_SUPPORT_OPENGL_ARB                  0x2010
#define WGL_DOUBLE_BUFFER_ARB                   0x2011
#define WGL_PIXEL_TYPE_ARB                      0x2013
#define WGL_TYPE_RGBA_ARB                       0x202B
#define WGL_FULL_ACCELERATION_ARB               0x2027
#define WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB        0x20A9
#define WGL_RED_BITS_ARB                        0x2015
#define WGL_GREEN_BITS_ARB                      0x2017
#define WGL_BLUE_BITS_ARB                       0x2019
#define WGL_ALPHA_BITS_ARB                      0x201B
#define WGL_DEPTH_BITS_ARB                      0x2022
#define WGL_STENCIL_BITS_ARB					0x2024
#define WGL_SAMPLE_BUFFERS_ARB					0x2041
#define WGL_SAMPLES_ARB							0x2042



typedef BOOL WINAPI wglGetPixelFormatAttribivARB_(HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int *piAttributes, int *piValues);
typedef BOOL WINAPI wglGetPixelFormatAttribfvARB_(HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int *piAttributes, FLOAT *pfValues);
typedef BOOL WINAPI wglChoosePixelFormatARB_(HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
static wglChoosePixelFormatARB_ *wglChoosePixelFormatARB;
static wgl_create_context_attribs_ARB *wglCreateContextAttribsARB;

int win32OpenGLAttribs[] =
{
	WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
	WGL_CONTEXT_MINOR_VERSION_ARB, 2,
	WGL_CONTEXT_FLAGS_ARB, 0
      | WGL_CONTEXT_DEBUG_BIT_ARB
      ,
	//WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
	WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
	0,
};

static void win32SetPixelFormat(HDC windowDC)
{
	int suggestedPixelFormatIndex = 0;
	u32 extendedPick = 0;
   
   
	if (wglChoosePixelFormatARB)
	{
		int ARBAttribs[] =
		{
			WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
			WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
			WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
			WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
			WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
			WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE,
			WGL_RED_BITS_ARB, 8,
			WGL_GREEN_BITS_ARB, 8,
			WGL_BLUE_BITS_ARB, 8,
			WGL_ALPHA_BITS_ARB, 8,
			WGL_DEPTH_BITS_ARB, 24,			
			WGL_SAMPLE_BUFFERS_ARB, 1,
			WGL_SAMPLES_ARB, 4,			
         
			//WGL_STENCIL_BITS_ARB, 8, aperantly this does not work with multisampling
			0, 0
		};
		wglChoosePixelFormatARB(windowDC, ARBAttribs, { 0 }, 1, &suggestedPixelFormatIndex, &extendedPick);
	}
   
	if(!extendedPick)
	{
		//wglGetPixelFormatAttribfvARB_ *wglGetPixelFormatAttribfvARB = (wglGetPixelFormatAttribfvARB_ *)wglGetProcAddress("wglGetPixelFormatAttribfvARB");
		//wglGetPixelFormatAttribivARB_ *wglGetPixelFormatAttribivARB = (wglGetPixelFormatAttribivARB_ *)wglGetProcAddress("wglGetPixelFormatAttribivARB");
      
		PIXELFORMATDESCRIPTOR desiredPixelFormat = {};
		desiredPixelFormat.nSize = sizeof(desiredPixelFormat);
		desiredPixelFormat.nVersion = 1;
		desiredPixelFormat.iPixelType = PFD_TYPE_RGBA;
		desiredPixelFormat.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
		desiredPixelFormat.cColorBits = 32;
		desiredPixelFormat.cAlphaBits = 8;
		desiredPixelFormat.cDepthBits = 24;
		desiredPixelFormat.cStencilBits = 8;
		desiredPixelFormat.iLayerType = PFD_MAIN_PLANE;
		suggestedPixelFormatIndex = ChoosePixelFormat(windowDC, &desiredPixelFormat);
	}
   
	PIXELFORMATDESCRIPTOR suggestedPixelFormat;
   
	DescribePixelFormat(windowDC, suggestedPixelFormatIndex, sizeof(suggestedPixelFormat), &suggestedPixelFormat);
	SetPixelFormat(windowDC, suggestedPixelFormatIndex, &suggestedPixelFormat);
}

static void win32LoadWglExtensions()
{
	WNDCLASSA windowClass = {};
	windowClass.lpfnWndProc = DefWindowProcA;
	windowClass.hInstance = GetModuleHandle(0);
	windowClass.lpszClassName = "WGLLoader";
   
	if (RegisterClassA(&windowClass))
	{
		
      HWND helperWindow = CreateWindowExA(0, windowClass.lpszClassName, "WGLLOADER", WS_BORDER, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, 0, 0, windowClass.hInstance, 0);
      
		HDC windowDC = GetDC(helperWindow);
		win32SetPixelFormat(windowDC);
		HGLRC contextRC = wglCreateContext(windowDC);
		if (wglMakeCurrent(windowDC, contextRC))
		{
			//wglChoosePixelFormatARB = (wglChoosePixelFormatARB_ *)wglGetProcAddress("wglChoosePixelFormatARB");
			WGLPROC(wglChoosePixelFormatARB);
			wglCreateContextAttribsARB = (wgl_create_context_attribs_ARB *)wglGetProcAddress("wglCreateContextAttribsARB");
			
		}
		wglMakeCurrent(0, 0);
		wglDeleteContext(contextRC);
		ReleaseDC(helperWindow, windowDC);
		DestroyWindow(helperWindow);
      
	}
}

static OpenGLContext win32InitOpenGL(HWND window)
{
   OpenGLContext ret = {};
   
	win32LoadWglExtensions();
	HDC windowDC = GetDC(window);
	win32SetPixelFormat(windowDC);
	bool modernContext = true;
   
	HGLRC context = 0;
	if (wglCreateContextAttribsARB)
	{
		//NOTE: modern Version of openGL
		HGLRC sharedContext = 0;
		context = wglCreateContextAttribsARB(windowDC, sharedContext, win32OpenGLAttribs);
	}
   
	if (!context)
	{
		modernContext = false;
		context = wglCreateContext(windowDC);
	}
   
	if (wglMakeCurrent(windowDC, context))
	{
		// vsinc
		wgl_swap_inverval_ext *wglSwapIntervalEXT= (wgl_swap_inverval_ext *)wglGetProcAddress("wglSwapIntervalEXT");
		if (wglSwapIntervalEXT)
		{
			wglSwapIntervalEXT(1);
		}
      
      WGLPROC(glGetStringi);
		WGLPROC(glTexImage2DMultisample);
		WGLPROC(glBufferData);
		WGLPROC(glBindBuffer);
		WGLPROC(glGenBuffers);
		WGLPROC(glBindVertexArray);
		WGLPROC(glGenVertexArrays);
		WGLPROC(glCreateShader);
		WGLPROC(glShaderSource);
		WGLPROC(glCompileShader);
		WGLPROC(glLinkProgram);
		WGLPROC(glCreateProgram);
		WGLPROC(glAttachShader);
		WGLPROC(glValidateProgram);
		WGLPROC(glGetProgramInfoLog);
		WGLPROC(glGetProgramiv);
		WGLPROC(glGetShaderInfoLog);
		WGLPROC(glUniformMatrix4fv);
		WGLPROC(glUniform4fv);
		WGLPROC(glUniform1iv);
		WGLPROC(glGetUniformLocation);
		WGLPROC(glUseProgram);
      WGLPROC(glVertexAttribIPointer);
		WGLPROC(glVertexAttribPointer);
		WGLPROC(glEnableVertexAttribArray);
		WGLPROC(glDisableVertexAttribArray);
		WGLPROC(glGetAttribLocation);
		WGLPROC(glGenFramebuffers);
		WGLPROC(glBindFramebuffer);
		WGLPROC(glVertexAttrib3f);
		WGLPROC(glCheckFramebufferStatus);
		WGLPROC(glFramebufferTexture);
		WGLPROC(glFramebufferTexture2D);
		WGLPROC(glBlitFramebuffer);
		WGLPROC(glDebugMessageCallback);
		WGLPROC(glActiveTexture);
		WGLPROC(glUniform1i);
		WGLPROC(glUniform1f);
		WGLPROC(glUniform2f);
		WGLPROC(glUniform3f);
		WGLPROC(glUniform4f);
		WGLPROC(glBindAttribLocation);
      
		ret = OpenGLInit(modernContext);
	}
   
	ReleaseDC(window, windowDC);
   
   return ret;
}


#define HANDLEINPUTKEY(VK_CODE, name) \
if (vkCode == VK_CODE) \
{ \
	globalKeybordInput.name.isDown = !globalKeybordInput.name.isDown; \
	if (globalKeybordInput.name.isDown) \
	{ \
		globalKeybordInput.name.pressedThisFrame = true; \
	} \
} 

static String OSGetClipBoard()
{
	if(OpenClipboard(NULL))
	{
      String ret;
		HANDLE handle = GetClipboardData(CF_TEXT);
		char *inp = (char *)GlobalLock(handle);
		String inpS = CreateString(inp);
		ret = CopyString(inpS);
		GlobalUnlock(handle);
		CloseClipboard();
      return ret;
	}
	return {};
}

static void OSSetClipBoard(String string)
{
	if (OpenClipboard(globalWindow))
	{
		EmptyClipboard();
      
		HGLOBAL clipbuffer = GlobalAlloc(GMEM_SHARE, string.length + 1);
		char* buffer = (char *)GlobalLock(clipbuffer);
		memcpy(buffer, ToNullTerminated(frameArena, string), sizeof(char) * (string.length + 1));
      
		GlobalUnlock(clipbuffer);
      
		HANDLE ret = SetClipboardData(CF_TEXT, buffer);
		CloseClipboard();
	}
	
}

static void DispatchKeyMessage(KeyMessageBuffer *buffer, KeyStateMessage keyMessage)
{
	if (buffer->amountOfMessages + 1 < buffer->maxAmountOfMessages)
	{
		buffer->messages[buffer->amountOfMessages++] = keyMessage;
	}
	else
	{
		ConsoleOutputError("to many keyboard messages");
	}
}

static void HandleWindowsMassages(KeyMessageBuffer *buffer) //todo make this buffer allocate on the frame arena? could just be the first thing that happens ont that
{
	buffer->amountOfMessages = 0;
	MSG message;
   
	// todo should these be values in the buffer?
	static b32 shiftDown = false;
	static b32 controlDown = false;
   
	while (PeekMessageA(&message, 0, 0, 0, PM_REMOVE))
	{
		u32 shiftCtrlFlag = ((shiftDown > 0) * KeyState_ShiftDown) | ((controlDown > 0) * KeyState_ControlDown);
      
		switch (message.message)
		{
         case WM_LBUTTONDOWN:
         {
            KeyStateMessage keyMessage;
            keyMessage.flag = KeyState_Down | KeyState_PressedThisFrame | shiftCtrlFlag;
            keyMessage.key = Key_leftMouse;
            DispatchKeyMessage(buffer, keyMessage);
         }break;
         case WM_LBUTTONUP: 
         {
            KeyStateMessage keyMessage;
            keyMessage.flag = KeyState_Up | KeyState_ReleasedThisFrame | shiftCtrlFlag;
            keyMessage.key = Key_leftMouse;
            DispatchKeyMessage(buffer, keyMessage);
         }break;
         case WM_RBUTTONDOWN:
         {
            KeyStateMessage keyMessage;
            keyMessage.flag = KeyState_Down | KeyState_PressedThisFrame | shiftCtrlFlag;
            keyMessage.key = Key_rightMouse;
            DispatchKeyMessage(buffer, keyMessage);
         }break;
         case WM_RBUTTONUP:
         {
            KeyStateMessage keyMessage;
            keyMessage.flag = KeyState_Up | KeyState_ReleasedThisFrame | shiftCtrlFlag;
            keyMessage.key = Key_rightMouse;
            DispatchKeyMessage(buffer, keyMessage);
         }break;
         case WM_MBUTTONDOWN:
         {
            KeyStateMessage keyMessage;
            keyMessage.flag = KeyState_Down | KeyState_PressedThisFrame | shiftCtrlFlag;
            keyMessage.key = Key_middleMouse;
            DispatchKeyMessage(buffer, keyMessage);
         }break;
         case WM_MBUTTONUP:
         {
            KeyStateMessage keyMessage;
            keyMessage.flag = KeyState_Up | KeyState_ReleasedThisFrame | shiftCtrlFlag;
            keyMessage.key = Key_middleMouse;
            DispatchKeyMessage(buffer, keyMessage);
         }break;
         case WM_MOUSEWHEEL:
         {
            u32 fwKeys = GET_KEYSTATE_WPARAM(message.wParam);
            int zDelta = GET_WHEEL_DELTA_WPARAM(message.wParam);
            
            KeyStateMessage keyMessage;
            keyMessage.flag = KeyState_PressedThisFrame | KeyState_Down | shiftCtrlFlag;
            keyMessage.key = (zDelta > 0) ? Key_mouseWheelForward : Key_mouseWheelBack;
            
            DispatchKeyMessage(buffer, keyMessage);
            
         }break;
         case WM_MOUSEMOVE:
         {
            POINT mousePos;
            GetCursorPos(&mousePos);
            
            if (GetCursorPos(&mousePos))
            {
               ScreenToClient(message.hwnd, &mousePos);
               globalMouseInput.x = mousePos.x;
               globalMouseInput.y = mousePos.y;
            }
            else
            {
               OutputDebugStringA("could not get mousePosition.\n");
            }
            
         }break;
         case WM_KEYUP:
         case WM_SYSKEYUP:
         case WM_KEYDOWN:
         case WM_SYSKEYDOWN:
         {
            u64 vkCode = message.wParam;
            u32 repeaded = message.lParam & 0xFF;
            b32 wasDown = ((message.lParam & (1UL << 30)) != 0);
            b32 isDown  = ((message.lParam & (1UL << 31)) == 0);
            
            KeyStateMessage keyMessage;
            keyMessage.key = (KeyEnum)vkCode;
            
            if (vkCode == Key_shift)
            {
               shiftDown = isDown;
            }
            if (vkCode == Key_control)
            {
               controlDown = isDown;
            }
            
            keyMessage.flag = shiftCtrlFlag;
            
            if (isDown) //todo could make this better, just oring em together
            {
               keyMessage.flag |= KeyState_Down;
               if (isDown != wasDown)
               {
                  keyMessage.flag |= KeyState_PressedThisFrame;
               }
               if (repeaded)
               {
                  keyMessage.flag |= KeyState_Repeaded;
               }
            }
            else
            {
               keyMessage.flag |= KeyState_Up;
               if (isDown != wasDown)
               {
                  keyMessage.flag |= KeyState_ReleasedThisFrame;
               }
            }
            
            DispatchKeyMessage(buffer, keyMessage);
            
            if (isDown && vkCode == VK_F4)
            {
               running = false;
            }
            
            if (isDown && vkCode == VK_F2)
            {
               globalDebugState.paused = !globalDebugState.paused;
            }
            
            if (isDown && vkCode == VK_F3)
            {
               globalGamePaused = !globalGamePaused;
            }
            
         }
         default:
         {
            TranslateMessage(&message);
            DispatchMessage(&message);
         }
		}
	}
}

#if 0
//int __stdcall WiaaanMainCRTStartup(void)
#endif

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR commandLine, int showCode)
{
   printf("do we work?\n");
   
#if 0
   AllocConsole();
   
   CONSOLE_SCREEN_BUFFER_INFO csbiInfo; 
   
   
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
   Assert(hStdout != INVALID_HANDLE_VALUE);
   
   Assert(GetConsoleScreenBufferInfo(hStdout, &csbiInfo));
   
   DWORD err0r = GetLastError();
   
   
   SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), csbiInfo.dwSize);
   
   
   Assert(SetConsoleTextAttribute(hStdout, FOREGROUND_RED | FOREGROUND_INTENSITY));
   
   LPSTR lpszPrompt1 = "Type a line and press Enter, or q to quit: ";
   DWORD cWritten;
   
	Assert(WriteFile(hStdout, lpszPrompt1, lstrlenA(lpszPrompt1), &cWritten, NULL));
#endif
   
   
	u32 constantMemorySize = GigaBytes(1);
	u32 frameMemorySize = MegaBytes(100);
	u32 workingMemorySize = MegaBytes(5);
   
	void *constantMemory = VirtualAlloc(0, constantMemorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	Arena *constantArena = InitArena(constantMemory, constantMemorySize);
	void *frameMem = VirtualAlloc(0, frameMemorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	frameArena = InitArena(frameMem, frameMemorySize);
	Assert(frameMem);
	Assert(constantMemory);
   
	alloc = PushStruct(constantArena, BuddyAllocator);
	*alloc = CreateBuddyAllocator(constantArena, MegaBytes(128), KiloBytes(64));
   
	//TestAllocator(&buddyAlloc);
   
	void *debugMemory = VirtualAlloc(0, MegaBytes(300), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	globalDebugState.arena = InitArena(debugMemory, MegaBytes(300));
	Assert(debugMemory);
   
	InitDebug();
	ResetDebugState();
   
	// setting up windows timing stuff
	UINT desiredSchedulerMS = 1;
	bool sleepIsGranular = (timeBeginPeriod(desiredSchedulerMS) == TIMERR_NOERROR);
	LARGE_INTEGER perfCountFrequencyResult;
	QueryPerformanceFrequency(&perfCountFrequencyResult);
	globalPerformanceCountFrequency = perfCountFrequencyResult.QuadPart;
   
   
#if 0
	const u32 threadCount = 3 - 1;
	ThreadInfo threadInfo[1]; //threadCount
   
	semaphoreHandle = CreateSemaphoreEx(0, 0, threadCount, 0, 0, SEMAPHORE_ALL_ACCESS);
   
	for (int threadIndex = 0; threadIndex < threadCount; ++threadIndex)
	{
		threadInfo[threadIndex].threadIndex = threadIndex;
		threadInfo->semaphoreHandle = semaphoreHandle;
		DWORD threadID;
		HANDLE threadHandle = CreateThread(0, 0, ThreadProc, &(threadInfo[threadIndex]), 0, &threadID);
	}
#else
	const u32 threadCount = 0;
#endif
   
   workHandler.workIndex = 0;
   workHandler.storeIndex = 0;
   workHandler.completionCount = 0;
   workHandler.completionGoal = 0;
	
	workHandler.queue = PushData(constantArena, Work, WorkQueueSize);
   
	// todo make this work off of frame arena.
	const u32 inputBufferSize = 100;
	KeyStateMessage inputBuffer[inputBufferSize];
	KeyMessageBuffer keyMessageBuffer;
	keyMessageBuffer.amountOfMessages = 0;
	keyMessageBuffer.maxAmountOfMessages = inputBufferSize;
	keyMessageBuffer.messages = inputBuffer;
   
	WNDCLASSA windowClass = {};
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = win32MainWindowCallback;
	windowClass.hInstance = instance;
	windowClass.lpszClassName = "WindowClass";
   
	i32 windowWidth = 1280, windowHeight = 720;
	initializeImageBuffer(&globalInfo, &globalImageBuffer, windowWidth, windowHeight);
	if (!RegisterClass(&windowClass))
	{
      return 0;
   }
   globalWindow = CreateWindowEx(0, windowClass.lpszClassName, "Game Try 1", WS_POPUP|WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, windowWidth, windowHeight, 0, 0, instance, 0);
   if (!globalWindow)
   {
      return 0;
   }
   
   running = true;
   HDC deviceContext = GetDC(globalWindow);
   
   OpenGLContext openGLContext = win32InitOpenGL(globalWindow);
   
   //todo move the allocation stuff into the game?
   u32 pushBufferSize = MegaBytes(4);
   void* pushBuffer = VirtualAlloc(0, pushBufferSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
   RenderCommands renderCommands = {};
   renderCommands.maxBufferSize = pushBufferSize;
   renderCommands.pushBufferSize = 0;
   renderCommands.pushBufferBase = (u8 *)pushBuffer;
   
   renderCommands.height = globalImageBuffer.height;
   renderCommands.width = globalImageBuffer.width;
   renderCommands.aspectRatio = (f32) globalImageBuffer.width / (f32) globalImageBuffer.height;
   
   
   // todo redo this, but make it so that the mouse position gets adjusted at the beginning of the frame, so if the window is not active, this does not do anything
   /*
      RECT windowRect;
if (GetWindowRect(window, &windowRect))
ClipCursor(&windowRect);
*/
   SoundBuffer soundOutput = {};
   initializeSoundBuffer(globalWindow, &soundOutput);
   i16 *samples = (i16 *)VirtualAlloc(0, soundOutput.secondaryBufferSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
   soundOutput.soundSamples = samples;
   int sampleAmount = soundOutput.secondaryBufferSize / sizeof(i16);
   for (u32 i = 0; i < (soundOutput.secondaryBufferSize/sizeof(i16)); i++)
   {
      samples[i] = 0;
   }
   win32FillSoundBuffer(&soundOutput, 0, 0, samples);
   
   int monitorRefreshHz = 60;
   HDC refreshDC = GetDC(globalWindow);
   int win32RefreshRate = GetDeviceCaps(refreshDC, VREFRESH);
   ReleaseDC(globalWindow, refreshDC);
   if (win32RefreshRate > 1)
   {
      monitorRefreshHz = win32RefreshRate;
   }
   f32 gameUpdateHz = (float)(monitorRefreshHz);
   f32 targetSecondsPerFrame = 1.0f / (f32)gameUpdateHz;
   
   gameState = InitGame(windowWidth, windowHeight, &workHandler, constantArena);
   
   Clear(frameArena);
   
   LARGE_INTEGER timeCounter;
   QueryPerformanceCounter(&timeCounter);
   
   while (running)
   {	
      if (globalGamePaused)
      {
         HandleWindowsMassages(&keyMessageBuffer);
         SwapBuffers(deviceContext);
         continue;
      }
      
      ResetDebugState();
      
      TimedBlock;
      
      LARGE_INTEGER endCounter;
      QueryPerformanceCounter(&endCounter);
      f32 deltaTime = win32GetSecondsElapsed(timeCounter, endCounter);
      timeCounter = endCounter;
      
      CollectDebugRecords(deltaTime); // collect for last frame
      
      HandleWindowsMassages(&keyMessageBuffer);
      
      soundOutput.soundIsValid = false;
      DWORD playCursor = 0;
      DWORD writeCursor = 0;
      DWORD targetCursor = 0;
      DWORD bytesToWrite = 0;
      DWORD byteToLock = 0;
      if (SUCCEEDED(globalSoundBuffer->GetCurrentPosition(&playCursor, &writeCursor)))
      {
         
         byteToLock = (soundOutput.runningSampleIndex*soundOutput.bytesPerSample) % soundOutput.secondaryBufferSize;
         targetCursor =((playCursor +(soundOutput.latencySampleCount*soundOutput.bytesPerSample)) % soundOutput.secondaryBufferSize);
         
         if (byteToLock > targetCursor) //we looped
         {
            bytesToWrite = soundOutput.secondaryBufferSize - byteToLock; //everything to the end of the buffer
            bytesToWrite += targetCursor; //what is still left
         }
         else // did not loop
         {
            bytesToWrite = targetCursor - byteToLock; //fill from the byteToLock till the Cursor
         }
         soundOutput.soundIsValid = true;
      }
      soundOutput.sampleAmount = bytesToWrite / soundOutput.bytesPerSample;
      Input input;
      UpdateInput(&input, globalMouseInput, windowWidth, windowHeight, deltaTime, keyMessageBuffer);
      
      GameUpdateAndRender(&gameState, &renderCommands, input, &soundOutput);
      
      //AnimationCreatorUpdateAndRender(&renderCommands, &input);
      
      if (soundOutput.soundIsValid)
      {
         win32FillSoundBuffer(&soundOutput, byteToLock, bytesToWrite, samples);
      }
      if (!soundOutput.soundIsPlaying)
      {
         globalSoundBuffer->Play(0, 0, DSBPLAY_LOOPING);
         soundOutput.soundIsPlaying = true;
      }
      
#if 1
      RenderGroup renderGroup = InitRenderGroup(&gameState.assetHandler, &renderCommands);
      RenderGroup *rg = &renderGroup;
      Font font = gameState.font;
      
      String s = FtoS(deltaTime);
      
      float screenWidth = (f32)renderCommands.width;
      float screenHeight = (f32)renderCommands.height;
      
      PushRenderSetup(rg, {}, {}, Setup_Orthogonal);
      
      PushString(rg, V2(20.1f, 10.1f), s, 20, font);
      
      PushRenderSetup(rg, {}, {}, Setup_Orthogonal | Setup_ZeroToOne);
      
      DrawDebugRecords(rg, gameState.font, targetSecondsPerFrame, input);
      //DrawTweekers(rg, font);
      
#endif
      
      OpenGlRenderGroupToOutput(&renderCommands, &openGLContext);
      
      displayImageBuffer(deviceContext);
      
      Clear(frameArena);
   }
   //ClipCursor(NULL);
   
   return 0;
}
//todo clean this file up really good....


u32 const debugRecordsAmount = __COUNTER__;
DebugBlockInfo debugInfoArray[debugRecordsAmount];

