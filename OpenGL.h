#ifndef RR_OPENGL
#define RR_OPENGL

#define GL_ARRAY_BUFFER                   0x8892
#define GL_ELEMENT_ARRAY_BUFFER           0x8893

#define GL_CLAMP_TO_EDGE                  0x812F
#define GL_CLAMP_TO_BORDER                0x812D

#define GL_FRAMEBUFFER_SRGB               0x8DB9
#define GL_SRGB                           0x8C40
#define GL_SRGB8                          0x8C41
#define GL_SRGB_ALPHA                     0x8C42
#define GL_SRGB8_ALPHA8                   0x8C43
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE 0x8D56

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
#define GL_DEPTH_COMPONENT32F             0x8CAC
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

#define GL_PRIMITIVE_RESTART_FIXED_INDEX  0x8D69

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
typedef void WINAPI glUniform4fv_(GLint location, GLsizei count, const GLfloat *value);
typedef void WINAPI glUniform1iv_(GLint location, GLsizei count, const GLint *value);
typedef void WINAPI glUniformMatrix4fv_(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void WINAPI glUniform1f_(GLint location, GLfloat v0);
typedef void WINAPI glUniform2f_(GLint location, GLfloat v0, GLfloat v1);
typedef void WINAPI glUniform3f_(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void WINAPI glUniform4f_(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);


//typedef void WINAPI glDrawElements_(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices);


//static glDrawElements_ *glDrawElements;
static glTexImage2DMultisample_ *glTexImage2DMultisample;
static glUniform1f_ *glUniform1f;
static glUniform2f_	*glUniform2f;
static glUniform3f_	*glUniform3f;
static glUniform4f_ *glUniform4f;
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

	//uniforms
	GLuint projection;
	GLuint cameraTransform;
	GLuint shadowTransform;
	GLuint lightPos;
	GLuint scaleColor;

	//GLuint cameraPos;
	GLuint specularExponent;

	GLuint depthSampler;
	GLuint textureSampler;

	//attributes
	GLuint vertP;
	GLuint vertC;
	GLuint vertUV;
	GLuint vertN;
};

//globals
static OpenGLInfo openGLInfo;

static GLuint vertexBuffer;
static GLuint elementBuffer;
static GLuint defaultInternalTextureFormat;

static OpenGLProgram basic;
static OpenGLProgram basicMesh;
static OpenGLProgram shadow;
static OpenGLProgram zBias;

static GLuint renderFrameBuffer;
static GLuint renderTexture;
static GLuint renderDepth;

static GLuint shadowFrameBuffer;
static GLuint shadowTexture;

bool CStringEquals(char *first, u64 firstSize, char* second)
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

static OpenGLInfo OpenGLGetInfo(bool modernContext)
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

		if (CStringEquals(at, count, "GL_EXT_texture_sRGB")) { info.GL_EXT_texture_sRGB = true; }
		at = end;
	}

	return info;
}

static GLuint OpenGLCreateProgram(char *headerCode, char *vertexCode, char *fragmentCode)
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

static void OpenGLRect3D(v3 pos, v3 v1, v3 v2, v4 color)
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
static void OpenGLRect3D(v3 p1, v3 p2, v3 p3, v3 p4, v4 c1, v4 c2, v4 c3, v4 c4)
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

static void UpdateWrapingTexture(Bitmap bitmap)
{
	//Assert(bitmap.textureHandle);

	glBindTexture(GL_TEXTURE_2D, (GLuint)bitmap.textureHandle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, bitmap.width, bitmap.height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, bitmap.pixels);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


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

void RegisterTriangleMesh(TriangleMesh *mesh)
{
	glGenBuffers(1, &mesh->vertexVBO);
	glGenBuffers(1, &mesh->indexVBO);

	Assert(mesh->vertices.amount < 65536); // <, because reset index

	//todo look up what those GL_Stream_Draw mean?
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexVBO);
	glBufferData(GL_ARRAY_BUFFER, mesh->vertices.amount * sizeof(VertexFormat), mesh->vertices.data, GL_STREAM_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indices.amount * sizeof(mesh->indices[0]), mesh->indices.data, GL_STREAM_DRAW);

}

