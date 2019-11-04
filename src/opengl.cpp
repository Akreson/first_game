#define WGL_DRAW_TO_WINDOW_ARB                  0x2001
#define WGL_DRAW_TO_BITMAP_ARB                  0x2002
#define WGL_ACCELERATION_ARB                    0x2003
#define WGL_SUPPORT_OPENGL_ARB                  0x2010
#define WGL_DOUBLE_BUFFER_ARB                   0x2011
#define WGL_PIXEL_TYPE_ARB                      0x2013

#define WGL_NO_ACCELERATION_ARB                 0x2025
#define WGL_GENERIC_ACCELERATION_ARB            0x2026
#define WGL_FULL_ACCELERATION_ARB               0x2027

#define WGL_SWAP_EXCHANGE_ARB                   0x2028
#define WGL_SWAP_COPY_ARB                       0x2029
#define WGL_SWAP_UNDEFINED_ARB                  0x202A

#define WGL_TYPE_RGBA_ARB                       0x202B
#define WGL_TYPE_COLORINDEX_ARB                 0x202C

#define WGL_CONTEXT_DEBUG_BIT_ARB               0x0001
#define WGL_CONTEXT_MAJOR_VERSION_ARB           0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB           0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB             0x2093
#define WGL_CONTEXT_FLAGS_ARB                   0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB            0x9126

#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB        0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002

#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82

#define GL_ARRAY_BUFFER                   0x8892
#define GL_STATIC_DRAW                    0x88E4
#define GL_DYNAMIC_DRAW                   0x88E8

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
#define GL_TEXTURE10                      0x84CA
#define GL_TEXTURE11                      0x84CB
#define GL_TEXTURE12                      0x84CC
#define GL_TEXTURE13                      0x84CD
#define GL_TEXTURE14                      0x84CE
#define GL_TEXTURE15                      0x84CF

typedef char GLchar;
typedef size_t GLsizeiptr;
typedef int* GLintptr;
typedef void (APIENTRY *DEBUGPROC)(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam);

typedef GLuint type_glCreateProgram(void);
typedef void type_glDeleteProgram(GLuint program);
typedef void type_glLinkProgram(GLuint program);
typedef GLuint type_glCreateShader(GLuint shaderType);
typedef void type_glCompileShader(GLuint program);
typedef void type_glShaderSource(GLuint shader, GLsizei count, const GLchar **string, const GLint *length);
typedef void type_glDeleteShader(GLuint shader);
typedef void type_glAttachShader(GLuint program, GLuint shader);
typedef void type_glDetachShader(GLuint program, GLuint shader);
typedef void type_glUseProgram(GLuint program);

typedef void type_glActiveTexture(GLenum texture);

typedef void type_glUniform1i(GLint location, GLint v0);
typedef void type_glUniform2i(GLint location, GLint v0, GLint v1);
typedef void type_glUniform3i(GLint location, GLint v0, GLint v1, GLint v2);
typedef void type_glUniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
typedef void type_glUniform1f(GLint location, GLfloat v0);
typedef void type_glUniform2f(GLint location, GLfloat v0, GLfloat v1);
typedef void type_glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void type_glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void type_glUniform2fv(GLint location, GLsizei count, const GLfloat *value);
typedef void type_glUniform3fv(GLint location, GLsizei count, const GLfloat *value);
typedef void type_glUniform4fv(GLint location, GLsizei count, const GLfloat *value);
typedef void type_glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void type_glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void type_glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);

typedef GLint type_glGetUniformLocation(GLuint program,	const GLchar *name);

typedef void type_glBindVertexArray(GLuint array);
typedef void type_glDeleteVertexArrays(GLsizei n, const GLuint *arrays);
typedef void type_glGenVertexArrays(GLsizei n, GLuint *arrays);

typedef void type_glGenBuffers(GLsizei n, GLuint * buffers);
typedef void type_glBindBuffer(GLenum target, GLuint buffer);
typedef void type_glBufferData(GLenum target, GLsizeiptr size, const GLvoid * data, GLenum usage);
typedef void type_glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid * data);
typedef void type_glDeleteBuffers(GLsizei n, const GLuint * buffers);

