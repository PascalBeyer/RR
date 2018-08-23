#ifndef RR_OPENGL
#define RR_OPENGL

#include "Math.h"
#include "Renderer.h"

#define GL_ARRAY_BUFFER                   0x8892

#define GL_CLAMP_TO_EDGE                  0x812F
#define GL_CLAMP_TO_BORDER                0x812D

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

#define GL_FRAMEBUFFER_COMPLETE           0x8CD5
#define GL_READ_FRAMEBUFFER               0x8CA8
#define GL_DRAW_FRAMEBUFFER               0x8CA9
#define GL_FRAMEBUFFER                    0x8D40

#define GL_DEPTH_COMPONENT16              0x81A5
#define GL_DEPTH_COMPONENT24              0x81A6
#define GL_DEPTH_COMPONENT32              0x81A7
#define GL_DEPTH_ATTACHMENT               0x8D00
#define GL_COLOR_ATTACHMENT0              0x8CE0

#define GL_DEBUG_OUTPUT_SYNCHRONOUS       0x8242
#define GL_DEBUG_SEVERITY_HIGH            0x9146

#define GL_TEXTURE0                       0x84C0
#define GL_TEXTURE1                       0x84C1
#define GL_TEXTURE2                       0x84C2
#define GL_TEXTURE3                       0x84C3
#define GL_TEXTURE4                       0x84C4
#define GL_TEXTURE5                       0x84C5
#define GL_TEXTURE6                       0x84C6
#define GL_TEXTURE7                       0x84C7
#define GL_TEXTURE8                       0x84C8
#define GL_TEXTURE9                       0x84C9

#define GL_STREAM_DRAW                    0x88E0
#define GL_STREAM_READ                    0x88E1
#define GL_STREAM_COPY                    0x88E2

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

