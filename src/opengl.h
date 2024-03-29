#pragma once

#define GL_RGBA32F                        0x8814
#define GL_RGB32F                         0x8815
#define GL_RGBA16F                        0x881A
#define GL_RGB16F                         0x881B

#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82

#define GL_FRAMEBUFFER_COMPLETE           0x8CD5

#define GL_ARRAY_BUFFER                   0x8892
#define GL_ELEMENT_ARRAY_BUFFER           0x8893

#define GL_STREAM_DRAW                    0x88E0
#define GL_STREAM_READ                    0x88E1
#define GL_STREAM_COPY                    0x88E2
#define GL_STATIC_DRAW                    0x88E4
#define GL_STATIC_READ                    0x88E5
#define GL_STATIC_COPY                    0x88E6
#define GL_DYNAMIC_DRAW                   0x88E8
#define GL_DYNAMIC_READ                   0x88E9
#define GL_DYNAMIC_COPY                   0x88EA

//#define GL_LINES                          0x0001
//#define GL_LINE_LOOP                      0x0002
//#define GL_LINE_STRIP                     0x0003

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

#define GL_R8                             0x8229
#define GL_R16                            0x822A
#define GL_RG8                            0x822B
#define GL_RG16                           0x822C
#define GL_R16F                           0x822D
#define GL_R32F                           0x822E
#define GL_RG16F                          0x822F
#define GL_RG32F                          0x8230
#define GL_R8I                            0x8231
#define GL_R8UI                           0x8232
#define GL_R16I                           0x8233
#define GL_R16UI                          0x8234
#define GL_R32I                           0x8235
#define GL_R32UI                          0x8236
#define GL_RG8I                           0x8237
#define GL_RG8UI                          0x8238
#define GL_RG16I                          0x8239
#define GL_RG16UI                         0x823A
#define GL_RG32I                          0x823B
#define GL_RG32UI                         0x823C

#define GL_FRAMEBUFFER                    0x8D40
#define GL_DEPTH_ATTACHMENT               0x8D00
#define GL_COLOR_ATTACHMENT0              0x8CE0
#define GL_RENDERBUFFER                   0x8D41
#define GL_DEPTH24_STENCIL8               0x88F0
#define GL_DEPTH_STENCIL_ATTACHMENT       0x821A
#define GL_DEPTH_STENCIL                  0x84F9

#define GL_UNSIGNED_INT_24_8              0x84FA
#define GL_DEPTH_COMPONENT16              0x81A5
#define GL_DEPTH_COMPONENT24              0x81A6
#define GL_DEPTH_COMPONENT32              0x81A7

#define GL_READ_FRAMEBUFFER               0x8CA8
#define GL_DRAW_FRAMEBUFFER               0x8CA9

#define GL_CLAMP_TO_EDGE                  0x812F

#define GL_NUM_EXTENSIONS                 0x821D
#define GL_DEBUG_SEVERITY_HIGH            0x9146
#define GL_DEBUG_SEVERITY_MEDIUM          0x9147
#define GL_DEBUG_SEVERITY_LOW             0x9148

#define GL_ALIASED_LINE_WIDTH_RANGE       0x846E

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
typedef void type_glBlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
	GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);

typedef void type_glEnableVertexAttribArray(GLuint index);
typedef void type_glDisableVertexAttribArray(GLuint index);
typedef void type_glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer);
typedef void type_glVertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid * pointer);

typedef void type_glGetShaderiv(GLuint shader, GLenum pname, GLint *params);
typedef void type_glGetProgramiv(GLuint program, GLenum pname, GLint *params);
typedef void type_glGetProgramInfoLog(GLuint program, GLsizei maxLength, GLsizei *length, GLchar *infoLog);
typedef void type_glGetShaderInfoLog(GLuint shader, GLsizei maxLength, GLsizei *length, GLchar *infoLog);
typedef void type_glValidateProgram(GLuint program);

typedef void type_glDebugMessageCallback(DEBUGPROC callback, void * userParam);

typedef const GLubyte* type_glGetStringi(GLenum name, GLuint index);

