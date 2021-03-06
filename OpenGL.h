#ifndef RR_OPENGL
#define RR_OPENGL

// GL Version 1.5
#define GL_ARRAY_BUFFER                   0x8892
#define GL_ELEMENT_ARRAY_BUFFER           0x8893


// GL Version 1.2
#define GL_CLAMP_TO_EDGE                  0x812F

// GL Version 1.3
#define GL_CLAMP_TO_BORDER                0x812D
#define GL_MULTISAMPLE                    0x809D
#define GL_SAMPLE_ALPHA_TO_COVERAGE       0x809E
#define GL_SAMPLE_ALPHA_TO_ONE            0x809F
#define GL_SAMPLE_COVERAGE                0x80A0
#define GL_SAMPLE_BUFFERS                 0x80A8
#define GL_SAMPLES                        0x80A9
#define GL_SAMPLE_COVERAGE_VALUE          0x80AA
#define GL_SAMPLE_COVERAGE_INVERT         0x80AB
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

// GL Version 1.4
#define GL_DEPTH_COMPONENT16              0x81A5
#define GL_DEPTH_COMPONENT24              0x81A6
#define GL_DEPTH_COMPONENT32              0x81A7
#define GL_FUNC_ADD                       0x8006

// GL Version 1.5
#define GL_STREAM_DRAW                    0x88E0
#define GL_STREAM_READ                    0x88E1
#define GL_STREAM_COPY                    0x88E2


// GL Version 2.0
#define GL_SHADING_LANGUAGE_VERSION       0x8B8C
#define GL_VERTEX_SHADER                  0x8B31
#define GL_FRAGMENT_SHADER				0x8B30
#define GL_VALIDATE_STATUS                0x8B83
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82


// GL Version 2.1
#define GL_SRGB                           0x8C40
#define GL_SRGB8                          0x8C41
#define GL_SRGB_ALPHA                     0x8C42
#define GL_SRGB8_ALPHA8                   0x8C43

// GL Version 3.0
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT         0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT 0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER        0x8CDB
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER        0x8CDC
#define GL_FRAMEBUFFER_UNSUPPORTED                   0x8CDD
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE        0x8D56
#define GL_INVALID_FRAMEBUFFER_OPERATION  0x0506
#define GL_FRAMEBUFFER_UNDEFINED          0x8219
#define GL_FRAMEBUFFER_COMPLETE           0x8CD5
#define GL_READ_FRAMEBUFFER               0x8CA8
#define GL_DRAW_FRAMEBUFFER               0x8CA9
#define GL_FRAMEBUFFER                    0x8D40
#define GL_FRAMEBUFFER_SRGB               0x8DB9
#define GL_MAJOR_VERSION                  0x821B // use these?
#define GL_MINOR_VERSION                  0x821C // use these?
#define GL_NUM_EXTENSIONS                 0x821D
#define GL_MAX_SAMPLES                    0x8D57
#define GL_DEPTH_ATTACHMENT               0x8D00
#define GL_COLOR_ATTACHMENT0              0x8CE0
#define GL_DEPTH_COMPONENT32F             0x8CAC
#define GL_TEXTURE_2D_ARRAY               0x8C1A

// GL Version 3.1
#define GL_PRIMITIVE_RESTART_FIXED_INDEX  0x8D69


// GL Version 3.2
#define GL_TEXTURE_2D_MULTISAMPLE         0x9100
#define GL_GEOMETRY_SHADER                0x8DD9

// GL Version 4.3
#define GL_DEBUG_OUTPUT_SYNCHRONOUS       0x8242
#define GL_DEBUG_SEVERITY_HIGH            0x9146
#define GL_DEBUG_SEVERITY_MEDIUM          0x9147
#define GL_DEBUG_SEVERITY_LOW             0x9148

// windows
#define WGL_CONTEXT_MAJOR_VERSION_ARB           0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB           0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB             0x2093
#define WGL_CONTEXT_FLAGS_ARB                   0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB            0x9126
#define WGL_CONTEXT_DEBUG_BIT_ARB               0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB  0x0002
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002



typedef char GLchar;
typedef ptrdiff_t GLsizeiptr;
typedef void (APIENTRY *GLDEBUGPROC)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, void *userParam);

// GL Version 1.1
typedef void WINAPI glTexSubImage3D_(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid * data);
typedef void WINAPI glTexImage3D_(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid * data);

// GL Version 1.3
typedef void WINAPI glActiveTexture_(GLenum texture);
typedef void WINAPI glBlendEquation_(GLenum mode);

// GL Version 1.5
typedef void WINAPI glGenBuffers_(GLsizei n, GLuint *buffers);
typedef void WINAPI glBindBuffer_(GLenum target, GLuint buffer);
typedef void WINAPI glBufferData_(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage);

