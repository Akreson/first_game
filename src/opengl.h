#pragma once

#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82

#define GL_ARRAY_BUFFER                   0x8892

#define GL_STREAM_DRAW                    0x88E0
#define GL_STREAM_READ                    0x88E1
#define GL_STREAM_COPY                    0x88E2
#define GL_STATIC_DRAW                    0x88E4
#define GL_STATIC_READ                    0x88E5
#define GL_STATIC_COPY                    0x88E6
#define GL_DYNAMIC_DRAW                   0x88E8
#define GL_DYNAMIC_READ                   0x88E9
#define GL_DYNAMIC_COPY                   0x88EA

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

#define GL_FRAMEBUFFER                    0x8D40
#define GL_DEPTH_ATTACHMENT               0x8D00
#define GL_COLOR_ATTACHMENT0              0x8CE0
#define GL_RENDERBUFFER                   0x8D41
#define GL_DEPTH24_STENCIL8               0x88F0
#define GL_DEPTH_STENCIL_ATTACHMENT       0x821A

#define GL_CLAMP_TO_EDGE                  0x812F

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

typedef GLint type_glGetUniformLocation(GLuint program, const GLchar *name);

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

typedef void type_glGenRenderbuffers(GLsizei n, GLuint *renderbuffers);
typedef void type_glBindRenderbuffer(GLenum target, GLuint renderbuffer);
typedef void type_glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
typedef void type_glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);

typedef void type_glEnableVertexAttribArray(GLuint index);
typedef void type_glDisableVertexAttribArray(GLuint index);
typedef void type_glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer);

typedef void type_glGetShaderiv(GLuint shader, GLenum pname, GLint *params);
typedef void type_glGetProgramiv(GLuint program, GLenum pname, GLint *params);
typedef void type_glGetProgramInfoLog(GLuint program, GLsizei maxLength, GLsizei *length, GLchar *infoLog);
typedef void type_glGetShaderInfoLog(GLuint shader, GLsizei maxLength, GLsizei *length, GLchar *infoLog);
typedef void type_glValidateProgram(GLuint program);

typedef void type_glDebugMessageCallback(DEBUGPROC callback, void * userParam);

#define OpenGLGlobalVariable(Name) global type_##Name *Name;

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

OpenGLGlobalVariable(glEnableVertexAttribArray);
OpenGLGlobalVariable(glDisableVertexAttribArray);
OpenGLGlobalVariable(glVertexAttribPointer);

OpenGLGlobalVariable(glBindFramebuffer);
OpenGLGlobalVariable(glGenFramebuffers);
OpenGLGlobalVariable(glFramebufferTexture2D);
OpenGLGlobalVariable(glDeleteFramebuffers);
OpenGLGlobalVariable(glCheckFramebufferStatus);

OpenGLGlobalVariable(glGenRenderbuffers);
OpenGLGlobalVariable(glBindRenderbuffer);
OpenGLGlobalVariable(glRenderbufferStorage);
OpenGLGlobalVariable(glFramebufferRenderbuffer);

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
	GLuint BitmapProgramID;
	GLuint BitmapVAO;
	GLuint BitmapVBO;

	GLuint VertexBufferVAO;
	GLuint VertexBufferVBO;

	GLuint ModelProgramID; //
	GLuint ModelVAO; // TODO: Delete?
	GLuint ModelVBO; //

	GLuint BitmapColorID;
	GLuint BitmapProjID;

	GLuint ModelColorID;
	GLuint ModelProjID;
	GLuint ModelTransformID;
	GLuint ModelEdgeColor;

	/*GLuint FrameBufferDisplayProgramID;
	GLuint FrameBuffer;
	GLuint FrameBufferText;
	GLuint DepthRBO;
	GLuint FrameBufferVAO;
	GLuint FrameBufferVBO;*/
};
