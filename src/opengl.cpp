#include "opengl.h"

global opengl_render_info OpenGL;
global const char *SharedHeaderCode;

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

internal framebuffer_info
CreateFramebuffer(u32 Width, u32 Height, u32 Flags, u32 ColorFormat = GL_RGB)
{
	framebuffer_info Result = {};

	b32 HasColor = Flags & OpenGLFramebufferSetParam_Color;
	b32 HasDepth = Flags & OpenGLFramebufferSetParam_Depth;
	b32 HasDepthStencil = Flags & OpenGLFramebufferSetParam_DepthStencil;

	glGenFramebuffers(1, &Result.Handle);
	glBindFramebuffer(GL_FRAMEBUFFER, Result.Handle);
	
	if (HasColor)
	{
		glGenTextures(1, &Result.Color);
		glBindTexture(GL_TEXTURE_2D, Result.Color);

		glTexImage2D(GL_TEXTURE_2D, 0, ColorFormat, (GLsizei)Width, (GLsizei)Height,
			0, GL_RGB, GL_UNSIGNED_BYTE, 0);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
			Result.Color, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	if (HasDepthStencil)
	{
		glGenTextures(1, &Result.DepthStencil);
		glBindTexture(GL_TEXTURE_2D, Result.DepthStencil);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, (GLsizei)Width, (GLsizei)Height,
			0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D,
			Result.DepthStencil, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	else if (HasDepth)
	{
		glGenTextures(1, &Result.Depth);
		glBindTexture(GL_TEXTURE_2D, Result.Depth);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, (GLsizei)Width, (GLsizei)Height,
			0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
			Result.Depth, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		Assert(0);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return Result;
}

#if 0
internal inline framebuffer_info *
GetFrameBufferInfo(u32 ID)
{
	framebuffer_info *Result = OpenGL.Framebuffers + ID;
	return Result;
}
#endif

internal void
OpenGLInternalAllocateTexture(GLuint *Handle, u32 Width, u32 Height,
	b32 Filter, u32 InternalFormat, u32 Format,
	u32 PixelType, void *Data = 0)
{
	glGenTextures(1, Handle);
	glBindTexture(GL_TEXTURE_2D, *Handle);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, Filter);
	glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, (GLsizei)Width, (GLsizei)Height,
		0, Format, PixelType, Data);

	glBindTexture(GL_TEXTURE_2D, 0);
}

// TODO: Provide more options?
PLATFORM_ALLOCATE_TEXTURE(OpenGLAllocateTexture)
{
	u32 TextureID = 0;

	if (OpenGL.TexutreIDCount < ArrayCount(OpenGL.TexturesID))
	{
		TextureID = OpenGL.TexutreIDCount++;

		OpenGLInternalAllocateTexture(&OpenGL.TexturesID[TextureID],
			Width, Height, GL_LINEAR, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, Data);
	}
	else
	{
		Assert(0);
	}

	return TextureID;
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
CompileBitmapProgram(bitmap_program *Prog)
{
	const char *VertexCode = R"FOO(
	layout (location = 0) in vec2 Vertex;
	layout (location = 1) in vec2 TextCoord;

	out vec2 TexCoords;
	
	uniform mat4 Proj;

	void main()
	{
		TexCoords = TextCoord;
		gl_Position = Proj * vec4(Vertex, 0, 1.0);
	}

	)FOO";
	const char *FragmentCode = R"FOO(
	in vec2 TexCoords;	
	out vec4 FragColor;

	uniform sampler2D BitmapTexture;
	uniform vec3 Color;
	
	void main()
	{
		vec4 Texel = texture(BitmapTexture, TexCoords);
		Texel.rgb *= Color;
		FragColor = Texel;
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

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, (sizeof(f32) * 4), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, (sizeof(f32) * 4), (void*)(sizeof(f32) * 2));
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
}

internal void
UseProgramBegin(bitmap_program *Prog, v3 Color, m4x4 *ProgMat)
{
	glUseProgram(Prog->ID);

	glUniform3f(Prog->ColorID, Color.r, Color.g, Color.b);
	glUniformMatrix4fv(Prog->ProjID, 1, GL_FALSE, &ProgMat->E[0][0]);
}

internal void
UseProgramEnd(bitmap_program *Prog)
{
	glUseProgram(0);
}

internal void
CompileModelProgram(model_program *Prog)
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
UseProgramBegin(model_program *Prog, v4 Color, v3 EdgeColor, m4x4 *ProgMat, m4x4 *ModelMat)
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
UseProgramEnd(model_program *Prog)
{
	glUseProgram(0);
}

// NOTE: It's not exactly right set outline color here, look just ok.
// TODO: Find to set in proper way outline color in outline_program?
void
CompileModelColorPassProgram(model_color_pass_program *Prog)
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
	
	void main()
	{
		FragColor = vec4(1.0f);
	}
	)FOO";

	GLuint ProgID = OpenGLCreateProgram((GLchar *)SharedHeaderCode, (GLchar *)VertexCode, (GLchar *)FragmentCode);
	Prog->ID = ProgID;

	glUseProgram(ProgID);
	Prog->ModelProjID = glGetUniformLocation(ProgID, "Proj");
	Prog->ModelTransformID = glGetUniformLocation(ProgID, "ModelTransform");
	glUseProgram(0);
}