static u32 RegisterWrapingTexture(u32 width, u32 height, u32* pixels)
{
	GLuint handle;
	glGenTextures(1, &handle);
	glBindTexture(GL_TEXTURE_2D, handle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, pixels);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


	glBindTexture(GL_TEXTURE_2D, 0);
	return handle;
}


static u32 RegisterWrapingImage(u32 width, u32 height, u32* pixels)
{
	GLuint handle;
	glGenTextures(1, &handle);
	glBindTexture(GL_TEXTURE_2D, handle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, pixels);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

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

static void SetUpBasicZBiasProgram()
{

	char *vertexCodeTex =
		R"FOO(
	//Vertex Code

	uniform mat4x4 cameraTransform;
	uniform mat4x4 projection;

	in vec3 vertP;
	in vec4 vertC;
	in vec2 vertUV;

	smooth out vec4 fragColor;
	smooth out vec2 fragCoord;

	void main(void)
	{
		vec4 inputVertex = vec4(vertP, 1.0f);
		gl_Position = projection * cameraTransform * inputVertex;

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
	zBias.cameraTransform = glGetUniformLocation(zBias.program, "cameraTransform");
	zBias.projection = glGetUniformLocation(zBias.program, "projection");
	zBias.depthSampler = glGetUniformLocation(zBias.program, "texSampler");
	zBias.vertP = glGetAttribLocation(zBias.program, "vertP");
	zBias.vertC = glGetAttribLocation(zBias.program, "vertC");
	zBias.vertUV = glGetAttribLocation(zBias.program, "vertUV");
	Assert(zBias.program);
}

static void SetUpBasicProgram()
{

	char *vertexCode =
		R"FOO(
	//Vertex Code

	uniform mat4x4 projection;
	uniform mat4x4 cameraTransform;
	uniform mat4x4 shadowTransform;

	in vec3 vertP;
	in vec4 vertC;

	smooth out vec4 fragColor;
	smooth out vec4 shadowCoord;

	void main(void)
	{
		vec4 inputVertex = vec4(vertP, 1);
		gl_Position = projection * cameraTransform * inputVertex;
		
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
		vec3 shadowCoord3 = shadowCoord2;
		
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
	

	basic.projection = glGetUniformLocation(basic.program, "projection");
	basic.cameraTransform = glGetUniformLocation(basic.program, "cameraTransform");
	basic.depthSampler = glGetUniformLocation(basic.program, "depthTexture");
	basic.shadowTransform = glGetUniformLocation(basic.program, "shadowTransform");
	glUseProgram(0);
}

static void SetUpBasicMeshProgram()
{

	char *vertexCode =
		R"FOO(
	//Vertex Code

	uniform mat4x4 projection;
	uniform mat4x4 cameraTransform;
	uniform mat4x4 shadowTransform;

	uniform vec3 lightPos; // allready transfomed for now, so we do not need a third matrix, that is the transform with out the object Transform
	//uniform vec3 cameraPos;
	uniform float specularExponent;
	uniform vec4 scaleColor;

	in vec3 vertP;
	in vec4 vertC;
	in vec2 vertUV;
	in vec3 vertN;

	smooth out vec4 fragColor;
	smooth out vec4 shadowCoord;
	smooth out vec2 fragCoord;
	smooth out float cosinAttenuation;
	smooth out float specular;

	void main(void)
	{
		//pass through
		fragColor = vertC * scaleColor;
		fragCoord = vertUV;

		vec4 inputVertex = vec4(vertP, 1);
		vec4 vertexInCameraSpace = cameraTransform * inputVertex;
		gl_Position = projection * vertexInCameraSpace;

		// shadow Mapping
		shadowCoord = shadowTransform * inputVertex;
		
		// simple diffuse light
		vec3 point = vertexInCameraSpace.xyz;
		vec3 lightDirection = normalize(lightPos - point);

		vec4 transformedNormal = cameraTransform * vec4(vertN, 0);
		vec3 normal = normalize(transformedNormal.xyz);
		float interpolationC = 0.1;
		cosinAttenuation = (1-interpolationC) * max(dot(lightDirection, normal), 0) + interpolationC;
		//cosinAttenuation = 1.0f;

		// simple specular light
		vec3 incomingLightDir = -lightDirection;
		vec3 reflected = reflect(incomingLightDir, normal); //incomingLightDir - 2.0 * dot(incomingLightDir, normal) * normal;
		vec3 viewing = normalize(-point);
		float specularBase = max(dot(viewing, reflected),0);
		specular = pow(specularBase, specularExponent);
		specular = 0;
	}

	)FOO";
	char *fragmentCode =
		R"FOO(
	//Fragment Code

	out vec4 resultColor;

	smooth in vec4 fragColor;
	smooth in vec4 shadowCoord;
	smooth in vec2 fragCoord;
	smooth in float cosinAttenuation;
	smooth in float specular;

	uniform sampler2D depthTexture;
	uniform sampler2D texture;
	void main(void)
	{
		vec3 projectedShadowCoord = shadowCoord.xyz / shadowCoord.w;
		float visibility = 1.0f;
		
		float distanceLightNearestGeometry = texture2D(depthTexture, projectedShadowCoord.xy).r;
		float distanceFromLight = projectedShadowCoord.z;

		float bias = 0.001;

		if (distanceLightNearestGeometry < distanceFromLight - bias)
		{
			visibility = 0.5f;
		}
		resultColor =  vec4( cosinAttenuation * visibility * fragColor.rgb, fragColor.a) *  texture2D(texture, fragCoord) + vec4(specular);
		//resultColor = texture2D(texture, fragCoord);
	}

	)FOO";


	basicMesh.program = OpenGLCreateProgram(headerCodeOpenGL, vertexCode, fragmentCode);
	Assert(basicMesh.program);
	glUseProgram(basicMesh.program);
	basicMesh.vertP  = glGetAttribLocation(basicMesh.program, "vertP");
	basicMesh.vertC  = glGetAttribLocation(basicMesh.program, "vertC");
	basicMesh.vertUV = glGetAttribLocation(basicMesh.program, "vertUV");
	basicMesh.vertN  = glGetAttribLocation(basicMesh.program, "vertN");

	basicMesh.projection = glGetUniformLocation(basicMesh.program, "projection");
	basicMesh.cameraTransform = glGetUniformLocation(basicMesh.program, "cameraTransform");
	basicMesh.depthSampler = glGetUniformLocation(basicMesh.program, "depthTexture");
	basicMesh.textureSampler = glGetUniformLocation(basicMesh.program, "texture");
	basicMesh.shadowTransform = glGetUniformLocation(basicMesh.program, "shadowTransform");
	basicMesh.lightPos = glGetUniformLocation(basicMesh.program, "lightPos");
	//basicMesh.cameraPos = glGetUniformLocation(basicMesh.program, "cameraPos");
	basicMesh.specularExponent = glGetUniformLocation(basicMesh.program, "specularExponent");
	basicMesh.scaleColor = glGetUniformLocation(basicMesh.program, "scaleColor");

	glUniform1i(basicMesh.depthSampler, 1);
	glUniform1i(basicMesh.textureSampler, 0);

	glUseProgram(0);
}

static void SetUpDepthProgram()
{
	glGenFramebuffers(1, &shadowFrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFrameBuffer);
	
	glGenTextures(1, &shadowTexture);
	glBindTexture(GL_TEXTURE_2D, shadowTexture);
	
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1024, 1024, 0, GL_BGRA_EXT, GL_FLOAT, 0 );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowTexture, 0);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	Assert(status == GL_FRAMEBUFFER_COMPLETE);

	char *vertexCode =
		R"FOO(
	//Vertex Code

	uniform mat4x4 cameraTransform;
	uniform mat4x4 projection;

	in vec3 vertP;
	in vec4 vertC;

	smooth out vec4 color;
	void main(void)
	{
		vec4 inputVertex = vec4(vertP, 1);
		gl_Position = projection * cameraTransform * inputVertex;
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
	
	shadow.cameraTransform = glGetUniformLocation(shadow.program, "cameraTransform");
	shadow.projection = glGetUniformLocation(shadow.program, "projection");

	GLuint error = glGetError();
	Assert(glGetError() == GL_NO_ERROR);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(0);

}