typedef void type_glBindFramebuffer(GLenum target, GLuint framebuffer);
typedef void type_glGenFramebuffers(GLsizei n, GLuint *framebuffers);
typedef void type_glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void type_glDeleteFramebuffers(GLsizei n, GLuint *framebuffers);
typedef GLenum type_glCheckFramebufferStatus(GLenum target);

typedef void type_glEnableVertexAttribArray(GLuint index);
typedef void type_glDisableVertexAttribArray(GLuint index);
typedef void type_glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer);

typedef void type_glGetShaderiv(GLuint shader, GLenum pname, GLint *params);
typedef void type_glGetProgramiv(GLuint program, GLenum pname, GLint *params);
typedef void type_glGetProgramInfoLog(GLuint program, GLsizei maxLength, GLsizei *length, GLchar *infoLog);
typedef void type_glGetShaderInfoLog(GLuint shader, GLsizei maxLength, GLsizei *length,	GLchar *infoLog);
typedef void type_glValidateProgram(GLuint program);

typedef void type_glDebugMessageCallback(DEBUGPROC callback, void * userParam);

#define OpenGLGlobalVariable(Name) global_variable type_##Name *Name;

OpenGLGlobalVariable(glCreateProgram);
OpenGLGlobalVariable(glDeleteProgram);
OpenGLGlobalVariable(glLinkProgram);
OpenGLGlobalVariable(glCreateShader);
OpenGLGlobalVariable(glCompileShader);
OpenGLGlobalVariable(glShaderSource);
OpenGLGlobalVariable(glDeleteShader);
OpenGLGlobalVariable(glAttachShader);
OpenGLGlobalVariable(glDetachShader);
OpenGLGlobalVariable(glUseProgram);

OpenGLGlobalVariable(glActiveTexture);

OpenGLGlobalVariable(glUniform1i);
OpenGLGlobalVariable(glUniform2i);
OpenGLGlobalVariable(glUniform3i);
OpenGLGlobalVariable(glUniform4i);
OpenGLGlobalVariable(glUniform1f);
OpenGLGlobalVariable(glUniform2f);
OpenGLGlobalVariable(glUniform3f);
OpenGLGlobalVariable(glUniform4f);
OpenGLGlobalVariable(glUniform2fv);
OpenGLGlobalVariable(glUniform3fv);
OpenGLGlobalVariable(glUniform4fv);
OpenGLGlobalVariable(glUniformMatrix2fv);
OpenGLGlobalVariable(glUniformMatrix3fv);
OpenGLGlobalVariable(glUniformMatrix4fv);

OpenGLGlobalVariable(glGetUniformLocation);

OpenGLGlobalVariable(glBindVertexArray);
OpenGLGlobalVariable(glDeleteVertexArrays);
OpenGLGlobalVariable(glGenVertexArrays);

OpenGLGlobalVariable(glGenBuffers);
OpenGLGlobalVariable(glBindBuffer);
OpenGLGlobalVariable(glBufferData);
OpenGLGlobalVariable(glBufferSubData);
OpenGLGlobalVariable(glDeleteBuffers);

OpenGLGlobalVariable(glBindFramebuffer);
OpenGLGlobalVariable(glGenFramebuffers);
OpenGLGlobalVariable(glFramebufferTexture2D);
OpenGLGlobalVariable(glDeleteFramebuffers);
OpenGLGlobalVariable(glCheckFramebufferStatus);

OpenGLGlobalVariable(glEnableVertexAttribArray);
OpenGLGlobalVariable(glDisableVertexAttribArray);
OpenGLGlobalVariable(glVertexAttribPointer);

OpenGLGlobalVariable(glGetShaderiv);
OpenGLGlobalVariable(glGetProgramiv);
OpenGLGlobalVariable(glGetProgramInfoLog);
OpenGLGlobalVariable(glGetShaderInfoLog);
OpenGLGlobalVariable(glValidateProgram);

OpenGLGlobalVariable(glDebugMessageCallback);

struct opengl_info
{
	char *Vendor;
	char *Renderer;
	char *Version;
	char *ShadingLanguageVersion;
	char *Extension;

	b32 GL_EXT_framebuffer_object;
};