internal void
UseProgramBegin(model_color_pass_program *Prog, m4x4 *ProgMat, m4x4 *ModelMat)
{
	glUseProgram(Prog->ID);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(render_model_face_vertex), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(render_model_face_vertex), (void*)(sizeof(f32) * 4));

	glUniformMatrix4fv(Prog->ModelProjID, 1, GL_FALSE, &ProgMat->E[0][0]);
	glUniformMatrix4fv(Prog->ModelTransformID, 1, GL_FALSE, &ModelMat->E[0][0]);
}

internal void
UseProgramEnd(model_color_pass_program *Prog)
{
	glUseProgram(0);
}

internal void
CompileBlurProgram(blur_program *Prog)
{
	const char *VertexCode = R"FOO(
	layout (location = 0) in vec2 Vertex;
	layout (location = 1) in vec2 TextCoord;

	out vec2 TexCoords;

	void main()
	{
		TexCoords = TextCoord;
		gl_Position = vec4(Vertex, 0, 1.0);
	}

	)FOO";
	const char *FragmentCode = R"FOO(
	out vec4 FragColor;

	in vec2 TexCoords;	

	uniform sampler2D Texture;
	
	void main()
	{
		vec2 TexelSize = 1.0f / textureSize(Texture, 0);
		
		float Color = texture(Texture, TexCoords).r;
		Color += texture(Texture, TexCoords + (vec2(-1.0f, 1.0f) * TexelSize)).r;
		Color += texture(Texture, TexCoords + (vec2(1.0f, 1.0f) * TexelSize)).r;
		Color += texture(Texture, TexCoords + (vec2(1.0f, -1.0f) * TexelSize)).r;
		Color += texture(Texture, TexCoords + (vec2(-1.0f, -1.0f) * TexelSize)).r;

		FragColor = vec4(Color / 5.0f, 0, 0, 1.0f);
	}
	)FOO";

	Prog->ID = OpenGLCreateProgram((GLchar *)SharedHeaderCode, (GLchar *)VertexCode, (GLchar *)FragmentCode);
}

internal void
UseProgramBegin(blur_program *Prog)
{
	glUseProgram(Prog->ID);
}

internal void
UseProgramEnd(blur_program *Prog)
{
	glUseProgram(0);
}