static void WINAPI OpenGLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, void *userParam)
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

static void BeginUseProgram(OpenGLProgram prog, RenderSetup setup, bool textured = false)
{
	glUseProgram(prog.program);

	glUniformMatrix4fv(prog.cameraTransform, 1, GL_TRUE, setup.cameraTransform.a[0]);
	glUniformMatrix4fv(prog.projection, 1, GL_TRUE, setup.projection.a[0]);

	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

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

	defaultInternalTextureFormat = GL_RGBA;
	if (info.GL_EXT_texture_sRGB)
	{
		defaultInternalTextureFormat = GL_SRGB_ALPHA;
	}

	GLuint DummyVertexArray;
	glGenVertexArrays(1, &DummyVertexArray);
	glBindVertexArray(DummyVertexArray);

	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	
	glGenBuffers(1, &elementBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);


	SetUpBasicZBiasProgram();

	SetUpBasicMeshProgram();
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
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture_map);
	
	//glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	//glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, defaultInternalTextureFormat, 1280, 720, GL_FALSE);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1280, 720, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	renderTexture = texture_map;

	glBindTexture(GL_TEXTURE_2D, 0);

	// Build the texture that will serve as the depth attachment for the framebuffer.
	GLuint depth_texture;
	glGenTextures(1, &depth_texture);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, depth_texture);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 1280, 720, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_DEPTH_COMPONENT, 1280, 720, GL_FALSE);
	glBindTexture(GL_TEXTURE_2D, 0);
	renderDepth = depth_texture;

	// Build the framebuffer.
	GLuint framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, texture_map, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, depth_texture, 0);
	renderFrameBuffer = framebuffer;


	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	Assert(status == GL_FRAMEBUFFER_COMPLETE);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	Assert(glGetError() == GL_NO_ERROR);
}