struct opengl_render_info
{
	GLuint ProgramID;
	GLuint FontRenderProgram;
	GLuint FontVAO;
	GLuint FontVBO;
};

global_variable opengl_render_info OpenGL;

float vertices[] = {
	// first triangle
	0.5f, 0.5f, 0.0f, 1.0f, 1.0f,// top right
	0.5f, -0.5f, 0.0f, 1.0f, 0.0f,// bottom right
	-0.5f, 0.5f, 0.0f,  0.0f, 1.0f,// top left 
	// second triangle
	0.5f, -0.5f, 0.0f,  1.0f, 0.0f,// bottom right
	-0.5f, -0.5f, 0.0f, 0.0f, 0.0f, // bottom left
	-0.5f, 0.5f, 0.0f, 0.0f, 1.0f // top left
};

float _vertices[] = {
	// first triangle
	100.0f, 100.0f, 1.0f, 1.0f,// top right
	100.0, 0, 1.0f, 0.0f,// bottom right
	0, 100.0, 0.0f, 1.0f,// top left 
	// second triangle
	100.0, 0, 1.0f, 0.0f,// bottom right
	0, 0, 0.0f, 0.0f, // bottom left
	0, 100.0, 0.0f, 1.0f // top left
};

void OpenGLMessageDebugCallback(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	// TODO: More log information?
	Assert(0)
}

// TODO: Provide more options?
void *
AllocateTexture(u32 Width, u32 Height, void *Data)
{
	GLuint TextureIndex;
	glGenTextures(1, &TextureIndex);
	glBindTexture(GL_TEXTURE_2D, TextureIndex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height,
		0, GL_RGBA, GL_UNSIGNED_BYTE, Data);

	glBindTexture(GL_TEXTURE_2D, 0);

	return PointerFromU32(void, TextureIndex);
}

void
DeallocateTexture(u32 Texture)
{
	glDeleteTextures(1, &Texture);
}

internal opengl_info
OpenGLGetInfo()
{
	opengl_info Result = {};

	Result.Vendor = (char *)glGetString(GL_VENDOR);
	Result.Renderer = (char *)glGetString(GL_RENDERER);
	Result.Version = (char *)glGetString(GL_VERSION);

	Result.Extension = (char *)glGetString(GL_EXTENSIONS);

	char *At = Result.Extension;
	for (; *At;)
	{
		while (IsWhitespace(*At)) { ++At; }
		char *End = At;
		while (*End && !IsWhitespace(*End)) { ++End; }

		umm Count = End - At;

		if (StringAreEqual((u32)Count, At, "GL_EXT_framebuffer_object")) { Result.GL_EXT_framebuffer_object = true; }

		At = End;
	}

	return Result;
}

internal GLuint
OpenGLCreateProgram(GLchar *HeaderCode, GLchar *VertexCode, GLchar *FragmentCode)
{
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);

	GLchar *VertexShaderCode[] = 
	{
		HeaderCode,
		VertexCode,
	};
	glShaderSource(VertexShaderID, ArrayCount(VertexShaderCode), (const GLchar **)VertexShaderCode, 0);
	glCompileShader(VertexShaderID);

	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	GLchar *FragmentShaderCode[] =
	{
		HeaderCode,
		FragmentCode,
	};
	glShaderSource(FragmentShaderID, ArrayCount(FragmentShaderCode), (const GLchar **)FragmentShaderCode, 0);
	glCompileShader(FragmentShaderID);

	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	glValidateProgram(ProgramID);
	GLint Linked = false;
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Linked);

	if (!Linked)
	{
		GLsizei IgnoredLength;

		char VertexShaderInfoLog[4096];
		char FragmentShaderInfoLog[4096];
		char ProgramShaderInfoLog[4096];
		glGetShaderInfoLog(VertexShaderID, ArrayCount(VertexShaderInfoLog), &IgnoredLength, VertexShaderInfoLog);
		glGetShaderInfoLog(FragmentShaderID, ArrayCount(FragmentShaderInfoLog), &IgnoredLength, FragmentShaderInfoLog);
		glGetProgramInfoLog(ProgramID, ArrayCount(ProgramShaderInfoLog), &IgnoredLength, ProgramShaderInfoLog);

		Assert(!"Shader validation failed")
	}

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

