#include "OpenGL.h"

GLFunctions gl = {};

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
	GLuint vertexShaderID = gl.glCreateShader(GL_VERTEX_SHADER);
	GLchar *vertexShaderCode[] =
	{
		headerCode, vertexCode
	};
	gl.glShaderSource(vertexShaderID, ArrayCount(vertexShaderCode), vertexShaderCode, 0);
	gl.glCompileShader(vertexShaderID);

	GLuint fragmentShaderID = gl.glCreateShader(GL_FRAGMENT_SHADER);
	GLchar *fragmentShaderCode[] =
	{
		headerCode, fragmentCode
	};
	gl.glShaderSource(fragmentShaderID, ArrayCount(fragmentShaderCode), fragmentShaderCode, 0);
	gl.glCompileShader(fragmentShaderID);

	GLuint programID = gl.glCreateProgram();
	gl.glAttachShader(programID, vertexShaderID);
	gl.glAttachShader(programID, fragmentShaderID);
	gl.glLinkProgram(programID);

	gl.glValidateProgram(programID);
	GLint linkValidated = false;
	gl.glGetProgramiv(programID, GL_LINK_STATUS, &linkValidated);
	//glGetProgramiv(programID, GL_COMPILE_STATUS, &compileValidated);
	GLint validated = linkValidated;
	if (!validated)
	{
		GLsizei length;
		char programError[4096];
		char vertexError[4096];
		char fragmentError[4096];
		gl.glGetProgramInfoLog(programID, sizeof(programError), &length, programError);
		gl.glGetShaderInfoLog(vertexShaderID, sizeof(vertexError), &length, vertexError);
		gl.glGetShaderInfoLog(fragmentShaderID, sizeof(fragmentError), &length, fragmentError);

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

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

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

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glBindTexture(GL_TEXTURE_2D, 0);
	return handle;
}

void OpenGLInit(HGLRC modernContext, GLFunctions *incomingFunctions)
{
	//GLuint reservedBltTexture;
	//glGenTextures(1, &reservedBltTexture);

	OpenGLInfo info = OpenGLGetInfo(true);

	gl = *incomingFunctions;
	openGLGlobals.defaultInternalTextureFormat = GL_RGBA8;
	if (info.GL_EXT_texture_sRGB)
	{
		openGLGlobals.defaultInternalTextureFormat = GL_SRGB8_ALPHA8;
	}

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	char *headerCodeTex =
		R"FOO(
	//Header Code
	#version 130
	)FOO";
	char *vertexCodeTex =
		R"FOO(
	//Vertex Code

	uniform mat4x4 transform;

	smooth out vec4 fragColor;
	smooth out vec2 fragCoord;

	void main(void)
	{
		vec4 inputVertex = gl_Vertex;
		gl_Position = transform * inputVertex;

		fragColor = gl_Color;
		fragCoord = gl_MultiTexCoord0.xy;

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
		resultColor =	fragColor * result;
	}

	)FOO";
	openGLGlobals.basicZBiasProgram = OpenGLCreateProgram(headerCodeTex, vertexCodeTex, fragmentCodeTex);
	Assert(openGLGlobals.basicZBiasProgram);
	openGLGlobals.transformID = gl.glGetUniformLocation(openGLGlobals.basicZBiasProgram, "transform");
	openGLGlobals.texSamplerID = gl.glGetUniformLocation(openGLGlobals.basicZBiasProgram, "texSampler");



	char *headerCode =
		R"FOO(
	//Header Code
	#version 130
	)FOO";
	char *vertexCode =
		R"FOO(
	//Vertex Code

	uniform mat4x4 transform;

	in vec3 vertP;
	in vec4 vertC;

	smooth out vec4 fragColor;

	void main(void)
	{
		vec4 inputVertex = vec4(vertP, 1);
		gl_Position = transform * inputVertex;

		fragColor = vertC;

	}

	)FOO";
	char *fragmentCode =
		R"FOO(
	//Fragment Code

	out vec4 resultColor;
	smooth in vec4 fragColor;

	void main(void)
	{
		resultColor = fragColor;
	}

	)FOO";

	
	openGLGlobals.basicProgram = OpenGLCreateProgram(headerCode, vertexCode, fragmentCode);
	Assert(openGLGlobals.basicProgram);
	openGLGlobals.transformID2 = gl.glGetUniformLocation(openGLGlobals.basicZBiasProgram, "transform");
	openGLGlobals.vertPArrayID = gl.glGetAttribLocation(openGLGlobals.basicProgram, "vertP");
	openGLGlobals.vertCArrayID = gl.glGetAttribLocation(openGLGlobals.basicProgram, "vertC");

	Assert(glGetError() == GL_NO_ERROR);
}