u32 HeaderSize(RenderGroupEntryHeader *header)
{
	switch (header->type)
	{
	case RenderGroup_EntryClear:
	{
		return sizeof(EntryClear);
	}break;
	case RenderGroup_EntryLines:
	{
		return sizeof(EntryColoredVertices);
	}break;
	case RenderGroup_EntryTexturedQuads:
	{
		return sizeof(EntryTexturedQuads);
	}break;
	case RenderGroup_EntryTriangles:
	{
		return sizeof(EntryColoredVertices);
	}break;
	default:
	{
		Die;
		return 1;
	}break;
	}
}

void RenderIntoShadowMap(RenderCommands *rg)
{
	glDisable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);

	glUseProgram(shadow.program);

	glBindFramebuffer(GL_FRAMEBUFFER, shadowFrameBuffer);
	
	glViewport(0, 0, 1024, 1024);  // todo hardcoded. this is the size of the shadow texture
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (u32 pBufferIt = 0; pBufferIt < rg->pushBufferSize;)
	{

		RenderGroupEntryHeader *header = (RenderGroupEntryHeader *)(rg->pushBufferBase + pBufferIt);
		RenderSetup setup = header->setup;
		if (header->type != RenderGroup_EntryTriangleMesh || !(setup.flag & Setup_ShadowMapping))
		{
			pBufferIt += HeaderSize(header);
			continue;
		}

		EntryTriangleMesh *meshHeader = (EntryTriangleMesh *)header;

		Quaternion q = meshHeader->orientation;
		m4x4 qmat = Translate(QuaternionToMatrix(q) * ScaleMatrix(meshHeader->scale), meshHeader->pos);

		//todo: slow. and hardcoded
		m4x4 projection = setup.projection; // think this is right 
		m4x4 shadowMat =  CameraTransform(V3(1, 0, 0), V3(0, 1, 0), V3(0, 0, 1), setup.lightPos);
		m4x4 mat = shadowMat * qmat;

		TriangleMesh mesh = meshHeader->mesh;

		glUniformMatrix4fv(shadow.cameraTransform, 1, GL_TRUE, mat.a[0]);
		glUniformMatrix4fv(shadow.projection, 1, GL_TRUE, projection.a[0]);

		glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexVBO);

		glEnableVertexAttribArray(shadow.vertP);
		glEnableVertexAttribArray(shadow.vertC);

		glVertexAttribPointer(shadow.vertP, 3, GL_FLOAT, false, sizeof(VertexFormat), (void *)OffsetOf(VertexFormat, p));
		glVertexAttribPointer(shadow.vertC, 4, GL_UNSIGNED_BYTE, true, sizeof(VertexFormat), (void *)OffsetOf(VertexFormat, c));

		For(mesh.indexSets)
		{

			switch (mesh.type)
			{
			case TriangleMeshType_List:
			{
				glDrawElements(GL_TRIANGLES, it->amount, GL_UNSIGNED_SHORT, (void *)(u64)(it->offset * sizeof(u16)));
			}break;
			case TrianlgeMeshType_Strip:
			{
				glDrawElements(GL_TRIANGLE_STRIP, it->amount, GL_UNSIGNED_SHORT, (void *)((u64)it->offset * sizeof(u16)));
			}break;
			default:
			{
				Die;
			}break;
			}

		}

		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glDisableVertexAttribArray(shadow.vertP);
		glDisableVertexAttribArray(shadow.vertC);

		pBufferIt += sizeof(*meshHeader);

	}

	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGlRenderGroupToOutput(RenderCommands *rg)
{
	TimedBlock;

	// todo maybe put this in the init.
	glEnable(GL_PRIMITIVE_RESTART_FIXED_INDEX);

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
	
	glUseProgram(basic.program); 
	glUniform1i(basic.depthSampler, 1);

	m4x4 biasMatrix =
	{
		{
			{ 0.5f, 0.0f, 0.0f, 0.5f },
			{ 0.0f, 0.5f, 0.0f, 0.5f },
			{ 0.0f, 0.0f, 0.5f, 0.5f },
			{ 0.0f, 0.0f, 0.0f, 1.0f }
		}
	};
	m4x4 zero =
	{
		{
			{ 0.0f, 0.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f, 0.0f }
		}
	};
	
	glUseProgram(0);
	
	RenderIntoShadowMap(rg);

	glBindFramebuffer(GL_FRAMEBUFFER, renderFrameBuffer);
	glViewport(0, 0, rg->width, rg->height);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	//glCullFace(GL_BACK);
	//glDisable(GL_CULL_FACE);

	for (u32 pBufferIt = 0; pBufferIt < rg->pushBufferSize;)
	{

		RenderGroupEntryHeader *header = (RenderGroupEntryHeader *)(rg->pushBufferBase + pBufferIt);
		RenderSetup setup = header->setup;

		switch (header->type)
		{
		case RenderGroup_EntryTexturedQuads:
		{
			EntryTexturedQuads *quadHeader = (EntryTexturedQuads *)header;
			Bitmap *bitmaps = quadHeader->quadBitmaps;

			BeginUseProgram(zBias, quadHeader->header.setup, true);
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

			BeginUseProgram(basic, trianglesHeader->header.setup);

			if (setup.flag & Setup_ShadowMapping)
			{
				// same as above setup.projection, should always yeald the right projection
				m4x4 shadowMat = biasMatrix * setup.projection * CameraTransform(V3(1, 0, 0), V3(0, 1, 0), V3(0, 0, 1), setup.lightPos);
				glUniformMatrix4fv(basic.shadowTransform, 1, GL_TRUE, shadowMat.a[0]);
			}
			else
			{
				glUniformMatrix4fv(basic.shadowTransform, 1, GL_TRUE, &zero.a[0][0]);
			}
				

			glBufferData(GL_ARRAY_BUFFER, trianglesHeader->vertexCount * sizeof(ColoredVertex), trianglesHeader->data, GL_STREAM_COPY);
			glDrawArrays(GL_TRIANGLES, 0, trianglesHeader->vertexCount);

			EndUseProgram(basic);

			pBufferIt += sizeof(*trianglesHeader);
		}break;
		case RenderGroup_EntryTriangleMesh:
		{
			EntryTriangleMesh *meshHeader = (EntryTriangleMesh *)header;
			
			v3 pos = meshHeader->pos;
			
			Quaternion q = meshHeader->orientation;
			m4x4 qmat = Translate(QuaternionToMatrix(q) * ScaleMatrix(meshHeader->scale), pos);

			//todo: slow.
			m4x4 shadowMat = biasMatrix * setup.projection * CameraTransform(V3(1, 0, 0), V3(0, 1, 0), V3(0, 0, 1), setup.lightPos) * qmat;

			m4x4 mat  = setup.cameraTransform * qmat;
			v4 lightP = setup.cameraTransform * V4(setup.lightPos, 1.0f);

			TriangleMesh mesh = meshHeader->mesh;

 			glUseProgram(basicMesh.program);

			glUniformMatrix4fv(basicMesh.cameraTransform, 1, GL_TRUE, mat.a[0]);
			glUniformMatrix4fv(basicMesh.projection, 1, GL_TRUE, setup.projection.a[0]);
			glUniformMatrix4fv(basicMesh.shadowTransform, 1, GL_TRUE, shadowMat.a[0]);
			glUniform3f(basicMesh.lightPos, lightP.x, lightP.y, lightP.z); 
			glUniform4f(basicMesh.scaleColor, meshHeader->scaleColor.r, meshHeader->scaleColor.g, meshHeader->scaleColor.b, meshHeader->scaleColor.a);

			glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexVBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexVBO);

			glEnableVertexAttribArray(basicMesh.vertP);
			glEnableVertexAttribArray(basicMesh.vertN);
			glEnableVertexAttribArray(basicMesh.vertUV);
			glEnableVertexAttribArray(basicMesh.vertC);

			
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, shadowTexture);
			glActiveTexture(GL_TEXTURE0);
			

			glVertexAttribPointer(basicMesh.vertN, 3, GL_FLOAT, false, sizeof(VertexFormat), (void *)OffsetOf(VertexFormat, n));
			glVertexAttribPointer(basicMesh.vertP, 3, GL_FLOAT, false, sizeof(VertexFormat), (void *)OffsetOf(VertexFormat, p));
			glVertexAttribPointer(basicMesh.vertUV, 2, GL_FLOAT, false, sizeof(VertexFormat), (void *)OffsetOf(VertexFormat, uv));
			glVertexAttribPointer(basicMesh.vertC, 4, GL_UNSIGNED_BYTE, true, sizeof(VertexFormat), (void *)OffsetOf(VertexFormat, c));

			for(u32 i = 0; i < mesh.indexSets.amount; i++)
			{
				auto it = &mesh.indexSets[i];
				glUniform1f(basicMesh.specularExponent, it->mat.spectularExponent);
				glBindTexture(GL_TEXTURE_2D, meshHeader->textureIDs[i]);

				switch (mesh.type)
				{
				case TriangleMeshType_List:
				{
					glDrawElements(GL_TRIANGLES, it->amount, GL_UNSIGNED_SHORT, (void *) ((u64)it->offset * sizeof(u16)));
				}break;
				case TrianlgeMeshType_Strip:
				{
					glDrawElements(GL_TRIANGLE_STRIP, it->amount, GL_UNSIGNED_SHORT, (void *)((u64)it->offset * sizeof(u16)));
				}break;
				default:
				{
					Die;
				}break;
				}
			}

			glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
			glDisableVertexAttribArray(basicMesh.vertN);
			glDisableVertexAttribArray(basicMesh.vertP);
			glDisableVertexAttribArray(basicMesh.vertUV);
			glDisableVertexAttribArray(basicMesh.vertC);

			//glActiveTexture(GL_TEXTURE0);

			glUseProgram(0);
			pBufferIt += sizeof(*meshHeader);
		}break;
		case RenderGroup_EntryClear: // this can probably go.
		{
			EntryClear *clearHeader = (EntryClear *)header;
			glClearColor(clearHeader->color.r, clearHeader->color.g, clearHeader->color.b, clearHeader->color.a);
			glClear(GL_COLOR_BUFFER_BIT);

			pBufferIt += sizeof(*clearHeader);
		}break;
		case RenderGroup_EntryLines:
		{
			EntryColoredVertices *lineHeader = (EntryColoredVertices*)header;

			BeginUseProgram(basic, lineHeader->header.setup);
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

	glBindFramebuffer(GL_READ_FRAMEBUFFER, renderFrameBuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	glBlitFramebuffer(0, 0, rg->width, rg->height, 0, 0, rg->width, rg->height, GL_COLOR_BUFFER_BIT, GL_NEAREST);


	//flush
	rg->pushBufferSize = 0;
	
	GLuint err12or = glGetError();
	Assert(!err12or);


}

#endif