#define OpenGLGlobalVariable(Name) global type_##Name *Name;

OpenGLGlobalVariable(glGetStringi);

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
OpenGLGlobalVariable(glVertexAttribIPointer);

OpenGLGlobalVariable(glBindFramebuffer);
OpenGLGlobalVariable(glGenFramebuffers);
OpenGLGlobalVariable(glFramebufferTexture2D);
OpenGLGlobalVariable(glDeleteFramebuffers);
OpenGLGlobalVariable(glCheckFramebufferStatus);

OpenGLGlobalVariable(glGenRenderbuffers);
OpenGLGlobalVariable(glBindRenderbuffer);
OpenGLGlobalVariable(glRenderbufferStorage);
OpenGLGlobalVariable(glFramebufferRenderbuffer);
OpenGLGlobalVariable(glBlitFramebuffer)

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

enum opengl_framebuffer_set_params
{
	OpenGLFramebufferSetParam_Color = (1 << 0),
	OpenGLFramebufferSetParam_Depth = (1 << 1),
	OpenGLFramebufferSetParam_Stencil = (1 << 2),
	OpenGLFramebufferSetParam_DepthStencil = (1 << 3),
};

struct framebuffer_info
{
	GLuint Handle;
	GLuint Color;
	union
	{
		GLuint Depth;
		GLuint DepthStencil;
	};
};

struct bitmap_program
{
	GLuint ID;

	GLuint ColorID;
	GLuint ProjID;
};

struct model_program
{
	GLuint ID; //
	GLuint ModelVAO; // TODO: Delete?
	GLuint ModelVBO; //

	GLuint SpotLightDirID;
	GLuint ModelColorID;
	GLuint CameraPosID;
	GLuint ModelProjID;
	GLuint ModelTransformID;
	GLuint ModelEdgeColor;
};

struct model_color_pass_program
{
	GLuint ID;

	GLuint ModelProjID;
	GLuint ModelTransformID;
	GLuint OutlineColor;
};

struct blur_program
{
	GLuint ID;
};

struct outline_program
{
	GLuint ID;
	GLuint OutlineColor;
};

struct static_mesh_program
{
	GLuint ID;

	GLuint ProjID;
	GLuint TransformID;
	GLuint ColorID;
};

struct rotate_tool_program
{
	GLuint ID;
	GLuint PersProj;
	GLuint ModelTransform;
	GLuint CameraTransform;

	GLuint XAxis;
	GLuint YAxis;
	GLuint ZAxis;
	GLuint AxisActivityState;
	GLuint CenterPos;
	GLuint ViewDir;
	GLuint PerpInfo;
};

struct trin_model_color_pass_program
{
	GLuint ID;
	GLuint ModelProjID;
	GLuint ModelTransformID;
};

struct line_pass_program
{
	GLuint ID;
	GLuint ProjID;
	GLuint TransformID;
	GLuint ColorID;
};

struct opengl_renderer_info
{
	framebuffer_info MainFB;

	GLuint VertexBufferVAO;
	GLuint VertexBufferVBO;

	GLuint TrinBufferVAO;
	GLuint TrinBufferVBO;

	GLuint LineBufferVAO;
	GLuint LineBufferVBO;
	
	GLuint FullScreenVAO;
	GLuint FullScreenVBO;

	bitmap_program BitmapProg;
	model_program ModelProg;
	model_color_pass_program ModelColorPassProg;
	trin_model_color_pass_program TrinModelColorPassProg;
	blur_program BlurProg;
	outline_program OutlineProg;
	rotate_tool_program RotateTools;
	line_pass_program LinePass;
	
	static_mesh_program StaticMeshProg;

	framebuffer_info Prepass;
	framebuffer_info BlurBlit[2];
	framebuffer_info OutlineSolve;

	b32 OutlineSet;
	v3 OutlineColor;

	v2 LineWidthParam;
	//framebuffer_info Framebuffers[16];
	//u32 FramebufferCount;
};

internal GLuint OpenGLCreateProgram(GLchar *HeaderCode, GLchar *VertexCode, GLchar *FragmentCode);