internal void
OpenGLInit()
{
	const char *HeaderCode = R"FOO(
	#version 440	
	)FOO";
	
	const char *VertexCode = R"FOO(
    layout (location = 0) in vec3 aPos;
	layout (location = 1) in vec2 aTextCoord;
	uniform mat4 Projection;
	out vec2 TextCoord;
	
    void main()
    {
		gl_Position = Projection * vec4(aPos, 1.0);
		TextCoord = vec2(aTextCoord);
    }
	)FOO";
	
	const char *FragmentCode = R"FOO(
	out vec4 FragColor;

	in vec2 TextCoord;

	uniform sampler2D Texture1;

	void main()
	{
		FragColor = texture(Texture1, TextCoord);
	}	
	)FOO";

	const char *FontVertexCode =  R"FOO(
	layout (location = 0) in vec4 vertex;
	out vec2 TexCoords;
	
	uniform mat4 Projection;

	void main()
	{
		gl_Position = Projection * vec4(vertex.xy, 0, 1.0);
		TexCoords = vertex.zw;
	}

	)FOO";
	const char *FontFragmentCode = R"FOO(
	in vec2 TexCoords;	
	out vec4 FragColor;

	uniform sampler2D FontTexture;
	uniform vec3 TextColor;
	
	void main()
	{
		vec4 TexTexel = texture(FontTexture, TexCoords);
		TexTexel.xyz *= TextColor;
		FragColor = TexTexel;
	}
	)FOO";

	OpenGL.FontRenderProgram = OpenGLCreateProgram((GLchar *)HeaderCode, (GLchar *)FontVertexCode, (GLchar *)FontFragmentCode);
}

// TODO: get rid of border
void
OpenGLRenderText(font_asset_info *FontAsset, char *Text, v3 TextColor, f32 ScreenX, f32 ScreenY, f32 Scale)
{
	glUseProgram(OpenGL.FontRenderProgram);
	glUniform3f(glGetUniformLocation(OpenGL.FontRenderProgram, "TextColor"), TextColor.x, TextColor.y, TextColor.z);
	
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(OpenGL.FontVAO);

	ScreenY -= FontAsset->AscenderHeight*Scale;

	// TODO: Use codepoint?
	u32 PrevGlyphIndex = 0;
	for (;
		*Text;
		++Text)
	{
		u32 GlyphIndex = GetGlyphIndexFromCodePoint(FontAsset, *Text);
		
		if (*Text != ' ')
		{
			bitmap_info *Glyph = GetGlyphBitmap(FontAsset, GlyphIndex);
			u32 FontTextureIndex = U32FromPointer(Glyph->TextureHandler);
			
			f32 Width = (f32)Glyph->Width * Scale;
			f32 Height = (f32)Glyph->Height * Scale;

			f32 Xpos = ScreenX;
			f32 YPos = ScreenY - (FontAsset->VerticalAdjast[GlyphIndex]*(f32)Glyph->Height);

			float FontVertices[] = {
				// first triangle
				Xpos + Width, YPos + Height, 1.0f, 1.0f,// top right
				Xpos + Width, YPos, 1.0f, 0.0f,// bottom right
				Xpos, YPos + Height, 0.0f, 1.0f,// top left 
				// second triangle
				Xpos + Width, YPos, 1.0f, 0.0f,// bottom right
				Xpos, YPos, 0.0f, 0.0f, // bottom left
				Xpos, YPos + Height, 0.0f, 1.0f // top left
			};

			glBindTexture(GL_TEXTURE_2D, FontTextureIndex);
			glBindBuffer(GL_ARRAY_BUFFER, OpenGL.FontVBO);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(FontVertices), FontVertices);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

		ScreenX += (f32)FontAsset->GlyphAdvance[GlyphIndex]*Scale;
		if (PrevGlyphIndex)
		{
			ScreenX += (f32)FontAsset->KerningTable[PrevGlyphIndex*FontAsset->GlyphCount + GlyphIndex];
		}

		PrevGlyphIndex = GlyphIndex;
	}

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}