void
CompileOutlinePassProgram(outline_program *Prog)
{
	const char *VertexCode = R"FOO(
	layout (location = 0) in vec2 Vertex;
	layout (location = 1) in vec2 TextCoord;

	out vec2 TexCoords;

	void main()
	{
		TexCoords = TextCoord;
		gl_Position = vec4(Vertex, 0, 1.0f);
	}

	)FOO";
	// TODO: Set default framebuffer for
	const char *FragmentCode = R"FOO(
	out vec4 FragColor;

	in vec2 TexCoords;	

	uniform sampler2D PrepassTex;
	uniform sampler2D BlurTex;
	uniform sampler2D MainTex;
	uniform vec3 OutlineColor;

	float when_neq(vec3 x, vec3 y) {
	  return abs(sign(x - y)).x;
	}

	float when_neq(float x, float y) {
	  return abs(sign(x - y));
	}
	
	void main()
	{
		vec3 MainColor = texture(MainTex, TexCoords).rgb;
		float PrepassColor = texture(PrepassTex, TexCoords).r;
		float BlurColor = texture(BlurTex, TexCoords).r;
		
		vec3 DiffColor = vec3(max(0, BlurColor - PrepassColor));
		
		FragColor = vec4(MainColor + ((DiffColor * 1.2) * OutlineColor), 1.0f);
	}
	)FOO";

	GLuint ProgID = OpenGLCreateProgram((GLchar *)SharedHeaderCode, (GLchar *)VertexCode, (GLchar *)FragmentCode);
	Prog->ID = ProgID;

	glUseProgram(ProgID);
	glUniform1i(glGetUniformLocation(ProgID, "PrepassTex"), 0);
	glUniform1i(glGetUniformLocation(ProgID, "BlurTex"), 1);
	glUniform1i(glGetUniformLocation(ProgID, "MainTex"), 2);
	Prog->OutlineColor = glGetUniformLocation(ProgID, "OutlineColor");
	glUseProgram(0);
}

internal void
UseProgramBegin(outline_program *Prog, v3 Color)
{
	glUseProgram(Prog->ID);
	glUniform3f(Prog->OutlineColor, Color.r, Color.g, Color.b);
}

internal void
UseProgramEnd(outline_program *Prog)
{
	glUseProgram(0);
}

