#include <Windows.h>
#include <dsound.h>

#define WGLPROC(a) a = (a##_ *)wglGetProcAddress(#a);

#include "win32.h"
#include "Game.h"
#include "File.h"
#include "Debug.h"
#include "OpenGL.h"
#include "WorkHandler.h"
#include "AnimationCreator.h"

struct windowDimension
{
	int height;
	int width;
};

//sound
#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);

static bool running;
static ImageBuffer globalImageBuffer;
static LPDIRECTSOUNDBUFFER globalSoundBuffer;
static MouseInput globalMouseInput;
static KeybordInput globalKeybordInput;
static BITMAPINFO globalInfo;
static GameState gameState;
static WorkHandler workHandler;
static s64 globalPerformanceCountFrequency;
static HANDLE semaphoreHandle;
static GLuint globalTextureHandle;
static OpenGLInfo openGLInfo;

DebugCycleCounter debugCounters[DebugCycleCounter_Count];
PushArray<Button> *debugButtons = NULL;
u32 (*AllocateGPUTexture)(u32 width, u32 height, u32 *pixels);
Arena *constantArena;
Arena *frameArena;
Arena *workingArena;

static void freeFile(File *file)
{
	void *memory = file->GetMemory();
	if (memory)
	{
		VirtualFree(memory, 0, MEM_RELEASE);
	}
}

static File *readEntireFile(char * fileName)
{
	void * memory = 0;
	unsigned int size = 0;
	
	HANDLE fileHandle = CreateFileA(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fileHandle != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER fileSize;
		if (GetFileSizeEx(fileHandle, &fileSize))
		{
			u32 fileSize32 = (u32)fileSize.QuadPart;
			memory = VirtualAlloc(0, fileSize32, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
			if (memory)
			{
				DWORD bytesRead;
				if (ReadFile(fileHandle, memory, fileSize32, &bytesRead, 0) && fileSize32 == bytesRead)
				{
					size = fileSize32;
				}
				else
				{
					VirtualFree(memory, 0, MEM_RELEASE);
					memory = 0;
				}
			}
			
		}
		CloseHandle(fileHandle);
	}
	File* result = new File(memory, size);
	return result;
}

static bool writeEntireFile(char * fileName, File *file)
{
	void * memory = file->GetMemory();
	unsigned int size = file->GetSize();

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
			OutputDebugStringA("Error writing file.");
		}

		CloseHandle(fileHandle);
	}
	return result;

}

static void initializeImageBuffer(BITMAPINFO *info, ImageBuffer *buffer, int width, int height)
{
	//TODO: BULLETPROOF THIS

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

static void displayImageBuffer(BITMAPINFO *info, HDC deviceContext, int windowWidth, int windowHeight, ImageBuffer *buffer)
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

	BEGIN_TIMED_BLOCK(SwitchBuffers);

	SwapBuffers(deviceContext);

	END_TIMED_BLOCK(SwitchBuffers);

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
	soundOutput->bytesPerSample = sizeof(s16) * 2;
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
	s16 *samples)
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
		s16 *sampleOut = (s16*)region1;
		s16 *sourceSample = samples;
		DWORD Region1SampleCount = region1Size / soundOutput->bytesPerSample;
		for (DWORD sampleIndex = 0; sampleIndex < Region1SampleCount; ++sampleIndex)
		{
			*sampleOut++ = *sourceSample++;
			*sampleOut++ = *sourceSample++;
			soundOutput->runningSampleIndex++;
		}
		DWORD Region2SampleCount = region2Size / soundOutput->bytesPerSample;
		sampleOut = (s16*)region2;
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

