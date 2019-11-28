#define WGL_DRAW_TO_WINDOW_ARB                  0x2001
#define WGL_DRAW_TO_BITMAP_ARB                  0x2002
#define WGL_ACCELERATION_ARB                    0x2003
#define WGL_SUPPORT_OPENGL_ARB                  0x2010
#define WGL_DOUBLE_BUFFER_ARB                   0x2011

#define WGL_PIXEL_TYPE_ARB                      0x2013
#define WGL_RED_BITS_ARB                        0x2015
#define WGL_GREEN_BITS_ARB                      0x2017
#define WGL_BLUE_BITS_ARB                       0x2019
#define WGL_ALPHA_BITS_ARB                      0x201B
#define WGL_DEPTH_BITS_ARB                      0x2022
#define WGL_COLOR_BITS_ARB                      0x2014
#define WGL_STENCIL_BITS_ARB                    0x2023

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

typedef void type_glGenRenderbuffers(GLsizei n,	GLuint *renderbuffers);
typedef void type_glBindRenderbuffer(GLenum target,	GLuint renderbuffer);
typedef void type_glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
typedef void type_glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);

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

	
	/*GLuint FrameBufferDisplayProgramID;
	GLuint FrameBuffer;
	GLuint FrameBufferText;
	GLuint DepthRBO;
	GLuint FrameBufferVAO;
	GLuint FrameBufferVBO;*/
};

global_variable opengl_render_info OpenGL;

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
PLATFORM_ALLOCATE_TEXTURE(OpenGLAllocateTexture)
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

PLATFORM_DEALLOCATE_TEXTURE(OpenGLDeallocateTexture)
{
	glDeleteTextures(1, &TextureHandler);
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

	const char *BitmapVertexCode =  R"FOO(
	layout (location = 0) in vec4 vertex;
	out vec2 TexCoords;
	
	uniform mat4 Proj;

	void main()
	{
		gl_Position = Proj * vec4(vertex.xy, 0, 1.0);
		TexCoords = vertex.zw;
	}

	)FOO";
	const char *BitmapFragmentCode = R"FOO(
	in vec2 TexCoords;	
	out vec4 FragColor;

	uniform sampler2D BitmapTexture;
	uniform vec3 Color;
	
	void main()
	{
		vec4 TexTexel = texture(BitmapTexture, TexCoords);
		TexTexel.xyz *= Color;
		FragColor = TexTexel;
	}
	)FOO";

#ifdef DEVELOP_MODE
	glDebugMessageCallback(OpenGLMessageDebugCallback, 0);
#endif

	OpenGL.BitmapProgramID = OpenGLCreateProgram((GLchar *)HeaderCode, (GLchar *)BitmapVertexCode, (GLchar *)BitmapFragmentCode);
	
	glUseProgram(OpenGL.BitmapProgramID);
	glGenVertexArrays(1, &OpenGL.BitmapVAO);
	glGenBuffers(1, &OpenGL.BitmapVBO);

	glBindVertexArray(OpenGL.BitmapVAO);

	glBindBuffer(GL_ARRAY_BUFFER, OpenGL.BitmapVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(f32) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	OpenGL.BitmapColorID = glGetUniformLocation(OpenGL.BitmapProgramID, "Color");
	OpenGL.BitmapProjID = glGetUniformLocation(OpenGL.BitmapProgramID, "Proj");

	const char *ModelVertexCode = R"FOO(
	layout (location = 0) in vec3 aPos;
	layout (location = 1) in vec3 aBarCoord;

	uniform mat4 Proj;
	uniform mat4 ModelTransform;

	out vec3 BarCoord;

	void main()
	{
		BarCoord = aBarCoord;
		gl_Position = Proj * ModelTransform * vec4(aPos, 1.0f);
	}

	)FOO";
	// TODO: Set model color?
	const char *ModelFragmentCode = R"FOO(
	out vec4 FragColor;

	uniform vec4 Color;

	in vec3 BarCoord;

	float edgeFactor(){
		vec3 dBarCoord = fwidth(BarCoord);
		vec3 a3 = smoothstep(vec3(0.0), dBarCoord*1.5, BarCoord);
		return min(min(a3.x, a3.y), a3.z);
	}

	void main()
	{
		float Factor = edgeFactor();
		FragColor = vec4(vec3(Factor), 1.0);
	}
	)FOO";

	OpenGL.ModelProgramID = OpenGLCreateProgram((GLchar *)HeaderCode, (GLchar *)ModelVertexCode, (GLchar *)ModelFragmentCode);

	glUseProgram(OpenGL.ModelProgramID);
	glGenVertexArrays(1, &OpenGL.ModelVAO);
	glGenBuffers(1, &OpenGL.ModelVBO);

	OpenGL.ModelColorID = glGetUniformLocation(OpenGL.ModelProgramID, "Color");
	OpenGL.ModelProjID = glGetUniformLocation(OpenGL.ModelProgramID, "Proj");
	OpenGL.ModelTransformID = glGetUniformLocation(OpenGL.ModelProgramID, "ModelTransform");

	glGenVertexArrays(1, &OpenGL.VertexBufferVAO);
	glGenBuffers(1, &OpenGL.VertexBufferVBO);

	// NOTE: use framebuffer in future
}