// GL Version 2.0
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
typedef void WINAPI glVertexAttrib3f_(GLuint index, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void WINAPI glUniform1i_(GLint location, GLint v0);
typedef void WINAPI glBindAttribLocation_(GLuint program, GLuint index, const GLchar *name);
typedef void WINAPI glUniform4fv_(GLint location, GLsizei count, const GLfloat *value);
typedef void WINAPI glUniform1iv_(GLint location, GLsizei count, const GLint *value);
typedef void WINAPI glUniformMatrix4fv_(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void WINAPI glUniform1f_(GLint location, GLfloat v0);
typedef void WINAPI glUniform2f_(GLint location, GLfloat v0, GLfloat v1);
typedef void WINAPI glUniform3f_(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void WINAPI glUniform4f_(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);




// GL Version 3.0
typedef const GLubyte* WINAPI glGetStringi_(GLenum name, GLuint index);
typedef void WINAPI glVertexAttribIPointer_(GLuint index, GLint size, GLenum type,GLsizei stride,const GLvoid *pointer);
typedef void WINAPI glGenFramebuffers_(GLsizei n, GLuint *ids); 
typedef void WINAPI glBindFramebuffer_(GLenum target, GLuint framebuffer);
typedef GLenum WINAPI glCheckFramebufferStatus_(GLenum target);
typedef void WINAPI glFramebufferTexture2D_(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void WINAPI glBlitFramebuffer_(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
typedef void WINAPI glGenVertexArrays_(GLsizei n, GLuint *arrays);
typedef void WINAPI glBindVertexArray_(GLuint arrayName);

// GL Version 3.2
typedef void WINAPI glTexImage2DMultisample_(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
typedef void WINAPI glFramebufferTexture_(GLenum target, GLenum attachment, GLuint texture, GLint level);


// GL Version 4.3
typedef void WINAPI glDebugMessageCallback_(GLDEBUGPROC callback, void * userParam);



static glBlendEquation_ *glBlendEquation;
static glTexImage3D_ *glTexImage3D;
static glTexSubImage3D_ *glTexSubImage3D;
static glGetStringi_ *glGetStringi;
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
static glVertexAttribIPointer_ *glVertexAttribIPointer;
static glVertexAttribPointer_ *glVertexAttribPointer;
static glEnableVertexAttribArray_ *glEnableVertexAttribArray;
static glDisableVertexAttribArray_ *glDisableVertexAttribArray;
static glGetAttribLocation_ *glGetAttribLocation;

struct OpenGLInfo
{
	bool modernContext;
   
	bool  GL_EXT_texture_sRGB;
   
	//char *extensions; this crap is now an array?
	char *renderer;
	char *version;
	char *shadingLanguageVersion;
	char *vendor;
   
	GLint amountOfMultiSamples;
   
};

enum SamplerLocations
{
   Sampler_Texture,
   Sampler_Depth,
   
};

struct OpenGLProgram
{
	b32 compiled;
   
   GLuint program;
   
   u32 flags;
   
   //uniforms
   GLuint projection;
	GLuint cameraTransform;
   GLuint objectTransform;
	GLuint scaleColor;
   GLuint shadowTransform;
   GLuint boneStates;
   GLuint ka;
   GLuint kd;
   GLuint ks;
   GLuint lightP;
   GLuint cameraP;
   
	GLuint specularExponent;
   
	GLuint depthSampler;
	GLuint textureSampler;
   
   // this is either a uniform or an attribute, depending on 'MulitTextured'
   GLuint textureIndex;
   
	//attributes
	GLuint vertP;
	GLuint vertC;
	GLuint vertUV;
	GLuint vertN;
   GLuint boneIndices;
   GLuint boneWeights;
};


struct FrameBuffer
{
   GLuint id;
   GLuint texture;
   GLuint depth;
   
   u32 width;
   u32 height;
};


struct BlitProgram
{
   GLuint program;
   GLuint vertP;
   GLuint vertUV;
   GLuint textureSampler;
   
};

struct OpenGLContext
{
   OpenGLInfo info;
   GLuint vertexBuffer; 
   GLuint indexBuffer; 
   GLuint defaultInternalTextureFormat;
   
   File shaderFile;
   
   FrameBuffer renderBuffer;
   FrameBuffer shadowBuffer;
   FrameBuffer orthogonalBuffer;
   
   BlitProgram blitProgram;
   
   OpenGLProgram shaders[ShaderArray_Size];
};

// globals
static GLuint globalTextureArray;

static bool GLCStringEquals(char *first, u64 firstSize, char* second)
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

// not sure how to handle this whole situation todo
static OpenGLInfo OpenGLGetInfo(bool modernContext)
{
	OpenGLInfo info = {};
   
	info.modernContext = modernContext;
   info.renderer = (char *)glGetString(GL_RENDERER);
   
   
	if (modernContext)
	{
		info.shadingLanguageVersion = (char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
	}
   
	info.vendor = (char *)glGetString(GL_VENDOR);
   
	info.version = (char *)glGetString(GL_VERSION);
   
	glGetIntegerv(GL_MAX_SAMPLES, &(info.amountOfMultiSamples));
   
   GLint amountOfExtensions = 0;
   glGetIntegerv(GL_NUM_EXTENSIONS, &amountOfExtensions);
   
   if(amountOfExtensions < 0) amountOfExtensions = 0;
   
   for(u32 i = 0; i < (u32)amountOfExtensions; i++)
   {
      char *extension = (char *)glGetStringi(GL_EXTENSIONS, i);
      if (CStringsAreEqual(extension, "GL_EXT_texture_sRGB")) 
      { 
         info.GL_EXT_texture_sRGB = true; 
      }
	}
   
   
   {
      GLuint err = glGetError();
      Assert(err == GL_NO_ERROR);
   }
	
	return info;
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
		//Assert(!"OpenGL Error encountered");
      
      fprintf(stderr, "OPENGL:\n");
      fprintf(stderr, ErrorMessage);
      fprintf(stderr, "\n");
      
      fflush(stderr);
      
      ExitProcess(0);
      //running = false;
	}
}

static BlitProgram CompileBlitProgram()
{
   BlitProgram ret;
   File file = LoadFile("src/simple.glsl", frameArena);
   
   char *shaderCode = (char *)file.data;
   
   GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
   GLchar *vertexShaderCode[] =
   {
      "#version 150\n", "#define VertexCode \n", shaderCode
   };
   glShaderSource(vertexShaderID, ArrayCount(vertexShaderCode), vertexShaderCode, 0);
   glCompileShader(vertexShaderID);
   
   GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
   GLchar *fragmentShaderCode[] =
   {
      "#version 150\n", "#define FragmentCode \n", shaderCode
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
   
   {
      GLuint err = glGetError();
      Assert(err == GL_NO_ERROR);
   }
   
   glUseProgram(programID);
   
   ret.program = programID;
   ret.vertP  = glGetAttribLocation(ret.program, "vertP");
   ret.vertUV  = glGetAttribLocation(ret.program, "vertUV");
   
   ret.textureSampler  = glGetUniformLocation(programID, "textureSampler");
   glUniform1i(ret.textureSampler, Sampler_Texture);
   
   glUseProgram(0);
   
   
   return ret;
}

static OpenGLProgram OpenGLMakeProgram(char *shaderCode, u32 flags)
{
   String defines = PushArray(frameArena, Char, 0);
   
   {
      GLuint err = glGetError();
      Assert(err == GL_NO_ERROR);
   }
   
   S("#version 150 \n", frameArena);
   
   if(flags & ShaderFlags_Phong)
   {
      S("#define Phong \n", frameArena);
   }
   if(flags & ShaderFlags_Textured)
   {
      S("#define Textured \n", frameArena);
   }
   if(flags & ShaderFlags_ShadowMapping)
   {
      S("#define ShadowMapping \n", frameArena);
   }
   if(flags & ShaderFlags_Animated)
   {
      S("#define Animated \n", frameArena);
   }
   if(flags & ShaderFlags_ZBias)
   {
      S("#define ZBias \n", frameArena);
   }
   if(flags & ShaderFlags_MultiTextured)
   {
      S("#define MultiTextured \n", frameArena);
   }
   *PushStruct(frameArena, Char) = '\0';
   
   EndArray(frameArena, Char, defines);
   
   GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
   GLchar *vertexShaderCode[] =
   {
      defines.cstr, "#define VertexCode \n", shaderCode
   };
   glShaderSource(vertexShaderID, ArrayCount(vertexShaderCode), vertexShaderCode, 0);
   glCompileShader(vertexShaderID);
   
#define GEOM 1
#if GEOM
   GLuint geometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);
   GLchar *geometryShaderCode[] =
   {
      defines.cstr, "#define GeometryCode \n", shaderCode
   };
   glShaderSource(geometryShaderID, ArrayCount(geometryShaderCode), geometryShaderCode, 0);
   glCompileShader(geometryShaderID);
#endif
   
   GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
   GLchar *fragmentShaderCode[] =
   {
      defines.cstr, "#define FragmentCode \n", shaderCode
   };
   glShaderSource(fragmentShaderID, ArrayCount(fragmentShaderCode), fragmentShaderCode, 0);
   glCompileShader(fragmentShaderID);
   
   GLuint programID = glCreateProgram();
   
   glAttachShader(programID, vertexShaderID);
#if GEOM
   glAttachShader(programID, geometryShaderID);
#endif
   glAttachShader(programID, fragmentShaderID);
   
   glLinkProgram(programID);
   
   glValidateProgram(programID);
   GLint linkValidated = false;
   glGetProgramiv(programID, GL_LINK_STATUS, &linkValidated);
   //glGetProgramiv(programID, GL_COMPILE_STATUS, &compileValidated);
   
   
   {
      GLuint err = glGetError();
      Assert(err == GL_NO_ERROR);
   }
   
   
   GLint validated = linkValidated;
   if (!validated)
   {
      GLsizei length;
      char programError[4096];
      char vertexError[4096];
#if GEOM
      char geomError[4096];
#endif
      char fragmentError[4096];
      
      glGetProgramInfoLog(programID, sizeof(programError), &length, programError);
      glGetShaderInfoLog(vertexShaderID, sizeof(vertexError), &length, vertexError);
#if GEOM
      glGetShaderInfoLog(geometryShaderID, sizeof(geomError), &length, geomError);
#endif
      glGetShaderInfoLog(fragmentShaderID, sizeof(fragmentError), &length, fragmentError);
      
      
      fprintf(stderr, "OpenGL shader error:\n");
      fprintf(stderr, "Program error:\n");
      fprintf(stderr, programError);
      fprintf(stderr, "Vertex error:\n");
      fprintf(stderr, vertexError);
      
      // todo here goem error
      
      fprintf(stderr, "Fragment error:\n");
      fprintf(stderr, fragmentError);
      fprintf(stderr, "\n");
      
      fflush(stderr);
      
      ExitProcess(0);
      
      Assert(!"ShaderError");
   }
   
   Assert(programID);
   
   OpenGLProgram ret;
   ret.compiled = true;
   
   ret.flags = flags;
   ret.program = programID;
   
   glUseProgram(programID);
   ret.vertP  = glGetAttribLocation(ret.program, "vertP");
   ret.vertC  = glGetAttribLocation(ret.program, "vertC");
   ret.vertUV = glGetAttribLocation(ret.program, "vertUV");
   ret.vertN  = glGetAttribLocation(ret.program, "vertN");
   ret.boneIndices = glGetAttribLocation(ret.program, "boneIndices");
   ret.boneWeights = glGetAttribLocation(ret.program, "boneWeights");
   
   ret.projection      = glGetUniformLocation(ret.program, "projection");
   ret.cameraTransform = glGetUniformLocation(ret.program, "cameraTransform");
   ret.objectTransform = glGetUniformLocation(ret.program, "objectTransform");
   ret.depthSampler    = glGetUniformLocation(ret.program, "depthTexture");
   ret.textureSampler  = glGetUniformLocation(ret.program, "textureSampler");
   ret.shadowTransform = glGetUniformLocation(ret.program, "shadowTransform");
   
   ret.lightP           = glGetUniformLocation(ret.program, "lightPos");
   ret.specularExponent = glGetUniformLocation(ret.program, "specularExponent");
   ret.ka               = glGetUniformLocation(ret.program, "ka");
   ret.kd               = glGetUniformLocation(ret.program, "kd");
   ret.ks               = glGetUniformLocation(ret.program, "ks");
   ret.cameraP          = glGetUniformLocation(ret.program, "cameraPos");
   
   ret.textureIndex = (flags & ShaderFlags_MultiTextured) ? glGetAttribLocation(ret.program, "textureIndex") : glGetUniformLocation(ret.program, "textureIndex");
   
   ret.scaleColor = glGetUniformLocation(ret.program, "scaleColor");
   ret.boneStates = glGetUniformLocation(ret.program, "boneStates");
   
   glUniform1i(ret.textureSampler, Sampler_Texture);
   glUniform1i(ret.depthSampler,   Sampler_Depth);
   
   {
      GLuint err = glGetError();
      Assert(err == GL_NO_ERROR);
   }
   
   
   return ret;
}

static void RegisterTriangleMesh(TriangleMesh *mesh)
{
   void *data = NULL;
   u32 stride = 0; 
   
   Arena *arena = globalDebugState.arena;
   
   // todo maybe make flags
   if(mesh->skeleton.bones.amount)
   {
      stride = sizeof(VertexFormatPCUNBD);
      v3Array colors = PushArray(frameArena, v3, mesh->skeleton.bones.amount);
      
      RandomSeries series = GetRandomSeries();
      
      For(colors)
      {
         it->r = (u32)(RandomU32(&series) & 0xFF) / 255.0f;
         it->g = (u32)(RandomU32(&series) & 0xFF) / 255.0f;
         it->b = (u32)(RandomU32(&series) & 0xFF) / 255.0f;
      }
      
      VertexFormatPCUNBD *packedData = PushData(arena, VertexFormatPCUNBD, 0);
      
      for(u32 i = 0; i < mesh->positions.amount; i++)
      {
         VertexFormatPCUNBD *v =  PushStruct(arena, VertexFormatPCUNBD);
         v->p  = mesh->positions[i];
         v->uv = mesh->uvs[i];
         v->n  = mesh->normals[i];
         v->c  = mesh->colors[i];
         
         u32 unflattendIndex = mesh->skeleton.vertexMap[i];
         WeightDataArray weights = mesh->skeleton.vertexToWeightsMap[unflattendIndex];
         
         switch(weights.amount)
         {
            case 0:
            {
               v->bw = V4();
               v->bi = V4i();
            }break;
            case 1:
            {
               v->bw = V4 (weights[0].weight,    0.0f, 0.0f, 0.0f);
               v->bi = V4i(weights[0].boneIndex, 0,    0,    0);
            }break;
            case 2:
            {
               v->bw = V4 (weights[0].weight,    weights[1].weight,    0.0f, 0.0f);
               v->bi = V4i(weights[0].boneIndex, weights[1].boneIndex, 0,    0);
            }break;
            case 3:
            {
               v->bw = V4 (weights[0].weight,    weights[1].weight,    weights[2].weight,    0.0f);
               v->bi = V4i(weights[0].boneIndex, weights[1].boneIndex, weights[2].boneIndex, 0);
            }break;
            default:
            {
               // todo I think the weights are presorted.... check this, otherwise do it in the parsing
               v->bw = V4(weights[0].weight, weights[1].weight, weights[2].weight, weights[3].weight);
               v->bw /= SumV4(v->bw);
               v->bi = V4i(weights[0].boneIndex, weights[1].boneIndex, weights[2].boneIndex, weights[3].boneIndex);
            }break;
            
            Assert(SumV4(v->bw) == 1.0f);
         }
         
      }
      
      data = packedData;
   }
   else
   {
      stride = sizeof(VertexFormatPCUN);
      
      VertexFormatPCUN *packedData = PushData(frameArena, VertexFormatPCUN, 0);
      
      for(u32 i = 0; i < mesh->positions.amount; i++)
      {
         VertexFormatPCUN *v =  PushStruct(frameArena, VertexFormatPCUN);
         v->p = mesh->positions[i];
         v->c = mesh->colors[i];
         v->uv = mesh->uvs[i];
         v->n = mesh->normals[i];
      }
      
      data = packedData;
   }
   
   mesh->vertexFormatSize = stride;
   
   Assert(mesh->positions.amount < 65536); // <, because reset index
   
   //todo look up what those GL_Stream_Draw mean?
   glGenBuffers(1, &mesh->vertexVBO);
   glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexVBO);
   glBufferData(GL_ARRAY_BUFFER, mesh->positions.amount * stride, data, GL_STREAM_DRAW);
   
   glGenBuffers(1, &mesh->indexVBO);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexVBO);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indices.amount * sizeof(mesh->indices[0]), mesh->indices.data, GL_STREAM_DRAW);
   
}

static void UpdateWrapingTexture(TextureIndex textureIndex, u32 width, u32 height, u32 *pixels)
{
#if 1
   glBindTexture(GL_TEXTURE_2D_ARRAY, globalTextureArray);
   glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, textureIndex.index, width, height, 1, GL_BGRA_EXT, GL_UNSIGNED_BYTE, pixels);
#else
   glBindTexture(GL_TEXTURE_2D, (GLuint)bitmap.textureHandle);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, bitmap.width, bitmap.height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, bitmap.pixels);
   
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
#endif
   
   glBindTexture(GL_TEXTURE_2D, 0);
}

struct OpenGLUniformInfo 
{
   GLuint vertexBuffer = 0xFFFFFFFF;
   GLuint indexBuffer  = 0xFFFFFFFF;
   m4x4 objectTransform = Identity();
   v4 scaleColor = V4(1.0f, 1.0f, 1.0f, 1.0f);
   m4x4Array boneStates = {};
   u32 textureIndex;
};

static void BeginUseProgram(OpenGLContext *context, OpenGLProgram *prog, RenderSetup setup)
{
   glUseProgram(prog->program);
   
   // this can be in the setup of the programs
   glBindTexture(GL_TEXTURE_2D_ARRAY, globalTextureArray);
   glUniformMatrix4fv(prog->projection, 1, GL_TRUE, setup.projection.a[0]);
   
   glUniformMatrix4fv(prog->cameraTransform, 1, GL_TRUE, setup.cameraTransform.a[0]);
   
   if (prog->flags & ShaderFlags_ShadowMapping)
   {
      glUniformMatrix4fv(prog->shadowTransform, 1, GL_TRUE, setup.shadowMat.a[0]);
      
      // this as well
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, context->shadowBuffer.depth);
      glActiveTexture(GL_TEXTURE0);
   }
   
   if(prog->flags & ShaderFlags_Phong)
   {
      v3 lightP = setup.lightP;
      glUniform3f(prog->lightP, lightP.x, lightP.y, lightP.z);
      v3 camP = setup.cameraP;
      glUniform3f(prog->cameraP, camP.x, camP.y, camP.z);
   }
   
}

