#include "opengl.h"

global opengl_render_info OpenGL;
global const char *SharedHeaderCode;

void OpenGLMessageDebugCallback(
	GLenum source,
	GLenum type,
	GLuint id,\
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

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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
CompileBitmapProgram(opengl_bitmap_program *Prog)
{
	const char *VertexCode = R"FOO(
	layout (location = 0) in vec4 vertex;
	out vec2 TexCoords;
	
	uniform mat4 Proj;

	void main()
	{
		gl_Position = Proj * vec4(vertex.xy, 0, 1.0);
		TexCoords = vertex.zw;
	}

	)FOO";
	const char *FragmentCode = R"FOO(
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

	GLuint ProgID = OpenGLCreateProgram((GLchar *)SharedHeaderCode, (GLchar *)VertexCode, (GLchar *)FragmentCode);
	Prog->ProgID = ProgID;

	glUseProgram(ProgID);
	
	Prog->ColorID = glGetUniformLocation(ProgID, "Color");
	Prog->ProjID = glGetUniformLocation(ProgID, "Proj");
	
	glGenVertexArrays(1, &Prog->BitmapVAO);
	glGenBuffers(1, &Prog->BitmapVBO);

	glBindVertexArray(Prog->BitmapVAO);

	glBindBuffer(GL_ARRAY_BUFFER, Prog->BitmapVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(f32) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
}

internal void
CompileModelProgram(opengl_model_program *Prog)
{
	const char *VertexCode = R"FOO(
	layout (location = 0) in vec4 aPos;
	layout (location = 1) in vec4 aBarCoord;

	uniform mat4 Proj;
	uniform mat4 ModelTransform;

	out vec4 BarCoord;
	out float FaceSelectionParam;

	void main()
	{
		BarCoord = aBarCoord;
		FaceSelectionParam = aPos.w;
		gl_Position = Proj * ModelTransform * vec4(aPos.xyz, 1.0f);
	}

	)FOO";

	// TODO: Set model color?
	// TODO: Compute color for selected edge in right way

	const char *FragmentCode = R"FOO(
	#define FaceSelectionType_Hot 1.0f
	#define FaceSelectionType_Select 2.0f

	out vec4 FragColor;

	uniform vec4 Color;
	uniform vec3 EdgeColor;

	in vec4 BarCoord;
	in float FaceSelectionParam;

	float when_eq(float x, float y) {
	  return 1.0 - abs(sign(x - y));
	}

	float edgeFactor(){
		vec3 dBarCoord = fwidth(BarCoord.xyz);
		vec3 a3 = smoothstep(vec3(0.0), dBarCoord*2.5, BarCoord.xyz);
		float Result = min(min(a3.x, a3.y), a3.z);
		return Result;
	}

	void main()
	{
		vec3 _EdgeColor = vec3(0.17f, 0.5f, 0.8f); // NOTE: For Debug

		vec3 SelectColor = vec3(0.86f, 0.65f, 0.2f);
		vec3 HotFaceColor = vec3(1.0f);

		float MinD = min(min(BarCoord.x, BarCoord.y), BarCoord.z);
		float dMinD = fwidth(MinD);
		float Thickness = dMinD*2.5f;
		float Factor = smoothstep(0, Thickness, MinD);
		float InvFactor = 1.0f - Factor;
		float A = step(1.0f - dMinD, BarCoord.w);
		
		vec3 FinalEdgeColor = mix(_EdgeColor, SelectColor, A);

		float SelectFaceColorFactor = step(FaceSelectionType_Select, FaceSelectionParam);
		float HotFaceColorFactor = (1.0f - SelectFaceColorFactor) * step(FaceSelectionType_Hot, FaceSelectionParam);
		vec3 FinalSelectionFaceColor = SelectColor*SelectFaceColorFactor + HotFaceColor*HotFaceColorFactor;

		// TODO: Properly specify mix factor
		vec3 FinalFaceColor = mix(Color.xyz, FinalSelectionFaceColor, 0.3f);

		FragColor = vec4(mix(FinalFaceColor, FinalEdgeColor, InvFactor), Color.w);
	}
	)FOO";

	GLuint ProgID = OpenGLCreateProgram((GLchar *)SharedHeaderCode, (GLchar *)VertexCode, (GLchar *)FragmentCode);
	Prog->ProgID = ProgID;

	glUseProgram(ProgID);
	Prog->ModelColorID = glGetUniformLocation(ProgID, "Color");
	Prog->ModelProjID = glGetUniformLocation(ProgID, "Proj");
	Prog->ModelTransformID = glGetUniformLocation(ProgID, "ModelTransform");

	// TODO: Use for debuging, delete later
	Prog->ModelEdgeColor = glGetUniformLocation(ProgID, "EdgeColor");
	
	glGenVertexArrays(1, &Prog->ModelVAO);
	glGenBuffers(1, &Prog->ModelVBO);

	glUseProgram(0);
}