typedef char GLchar;
typedef ptrdiff_t GLsizeiptr;
typedef void (APIENTRY *GLDEBUGPROC)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, void *userParam);

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
typedef void WINAPI glGenFramebuffers_(GLsizei n, GLuint *ids); 
typedef void WINAPI glBindFramebuffer_(GLenum target, GLuint framebuffer);
typedef void WINAPI glVertexAttrib3f_(GLuint index, GLfloat v0, GLfloat v1, GLfloat v2);
typedef GLenum WINAPI glCheckFramebufferStatus_(GLenum target);
typedef void WINAPI glFramebufferTexture_(GLenum target, GLenum attachment, GLuint texture, GLint level);
typedef void WINAPI glFramebufferTexture2D_(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void WINAPI glBlitFramebuffer_(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
typedef void WINAPI glActiveTexture_(GLenum texture);
typedef void WINAPI glDebugMessageCallback_(GLDEBUGPROC callback, void * userParam);
typedef void WINAPI glUniform1i_(GLint location, GLint v0);
typedef void WINAPI glBindAttribLocation_(GLuint program, GLuint index, const GLchar *name);
typedef void WINAPI glGenVertexArrays_(GLsizei n, GLuint *arrays);
typedef void WINAPI glBindVertexArray_(GLuint arrayName);
typedef void WINAPI glGenBuffers_(GLsizei n, GLuint *buffers);
typedef void WINAPI glBindBuffer_(GLenum target, GLuint buffer);
typedef void WINAPI glBufferData_(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage);

static glBufferData_ *glBufferData;
static glBindBuffer_ *glBindBuffer;
static glGenBuffers_ *glGenBuffers;
static glBindVertexArray_ *glBindVertexArray;
static glGenVertexArrays_ *glGenVertexArrays;
static glBindAttribLocation_ *glBindAttribLocation;
static glUniform1i_ *glUniform1i;
static glActiveTexture_ *glActiveTexture;
static glDebugMessageCallback_ *glDebugMessageCallback;
static glBlitFramebuffer_ *glBlitFramebuffer;
static glFramebufferTexture2D_ *glFramebufferTexture2D;
static glFramebufferTexture_ *glFramebufferTexture;
static glCheckFramebufferStatus_ *glCheckFramebufferStatus;
static glVertexAttrib3f_ *glVertexAttrib3f;
static glBindFramebuffer_ *glBindFramebuffer;
static glGenFramebuffers_ *glGenFramebuffers;
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

#define GL_VERTEX_SHADER                  0x8B31
#define GL_FRAGMENT_SHADER				  0x8B30
#define GL_VALIDATE_STATUS                0x8B83
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82

struct OpenGLProgram
{
	GLuint program;
	GLuint transform;
	GLuint vertP;
	GLuint vertC;
	GLuint vertUV;

	GLuint sampler;
};

//globals
static GLuint vertexBuffer;
static GLuint defaultInternalTextureFormat;

static OpenGLProgram basic;
static OpenGLProgram shadow;
static OpenGLProgram zBias;

static GLuint basicShadowTransform;

static GLuint renderFrameBuffer;
static GLuint renderTexture;
static GLuint renderDepth;

static GLuint shadowFrameBuffer;
static GLuint shadowTexture;

static m4x4 projectionMat;
static m4x4 shadowMat;

bool StringEquals(char *first, u64 firstSize, char* second)
{
	char* at = second;
	for (u64 i = 0; i < firstSize; i++, at++)
	{
		if (*at == '\0' || first[i] != *at)
		{
			return false;
		}
	}
	return true;
}

OpenGLInfo OpenGLGetInfo(bool modernContext)
{
	OpenGLInfo info = {};

	info.modernContext = modernContext;

	info.extensions = (char *)glGetString(GL_EXTENSIONS);
	info.renderer = (char *)glGetString(GL_RENDERER);
	if (modernContext)
	{
		info.shadingLanguageVersion = (char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
	}
	info.vendor = (char *)glGetString(GL_VENDOR);
	info.version = (char *)glGetString(GL_VERSION);


	glGetIntegerv(GL_MAX_SAMPLES, &(info.amountOfMultiSamples));

	char *at = info.extensions;
	while (*at)
	{
		while (*at == ' ') { at++; }
		char *end = at;
		while (*end && *end != ' ') { end++; }

		u64 count = end - at;

		if (StringEquals(at, count, "GL_EXT_texture_sRGB")) { info.GL_EXT_texture_sRGB = true; }
		at = end;
	}

	return info;
}

GLuint OpenGLCreateProgram(char *headerCode, char *vertexCode, char *fragmentCode)
{
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLchar *vertexShaderCode[] =
	{
		headerCode, vertexCode
	};
	glShaderSource(vertexShaderID, ArrayCount(vertexShaderCode), vertexShaderCode, 0);
	glCompileShader(vertexShaderID);

	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	GLchar *fragmentShaderCode[] =
	{
		headerCode, fragmentCode
	};
	glShaderSource(fragmentShaderID, ArrayCount(fragmentShaderCode), fragmentShaderCode, 0);
	glCompileShader(fragmentShaderID);

	GLuint programID = glCreateProgram();
	glAttachShader(programID, vertexShaderID);
	glAttachShader(programID, fragmentShaderID);
	glLinkProgram(programID);

	glValidateProgram(programID);
	GLint linkValidated = false;
	glGetProgramiv(programID, GL_LINK_STATUS, &linkValidated);
	//glGetProgramiv(programID, GL_COMPILE_STATUS, &compileValidated);
	GLint validated = linkValidated;
	if (!validated)
	{
		GLsizei length;
		char programError[4096];
		char vertexError[4096];
		char fragmentError[4096];
		glGetProgramInfoLog(programID, sizeof(programError), &length, programError);
		glGetShaderInfoLog(vertexShaderID, sizeof(vertexError), &length, vertexError);
		glGetShaderInfoLog(fragmentShaderID, sizeof(fragmentError), &length, fragmentError);

		Assert(!"ShaderError");
	}

	return programID;
}

void OpenGLRect3D(v3 pos, v3 v1, v3 v2, v4 color)
{

	v3 UL = pos;
	v3 UR = pos + v1;
	v3 DL = pos + v2;
	v3 DR = pos + v1 + v2;

	glBegin(GL_TRIANGLES);

	glColor4f(color.r, color.g, color.b, color.a);

	//Lower Triangle
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(DL.x, DL.y, DL.z);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(DR.x, DR.y, DR.z);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(UR.x, UR.y, UR.z);

	//upper Triangle
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(DL.x, DL.y, DL.z);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(UL.x, UL.y, UL.z);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(UR.x, UR.y, UR.z);
	glEnd();

	glBegin(GL_LINES);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	glVertex3f(DL.x, DL.y, DL.z);
	glVertex3f(UL.x, UL.y, UL.z);

	glVertex3f(DL.x, DL.y, DL.z);
	glVertex3f(DR.x, DR.y, DR.z);

	glVertex3f(UR.x, UR.y, UR.z);
	glVertex3f(UL.x, UL.y, UL.z);

	glVertex3f(UR.x, UR.y, UR.z);
	glVertex3f(DR.x, DR.y, DR.z);
	glEnd();
}
void OpenGLRect3D(v3 p1, v3 p2, v3 p3, v3 p4, v4 c1, v4 c2, v4 c3, v4 c4)
{
	glBegin(GL_TRIANGLES);
	//Lower Triangle
	glColor4f(c1.r, c1.g, c1.b, c1.a);
	glVertex3f(p1.x, p1.y, p1.z);

	glColor4f(c3.r, c3.g, c3.b, c3.a);
	glVertex3f(p3.x, p3.y, p3.z);

	glColor4f(c4.r, c4.g, c4.b, c4.a);
	glVertex3f(p4.x, p4.y, p4.z);

	//upper Triangle
	glColor4f(c1.r, c1.g, c1.b, c1.a);
	glVertex3f(p1.x, p1.y, p1.z);

	glColor4f(c2.r, c2.g, c2.b, c2.a);
	glVertex3f(p2.x, p2.y, p2.z);

	glColor4f(c4.r, c4.g, c4.b, c4.a);
	glVertex3f(p4.x, p4.y, p4.z);
	glEnd();

#if 0
	glBegin(GL_LINES);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	glVertex3f(p1.x, p1.y, p1.z);
	glVertex3f(p2.x, p2.y, p2.z);

	glVertex3f(p1.x, p1.y, p1.z);
	glVertex3f(p3.x, p3.y, p3.z);

	glVertex3f(p4.x, p4.y, p4.z);
	glVertex3f(p2.x, p2.y, p2.z);

	glVertex3f(p4.x, p4.y, p4.z);
	glVertex3f(p3.x, p3.y, p3.z);
	glEnd();
#endif

}

void OpenGLUpdateBitmap(Bitmap bitmap)
{
	Assert(bitmap.textureHandle);

	glBindTexture(GL_TEXTURE_2D, (GLuint)bitmap.textureHandle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, bitmap.width, bitmap.height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, bitmap.pixels);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	

	glBindTexture(GL_TEXTURE_2D, 0);
}

void OpenGLSetScreenSpace(u32 width, u32 height, float inGameWidth)
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);

	float a = 2.0f;
	float b = 2.0f * (float)width / (float)height;
	float projM[] =
	{
		a, 0, 0, 0,
		0, -b, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, inGameWidth,
	};
	glLoadMatrixf(projM);
}

u32 OpenGLDownLoadImage(u32 width, u32 height, u32* pixels)
{
	GLuint handle;
	glGenTextures(1, &handle);
	glBindTexture(GL_TEXTURE_2D, handle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, pixels);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	

	glBindTexture(GL_TEXTURE_2D, 0);
	return handle;
}

char *headerCodeOpenGL =
R"FOO(
//Header Code
#version 130

#define v2 vec2
#define V2 vec2

#define v3 vec3
#define V3 vec3

#define v4 vec4
#define V4 vec4



)FOO";

