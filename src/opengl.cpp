#include "opengl.h"

global opengl_render_info OpenGL;
global const char *SharedHeaderCode;

global framebuffer_info Framebuffers[16];
global u32 FramebufferCount = 0;

global GLuint TexturesID[64];
global u32 TexutreIDCount = 0;

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
		TexCoords = vertex.zw;
		gl_Position = Proj * vec4(vertex.xy, 0, 1.0);
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
		TexTexel.rgb *= Color;
		FragColor = TexTexel;
	}
	)FOO";

	GLuint ProgID = OpenGLCreateProgram((GLchar *)SharedHeaderCode, (GLchar *)VertexCode, (GLchar *)FragmentCode);
	Prog->ID = ProgID;

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
UseProgramBegin(opengl_bitmap_program *Prog, v3 Color, m4x4 *ProgMat)
{
	glUseProgram(Prog->ID);

	glUniform3f(Prog->ColorID, Color.r, Color.g, Color.b);
	glUniformMatrix4fv(Prog->ProjID, 1, GL_FALSE, &ProgMat->E[0][0]);

	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(Prog->BitmapVAO);
	glBindBuffer(GL_ARRAY_BUFFER, Prog->BitmapVBO);
}

internal void
UseProgramEnd(opengl_bitmap_program *Prog)
{
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
		//vec3 _EdgeColor = vec3(0.17f, 0.5f, 0.8f); // NOTE: For Debug

		vec3 SelectColor = vec3(0.86f, 0.65f, 0.2f);
		vec3 HotFaceColor = vec3(1.0f);

		float MinD = min(min(BarCoord.x, BarCoord.y), BarCoord.z);
		float dMinD = fwidth(MinD);
		float Thickness = dMinD*2.5f;
		float Factor = smoothstep(0, Thickness, MinD);
		float InvFactor = 1.0f - Factor;
		float A = step(1.0f - dMinD, BarCoord.w);
		
		vec3 FinalEdgeColor = mix(EdgeColor, SelectColor, A);

		float SelectFaceColorFactor = step(FaceSelectionType_Select, FaceSelectionParam);
		float HotFaceColorFactor = (1.0f - SelectFaceColorFactor) * step(FaceSelectionType_Hot, FaceSelectionParam);
		vec3 FinalSelectionFaceColor = SelectColor*SelectFaceColorFactor + HotFaceColor*HotFaceColorFactor;

		// TODO: Properly specify mix factor
		vec3 FinalFaceColor = mix(Color.xyz, FinalSelectionFaceColor, 0.3f);

		FragColor = vec4(mix(FinalFaceColor, FinalEdgeColor, InvFactor), Color.w);
	}
	)FOO";

	GLuint ProgID = OpenGLCreateProgram((GLchar *)SharedHeaderCode, (GLchar *)VertexCode, (GLchar *)FragmentCode);
	Prog->ID = ProgID;

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

internal void
UseProgramBegin(opengl_model_program *Prog, v4 Color, v3 EdgeColor, m4x4 *ProgMat, m4x4 *ModelMat)
{
	glUseProgram(Prog->ID);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(render_model_face_vertex), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(render_model_face_vertex), (void*)(sizeof(f32) * 4));

	glUniform4f(Prog->ModelColorID, Color.r, Color.g, Color.b, Color.a);
	glUniformMatrix4fv(Prog->ModelProjID, 1, GL_FALSE, &ProgMat->E[0][0]);
	glUniformMatrix4fv(Prog->ModelTransformID, 1, GL_FALSE, &ModelMat->E[0][0]);

	// TODO: Delete later
	glUniform3f(Prog->ModelEdgeColor, EdgeColor.r, EdgeColor.g, EdgeColor.b);

}

internal void
UseProgramEnd(opengl_model_program *Prog)
{
	glUseProgram(0);
}

