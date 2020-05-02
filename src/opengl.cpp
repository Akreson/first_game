#include "opengl.h"

global opengl_renderer_info OpenGL;
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

// TODO: Provide more options?
PLATFORM_ALLOCATE_TEXTURE(OpenGLAllocateTexture)
{
	renderer_texture Result;

	glGenTextures(1, (GLuint *)&Result.Handle);
	glBindTexture(GL_TEXTURE_2D, Result.Handle);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)Width, (GLsizei)Height,
		0, GL_RGBA, GL_UNSIGNED_BYTE, Data);

	glBindTexture(GL_TEXTURE_2D, 0);

	return Result;
}

PLATFORM_DEALLOCATE_TEXTURE(OpenGLDeallocateTexture)
{
	GLuint Handle = (u32)TextureHandler;
	glDeleteTextures(1, &Handle);
}

PLATFORM_ALLOCATE_MESH(OpenGLAllocateMesh)
{
	renderer_mesh Result;

	GLuint VAO;
	GLuint VBO;
	GLuint EBO;

	Assert(Params.VertexData);
	Assert(Params.Tris);

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	u32 ElementSize = sizeof(v3);
	u32 NormalsSize = (Params.Flags & AllocMeshParam_NormalSet) ? sizeof(v3) : 0;
	u32 UVSize = (Params.Flags & AllocMeshParam_UVSet) ? sizeof(v2) : 0;

	ElementSize += NormalsSize + UVSize;

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, ElementSize, (void*)0);
	u32 NextElementIndex = 1;

	if (NormalsSize)
	{
		glEnableVertexAttribArray(NextElementIndex);
		glVertexAttribPointer(NextElementIndex, 3, GL_FLOAT, GL_FALSE, ElementSize, (void*)sizeof(v3));
		++NextElementIndex;
	}

	if (UVSize)
	{
		glEnableVertexAttribArray(NextElementIndex);
		glVertexAttribPointer(NextElementIndex, 2, GL_FLOAT, GL_FALSE, ElementSize, (void*)(sizeof(v3)*2));
	}

	glBufferData(GL_ARRAY_BUFFER, Params.VertexCount * ElementSize, Params.VertexData, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, Params.TrisCount * (sizeof(u32) * 3), Params.Tris, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	Result.Handle = (u64)VAO;
	Result.ElementCount = Params.TrisCount * 3;
	return Result;
}