void
OpenGLRenderCommands(game_render_commands *Commands)
{
	glDepthMask(true);
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(0.16f, 0.16f, 0.16f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindVertexArray(OpenGL.VertexBufferVAO);
	glBindBuffer(GL_ARRAY_BUFFER, OpenGL.VertexBufferVBO);

	u32 SizeOfVertexData = Commands->VertexCount * sizeof(f32);
	glBufferData(GL_ARRAY_BUFFER, SizeOfVertexData, (GLvoid *)Commands->VertexBufferBase, GL_STREAM_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER,0);
	glBindVertexArray(0);

	for (u32 BufferOffset = 0;
		BufferOffset < Commands->PushBufferSize;
		)
	{
		render_entry_header *Header = (render_entry_header *)(Commands->PushBufferBase + BufferOffset);
		BufferOffset += sizeof(render_entry_header);

		switch (Header->Type)
		{
			case RenderEntryType_render_entry_bitmap:
			{
				glUseProgram(OpenGL.BitmapProgramID);

				render_entry_bitmap *BitmapEntry = (render_entry_bitmap *)(Commands->PushBufferBase + BufferOffset);
				BufferOffset += sizeof(render_entry_bitmap);

				u32 TextureIndex = U32FromPointer(BitmapEntry->Bitmap->TextureHandler);

				v2 MinPos = BitmapEntry->Min;
				v2 MaxPos = BitmapEntry->Max;

				float Vertices[] = {
					// first triangle
					MaxPos.x, MaxPos.y, 1.0f, 1.0f,// top right
					MaxPos.x, MinPos.y, 1.0f, 0.0f,// bottom right
					MinPos.x, MaxPos.y, 0.0f, 1.0f,// top left 
					// second triangle
					MaxPos.x, MinPos.y, 1.0f, 0.0f,// bottom right
					MinPos.x, MinPos.y, 0.0f, 0.0f, // bottom left
					MinPos.x, MaxPos.y, 0.0f, 1.0f // top left
				};

				glUniform3f(OpenGL.BitmapColorID,
					BitmapEntry->Color.x, BitmapEntry->Color.y, BitmapEntry->Color.z);
				glUniformMatrix4fv(OpenGL.BitmapProjID, 1, GL_FALSE, &Commands->OrthoProj.E[0][0]);

				glActiveTexture(GL_TEXTURE0);
				glBindVertexArray(OpenGL.BitmapVAO);

				glBindTexture(GL_TEXTURE_2D, TextureIndex);
				glBindBuffer(GL_ARRAY_BUFFER, OpenGL.BitmapVBO);
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertices), Vertices);
				glBindBuffer(GL_ARRAY_BUFFER, 0);
				glDrawArrays(GL_TRIANGLES, 0, 6);

				glBindVertexArray(0);
				glBindTexture(GL_TEXTURE_2D, 0);
			} break;

			// TODO: Batch all model(s) faces?
			case RenderEntryType_render_entry_model_face:
			{
				glUseProgram(OpenGL.ModelProgramID);

				render_entry_model_face *FaceEntry = (render_entry_model_face *)(Commands->PushBufferBase + BufferOffset);
				BufferOffset += sizeof(render_entry_model_face);

				m4x4 ModelTransform = Identity();
				Translate(&ModelTransform, FaceEntry->Offset);

				glUniform4f(OpenGL.ModelColorID,
					FaceEntry->Color.r, FaceEntry->Color.g, FaceEntry->Color.b, FaceEntry->Color.a);
				glUniformMatrix4fv(OpenGL.ModelProjID, 1, GL_FALSE, &Commands->PersProj.E[0][0]);
				glUniformMatrix4fv(OpenGL.ModelTransformID, 1, GL_FALSE, &ModelTransform.E[0][0]);

				glBindVertexArray(OpenGL.VertexBufferVAO);
				glBindBuffer(GL_ARRAY_BUFFER, OpenGL.VertexBufferVBO);

				u32 OffsetInBytes = FaceEntry->VertexBufferOffset * sizeof(f32);

				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(f32), (void*)0);
				glEnableVertexAttribArray(1);
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(f32), (void*)(sizeof(f32) * 3));

				glDrawArrays(GL_TRIANGLES, FaceEntry->VertexBufferOffset / 6, 6);
			} break;

			case RenderEntryType_render_entry_model:
			{
				glUseProgram(OpenGL.ModelProgramID);

				render_entry_model *ModelEntry = (render_entry_model *)(Commands->PushBufferBase + BufferOffset);
				BufferOffset += sizeof(render_entry_model);

				m4x4 ModelTransform = Identity();
				Translate(&ModelTransform, ModelEntry->Offset);

				glUniform4f(OpenGL.ModelColorID,
					ModelEntry->Color.r, ModelEntry->Color.g, ModelEntry->Color.b, ModelEntry->Color.a);
				glUniformMatrix4fv(OpenGL.ModelProjID, 1, GL_FALSE, &Commands->PersProj.E[0][0]);
				glUniformMatrix4fv(OpenGL.ModelTransformID, 1, GL_FALSE, &ModelTransform.E[0][0]);

				glBindVertexArray(OpenGL.ModelVAO);
				glBindBuffer(GL_ARRAY_BUFFER, OpenGL.ModelVBO);
				u32 SizeOfVertexData = ModelEntry->VertexCount * sizeof(v3);
				glBufferData(GL_ARRAY_BUFFER, SizeOfVertexData, (GLvoid *)ModelEntry->Vertex, GL_STREAM_DRAW);
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(f32), (void*)0);

				glDrawArrays(GL_TRIANGLES, 0, ModelEntry->VertexCount / 3);

				glBindVertexArray(0);
			} break;
		}
	}
}