// NOTE: It's not exactly right set outline color here, look just ok.
// TODO: Find way to set in proper way outline color in opengl_outline_program?
void
CompileModelColorPassProgram(opengl_model_color_pass_program *Prog)
{
	const char *VertexCode = R"FOO(
	layout (location = 0) in vec4 aPos;
	layout (location = 1) in vec4 aBarCoord;

	uniform mat4 Proj;
	uniform mat4 ModelTransform;

	void main()
	{
		gl_Position = Proj * ModelTransform * vec4(aPos.xyz, 1.0f);
	}
	)FOO";

	const char *FragmentCode = R"FOO(
	out vec4 FragColor;

	uniform vec3 OutlineColor;
	
	void main()
	{
		FragColor = vec4(OutlineColor, 1.0f);
	}
	)FOO";

	GLuint ProgID = OpenGLCreateProgram((GLchar *)SharedHeaderCode, (GLchar *)VertexCode, (GLchar *)FragmentCode);
	Prog->ID = ProgID;

	glUseProgram(ProgID);
	Prog->ModelProjID = glGetUniformLocation(ProgID, "Proj");
	Prog->ModelTransformID = glGetUniformLocation(ProgID, "ModelTransform");
	Prog->OutlineColor = glGetUniformLocation(ProgID, "OutlineColor");
	glUseProgram(0);
}

internal void
UseProgramBegin(opengl_model_color_pass_program *Prog, m4x4 *ProgMat, m4x4 *ModelMat, v3 Color)
{
	glUseProgram(Prog->ID);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(render_model_face_vertex), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(render_model_face_vertex), (void*)(sizeof(f32) * 4));

	glUniformMatrix4fv(Prog->ModelProjID, 1, GL_FALSE, &ProgMat->E[0][0]);
	glUniformMatrix4fv(Prog->ModelTransformID, 1, GL_FALSE, &ModelMat->E[0][0]);
	glUniform3f(Prog->OutlineColor, Color.r, Color.g, Color.b);
}

internal void
UseProgramEnd(opengl_model_color_pass_program *Prog)
{
	glUseProgram(0);
}

internal void
CompileBlurProgram(opengl_blur_program *Prog)
{
	const char *VertexCode = R"FOO(
	layout (location = 0) in vec3 Vertex;
	layout (location = 1) in vec2 TextCoord;

	out vec2 TexCoords;

	void main()
	{
		TexCoords = TextCoord;
		gl_Position = vec4(Vertex, 1.0);
	}

	)FOO";
	const char *FragmentCode = R"FOO(
	out vec4 FragColor;

	in vec2 TexCoords;	

	uniform sampler2D Texture;
	
	void main()
	{
		vec2 TexelSize = 1.0f / textureSize(Texture, 0);
		
		vec3 Color = texture(Texture, TexCoords).rgb;
		Color += texture(Texture, TexCoords + (vec2(-1.0f, 1.0f) * TexelSize)).rgb;
		Color += texture(Texture, TexCoords + (vec2(1.0f, 1.0f) * TexelSize)).rgb;
		Color += texture(Texture, TexCoords + (vec2(1.0f, -1.0f) * TexelSize)).rgb;
		Color += texture(Texture, TexCoords + (vec2(-1.0f, -1.0f) * TexelSize)).rgb;
		Color += texture(Texture, TexCoords + (vec2(-1.0f, 0) * TexelSize)).rgb;
		Color += texture(Texture, TexCoords + (vec2(1.0f, 0) * TexelSize)).rgb;

		FragColor = vec4(Color / 7.0f, 1.0f);
	}
	)FOO";

	Prog->ID = OpenGLCreateProgram((GLchar *)SharedHeaderCode, (GLchar *)VertexCode, (GLchar *)FragmentCode);
}

internal void
UseProgramBegin(opengl_blur_program *Prog)
{
	glUseProgram(Prog->ID);
}

internal void
UseProgramEnd(opengl_blur_program *Prog)
{
	glUseProgram(0);
}