PLATFORM_DEALLOCATE_MESH(OpenGLDeallocateMesh)
{

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

// TODO: Fix bug with colored non selected edge for
// small view degree on non slected face
internal void
CompileModelProgram(model_program *Prog)
{
	const char *VertexCode = R"FOO(
	layout (location = 0) in vec3 aPos;
	layout (location = 1) in vec3 aBarCoord;
	layout (location = 2) in vec4 aMetaInfo;
	layout (location = 3) in uint aEdgeBarIndex;

	// NOTE: aMetaInfo
	// y - Active
	// z - Hot 
	// w - use for adjust vertex value when no one active or hot (TODO: separate for active and hot)

	uniform mat4 Proj;
	uniform mat4 ModelTransform;

	out vec3 BarCoord;
	out vec3 BarSelectCoord;
	out vec3 BarHotCoord;
	out float FaceSelectionParam;

	vec3 ModifyBarCoord(vec3 BarCoord, float Value, float ClearMul, uint Index)
	{
		vec3 Result = BarCoord;

		float ClearValue = Result[Index] * ClearMul;
		Result[Index] = Value;
		
		Result.x += ClearValue;
		Result.y += ClearValue;
		Result.z += ClearValue;

		Result[Index] -= ClearValue;

		return Result;
	}	

	void main()
	{
		BarCoord = aBarCoord;
		FaceSelectionParam = aMetaInfo.x;
		BarSelectCoord = ModifyBarCoord(aBarCoord, aMetaInfo.y, aMetaInfo.w, aEdgeBarIndex);
		BarHotCoord = ModifyBarCoord(aBarCoord, aMetaInfo.z, aMetaInfo.w, aEdgeBarIndex);

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

	in vec3 BarCoord;
	in vec3 BarSelectCoord;
	in vec3 BarHotCoord;
	in float FaceSelectionParam;

	float GetStepFactor(vec3 A, float Thickness)
	{
		float MinD = min(min(A.x, A.y), A.z);
		float dMinD = fwidth(MinD);
		float Factor = smoothstep(0, Thickness*dMinD, MinD);

		return Factor;
	}	

	void main()
	{
		//vec3 _EdgeColor = vec3(0.17f, 0.5f, 0.8f); // NOTE: For Debug

		vec3 SelectColor = vec3(0.86f, 0.65f, 0.2f);
		vec3 HotFaceColor = vec3(1.3f);
		float Thickness = 2.0f;
	
		// NOTE: Edge color calc.
		float EdgeFactor = GetStepFactor(BarCoord, Thickness);
		float InvEdgeFactor = 1.0f - EdgeFactor;

		float MinSelectD = min(min(BarSelectCoord.x, BarSelectCoord.y), BarSelectCoord.z);
		float dMinSelectD = fwidth(MinSelectD);
		float SelectEdgeFactor = 1.0f - step(dMinSelectD * Thickness, MinSelectD);
		//float SelectEdgeFactor = 1.0f - GetStepFactor(BarSelectCoord, Thickness);

		float MinHotD = min(min(BarHotCoord.x, BarHotCoord.y), BarHotCoord.z);
		float dMinHotD = fwidth(MinHotD);
		float HotEdgeFactor = 1.0f - step(dMinHotD * Thickness, MinHotD);
		//float HotEdgeFactor = 1.0f - GetStepFactor(BarHotCoord, Thickness);

		//-------------
		/*float SelectEdgeFactor = 1.0f - (fwidth(MetaInfo.y)*Thickness);
		SelectEdgeFactor = step(SelectEdgeFactor, MetaInfo.y);
		
		float HotEdgeFactor = 1.0f - (fwidth(MetaInfo.z)*Thickness);
		HotEdgeFactor = step(HotEdgeFactor, MetaInfo.z);*/
		//------------

		vec3 FinalEdgeColor = mix(EdgeColor, SelectColor, SelectEdgeFactor);
		// TODO: Enable Hot Factor
		FinalEdgeColor = mix(FinalEdgeColor, FinalEdgeColor*HotFaceColor, 0);
		
		// NOTE: Face color calc
		float ModFaceSelParam = FaceSelectionParam;

		float SelectFaceColorFactor = step(FaceSelectionType_Select, ModFaceSelParam);
		ModFaceSelParam -= SelectFaceColorFactor * FaceSelectionType_Select;

		float HotFaceColorFactor = step(FaceSelectionType_Hot, ModFaceSelParam);
		ModFaceSelParam -= HotFaceColorFactor * FaceSelectionType_Hot;

		vec3 FinalFaceColor = mix(Color.rgb, (SelectColor*Color.rgb), SelectFaceColorFactor);
		FinalFaceColor = mix(FinalFaceColor, (FinalFaceColor*HotFaceColor), HotFaceColorFactor);

		FragColor = vec4(mix(FinalFaceColor, FinalEdgeColor, InvEdgeFactor), Color.a);
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
		
		float DiffColor = max(0, BlurColor - PrepassColor);
		
		FragColor = vec4(MainColor + (vec3(DiffColor * 1.2) * OutlineColor), 1.0f);
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

internal void
CompileStaticMeshProgram(static_mesh_program *Prog)
{
	const char *VertexCode = R"FOO(
	layout (location = 0) in vec3 aPos;

	uniform mat4 Proj;
	uniform mat4 ModelTransform;

	void main()
	{
		gl_Position = Proj * ModelTransform * vec4(aPos, 1.0f);
	}

	)FOO";

	// TODO: Set model color?
	// TODO: Compute color for selected edge in right way

	const char *FragmentCode = R"FOO(
	out vec4 FragColor;

	uniform vec3 MeshColor;
	
	void main()
	{
		FragColor = vec4(1.0);
	}
	)FOO";

	GLuint ProgID = OpenGLCreateProgram((GLchar *)SharedHeaderCode, (GLchar *)VertexCode, (GLchar *)FragmentCode);
	Prog->ID = ProgID;

	glUseProgram(ProgID);
	Prog->ProjID = glGetUniformLocation(ProgID, "Proj");
	Prog->TransformID = glGetUniformLocation(ProgID, "ModelTransform");
	Prog->ColorID = glGetUniformLocation(ProgID, "MeshColor");
	glUseProgram(0);
}

internal void
UseProgramBegin(static_mesh_program *Prog, m4x4 *ProgMat, m4x4 *ModelMat, v3 Color)
{
	glUseProgram(Prog->ID);

	glUniformMatrix4fv(Prog->ProjID, 1, GL_FALSE, &ProgMat->E[0][0]);
	glUniformMatrix4fv(Prog->TransformID, 1, GL_FALSE, &ModelMat->E[0][0]);
	glUniform3f(Prog->ColorID, Color.r, Color.g, Color.b);
}

internal void
UseProgramEnd(static_mesh_program *Prog)
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

	CompileStaticMeshProgram(&OpenGL.StaticMeshProg);
	
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
	
	// NOTE Set Editor model vertex VAO
	glGenVertexArrays(1, &OpenGL.VertexBufferVAO);
	glGenBuffers(1, &OpenGL.VertexBufferVBO);

	glBindVertexArray(OpenGL.VertexBufferVAO);
	glBindBuffer(GL_ARRAY_BUFFER, OpenGL.VertexBufferVBO);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(render_model_face_vertex), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(render_model_face_vertex), (void*)(sizeof(f32) * 3));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(render_model_face_vertex), (void*)(sizeof(f32) * 6));
	glEnableVertexAttribArray(3);
	glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, sizeof(render_model_face_vertex), (void*)(sizeof(f32) * 10));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
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