void SetUpBasicZBiasProgram()
{

	char *vertexCodeTex =
		R"FOO(
	//Vertex Code

	uniform mat4x4 transform;

	in vec3 vertP;
	in vec4 vertC;
	in vec2 vertUV;

	smooth out vec4 fragColor;
	smooth out vec2 fragCoord;

	void main(void)
	{
		vec4 inputVertex = vec4(vertP, 1.0f);
		gl_Position = transform * inputVertex;

		fragColor = vertC;
		fragCoord = vertUV;

	}

	)FOO";
	char *fragmentCodeTex =
		R"FOO(
	//Fragment Code

	out vec4 resultColor;

	smooth in vec4 fragColor;
	smooth in vec2 fragCoord;	

	uniform sampler2D texSampler;
	void main(void)
	{
		vec4 result = texture(texSampler, fragCoord);
		float zlevel = 0.5f * (gl_FragCoord.z + 1.0f);
		resultColor = fragColor * result;
		//resultColor = vec4(zlevel, zlevel, zlevel, 1.0f);
	}

	)FOO";
	zBias.program = OpenGLCreateProgram(headerCodeOpenGL, vertexCodeTex, fragmentCodeTex);
	zBias.transform = glGetUniformLocation(zBias.program, "transform");
	zBias.sampler = glGetUniformLocation(zBias.program, "texSampler");
	zBias.vertP = glGetAttribLocation(zBias.program, "vertP");
	zBias.vertC = glGetAttribLocation(zBias.program, "vertC");
	zBias.vertUV = glGetAttribLocation(zBias.program, "vertUV");
	Assert(zBias.program);
}