void InitWhiteOpenGL(GLuint whiteTextureID)
{
	openGLGlobals.whiteTextureID = whiteTextureID;
}


void OpenGlRenderGroupToOutput(RenderCommands *rg)
{	
	glViewport(0, 0, rg->width, rg->height);

	glDepthMask(GL_TRUE);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glAlphaFunc(GL_GREATER, 0.0f);
	glEnable(GL_ALPHA_TEST);
	glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
	glEnable(GL_SAMPLE_ALPHA_TO_ONE);
	glEnable(GL_MULTISAMPLE);
	
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
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
			//glMatrixMode(GL_MODELVIEW);
			//glLoadIdentity();
			//glMatrixMode(GL_PROJECTION);
			
			openGLGlobals.projectionMat = Transpose(transformHeader->m);

			//glLoadMatrixf(openGLGlobals.projectionMat.a[0]);
	
			gl.glUseProgram(openGLGlobals.basicZBiasProgram);
			gl.glUniformMatrix4fv(openGLGlobals.transformID, 1, GL_FALSE, openGLGlobals.projectionMat.a[0]);
			GLint texInt = 0;
			gl.glUniform1iv(openGLGlobals.texSamplerID, 1, &texInt);
			
			gl.glUseProgram(openGLGlobals.basicProgram);
			gl.glUniformMatrix4fv(openGLGlobals.transformID, 1, GL_FALSE, openGLGlobals.projectionMat.a[0]);
			gl.glUseProgram(0);

			GLuint error = glGetError();
			Assert(!error);

			pBufferIt += sizeof(*transformHeader);
		}break;
		case RenderGroup_EntryBitmap:
		{
			EntryBitmap *bitmapHeader = (EntryBitmap *)header;
			gl.glUseProgram(openGLGlobals.basicZBiasProgram);
			
			v3 UL = bitmapHeader->pos;
			v3 UR = bitmapHeader->pos + bitmapHeader->basis.d1;
			v3 LL = bitmapHeader->pos + bitmapHeader->basis.d2;
			v3 LR = bitmapHeader->pos + bitmapHeader->basis.d1 + bitmapHeader->basis.d2;
			GLuint bitmapTextureHandle = (GLuint)bitmapHeader->bitmap->textureHandle;

			GLuint error = glGetError();

			Assert(!error);

			glBindTexture(GL_TEXTURE_2D, bitmapTextureHandle);

			glBegin(GL_TRIANGLES);

			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

			//Lower Triangle
			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(LL.x, LL.y, LL.z);

			glTexCoord2f(1.0f, 0.0f);
			glVertex3f(LR.x, LR.y, LR.z);

			glTexCoord2f(1.0f, 1.0f);
			glVertex3f(UR.x, UR.y, UR.z);

			//upper Triangle
			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(LL.x, LL.y, LL.z);

			glTexCoord2f(0.0f, 1.0f);
			glVertex3f(UL.x, UL.y, UL.z);

			glTexCoord2f(1.0f, 1.0f);
			glVertex3f(UR.x, UR.y, UR.z);

			glEnd();

			gl.glUseProgram(0);
			
			pBufferIt += sizeof(*bitmapHeader);
		}break;
		case RenderGroup_EntryUpdateBitmap:
		{
			EntryUpdateBitmap *updateHeader = (EntryUpdateBitmap *)header;
			OpenGLUpdateBitmap(updateHeader->bitmap);
			pBufferIt += sizeof(*updateHeader);
		}break;
		case RenderGroup_EntryTexturedQuadrilateral:
		{
			EntryTexturedQuadrilateral *quadHeader = (EntryTexturedQuadrilateral *)header;
			
			v3 p1 = quadHeader->p1;
			v3 p2 = quadHeader->p2;
			v3 p3 = quadHeader->p3;
			v3 p4 = quadHeader->p4;

			gl.glUseProgram(openGLGlobals.basicZBiasProgram);
			glBindTexture(GL_TEXTURE_2D, (GLuint)quadHeader->bitmap->textureHandle);
			v4 color = quadHeader->color;
			OpenGLRect3D(p1, p2, p3, p4, color, color, color, color);

			//v3 translatedPoint = (Transpose(openGLGlobals.projectionMat) * pos);
			gl.glUseProgram(0);

			pBufferIt += sizeof(*quadHeader);
		}break;
		case RenderGroup_EntryTriangle:
		{
			EntryTriangle *triangleHeader = (EntryTriangle *)header;

			v3 p1 = triangleHeader->pos;
			v3 p2 = triangleHeader->pos + triangleHeader->d1;
			v3 p3 = triangleHeader->pos + triangleHeader->d2;
			v4 color = triangleHeader->color;

			gl.glUseProgram(openGLGlobals.basicProgram);			
			glBegin(GL_TRIANGLES);
			
			glColor4f(color.r, color.g, color.b, color.a);
			
			glVertex3f(p1.x, p1.y, p1.z);
			glVertex3f(p3.x, p3.y, p3.z);
			glVertex3f(p2.x, p2.y, p2.z);

			glEnd();
			//v3 translatedPoint = (Transpose(openGLGlobals.projectionMat) * pos);
			gl.glUseProgram(0);

			pBufferIt += sizeof(*triangleHeader);
		}break;
		case RenderGroup_EntryTriangles:
		{
			EntryTriangles *trianglesHeader = (EntryTriangles *)header;

			gl.glUseProgram(openGLGlobals.basicProgram);

			gl.glEnableVertexAttribArray(openGLGlobals.vertPArrayID);
			gl.glEnableVertexAttribArray(openGLGlobals.vertCArrayID);
			
			u8 *vertexBase = (u8 *)rg->vertexArray;
			gl.glVertexAttribPointer(openGLGlobals.vertPArrayID, 3, GL_FLOAT, false, sizeof(ColoredVertex), vertexBase + OffsetOf(ColoredVertex, pos));
			gl.glVertexAttribPointer(openGLGlobals.vertCArrayID, 4, GL_UNSIGNED_BYTE, true, sizeof(ColoredVertex), vertexBase + OffsetOf(ColoredVertex, color));
			glDrawArrays(GL_TRIANGLES, trianglesHeader->offset, trianglesHeader->vertexCount);
			
			gl.glDisableVertexAttribArray(openGLGlobals.vertPArrayID);
			gl.glDisableVertexAttribArray(openGLGlobals.vertCArrayID);
			gl.glUseProgram(0);

			pBufferIt += sizeof(*trianglesHeader);
		}break;
		
		case RenderGroup_EntryClear:
		{
			EntryClear *clearHeader = (EntryClear *)header;
			glClearColor(clearHeader->color.r, clearHeader->color.g, clearHeader->color.b, clearHeader->color.a);
			glClear(GL_COLOR_BUFFER_BIT);

			pBufferIt += sizeof(*clearHeader);
		}break;
		case RenderGroup_EntryLine:
		{
			EntryLine *lineHeader = (EntryLine *)header;
			v4 color = lineHeader->color;
			v3 p1 = lineHeader->p1;
			v3 p2 = lineHeader->p2;
			
			gl.glUseProgram(openGLGlobals.basicProgram);
			glBegin(GL_LINES);
			glColor4f(color.r, color.g, color.b, color.a);

			glVertex3f(p1.x, p1.y, p1.z);
			glVertex3f(p2.x, p2.y, p2.z);

			glEnd();			
			gl.glUseProgram(0);

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

	//flush
	rg->pushBufferSize = 0;
	rg->vertexCount = 0;
	
}