void
CompileOutlinePassProgram(opengl_outline_program *Prog)
{
	const char *VertexCode = R"FOO(
	layout (location = 0) in vec3 Vertex;
	layout (location = 1) in vec2 TextCoord;

	out vec2 TexCoords;

	void main()
	{
		TexCoords = TextCoord;
		gl_Position = vec4(Vertex, 1.0f);
	}

	)FOO";
	// TODO: Set default framebuffer for
	const char *FragmentCode = R"FOO(
	out vec4 FragColor;

	in vec2 TexCoords;	

	uniform sampler2D PrepassTex;
	uniform sampler2D BlurTex;
	uniform sampler2D MainTex;

	float when_neq(vec3 x, vec3 y) {
	  return abs(sign(x - y)).x;
	}
	
	void main()
	{
		vec3 PrepassColor = texture(PrepassTex, TexCoords).rgb;
		vec3 BlurColor = texture(BlurTex, TexCoords).rgb;
		vec3 MainColor = texture(MainTex, TexCoords).rgb;;
		
		vec3 DiffColor = max(vec3(0), BlurColor - PrepassColor);
		
		FragColor = vec4(MainColor + DiffColor * 1.4, 1.0f);
	}
	)FOO";

	GLuint ProgID = OpenGLCreateProgram((GLchar *)SharedHeaderCode, (GLchar *)VertexCode, (GLchar *)FragmentCode);
	Prog->ID = ProgID;

	glUseProgram(ProgID);
	glUniform1i(glGetUniformLocation(ProgID, "PrepassTex"), 0);
	glUniform1i(glGetUniformLocation(ProgID, "BlurTex"), 1);
	glUniform1i(glGetUniformLocation(ProgID, "MainTex"), 2);
	glUseProgram(0);
}

internal void
UseProgramBegin(opengl_outline_program *Prog)
{
	glUseProgram(Prog->ID);
}

internal void
UseProgramEnd(opengl_outline_program *Prog)
{
	glUseProgram(0);
}

internal u32
OpenGLAllocateFramebuffers(u32 Count = 1, v4 ClearColor = V4(0))
{
	u32 Result = 0;

	for (u32 Index = 0; Index < Count; ++Index)
	{
		if (FramebufferCount < ArrayCount(Framebuffers))
		{
			Result = FramebufferCount;
			framebuffer_info *FB = Framebuffers + FramebufferCount++;

			glGenFramebuffers(1, &FB->ID);
			FB->ClearColor = ClearColor;
		}
		else
		{
			Assert(0);
		}
	}

	return Result;
}

internal inline framebuffer_info *
GetFrameBufferInfo(u32 ID)
{
	framebuffer_info *Result = Framebuffers + ID;
	return Result;
}

internal u32
OpenGLInternalAllocateTexture(u32 Width, u32 Height, 
	u32 Wrap, u32 Filtering, u32 InternalFormat, u32 Format,
	u32 PixelType, void *Data = 0)
{
	u32 Result;

	if (TexutreIDCount < ArrayCount(TexturesID))
	{
		Result = TexutreIDCount++;

		glGenTextures(1, &TexturesID[Result]);
		glBindTexture(GL_TEXTURE_2D, TexturesID[Result]);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, Wrap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, Wrap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Filtering);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, Filtering);
		glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, (GLsizei)Width, (GLsizei)Height,
			0, Format, PixelType, Data);

		glBindTexture(GL_TEXTURE_2D, 0);
	}
	else
	{
		Assert(0);
	}

	return Result;
}