void SetUpBasicProgram()
{

	char *vertexCode =
		R"FOO(
	//Vertex Code

	uniform mat4x4 transform;
	uniform mat4x4 shadowTransform;

	in vec3 vertP;
	in vec4 vertC;

	smooth out vec4 fragColor;
	smooth out vec4 shadowCoord;

	void main(void)
	{
		vec4 inputVertex = vec4(vertP, 1);
		gl_Position = transform * inputVertex;
		
		fragColor = vertC;
		shadowCoord = shadowTransform * inputVertex;
	}

	)FOO";
	char *fragmentCode =
		R"FOO(
	//Fragment Code

	out vec4 resultColor;

	smooth in vec4 fragColor;
	smooth in vec4 shadowCoord;

	uniform sampler2D depthTexture;
	void main(void)
	{
		float visibility = 1.0f;
		vec3 shadowCoord2 = shadowCoord.xyz / shadowCoord.w;
		vec3 shadowCoord3 = 0.5f * shadowCoord2 + vec3(0.5);
		
		float distanceLightNearestGeometry = texture2D(depthTexture, shadowCoord3.xy).r;

		float distanceFromLight = shadowCoord3.z;

		float bias = 0.0001;

		if (distanceLightNearestGeometry < distanceFromLight - bias)
		{
			visibility = 0.5f;
		}
		resultColor = vec4(visibility * fragColor.rgb, fragColor.a);
		//resultColor = vec4(vec3(distanceLightNearestGeometry), 1);
		//resultColor = texture2D(depthTexture, shadowCoord3.xy);
	}

	)FOO";


	basic.program = OpenGLCreateProgram(headerCodeOpenGL, vertexCode, fragmentCode);
	Assert(basic.program);
	glUseProgram(basic.program);
	basic.vertP = glGetAttribLocation(basic.program, "vertP");
	basic.vertC = glGetAttribLocation(basic.program, "vertC");
	

	basic.transform = glGetUniformLocation(basic.program, "transform");
	basic.sampler = glGetUniformLocation(basic.program, "depthTexture");
	basicShadowTransform = glGetUniformLocation(basic.program, "shadowTransform");
	glUseProgram(0);
}

void SetUpDepthProgram()
{
	glGenFramebuffers(1, &shadowFrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFrameBuffer);
	
	glGenTextures(1, &shadowTexture);
	glBindTexture(GL_TEXTURE_2D, shadowTexture);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1024, 1024, 0, GL_BGRA_EXT, GL_FLOAT, 0 );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, shadowTexture, 0);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	Assert(status == GL_FRAMEBUFFER_COMPLETE);

	char *vertexCode =
		R"FOO(
	//Vertex Code

	uniform mat4x4 transform2;

	in vec3 vertP;
	in vec4 vertC;

	smooth out vec4 color;
	void main(void)
	{
		vec4 inputVertex = vec4(vertP, 1);
		gl_Position = transform2 * inputVertex;
		color = vertC;
	}

	)FOO";
	char *fragmentCode =
		R"FOO(
	//Fragment Code

	smooth in vec4 color;
	
	out vec4 result;
	//out float depth;

	void main(void)
	{
		result = color;
		//depth = gl_FragCoord.z;
	}

	)FOO";

	
	shadow.program = OpenGLCreateProgram(headerCodeOpenGL, vertexCode, fragmentCode);
	Assert(shadow.program);
	glUseProgram(shadow.program);
	shadow.vertP = glGetAttribLocation(shadow.program, "vertP");
	shadow.vertC = glGetAttribLocation(shadow.program, "vertC");
	
	shadow.transform = glGetUniformLocation(shadow.program, "transform2");

	GLuint error = glGetError();
	Assert(glGetError() == GL_NO_ERROR);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(0);

}