static void SetupUniforms(OpenGLProgram *prog, OpenGLUniformInfo uniforms)
{
   glUniform4f(prog->scaleColor, uniforms.scaleColor.r, uniforms.scaleColor.g, uniforms.scaleColor.b, uniforms.scaleColor.a); // rgba?
   
   glUniformMatrix4fv(prog->objectTransform, 1, GL_TRUE, uniforms.objectTransform.a[0]);
   
   if(prog->flags & ShaderFlags_Animated)
   {
      glUniformMatrix4fv(prog->boneStates, uniforms.boneStates.amount, GL_TRUE, (GLfloat *)uniforms.boneStates.data);
   }
   
   if(prog->flags & ShaderFlags_Textured)
   {
      glUniform1i(prog->textureIndex, uniforms.textureIndex);
   }
   
   glBindBuffer(GL_ARRAY_BUFFER, uniforms.vertexBuffer);
   if(uniforms.indexBuffer != 0xFFFFFFFF) 
   {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, uniforms.indexBuffer);
   }
}

static void BeginAttribArrays(OpenGLProgram *prog, u32 vertexFormatSize)
{
   glEnableVertexAttribArray(prog->vertP);
   glEnableVertexAttribArray(prog->vertC);
   
   glVertexAttribPointer(prog->vertP, 3, GL_FLOAT, false, vertexFormatSize, (void *)OffsetOf(VertexFormatTemplate, p));
   glVertexAttribPointer(prog->vertC, 4, GL_UNSIGNED_BYTE, true, vertexFormatSize, (void *)OffsetOf(VertexFormatTemplate, c));
   
   if(prog->flags & ShaderFlags_Textured)
   {
      glEnableVertexAttribArray(prog->vertUV);
      glVertexAttribPointer(prog->vertUV, 2, GL_FLOAT, false, vertexFormatSize, (void *)OffsetOf(VertexFormatTemplate, uv));
   }
   
   if(prog->flags & ShaderFlags_MultiTextured)
   {
      Assert(!(prog->flags & ShaderFlags_Textured));
      glEnableVertexAttribArray(prog->vertUV);
      glVertexAttribPointer(prog->vertUV, 2, GL_FLOAT, false, vertexFormatSize, (void *)OffsetOf(VertexFormatTemplate, uv));
      glEnableVertexAttribArray(prog->textureIndex);
      glVertexAttribIPointer(prog->textureIndex, 1, GL_UNSIGNED_SHORT, vertexFormatSize, (void *)OffsetOf(VertexFormatTemplate, textureIndex));
   }
   
   if(prog->flags & ShaderFlags_Phong)
   {
      glEnableVertexAttribArray(prog->vertN);
      glVertexAttribPointer(prog->vertN, 3, GL_FLOAT, false, vertexFormatSize, (void *)OffsetOf(VertexFormatTemplate, n));
   }
   
   if(prog->flags & ShaderFlags_Animated)
   {
      glEnableVertexAttribArray(prog->boneIndices);
      glVertexAttribIPointer(prog->boneIndices, 4, GL_INT, vertexFormatSize, (void *)OffsetOf(VertexFormatTemplate, bi));
      
      glEnableVertexAttribArray(prog->boneWeights);
      glVertexAttribPointer(prog->boneWeights, 4, GL_FLOAT, false, vertexFormatSize, (void *)OffsetOf(VertexFormatTemplate, bw));
   }
}