void
OpenGLInit(f32 ScreenWidth, f32 ScreenHeight)
{
#ifdef DEVELOP_MODE
	glDebugMessageCallback(OpenGLMessageDebugCallback, 0);
#endif

	SharedHeaderCode = R"FOO(
	#version 440
	)FOO";

	CompileBitmapProgram(&OpenGL.BitmapProg);
	CompileModelProgram(&OpenGL.ModelProg);
	CompileModelColorPassProgram(&OpenGL.ModelColorPassProg);
	CompileBlurProgram(&OpenGL.BlurProg);
	CompileOutlinePassProgram(&OpenGL.OutlineProg);

	glGenVertexArrays(1, &OpenGL.VertexBufferVAO);
	glGenBuffers(1, &OpenGL.VertexBufferVBO);

	// NOTE: Set main FBO
	OpenGL.MainFB = OpenGLAllocateFramebuffers(1, V4(0.16f, 0.16f, 0.16f, 1.0f));

	framebuffer_info *MainFB = GetFrameBufferInfo(OpenGL.MainFB);
	glBindFramebuffer(GL_FRAMEBUFFER, MainFB->ID);

	MainFB->Color = OpenGLInternalAllocateTexture(ScreenWidth, ScreenHeight,
		GL_CLAMP_TO_EDGE, GL_NEAREST, GL_RGBA, GL_RGB, GL_UNSIGNED_BYTE);
	MainFB->DepthStensil = OpenGLInternalAllocateTexture(ScreenWidth, ScreenHeight,
		GL_CLAMP_TO_EDGE, GL_NEAREST, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8);

	glBindTexture(GL_TEXTURE_2D, TexturesID[MainFB->Color]);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
		TexturesID[MainFB->Color], 0);
	glBindTexture(GL_TEXTURE_2D, TexturesID[MainFB->DepthStensil]);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D,
		TexturesID[MainFB->DepthStensil], 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	// NOTE: Set FBO and texture for ouline effect
	// TODO: Use lower resolution texture for ouline effect
	for (u32 Index = 0; Index < 4; ++Index)
	{
		u32 FBIndex = OpenGLAllocateFramebuffers();
		framebuffer_info *FB = GetFrameBufferInfo(FBIndex);
		OpenGL.OutlineFrameBuffer.FB[Index] = FBIndex;

		glBindFramebuffer(GL_FRAMEBUFFER, FB->ID);
		FB->Color = OpenGLInternalAllocateTexture(ScreenWidth, ScreenHeight,
			GL_CLAMP_TO_EDGE, GL_NEAREST, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE);

		glBindTexture(GL_TEXTURE_2D, TexturesID[FB->Color]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
			TexturesID[FB->Color], 0);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	// NOTE: Set full screen VAO
	const float Vertices[] = {
		-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,

		-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	};

	glGenVertexArrays(1, &OpenGL.FullScreenVAO);
	glGenBuffers(1, &OpenGL.FullScreenVBO);

	glBindVertexArray(OpenGL.FullScreenVAO);
	glBindBuffer(GL_ARRAY_BUFFER, OpenGL.FullScreenVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), (GLvoid *)&Vertices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (sizeof(f32) * 5), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, (sizeof(f32) * 5), (void*)(sizeof(f32) * 3));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
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

	for (u32 FBIndex = 0; FBIndex < FramebufferCount; ++FBIndex)
	{
		framebuffer_info *FB = GetFrameBufferInfo(FBIndex);
		v4 ClearColor = FB->ClearColor;

		glBindFramebuffer(GL_FRAMEBUFFER, FB->ID);
		glClearColor(ClearColor.r, ClearColor.g, ClearColor.b, ClearColor.a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}

	framebuffer_info *MainFB = GetFrameBufferInfo(OpenGL.MainFB);
	glBindFramebuffer(GL_FRAMEBUFFER, MainFB->ID);

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
					MinPos.x, MinPos.y, 0.0f, 0.0f,// bottom left
					MinPos.x, MaxPos.y, 0.0f, 1.0f // top left
				};

				UseProgramBegin(&OpenGL.BitmapProg, BitmapEntry->Color, &Commands->OrthoProj.Forward);
				
				glBindVertexArray(OpenGL.BitmapProg.BitmapVAO);
				glBindBuffer(GL_ARRAY_BUFFER, OpenGL.BitmapProg.BitmapVBO);

				glBindTexture(GL_TEXTURE_2D, TextureIndex);
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertices), Vertices);
				glBindBuffer(GL_ARRAY_BUFFER, 0);
				glDrawArrays(GL_TRIANGLES, 0, 6);

				glBindVertexArray(0);
				glBindTexture(GL_TEXTURE_2D, 0);
				
				UseProgramEnd(&OpenGL.BitmapProg);
			} break;

			case RenderEntryType_render_entry_model:
			{
				render_entry_model *ModelEntry = (render_entry_model *)(Commands->PushBufferBase + BufferOffset);
				BufferOffset += sizeof(render_entry_model);

				m4x4 ModelTransform = Identity();
				SetTranslation(&ModelTransform, ModelEntry->Offset);

				glBindVertexArray(OpenGL.VertexBufferVAO);
				glBindBuffer(GL_ARRAY_BUFFER, OpenGL.VertexBufferVBO);
				
				UseProgramBegin(&OpenGL.ModelProg, ModelEntry->Color, ModelEntry->EdgeColor,
					&Commands->PersProj.Forward, &ModelTransform);

				glDrawArrays(GL_TRIANGLES, ModelEntry->StartOffset / sizeof(render_model_face_vertex), ModelEntry->ElementCount);
			
				glBindBuffer(GL_ARRAY_BUFFER, 0);
				glBindVertexArray(0);

				UseProgramEnd(&OpenGL.ModelProg);
			} break;

			// TODO: Clean up
			case RenderEntryType_render_entry_model_outline:
			{
				OpenGL.OutlineSet = true;

				render_entry_model_outline *OutlineEntry = (render_entry_model_outline *)(Commands->PushBufferBase + BufferOffset);
				BufferOffset += sizeof(render_entry_model_outline);

				render_entry_model *OutlineModel = OutlineEntry->ModelEntry;

				m4x4 ModelTransform = Identity();
				SetTranslation(&ModelTransform, OutlineModel->Offset);

				framebuffer_info *PrepassFB = GetFrameBufferInfo(OpenGL.OutlineFrameBuffer.PrepassFB);
				framebuffer_info *BlitFB[2];
				BlitFB[0] = GetFrameBufferInfo(OpenGL.OutlineFrameBuffer.BlitFB[0]);
				BlitFB[1] = GetFrameBufferInfo(OpenGL.OutlineFrameBuffer.BlitFB[1]);

				glBindFramebuffer(GL_FRAMEBUFFER, PrepassFB->ID);

				glBindVertexArray(OpenGL.VertexBufferVAO);
				glBindBuffer(GL_ARRAY_BUFFER, OpenGL.VertexBufferVBO);

				UseProgramBegin(&OpenGL.ModelColorPassProg,	&Commands->PersProj.Forward, &ModelTransform, OutlineEntry->OutlineColor);
				glDrawArrays(GL_TRIANGLES, OutlineModel->StartOffset / sizeof(render_model_face_vertex), OutlineModel->ElementCount);
				
				glBindBuffer(GL_ARRAY_BUFFER, 0);
				glBindVertexArray(0);
				UseProgramEnd(&OpenGL.ModelColorPassProg);

				glBindVertexArray(OpenGL.FullScreenVAO);

				UseProgramBegin(&OpenGL.BlurProg);
				glActiveTexture(GL_TEXTURE0);

				u32 PingPongIndex = 1;
				for (u32 PassIndex = 0; PassIndex < 10; ++PassIndex)
				{
					framebuffer_info *FB = BlitFB[!PingPongIndex];
					glBindFramebuffer(GL_FRAMEBUFFER, FB->ID);

					GLuint TextureID = PassIndex ? TexturesID[BlitFB[PingPongIndex]->Color] : TexturesID[PrepassFB->Color];
					glBindTexture(GL_TEXTURE_2D, TextureID);
					glDrawArrays(GL_TRIANGLES, 0, 6);

					PingPongIndex = !PingPongIndex;
				}
				UseProgramEnd(&OpenGL.BlurProg);
				glBindVertexArray(0);
				glBindFramebuffer(GL_FRAMEBUFFER, MainFB->ID);
			} break;
		}
	}

	if (OpenGL.OutlineSet)
	{
		framebuffer_info *PrepassFB = GetFrameBufferInfo(OpenGL.OutlineFrameBuffer.PrepassFB);
		framebuffer_info *BluredFB = GetFrameBufferInfo(OpenGL.OutlineFrameBuffer.BlitFB[1]);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindVertexArray(OpenGL.FullScreenVAO);

		UseProgramBegin(&OpenGL.OutlineProg);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TexturesID[PrepassFB->Color]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, TexturesID[BluredFB->Color]);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, TexturesID[MainFB->Color]);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		UseProgramEnd(&OpenGL.OutlineProg);

		glBindVertexArray(0);
	}
	else
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, MainFB->ID);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

		glBlitFramebuffer(
			0, 0, Commands->ScreenDim.x, Commands->ScreenDim.y,
			0, 0, Commands->ScreenDim.x, Commands->ScreenDim.y,
			GL_COLOR_BUFFER_BIT, GL_NEAREST);
	}
}