static void OpenGLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, void *userParam)
{
	if (1 || severity == GL_DEBUG_SEVERITY_HIGH)
	{
		char *ErrorMessage = (char *)message;
#if 1
		OutputDebugStringA("OPENGL: ");
		OutputDebugStringA(ErrorMessage);
		OutputDebugStringA("\n");
#endif
		Assert(!"OpenGL Error encountered");
	}
}

static void BeginUseProgram(OpenGLProgram prog, bool textured = false)
{
	glUseProgram(prog.program);

	glEnableVertexAttribArray(prog.vertP);
	glEnableVertexAttribArray(prog.vertC);

	if (textured)
	{
		glEnableVertexAttribArray(prog.vertUV);
		glVertexAttribPointer(prog.vertP, 3, GL_FLOAT, false, sizeof(TexturedVertex), (void *)OffsetOf(TexturedVertex, pos));
		glVertexAttribPointer(prog.vertC, 4, GL_UNSIGNED_BYTE, true, sizeof(TexturedVertex), (void *)OffsetOf(TexturedVertex, color));
		glVertexAttribPointer(prog.vertUV, 2, GL_FLOAT, false, sizeof(TexturedVertex), (void *)OffsetOf(TexturedVertex, uv));
	}
	else
	{
		glVertexAttribPointer(prog.vertP, 3, GL_FLOAT, false, sizeof(ColoredVertex), (void *)OffsetOf(ColoredVertex, pos));
		glVertexAttribPointer(prog.vertC, 4, GL_UNSIGNED_BYTE, true, sizeof(ColoredVertex), (void *)OffsetOf(ColoredVertex, color));

	}	
}
static void EndUseProgram(OpenGLProgram prog)
{
	
	glDisableVertexAttribArray(prog.vertP);
	glDisableVertexAttribArray(prog.vertC);
	glDisableVertexAttribArray(prog.vertUV);

	glUseProgram(0);
}

void OpenGLInit(HGLRC modernContext)
{
	//GLuint reservedBltTexture;
	//glGenTextures(1, &reservedBltTexture);

	OpenGLInfo info = OpenGLGetInfo(true);

	if (glDebugMessageCallback)
	{
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(OpenGLDebugCallback, 0);
	}

	defaultInternalTextureFormat = GL_RGBA8;
	if (info.GL_EXT_texture_sRGB)
	{
		defaultInternalTextureFormat = GL_SRGB8_ALPHA8;
	}

	GLuint DummyVertexArray;
	glGenVertexArrays(1, &DummyVertexArray);
	glBindVertexArray(DummyVertexArray);

	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

	SetUpBasicZBiasProgram();

	SetUpBasicProgram();
	
	SetUpDepthProgram();

#if 0
	glGenFramebuffers(1, &renderFrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, renderFrameBuffer);

	glGenTextures(1, &renderTexture);
	glBindTexture(GL_TEXTURE_2D, renderTexture);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1280, 720, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTexture, 0);

	glGenTextures(1, &renderDepth);
	glBindTexture(GL_TEXTURE_2D, renderDepth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 1280, 720, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, renderDepth, 0);
#endif

	GLuint texture_map;
	glGenTextures(1, &texture_map);
	glBindTexture(GL_TEXTURE_2D, texture_map);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1280, 720, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	renderTexture = texture_map;

	glBindTexture(GL_TEXTURE_2D, 0);

	// Build the texture that will serve as the depth attachment for the framebuffer.
	GLuint depth_texture;
	glGenTextures(1, &depth_texture);
	glBindTexture(GL_TEXTURE_2D, depth_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 1280, 720, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);
	renderDepth = depth_texture;

	// Build the framebuffer.
	GLuint framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_map, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture, 0);
	renderFrameBuffer = framebuffer;


	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	Assert(status == GL_FRAMEBUFFER_COMPLETE);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	Assert(glGetError() == GL_NO_ERROR);
}

