#ifndef RR_OPENGL
#define RR_OPENGL

#include <Windows.h>
#include <gl/gl.h>
#include "Math.h"
#include "Renderer.h"

typedef char GLchar;

#define GL_CLAMP_TO_EDGE                  0x812F

#define GL_FRAMEBUFFER_SRGB               0x8DB9
#define GL_SRGB8_ALPHA8                   0x8C43

#define GL_SHADING_LANGUAGE_VERSION       0x8B8C

#define WGL_CONTEXT_MAJOR_VERSION_ARB           0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB           0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB             0x2093
#define WGL_CONTEXT_FLAGS_ARB                   0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB            0x9126

#define GL_MULTISAMPLE                    0x809D
#define GL_SAMPLE_ALPHA_TO_COVERAGE       0x809E
#define GL_SAMPLE_ALPHA_TO_ONE            0x809F
#define GL_SAMPLE_COVERAGE                0x80A0
#define GL_SAMPLE_BUFFERS                 0x80A8
#define GL_SAMPLES                        0x80A9
#define GL_SAMPLE_COVERAGE_VALUE          0x80AA
#define GL_SAMPLE_COVERAGE_INVERT         0x80AB
#define GL_TEXTURE_2D_MULTISAMPLE         0x9100
#define GL_MAX_SAMPLES                    0x8D57

#define WGL_CONTEXT_DEBUG_BIT_ARB               0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB  0x0002

#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB        0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002

typedef void WINAPI glTexImage2DMultisample_(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
typedef GLuint WINAPI glCreateShader_(GLenum type);
typedef void WINAPI glShaderSource_(GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
typedef void WINAPI glCompileShader_(GLuint shader);
typedef void WINAPI glLinkProgram_(GLuint program);
typedef GLuint WINAPI glCreateProgram_(void);
typedef void WINAPI glAttachShader_(GLuint program, GLuint shader);
typedef void WINAPI glValidateProgram_(GLuint program);
typedef void WINAPI glGetProgramInfoLog_(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void WINAPI glGetProgramiv_(GLuint program, GLenum pname, GLint *params);
typedef void WINAPI glGetShaderInfoLog_(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void WINAPI glUniformMatrix4fv_(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void WINAPI glUniform4fv_(GLint location, GLsizei count, const GLfloat *value);
typedef void WINAPI glUniform1iv_(GLint location, GLsizei count, const GLint *value);
typedef GLint WINAPI glGetUniformLocation_(GLuint program, const GLchar *name);
typedef void WINAPI glUseProgram_(GLuint program);
typedef void WINAPI glVertexAttribPointer_(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);
typedef void WINAPI glEnableVertexAttribArray_(GLuint index);
typedef void WINAPI glDisableVertexAttribArray_(GLuint index);
typedef GLint WINAPI glGetAttribLocation_(GLuint program, const GLchar *name);

struct OpenGLInfo
{
	bool modernContext;

	bool  GL_EXT_texture_sRGB;

	char *extensions;
	char *renderer;
	char *version;
	char *shadingLanguageVersion;
	char *vendor;

	GLint amountOfMultiSamples;

};
struct OpenGLGlobals
{
	GLuint defaultInternalTextureFormat;
	GLuint basicZBiasProgram;
	GLuint basicProgram;
	GLint transformID;
	GLint transformID2;
	GLint texSamplerID;
	m4x4 projectionMat;
	GLuint whiteTextureID;

	GLuint vertPArrayID;
	GLuint vertCArrayID;
};

struct GLFunctions
{
	glCreateShader_ *glCreateShader;
	glShaderSource_ *glShaderSource;
	glCompileShader_ *glCompileShader;
	glLinkProgram_ *glLinkProgram;
	glCreateProgram_ *glCreateProgram;
	glAttachShader_ *glAttachShader;
	glValidateProgram_ *glValidateProgram;
	glGetProgramInfoLog_ *glGetProgramInfoLog;
	glGetProgramiv_ *glGetProgramiv;
	glGetShaderInfoLog_ *glGetShaderInfoLog;
	glUniformMatrix4fv_ *glUniformMatrix4fv;
	glUniform4fv_ *glUniform4fv;
	glUniform1iv_ *glUniform1iv;
	glGetUniformLocation_ *glGetUniformLocation;
	glUseProgram_ *glUseProgram;
	glVertexAttribPointer_ *glVertexAttribPointer;
	glEnableVertexAttribArray_ *glEnableVertexAttribArray;
	glDisableVertexAttribArray_ *glDisableVertexAttribArray;
	glGetAttribLocation_ *glGetAttribLocation;
};

static OpenGLGlobals openGLGlobals;

#define GL_VERTEX_SHADER                  0x8B31
#define GL_FRAGMENT_SHADER				  0x8B30
#define GL_VALIDATE_STATUS                0x8B83
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82

bool StringEquals(char *first, u64 firstSize, char* second);
OpenGLInfo OpenGLGetInfo(bool modernContext);
GLuint OpenGLCreateProgram(char *headerCode, char *vertexCode, char *fragmentCode);
void OpenGLRect3D(v3 pos, v3 v1, v3 v2, v4 color);
void OpenGLRect3D(v3 d1, v3 d2, v3 d3, v3 d4, v4 c1, v4 c2, v4 c3, v4 c4);
void OpenGLSetScreenSpace(u32 width, u32 height, float inGameWidth);
u32 OpenGLDownLoadImage(u32 width, u32 height, u32* pixels);
void OpenGLInit(HGLRC modernContext, GLFunctions *extensions);
void InitWhiteOpenGL(GLuint whiteTextureID);

void OpenGlRenderGroupToOutput(RenderCommands *rg);

#endif
