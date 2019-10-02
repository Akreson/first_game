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

typedef char GLchar;
typedef size_t GLsizeiptr;

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

typedef void type_glBindVertexArray(GLuint array);
typedef void type_glDeleteVertexArrays(GLsizei n, const GLuint *arrays);
typedef void type_glGenVertexArrays(GLsizei n, GLuint *arrays);

typedef void type_glGenBuffers(GLsizei n, GLuint * buffers);
typedef void type_glDeleteBuffers(GLsizei n, const GLuint * buffers);
typedef void type_glBufferData(GLenum target, GLsizeiptr size, const GLvoid * data, GLenum usage);

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

OpenGLGlobalVariable(glBindVertexArray);
OpenGLGlobalVariable(glDeleteVertexArrays);
OpenGLGlobalVariable(glGenVertexArrays);

OpenGLGlobalVariable(glGenBuffers);
OpenGLGlobalVariable(glDeleteBuffers);
OpenGLGlobalVariable(glBufferData);

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

struct opengl_info
{
	char *Vendor;
	char *Renderer;
	char *Version;
	char *ShadingLanguageVersion;
	char *Extension;

	b32 GL_EXT_framebuffer_object;
};

float vertices[] = {
	// positions         // colors
	0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,  // bottom right
	-0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,  // bottom left
	0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f   // top 
};

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
	layout (location = 1) in vec3 aColor;

	out vec3 ourColor;

	void main()
	{
		gl_Position = vec4(aPos, 1.0);
		ourColor = aColor;
	}
	)FOO";
	
	const char *FragmentCode = R"FOO(
	out vec4 FragColor;

	in vec3 ourColor;

	void main()
	{
		FragColor = vec4(ourColor, 1.0f);
	}	
	)FOO";

	GLuint ProgramID = OpenGLCreateProgram((GLchar *)HeaderCode, (GLchar *)VertexCode, (GLchar *)FragmentCode);
}