inline void
OpenGLBindTex(GLenum Target, GLenum Slot, GLuint Handle)
{
	glActiveTexture(Slot);
	glBindTexture(Target, Handle);
}

void
OpenGLRenderCommands(game_render_commands *Commands)
{
	glDepthMask(true);
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// NOTE: Seems i have bug with this option when they enable (image get staking)
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);
	//glFrontFace(GL_CCW);

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

				OpenGLBindTex(GL_TEXTURE_2D, GL_TEXTURE0, (GLuint)BitmapEntry->Texture.Handle);
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

				glDrawArrays(GL_TRIANGLES,
					ModelEntry->StartOffset / sizeof(render_model_face_vertex),
					ModelEntry->ElementCount);

				glBindBuffer(GL_ARRAY_BUFFER, 0);
				glBindVertexArray(0);

				UseProgramEnd(&OpenGL.ModelProg);
			} break;

			case RenderEntryType_render_entry_sphere:
			{
				render_entry_sphere *SphereEntry = (render_entry_sphere *)Data;
				BufferOffset += sizeof(render_entry_sphere);

				m4x4 I = Identity();

				UseProgramBegin(&OpenGL.StaticMeshProg,
					&Commands->PersProj.Forward, &I, SphereEntry->Color);

				glBindVertexArray((GLuint)SphereEntry->Mesh.Handle);
				glDrawElements(GL_TRIANGLES, SphereEntry->Mesh.ElementCount, GL_UNSIGNED_INT, 0);
				UseProgramEnd(&OpenGL.StaticMeshProg);
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

		OpenGLBindTex(GL_TEXTURE_2D, GL_TEXTURE0, OpenGL.Prepass.Color);
		OpenGLBindTex(GL_TEXTURE_2D, GL_TEXTURE1, OpenGL.BlurBlit[1].Color);
		OpenGLBindTex(GL_TEXTURE_2D, GL_TEXTURE2, OpenGL.MainFB.Color);

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