static void EndAttribArrays(OpenGLProgram *prog)
{
   glDisableVertexAttribArray(prog->vertP);
   glDisableVertexAttribArray(prog->vertC);
   
   if(prog->flags & ShaderFlags_Textured)
   {
      glDisableVertexAttribArray(prog->vertUV);
   }
   if(prog->flags & ShaderFlags_Phong)
   {
      glDisableVertexAttribArray(prog->vertN);
   }
   if(prog->flags & ShaderFlags_Animated)
   {
      glDisableVertexAttribArray(prog->boneIndices);
      glDisableVertexAttribArray(prog->boneWeights);
   }
   
   if(prog->flags & ShaderFlags_MultiTextured)
   {
      glDisableVertexAttribArray(prog->vertUV);
      glDisableVertexAttribArray(prog->textureIndex);
   }
   
}


static OpenGLContext OpenGLInit(bool modernContext)
{
   //GLuint reservedBltTexture;
   //glGenTextures(1, &reservedBltTexture);
   
   OpenGLContext ret = {};
   
   OpenGLInfo info = OpenGLGetInfo(modernContext);
   
   ret.info = info;
   
   if (glDebugMessageCallback)
   {
      glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
      glDebugMessageCallback(OpenGLDebugCallback, 0);
   }
   
   ret.defaultInternalTextureFormat = GL_RGBA;
   if (info.GL_EXT_texture_sRGB)
   {
      ret.defaultInternalTextureFormat = GL_SRGB_ALPHA;
   }
   
   GLuint DummyVertexArray;
   glGenVertexArrays(1, &DummyVertexArray);
   glBindVertexArray(DummyVertexArray);
   
   glGenBuffers(1, &ret.vertexBuffer);
   glBindBuffer(GL_ARRAY_BUFFER, ret.vertexBuffer);
   glGenBuffers(1, &ret.indexBuffer);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ret.indexBuffer);
   
   ret.shaderFile = LoadFile("src/ubershader.glsl", globalAlloc);
   
   ret.blitProgram = CompileBlitProgram();
   
   u32 amountOfMultiSamples = Min(info.amountOfMultiSamples, 4u);
   
   FrameBuffer renderBuffer;
   // todo make this passed in
   renderBuffer.width  = 1280;
   renderBuffer.height = 720;
   