void
OpenGLInit(f32 ScreenWidth, f32 ScreenHeight)
{
#ifdef DEBUG_MODE
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
	OpenGL.MainFB = CreateFramebuffer(ScreenWidth, ScreenHeight,
		OpenGLFramebufferSetParam_Color|OpenGLFramebufferSetParam_Depth);

	// NOTE: Set FBO and texture for ouline effect
	// TODO: Use lower resolution texture for ouline effect
	OpenGL.Prepass = CreateFramebuffer(ScreenWidth, ScreenHeight,
		OpenGLFramebufferSetParam_Color, GL_R8);
	OpenGL.BlurBlit[0] = CreateFramebuffer(ScreenWidth, ScreenHeight,
		OpenGLFramebufferSetParam_Color, GL_R8);
	OpenGL.BlurBlit[1] = CreateFramebuffer(ScreenWidth, ScreenHeight,
		OpenGLFramebufferSetParam_Color, GL_R8);
	//OpenGL.OutlineSolve = CreateFramebuffer(ScreenWidth, ScreenHeight,
	//	OpenGLFramebufferSetParam_Color);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	// NOTE: Set full screen VAO
	const float Vertices[] = {
		// first triangle
		 1.0f,  1.0f, 1.0f, 1.0f,// top right
		 1.0f, -1.0f, 1.0f, 0.0f,// bottom right
		-1.0f,  1.0f, 0.0f, 1.0f,// top left 
		// second triangle
		 1.0f, -1.0f, 1.0f, 0.0f,// bottom right
		-1.0f, -1.0f, 0.0f, 0.0f,// bottom left
		-1.0f,  1.0f, 0.0f, 1.0f // top left
	};

	glGenVertexArrays(1, &OpenGL.FullScreenVAO);
	glGenBuffers(1, &OpenGL.FullScreenVBO);

	glBindVertexArray(OpenGL.FullScreenVAO);
	glBindBuffer(GL_ARRAY_BUFFER, OpenGL.FullScreenVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), (GLvoid *)&Vertices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, (sizeof(f32) * 4), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, (sizeof(f32) * 4), (void*)(sizeof(f32) * 2));

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

	glBindVertexArray(OpenGL.VertexBufferVAO);
	glBindBuffer(GL_ARRAY_BUFFER, OpenGL.VertexBufferVBO);
	glBufferData(GL_ARRAY_BUFFER, Commands->VertexBufferOffset, (GLvoid *)Commands->VertexBufferBase, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glClearColor(0.16f, 0.16f, 0.16f, 1.0f);
	glBindFramebuffer(GL_FRAMEBUFFER, OpenGL.MainFB.Handle);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0, 0, 0, 0);

	for (u32 BufferOffset = 0;
		BufferOffset < Commands->PushBufferSize;
		)
	{
		render_entry_header *Header = (render_entry_header *)(Commands->PushBufferBase + BufferOffset);
		BufferOffset += sizeof(render_entry_header);
		void *Data = (Commands->PushBufferBase + BufferOffset);

		switch (Header->Type)
		{
			case RenderEntryType_render_entry_bitmap:
			{
				render_entry_bitmap *BitmapEntry = (render_entry_bitmap *)Data;
				BufferOffset += sizeof(render_entry_bitmap);

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

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, OpenGL.TexturesID[BitmapEntry->TextureID]);
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertices), Vertices);
				glBindBuffer(GL_ARRAY_BUFFER, 0);
				glDrawArrays(GL_TRIANGLES, 0, 6);

				glBindVertexArray(0);
				glBindTexture(GL_TEXTURE_2D, 0);

				UseProgramEnd(&OpenGL.BitmapProg);
			} break;

			case RenderEntryType_render_entry_model:
			{
				render_entry_model *ModelEntry = (render_entry_model *)Data;
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
				glDisable(GL_BLEND);

				render_entry_model_outline *OutlineEntry = (render_entry_model_outline *)Data;
				BufferOffset += sizeof(render_entry_model_outline);

				render_entry_model *OutlineModel = OutlineEntry->ModelEntry;
				
				OpenGL.OutlineSet = true;
				OpenGL.OutlineColor = OutlineEntry->OutlineColor;

				m4x4 ModelTransform = Identity();
				SetTranslation(&ModelTransform, OutlineModel->Offset);

				glBindFramebuffer(GL_FRAMEBUFFER, OpenGL.BlurBlit[0].Handle);
				glClear(GL_COLOR_BUFFER_BIT);
				glBindFramebuffer(GL_FRAMEBUFFER, OpenGL.BlurBlit[1].Handle);
				glClear(GL_COLOR_BUFFER_BIT);

				glBindFramebuffer(GL_FRAMEBUFFER, OpenGL.Prepass.Handle);
				glClear(GL_COLOR_BUFFER_BIT);

				glBindVertexArray(OpenGL.VertexBufferVAO);
				glBindBuffer(GL_ARRAY_BUFFER, OpenGL.VertexBufferVBO);

				UseProgramBegin(&OpenGL.ModelColorPassProg, &Commands->PersProj.Forward, &ModelTransform);
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
					glBindFramebuffer(GL_FRAMEBUFFER, OpenGL.BlurBlit[!PingPongIndex].Handle);

					GLuint TextureID = PassIndex ? OpenGL.BlurBlit[PingPongIndex].Color : OpenGL.Prepass.Color;
					glBindTexture(GL_TEXTURE_2D, TextureID);
					glDrawArrays(GL_TRIANGLES, 0, 6);

					PingPongIndex = !PingPongIndex;
				}
				UseProgramEnd(&OpenGL.BlurProg);

				glBindVertexArray(0);
				glBindFramebuffer(GL_FRAMEBUFFER, OpenGL.MainFB.Handle);

				glEnable(GL_BLEND);
			} break;
		}
	}

	if (OpenGL.OutlineSet)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindVertexArray(OpenGL.FullScreenVAO);

		UseProgramBegin(&OpenGL.OutlineProg, OpenGL.OutlineColor);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, OpenGL.Prepass.Color);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, OpenGL.BlurBlit[1].Color);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, OpenGL.MainFB.Color);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		UseProgramEnd(&OpenGL.OutlineProg);

		glBindVertexArray(0);

		OpenGL.OutlineSet = false;
	}
	else
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, OpenGL.MainFB.Handle);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

		glBlitFramebuffer(
			0, 0, Commands->ScreenDim.x, Commands->ScreenDim.y,
			0, 0, Commands->ScreenDim.x, Commands->ScreenDim.y,
			GL_COLOR_BUFFER_BIT, GL_NEAREST);
	}
}