void
OpenGLInit()
{
#ifdef DEVELOP_MODE
	glDebugMessageCallback(OpenGLMessageDebugCallback, 0);
#endif

	SharedHeaderCode = R"FOO(
	#version 440
	)FOO";

	CompileBitmapProgram(&OpenGL.BitmapProg);
	CompileModelProgram(&OpenGL.ModelProg);

	glGenVertexArrays(1, &OpenGL.VertexBufferVAO);
	glGenBuffers(1, &OpenGL.VertexBufferVBO);

	// NOTE: use framebuffer in future
}

internal void
UseProgramBegin(opengl_bitmap_program *Prog, v3 Color, m4x4 *ProgMat)
{
	glUseProgram(Prog->ProgID);

	glUniform3f(Prog->ColorID, Color.x, Color.y, Color.z);
	glUniformMatrix4fv(Prog->ProjID, 1, GL_FALSE, &ProgMat->E[0][0]);

	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(Prog->BitmapVAO);
	glBindBuffer(GL_ARRAY_BUFFER, Prog->BitmapVBO);
}

internal void
UseProgramEnd(opengl_bitmap_program *Prog)
{
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);
}

internal void
UseProgramBegin(opengl_model_program *Prog, v4 Color, v3 EdgeColor, m4x4 *ProgMat, m4x4 *ModelMat)
{
	glUseProgram(Prog->ProgID);

	glUniform4f(Prog->ModelColorID, Color.r, Color.g, Color.b, Color.a);
	glUniformMatrix4fv(Prog->ModelProjID, 1, GL_FALSE, &ProgMat->E[0][0]);
	glUniformMatrix4fv(Prog->ModelTransformID, 1, GL_FALSE, &ModelMat->E[0][0]);

	// TODO: Delete later
	glUniform3f(Prog->ModelEdgeColor, EdgeColor.r, EdgeColor.g, EdgeColor.b);

	glBindVertexArray(OpenGL.VertexBufferVAO);
	glBindBuffer(GL_ARRAY_BUFFER, OpenGL.VertexBufferVBO);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(render_model_face_vertex), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(render_model_face_vertex), (void*)(sizeof(f32) * 4));
}

internal void
UseProgramEnd(opengl_model_program *Prog)
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
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

	glBufferData(GL_ARRAY_BUFFER, Commands->VertexBufferOffset, (GLvoid *)Commands->VertexBufferBase, GL_STREAM_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
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

				UseProgramBegin(&OpenGL.BitmapProg, BitmapEntry->Color, &Commands->OrthoProj.Forward);

				glBindTexture(GL_TEXTURE_2D, TextureIndex);
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertices), Vertices);
				glBindBuffer(GL_ARRAY_BUFFER, 0);
				glDrawArrays(GL_TRIANGLES, 0, 6);

				UseProgramEnd(&OpenGL.BitmapProg);
			} break;

			case RenderEntryType_render_entry_model:
			{
				render_entry_model *ModelEntry = (render_entry_model *)(Commands->PushBufferBase + BufferOffset);
				BufferOffset += sizeof(render_entry_model);

				m4x4 ModelTransform = Identity();
				SetTranslation(&ModelTransform, ModelEntry->Offset);

				UseProgramBegin(&OpenGL.ModelProg, ModelEntry->Color, ModelEntry->EdgeColor,
					&Commands->PersProj.Forward, &ModelTransform);

				glDrawArrays(GL_TRIANGLES, ModelEntry->StartOffset / sizeof(render_model_face_vertex), ModelEntry->ElementCount);
			
				UseProgramEnd(&OpenGL.ModelProg);
			} break;
		}
	}
}