#if 0 // to work around renderdoc not working...
   {
      GLuint texture_map;
      glGenTextures(1, &texture_map);
      glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture_map);
      
      glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, amountOfMultiSamples, ret.defaultInternalTextureFormat, renderBuffer.width, renderBuffer.height, GL_FALSE);
      
      renderBuffer.texture = texture_map;
      glBindTexture(GL_TEXTURE_2D, 0);
      
      
      GLuint depth_texture;
      glGenTextures(1, &depth_texture);
      glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, depth_texture);
      glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, amountOfMultiSamples, GL_DEPTH_COMPONENT, renderBuffer.width, renderBuffer.height, GL_FALSE);
      
      
      renderBuffer.depth = depth_texture;
      
      // Build the framebuffer.
      GLuint framebuffer;
      glGenFramebuffers(1, &framebuffer);
      glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)framebuffer);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, texture_map, 0);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, depth_texture, 0);
      renderBuffer.id = framebuffer;
   }
   
#else // not multisampled
   {
      // renderBuffer
      GLuint texture_map;
      glGenTextures(1, &texture_map);
      glBindTexture(GL_TEXTURE_2D, texture_map);
      
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
      
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, renderBuffer.width, renderBuffer.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
      
      renderBuffer.texture = texture_map;
      glBindTexture(GL_TEXTURE_2D, 0);
      
      GLuint depth_texture;
      glGenTextures(1, &depth_texture);
      glBindTexture(GL_TEXTURE_2D, depth_texture);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, renderBuffer.width, renderBuffer.height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
      
      renderBuffer.depth = depth_texture;
      
      // Build the framebuffer.
      GLuint framebuffer;
      glGenFramebuffers(1, &framebuffer);
      glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)framebuffer);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_map, 0);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture, 0);
      renderBuffer.id = framebuffer;
   }
   