void RenderIntoShadowMap(RenderCommands *rg)
{
	glDisable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);

	glUseProgram(shadow.program);
	glUniformMatrix4fv(shadow.transform, 1, GL_TRUE, &shadowMat.a[0][0]);
	glUseProgram(0);

	glBindFramebuffer(GL_FRAMEBUFFER, renderFrameBuffer);
	
	glViewport(0, 0, rg->width, rg->height);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (u32 pBufferIt = 0; pBufferIt < rg->pushBufferSize;)
	{

		RenderGroupEntryHeader *header = (RenderGroupEntryHeader *)(rg->pushBufferBase + pBufferIt);
		switch (header->type)
		{
		case RenderGroup_EntryTransform:
		{
			EntryTransform *transformHeader = (EntryTransform *)header;
			
			GLuint error = glGetError();
			Assert(!error);

			pBufferIt += sizeof(*transformHeader);
		}break;
		case RenderGroup_EntryTriangles:
		{
			EntryColoredVertices *trianglesHeader = (EntryColoredVertices *)header;

			BeginUseProgram(shadow);

			glBufferData(GL_ARRAY_BUFFER, trianglesHeader->vertexCount * sizeof(ColoredVertex), trianglesHeader->data, GL_STREAM_COPY);
			glDrawArrays(GL_TRIANGLES, 0, trianglesHeader->vertexCount);

			EndUseProgram(shadow);

			pBufferIt += sizeof(*trianglesHeader);
		}break;
		case RenderGroup_EntryUpdateTexture:
		{
			EntryUpdateTexture *updateHeader = (EntryUpdateTexture *)header;
			pBufferIt += sizeof(*updateHeader);
		}break;
		case RenderGroup_EntryTexturedQuads:
		{
			EntryTexturedQuads *quadHeader = (EntryTexturedQuads *)header;

			pBufferIt += sizeof(*quadHeader);
		}break;
		case RenderGroup_EntryClear:
		{
			EntryClear *clearHeader = (EntryClear *)header;

			pBufferIt += sizeof(*clearHeader);
		}break;
		case RenderGroup_EntryLines:
		{
			EntryColoredVertices *lineHeader = (EntryColoredVertices*)header;


			pBufferIt += sizeof(*lineHeader);
		}break;

		}
	}

	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//todo look into vertex sharing
void OpenGlRenderGroupToOutput(RenderCommands *rg)
{
	glDepthMask(GL_TRUE);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
	glEnable(GL_SAMPLE_ALPHA_TO_ONE);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_CULL_FACE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	
	shadowMat = /*biasMatrix * */Projection(rg->aspectRatio, rg->focalLength) * CameraTransform(V3(1, 0, 0), V3(0, 1, 0), V3(0, 0, 1), V3(0, 0, -30));

	glUseProgram(basic.program); 
	glUniform1i(basic.sampler, 1);

	m4x4 biasMatrix =
	{
		{
			{ 0.5f, 0.0f, 0.0f, 0.5f },
			{ 0.0f, 0.5f, 0.0f, 0.5f },
			{ 0.0f, 0.0f, 0.5f, 0.5f },
			{ 0.0f, 0.0f, 0.0f, 1.0f }
		}
	};
	m4x4 biasedShadowMat = /*biasMatrix * */ shadowMat;

	glUniformMatrix4fv(basicShadowTransform, 1, GL_TRUE, &biasedShadowMat.a[0][0]);

	
	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, rg->width, rg->height);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	GLuint err12or = glGetError();
	Assert(!err12or);

	RenderIntoShadowMap(rg);

	glUseProgram(basic.program);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, renderDepth);
	glActiveTexture(GL_TEXTURE0);

	glUseProgram(0);
	
	//glCullFace(GL_BACK);
	//glDisable(GL_CULL_FACE);