static void HandleDebugCycleCount()
{
	for (int counterIndex = 0; counterIndex < DebugCycleCounter_Count; counterIndex++)
	{
		
		DebugCycleCounter *counter = debugCounters + counterIndex;

		if (counter->hit > 0)
		{
			char textBuffer[256];
			_snprintf_s(textBuffer, sizeof(textBuffer), "%d: c: %I64u h: %u c/h: %I64u\n", counterIndex, counter->cycleCount, counter->hit, counter->cycleCount/counter->hit);
			OutputDebugStringA(textBuffer);
			counter->cycleCount = 0;
			counter->hit = 0;
		}
		
	}
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
static u32 volatile todo;
WorkQueueEntry entries[256];

struct ThreadInfo
{
	u32 threadIndex;
	HANDLE semaphoreHandle;
};

static void PushString(HANDLE semiphoreHandle, char *string)
{
	WorkQueueEntry * entry = entries + entryCount;
	entry->stringToPrint = string;

	WRITEBARRIER

	entryCount++;
	ReleaseSemaphore(semiphoreHandle, 1, 0);

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

void WakeThreads()
{
	ReleaseSemaphore(semaphoreHandle, 1, 0);
}

DWORD WINAPI ThreadProc(LPVOID param)
{
	ThreadInfo *threadInfo = (ThreadInfo *)param;

	char *stringToOutput = (char *)param;
	for (;;)
	{
		if (workHandler.WorkDone())
		{
			WaitForSingleObjectEx(threadInfo->semaphoreHandle, INFINITE, FALSE);
		}
		else
		{
			workHandler.DoWork();
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
static glTexImage2DMultisample_ *glTexImage2DMultisample;


static glCreateShader_ *glCreateShader;
static glShaderSource_ *glShaderSource;
static glCompileShader_ *glCompileShader;
static glLinkProgram_ *glLinkProgram;
static glCreateProgram_ *glCreateProgram;
static glAttachShader_ *glAttachShader;
static glValidateProgram_ *glValidateProgram;
static glGetProgramInfoLog_ *glGetProgramInfoLog;
static glGetProgramiv_ *glGetProgramiv;
static glGetShaderInfoLog_ *glGetShaderInfoLog;
static glUniformMatrix4fv_ *glUniformMatrix4fv;
static glUniform4fv_ *glUniform4fv;
static glUniform1iv_ *glUniform1iv;
static glGetUniformLocation_ *glGetUniformLocation;
static glUseProgram_ *glUseProgram;
static glVertexAttribPointer_ *glVertexAttribPointer;
static glEnableVertexAttribArray_ *glEnableVertexAttribArray;
static glDisableVertexAttribArray_ *glDisableVertexAttribArray;
static glGetAttribLocation_ *glGetAttribLocation;

int win32OpenGLAttribs[] =
{
	WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
	WGL_CONTEXT_MINOR_VERSION_ARB, 0,
	WGL_CONTEXT_FLAGS_ARB, 0
	| WGL_CONTEXT_DEBUG_BIT_ARB
	,
	WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
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
		HWND window = CreateWindowExA(0, windowClass.lpszClassName, "WGLLOADER", CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, 0, 0, windowClass.hInstance, 0);

		HDC windowDC = GetDC(window);		
		win32SetPixelFormat(windowDC);
		HGLRC contextRC = wglCreateContext(windowDC);
		if (wglMakeCurrent(windowDC, contextRC))
		{
			//wglChoosePixelFormatARB = (wglChoosePixelFormatARB_ *)wglGetProcAddress("wglChoosePixelFormatARB");
			WGLPROC(wglChoosePixelFormatARB);
			wglCreateContextAttribsARB = (wgl_create_context_attribs_ARB *)wglGetProcAddress("wglCreateContextAttribsARB");
			//glTexImage2DMultisample	= (glTexImage2DMultisample_ *)wglGetProcAddress("glTexImage2DMultisample");
			WGLPROC(glTexImage2DMultisample);
			
		}
		wglMakeCurrent(0, 0);
		wglDeleteContext(contextRC);
		ReleaseDC(window, windowDC);
		DestroyWindow(window);
			
	}
}

static void win32InitOpenGL(HWND window)
{
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
		bool modernContext = true;
		context = wglCreateContext(windowDC);
	}
	if (wglMakeCurrent(windowDC, context))
	{
		openGLInfo = OpenGLGetInfo(modernContext);
		wgl_swap_inverval_ext *wglSwapIntervalEXT= (wgl_swap_inverval_ext *)wglGetProcAddress("wglSwapIntervalEXT");
		if (wglSwapIntervalEXT)
		{
			wglSwapIntervalEXT(20);
		}

		WGLPROC(glCreateShader);
		WGLPROC(glShaderSource);
		WGLPROC(glCompileShader);
		WGLPROC(glLinkProgram);
		WGLPROC(glCreateProgram);
		WGLPROC(glAttachShader);
		//WGLPROC(glValidateProgram);
		WGLPROC(glValidateProgram);
		WGLPROC(glGetProgramInfoLog);
		WGLPROC(glGetProgramiv);
		WGLPROC(glGetShaderInfoLog);
		WGLPROC(glUniformMatrix4fv);
		WGLPROC(glUniform4fv);
		WGLPROC(glUniform1iv);
		WGLPROC(glGetUniformLocation);
		WGLPROC(glUseProgram);
		WGLPROC(glVertexAttribPointer);
		WGLPROC(glEnableVertexAttribArray);
		WGLPROC(glDisableVertexAttribArray);
		WGLPROC(glGetAttribLocation);


		GLFunctions incoming = 
		{
			glCreateShader,
			glShaderSource,
			glCompileShader,
			glLinkProgram,
			glCreateProgram,
			glAttachShader,
			glValidateProgram,
			glGetProgramInfoLog,
			glGetProgramiv,
			glGetShaderInfoLog,
			glUniformMatrix4fv,
			glUniform4fv,
			glUniform1iv,
			glGetUniformLocation,
			glUseProgram,
			glVertexAttribPointer,
			glEnableVertexAttribArray,
			glDisableVertexAttribArray,
			glGetAttribLocation,
		};

		OpenGLInit(context, &incoming);		
	}

	ReleaseDC(window, windowDC);
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

static void HandleWindowsMassages()
{
	MSG message;
	while (PeekMessageA(&message, 0, 0, 0, PM_REMOVE))
	{

		switch (message.message)
		{

		case WM_LBUTTONDOWN:
		{
			globalMouseInput.leftButtonDown = true;
			globalMouseInput.leftButtonPressedThisFrame = true;
		}break;
		case WM_LBUTTONUP:
		{
			globalMouseInput.leftButtonDown = false;
		}break;
		case WM_RBUTTONDOWN:
		{
			globalMouseInput.rightButtonDown = true;
			globalMouseInput.rightButtonPressedThisFrame = true;
		}break;
		case WM_RBUTTONUP:
		{
			globalMouseInput.rightButtonDown = false;
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
			bool32 wasDown = ((message.lParam &(1 << 30)) != 0);
			bool32 isDown = ((message.lParam &(1 << 31)) == 0);
			if (wasDown != isDown)
			{

				for (char i = 0; i < 26; i++)
				{
					u32 translatedI = i + 'A';
					if (vkCode == translatedI)
					{
						globalKeybordInput.input[i].isDown = !globalKeybordInput.input[i].isDown;

						if (globalKeybordInput.input[i].isDown)
						{
							globalKeybordInput.input[i].pressedThisFrame = true;
						}
					}
				}
				
				HANDLEINPUTKEY(VK_SPACE, space);				
				HANDLEINPUTKEY(VK_SHIFT, shift);


				if (vkCode == VK_ESCAPE)
				{
					running = false;
				}
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

int CALLBACK WinMain(
	HINSTANCE instance,
	HINSTANCE prevInstance,
	LPSTR commandLine,
	int showCode
)
{
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
	workHandler.Initiate(&InterlockedCompareExchange, &WakeThreads, &InterlockedIncrement, threadCount);


	WNDCLASSA windowClass = {};
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = win32MainWindowCallback;
	windowClass.hInstance = instance;
	windowClass.lpszClassName = "WindowClass";

	UINT desiredSchedulerMS = 1;
	bool sleepIsGranular = (timeBeginPeriod(desiredSchedulerMS) == TIMERR_NOERROR);
	LARGE_INTEGER perfCountFrequencyResult;
	QueryPerformanceFrequency(&perfCountFrequencyResult);
	globalPerformanceCountFrequency = perfCountFrequencyResult.QuadPart;

	File::Initialize(&readEntireFile, &freeFile, &writeEntireFile);

	int windowWidth = 1280, windowHeight = 720;
	void *gameMemory = VirtualAlloc(0, MegaBytes(500), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	constantArena = InitArena(gameMemory, MegaBytes(500));
	void *frameMem = VirtualAlloc(0, MegaBytes(500), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	frameArena = InitArena(frameMem, MegaBytes(500));
	void *workingMemory = VirtualAlloc(0, MegaBytes(100), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	workingArena = InitArena(workingMemory, MegaBytes(100));


	initializeImageBuffer(&globalInfo, &globalImageBuffer, windowWidth, windowHeight);
	if (RegisterClass(&windowClass))
	{
		HWND window = CreateWindowEx(0, windowClass.lpszClassName, "Game Try 1", WS_POPUP|WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, windowWidth, windowHeight, 0, 0, instance, 0);
		if (window)
		{
			running = true;
			HDC deviceContext = GetDC(window);

			win32InitOpenGL(window);

			u32 *whitePixels = new u32[16];
			for (u32 iWhite = 0; iWhite < 16; iWhite++)
			{
				whitePixels[iWhite] = 0xFFFFFFFF;
			}
			
			GLuint whiteTextureID = (GLuint)OpenGLDownLoadImage(1, 1, whitePixels);

			InitWhiteOpenGL(whiteTextureID);

			AllocateGPUTexture = OpenGLDownLoadImage;

			u32 vertexBufferSize = MegaBytes(4);
			void* vertexBuffer = VirtualAlloc(0, vertexBufferSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
			u32 pushBufferSize = MegaBytes(4);
			void* pushBuffer = VirtualAlloc(0, pushBufferSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
			RenderCommands renderCommands = {};
			renderCommands.maxBufferSize = pushBufferSize;
			renderCommands.pushBufferSize = 0;
			renderCommands.vertexArray = (ColoredVertex *)vertexBuffer;
			renderCommands.MaxVertexCount = vertexBufferSize / sizeof(ColoredVertex);
			renderCommands.pushBufferBase = (u8 *)pushBuffer;
			renderCommands.height = globalImageBuffer.height;
			renderCommands.width = globalImageBuffer.width;

			RECT windowRect;
			/*
			if (GetWindowRect(window, &windowRect))
				ClipCursor(&windowRect);
			*/
			SoundBuffer soundOutput = {};
			initializeSoundBuffer(window, &soundOutput);
			s16 *samples = (s16 *)VirtualAlloc(0, soundOutput.secondaryBufferSize,
				MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
			soundOutput.soundSamples = samples;
			int sampleAmount = soundOutput.secondaryBufferSize / sizeof(s16);
			for (unsigned int i = 0; i < (soundOutput.secondaryBufferSize/sizeof(s16)); i++)
			{
				samples[i] = 0;
			}
			win32FillSoundBuffer(&soundOutput, 0, 0, samples);

			int monitorRefreshHz = 60;
			HDC refreshDC = GetDC(window);
			int win32RefreshRate = GetDeviceCaps(refreshDC, VREFRESH);
			ReleaseDC(window, refreshDC);
			if (win32RefreshRate > 1)
			{
				monitorRefreshHz = win32RefreshRate;
			}
			f32 gameUpdateHz = (float)(monitorRefreshHz);
			f32 targetSecondsPerFrame = 1.0f / (f32)gameUpdateHz;
			globalMouseInput.expectedTimePerFrame = targetSecondsPerFrame;
			LARGE_INTEGER timeCounter;
			QueryPerformanceCounter(&timeCounter);

#if Internal
			for (u32 counterIndex = 0; counterIndex < DebugCycleCounter_Count; counterIndex++)
			{
				debugCounters[counterIndex].cycleCount = 0;
				debugCounters[counterIndex].hit = 0;
			}

			debugButtons = new PushArray<Button>(MAX_DEBUG_BUTTON_COUNT);

#endif
			Input input;
			UpdateInput(&input, &globalMouseInput, &globalKeybordInput);		
		
			AnimationCreatorInit(&input);

			gameState = InitGame(windowWidth, windowHeight, &workHandler);
			while (running)
			{
				BEGIN_TIMED_BLOCK(Frame);


				HandleWindowsMassages();
				
				GetClientRect(window, &windowRect);
				
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
				
				UpdateInput(&input, &globalMouseInput, &globalKeybordInput);

				//GameUpdateAndRender(&gameState, &renderCommands, &input, &soundOutput);
				
				//AnimationCreatorUpdateAndRender(&renderCommands, &input);

				LightingMain(&renderCommands, &workHandler, &input);
				
#if Internal
				HandleDebugCycleCount();
#endif				
				

				if (soundOutput.soundIsValid)
				{
					win32FillSoundBuffer(&soundOutput, byteToLock, bytesToWrite, samples);
				}
				if (!soundOutput.soundIsPlaying)
				{
					globalSoundBuffer->Play(0, 0, DSBPLAY_LOOPING);
					soundOutput.soundIsPlaying = true;
				}
				
				LARGE_INTEGER endCounter;
				QueryPerformanceCounter(&endCounter);
				
				
				windowDimension dim;
				dim = win32GetWindowDimension(window);
				
#if 1
				RenderGroup renderGroup = InitRenderGroup(gameState.assetHandler, &renderCommands);
				RenderGroup *rg = &renderGroup;
				f32 deltaTime = win32GetSecondsElapsed(timeCounter, endCounter);
				String s = CreateString(frameArena, deltaTime);
				v3 camPos = V3(0, 0, 0);
				float screenWidth = gameState.player->screen->width;
				float screenHeight = gameState.player->screen->height;
				v3 screenCenterOffset = V3(-screenWidth * 0.4f, -screenHeight * 0.4f, 1.1f);				
				PushOrthogonalTransform(rg, (float)windowWidth, (float)windowHeight);
				PushDebugString(rg, screenCenterOffset, s, 20);

				gameState.debugUI->buttons = debugButtons->pushArray;
				gameState.debugUI->amountOfButtons = debugButtons->size;
				gameState.debugUI->Update(&input);
				
				for (u32 i = 0; i < debugButtons->size; i++)
				{
					Button currentButton = debugButtons->Get(i);

					PushButton(rg, currentButton.pos + p12(screenCenterOffset), currentButton.width, currentButton.height, currentButton.text, currentButton.currentColor);
				}
#endif

				OpenGlRenderGroupToOutput(&renderCommands);


				for (unsigned int i = 0; i < globalKeybordInput.amountOfKeys; i++)
				{
					globalKeybordInput.input[i].pressedThisFrame = false;
				}
				globalMouseInput.leftButtonPressedThisFrame = false;
				globalMouseInput.rightButtonPressedThisFrame = false;
				
				displayImageBuffer(&globalInfo, deviceContext, dim.width, dim.height, &globalImageBuffer);

				Clear(frameArena);
				END_TIMED_BLOCK(Frame);
				timeCounter = endCounter;
			}

			ClipCursor(NULL);
		}
	}
}