#endif
   
   ret.renderBuffer = renderBuffer;
   
   {
      GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
      Assert(status == GL_FRAMEBUFFER_COMPLETE);
   }
   
   
   FrameBuffer orthogonalBuffer;
   // todo make this passed in
   orthogonalBuffer.width  = 1280;
   orthogonalBuffer.height = 720;
   {
      // orthogonalBuffer
      GLuint texture_map;
      glGenTextures(1, &texture_map);
      glBindTexture(GL_TEXTURE_2D, texture_map);
      
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
      
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, orthogonalBuffer.width, orthogonalBuffer.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
      
      orthogonalBuffer.texture = texture_map;
      glBindTexture(GL_TEXTURE_2D, 0);
      
      GLuint depth_texture;
      glGenTextures(1, &depth_texture);
      glBindTexture(GL_TEXTURE_2D, depth_texture);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, orthogonalBuffer.width, orthogonalBuffer.height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
      
      orthogonalBuffer.depth = depth_texture;
      
      // Build the framebuffer.
      GLuint framebuffer;
      glGenFramebuffers(1, &framebuffer);
      glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)framebuffer);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_map, 0);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture, 0);
      orthogonalBuffer.id = framebuffer;
   }
   
   ret.orthogonalBuffer = orthogonalBuffer;
   
   {
      GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
      Assert(status == GL_FRAMEBUFFER_COMPLETE);
   }
   
   
   FrameBuffer shadowBuffer;
   {
      GLuint shadowFrameBuffer;
      shadowBuffer.width  = 1024;
      shadowBuffer.height = 1024;
      
      
      glGenFramebuffers(1, &shadowFrameBuffer);
      glBindFramebuffer(GL_FRAMEBUFFER, shadowFrameBuffer);
      
      GLuint shadowTexture;
      glGenTextures(1, &shadowTexture);
      glBindTexture(GL_TEXTURE_2D, shadowTexture);
      
      //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1024, 1024, 0, GL_BGRA_EXT, GL_FLOAT, 0 );
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, shadowBuffer.width, shadowBuffer.height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
      
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowTexture, 0);
      
      shadowBuffer.id      = shadowFrameBuffer;
      shadowBuffer.depth   = shadowTexture;
      shadowBuffer.texture = 0xFFFFFFFF;
      
   }
   ret.shadowBuffer = shadowBuffer;
   
   {
      GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
      Assert(status == GL_FRAMEBUFFER_COMPLETE);
   }
   
   
   glBindTexture(GL_TEXTURE_2D, 0);
   glBindFramebuffer(GL_FRAMEBUFFER, 0);
   
   
   {   // Build the textureArray
      glGenTextures(1, &globalTextureArray);
      glBindTexture(GL_TEXTURE_2D_ARRAY, globalTextureArray);
      glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, ret.defaultInternalTextureFormat, Asset_Bitmap_Size, Asset_Bitmap_Size, Asset_Texture_Amount, 0, GL_BGRA_EXT, GL_UNSIGNED_INT, 0);
      
      glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   }
   
   
   //glEnable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
   glDepthMask(GL_TRUE);
   glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
   glDepthFunc(GL_LEQUAL);
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
   glEnable(GL_SAMPLE_ALPHA_TO_ONE);
   glEnable(GL_MULTISAMPLE);
   glEnable(GL_FRAMEBUFFER_SRGB); // not sure....
   glDisable(GL_CULL_FACE);
   
   glEnable(GL_BLEND);
   glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
   
   
   {
      GLuint err = glGetError();
      Assert(err == GL_NO_ERROR);
   }
   
   return ret;
}