#if 1
	for (u32 pBufferIt = 0; pBufferIt < rg->pushBufferSize;)
	{

		RenderGroupEntryHeader *header = (RenderGroupEntryHeader *)(rg->pushBufferBase + pBufferIt);
		switch (header->type)
		{
		//todo move this into BeginUseProgram
		case RenderGroup_EntryTransform:
		{
			EntryTransform *transformHeader = (EntryTransform *)header;
			
			projectionMat = transformHeader->m;

			glUseProgram(zBias.program);
			glUniformMatrix4fv(zBias.transform, 1, GL_TRUE, projectionMat.a[0]);
			GLint texInt = 0;
			glUniform1iv(zBias.sampler, 1, &texInt);

			glUseProgram(basic.program);
			glUniformMatrix4fv(basic.transform, 1, GL_TRUE, projectionMat.a[0]);
			//glUniformMatrix4fv(basicTransformID, 1, GL_TRUE, shadowMat.a[0]);
			glUseProgram(0);

			GLuint error = glGetError();
			Assert(!error);

			pBufferIt += sizeof(*transformHeader);
		}break;
		//todo: make this do the game
		case RenderGroup_EntryUpdateTexture:
		{
			EntryUpdateTexture *updateHeader = (EntryUpdateTexture *)header;
			OpenGLUpdateBitmap(updateHeader->bitmap);
			pBufferIt += sizeof(*updateHeader);
		}break;
		case RenderGroup_EntryTexturedQuads:
		{
			EntryTexturedQuads *quadHeader = (EntryTexturedQuads *)header;
			Bitmap *bitmaps = quadHeader->quadBitmaps;

			BeginUseProgram(zBias, true);
			glBufferData(GL_ARRAY_BUFFER, quadHeader->vertexCount * sizeof(TexturedVertex), quadHeader->data, GL_STREAM_DRAW);

			for (u32 vertIndex = 0; vertIndex < quadHeader->vertexCount; vertIndex += 4)
			{
				Bitmap bitmap = bitmaps[vertIndex >> 2];
				glBindTexture(GL_TEXTURE_2D, bitmap.textureHandle);
				glDrawArrays(GL_TRIANGLE_STRIP, vertIndex, 4);
			}

			glBindTexture(GL_TEXTURE_2D, 0);
			EndUseProgram(zBias);

			pBufferIt += sizeof(*quadHeader);
		}break;
		case RenderGroup_EntryTriangles:
		{
			EntryColoredVertices *trianglesHeader = (EntryColoredVertices *)header;

			BeginUseProgram(basic);

			glBufferData(GL_ARRAY_BUFFER, trianglesHeader->vertexCount * sizeof(ColoredVertex), trianglesHeader->data, GL_STREAM_COPY);
			glDrawArrays(GL_TRIANGLES, 0, trianglesHeader->vertexCount);

			EndUseProgram(basic);

			pBufferIt += sizeof(*trianglesHeader);
		}break;

		case RenderGroup_EntryClear:
		{
			EntryClear *clearHeader = (EntryClear *)header;
			glClearColor(clearHeader->color.r, clearHeader->color.g, clearHeader->color.b, clearHeader->color.a);
			glClear(GL_COLOR_BUFFER_BIT);

			pBufferIt += sizeof(*clearHeader);
		}break;
		case RenderGroup_EntryLines:
		{
			EntryColoredVertices *lineHeader = (EntryColoredVertices*)header;

			BeginUseProgram(basic);
			glBufferData(GL_ARRAY_BUFFER, lineHeader->vertexCount * sizeof(ColoredVertex), lineHeader->data, GL_STREAM_COPY);
			glDrawArrays(GL_LINES, 0, lineHeader->vertexCount);
			EndUseProgram(basic);

			pBufferIt += sizeof(*lineHeader);
		}break;

		default:
		{
			pBufferIt++;
			//logging
			Die;
		}break;
		}
	}
#endif
	//glUseProgram(basicZBiasProgram);
	//glBindTexture(GL_TEXTURE_2D, renderTexture);
	//u32 size = rg->height;
	//OpenGLRect3D(V3(0, 0, 0), V3(0, rg->height, 0), V3(rg->width, 0, 0), V3(rg->width, rg->height, 0), V4(1, 1, 1, 1), V4(1, 1, 1, 1), V4(1,1,1, 1), V4(1, 1, 1, 1)) ;
	//
	//glUseProgram(0);

	err12or = glGetError();
	Assert(!err12or);
	//glTexImage2DMultisample();
#if 0
	u32 size = rg->height;
	glBindFramebuffer(GL_READ_FRAMEBUFFER, renderFrameBuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	glBlitFramebuffer(0, 0, size, size, 0, 0, size, size, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	err12or = glGetError();
	Assert(!err12or);
#endif

	//flush
	rg->pushBufferSize = 0;
	
}

#endif