static u32 HeaderSize(RenderGroupEntryHeader *header)
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

static OpenGLProgram *GetOpenGLProgram(OpenGLContext *context, u32 shaderFlags)
{
   Assert(shaderFlags < ArrayCount(context->shaders));
   
   OpenGLProgram *ret = context->shaders + shaderFlags;
   if(!ret->compiled)
   {
      *ret = OpenGLMakeProgram((char *)context->shaderFile.data, shaderFlags);
   }
   
   return ret;
}

static void RenderIntoShadowMap(RenderCommands *rg, OpenGLContext *context)
{
   TimedBlock;
   
   //glCullFace(GL_FRONT);
   
   OpenGLProgram *prog = GetOpenGLProgram(context, ShaderFlags_None);
   
   glUseProgram(prog->program);
   
   glBindFramebuffer(GL_FRAMEBUFFER, context->shadowBuffer.id);
   glViewport(0, 0, context->shadowBuffer.width, context->shadowBuffer.height);  // is this per framebuffer?
   
   glClearDepth(1.0f);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   
   RenderSetup currentSetup = {};
   
   for (u32 pBufferIt = 0; pBufferIt < rg->pushBufferSize;)
   {
      
      RenderGroupEntryHeader *header = (RenderGroupEntryHeader *)(rg->pushBufferBase + pBufferIt);
      switch (header->type)
      {
         case RenderGroup_EntryChangeRenderSetup:
         {
            EntryChangeRenderSetup *setupHeader = (EntryChangeRenderSetup *)header;
            currentSetup = setupHeader->setup;
            currentSetup.cameraTransform = currentSetup.shadowMat;
            BeginUseProgram(context, prog, currentSetup);
            
            pBufferIt += sizeof(*setupHeader);
         }break;
         
         case RenderGroup_EntryTriangleMesh:
         {
            EntryTriangleMesh *meshHeader = (EntryTriangleMesh *)header;
            
            OpenGLUniformInfo uniforms;
            uniforms.objectTransform = meshHeader->objectTransform;
            uniforms.vertexBuffer    = meshHeader->vertexVBO;
            uniforms.indexBuffer     = meshHeader->indexVBO;
            
            SetupUniforms(prog, uniforms);
            
            BeginAttribArrays(prog, meshHeader->vertexFormatSize);
            
            For(meshHeader->indexSets)
            {
               
               switch (meshHeader->meshType)
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
            EndAttribArrays(prog);
            
            pBufferIt += sizeof(*meshHeader);
         }break;
         
         default:
         {
            pBufferIt += HeaderSize(header);
            continue;
         }break;
      }
      
   }
   
   glUseProgram(0);
   glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// todo make all  sizeof() dependent on the elements, not on the types.
void OpenGlRenderGroupToOutput(RenderCommands *rg, OpenGLContext *context)
{
   TimedBlock;
   
   RenderIntoShadowMap(rg, context);
   
   m4x4 biasMatrix =
   {
      {
         { 0.5f, 0.0f, 0.0f, 0.5f },
         { 0.0f, 0.5f, 0.0f, 0.5f },
         { 0.0f, 0.0f, 0.5f, 0.5f },
         { 0.0f, 0.0f, 0.0f, 1.0f }
      }
   };
   
   RenderSetup currentSetup = {};
   m4x4 shadowMat = {};
   OpenGLProgram *prog = NULL;
   
   for (u32 pBufferIt = 0; pBufferIt < rg->pushBufferSize;)
   {
      RenderGroupEntryHeader *header = (RenderGroupEntryHeader *)(rg->pushBufferBase + pBufferIt);
      
      switch (header->type)
      {
         
         case RenderGroup_EntryChangeRenderSetup:
         {
            EntryChangeRenderSetup *setupHeader = (EntryChangeRenderSetup *)header;
            
            RenderSetup *newSetup = &setupHeader->setup;
            
            if(newSetup->isProjective != currentSetup.isProjective)
            {
               FrameBuffer *buffer = newSetup->isProjective ? &context->renderBuffer : &context->orthogonalBuffer;
               
               glBindFramebuffer(GL_FRAMEBUFFER, buffer->id);
               glViewport(0, 0, buffer->width, buffer->height);
            }
            
            currentSetup = setupHeader->setup;
            shadowMat = biasMatrix * currentSetup.projection * currentSetup.shadowMat;
            prog = GetOpenGLProgram(context, currentSetup.flags);
            
            BeginUseProgram(context, prog, currentSetup);
            
            // todo do this in the setup, so we do not have to do it for every pass.
            pBufferIt += sizeof(*setupHeader);
         }break;
         case RenderGroup_EntryTexturedQuads:
         {
            EntryTexturedQuads *quadHeader = (EntryTexturedQuads *)header;
            
            OpenGLUniformInfo uniforms;
            uniforms.vertexBuffer = context->vertexBuffer;
            uniforms.indexBuffer  = context->indexBuffer;
            uniforms.textureIndex = quadHeader->vertexBuffer[0].textureIndex; // so textured works
            
            SetupUniforms(prog, uniforms);
            BeginAttribArrays(prog, sizeof(VertexFormatPCUI));
            
            glBufferData(GL_ARRAY_BUFFER, 4u * quadHeader->count * sizeof(quadHeader->vertexBuffer[0]), quadHeader->vertexBuffer, GL_STREAM_DRAW);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6u * quadHeader->count * sizeof(quadHeader->indexBuffer[0]), quadHeader->indexBuffer, GL_STREAM_DRAW);
            
            glDrawElements(GL_TRIANGLES, 6u * quadHeader->count, GL_UNSIGNED_SHORT, 0);
            
            EndAttribArrays(prog);
            
            pBufferIt += sizeof(*quadHeader);
         }break;
         case RenderGroup_EntryTriangles:
         {
            EntryColoredVertices *trianglesHeader = (EntryColoredVertices *)header;
            
            OpenGLUniformInfo uniforms;
            uniforms.vertexBuffer = context->vertexBuffer;
            
            SetupUniforms(prog, uniforms);
            
            BeginAttribArrays(prog, sizeof(VertexFormatPC));
            
            glBufferData(GL_ARRAY_BUFFER, trianglesHeader->vertexCount * sizeof(VertexFormatPC), trianglesHeader->data, GL_STREAM_COPY);
            glDrawArrays(GL_TRIANGLES, 0, trianglesHeader->vertexCount);
            
            EndAttribArrays(prog);
            
            pBufferIt += sizeof(*trianglesHeader);
         }break;
         case RenderGroup_EntryTriangleMesh:
         {
            EntryTriangleMesh *meshHeader = (EntryTriangleMesh *)header;
            
            OpenGLUniformInfo uniforms;
            uniforms.objectTransform = meshHeader->objectTransform;
            uniforms.scaleColor      = meshHeader->scaleColor;
            uniforms.vertexBuffer    = meshHeader->vertexVBO;
            uniforms.indexBuffer     = meshHeader->indexVBO;
            uniforms.boneStates      = meshHeader->boneStates;
            
            SetupUniforms(prog, uniforms);
            
            BeginAttribArrays(prog, meshHeader->vertexFormatSize);
            
            for(u32 i = 0; i < meshHeader->indexSets.amount; i++)
            {
               auto it = &meshHeader->indexSets[i];
               
               glUniform1f(prog->specularExponent, it->mat.specularExponent);
               glUniform3f(prog->ka, it->mat.ka.x, it->mat.ka.y, it->mat.ka.z);
               glUniform3f(prog->kd, it->mat.kd.x, it->mat.kd.y, it->mat.kd.z);
               glUniform3f(prog->ks, it->mat.ks.x, it->mat.ks.y, it->mat.ks.z);
               glUniform1i(prog->textureIndex, meshHeader->textureIDs[i]);
               
               switch (meshHeader->meshType)
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
            EndAttribArrays(prog);
            
            pBufferIt += sizeof(*meshHeader);
         }break;
         case RenderGroup_EntryClear:
         {
            EntryClear *clearHeader = (EntryClear *)header;
            
            // not sure
            
            glBindFramebuffer(GL_FRAMEBUFFER, context->renderBuffer.id);
            glClearDepth(1.0f);
            glClearColor(clearHeader->color.r, clearHeader->color.g, clearHeader->color.b, clearHeader->color.a);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            
            glBindFramebuffer(GL_FRAMEBUFFER, context->orthogonalBuffer.id);
            glClearDepth(1.0f);
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            
            pBufferIt += sizeof(*clearHeader);
         }break;
         case RenderGroup_EntryLines:
         {
            EntryColoredVertices *lineHeader = (EntryColoredVertices*)header;
            
#if 0
            OpenGLUniformInfo uniforms;
            uniforms.vertexBuffer = context->vertexBuffer;
            
            SetupUniforms(prog, currentSetup, uniforms);
            
            BeginAttribArrays(prog, sizeof(VertexFormatPC));
            glBufferData(GL_ARRAY_BUFFER, lineHeader->vertexCount * sizeof(VertexFormatPC), lineHeader->data, GL_STREAM_COPY);
            glDrawArrays(GL_LINES, 0, lineHeader->vertexCount);
            
            EndAttribArrays(prog);
#endif
            
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
   
   //glBlendEquation(GL_FUNC_ADD);
   
   glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
   
   glBindFramebuffer(GL_READ_FRAMEBUFFER, context->renderBuffer.id);
   glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
   
   glBlitFramebuffer(0, 0, context->renderBuffer.width, context->renderBuffer.height, 0, 0, rg->width, rg->height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
   
#if 1
   {
      BlitProgram *blit = &context->blitProgram;
      glUseProgram(blit->program);
      struct VertexFormatP2U
      {
         v2 p;
         v2 uv;
      };
      glBindTexture(GL_TEXTURE_2D, context->orthogonalBuffer.texture);
      
      VertexFormatP2U arr[6]
      {
         {V2(-1, -1), V2(0, 0)},
         {V2(+1, -1), V2(1, 0)},
         {V2(-1, +1), V2(0, 1)},
         
         {V2(-1, +1), V2(0, 1)},
         {V2(+1, -1), V2(1, 0)},
         {V2(+1, +1), V2(1, 1)},
      };
      
      glEnableVertexAttribArray(blit->vertP);
      glEnableVertexAttribArray(blit->vertUV);
      
      glVertexAttribPointer(blit->vertP, 2, GL_FLOAT, false, sizeof(arr[0]), (void *)OffsetOf(VertexFormatP2U, p));
      glVertexAttribPointer(blit->vertUV, 2, GL_FLOAT, false, sizeof(arr[0]), (void *)OffsetOf(VertexFormatP2U, uv));
      
      glBindBuffer(GL_ARRAY_BUFFER, context->vertexBuffer);
      glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(arr[0]), arr, GL_STREAM_COPY);
      glDrawArrays(GL_TRIANGLES, 0, 6);
      
      glDisableVertexAttribArray(blit->vertP);
      glDisableVertexAttribArray(blit->vertUV);
      
      
   }
#endif
   
   //flush
   rg->pushBufferSize = 0;
   
   GLuint err12or = glGetError();
   Assert(!err12or);
}

#endif
