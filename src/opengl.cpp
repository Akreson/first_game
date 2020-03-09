#include "opengl.h"

global opengl_render_info OpenGL;

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
	layout (location = 1) in vec4 aBarCoord;

	uniform mat4 Proj;
	uniform mat4 ModelTransform;

	out vec4 BarCoord;

	void main()
	{
		BarCoord = aBarCoord;
		gl_Position = Proj * ModelTransform * vec4(aPos, 1.0f);
	}

	)FOO";

	// TODO: Set model color?
	// TODO: Compute color for selected edge in right way
	const char *ModelFragmentCode = R"FOO(
	out vec4 FragColor;

	uniform vec4 Color;
	uniform vec3 EdgeColor;

	in vec4 BarCoord;

	// TODO: Decide which method use
	/*
	// NOTE: In this case, the selected edge is defined as the opposite to vertex,
	// same schema as with BarCoords
	float when_eq(float x, float y) {
	  return 1.0 - abs(sign(x - y));
	}
	
	float A = when_eq(MinD - BarCoord.w, 0);
	*/

	float edgeFactor(){
		vec3 dBarCoord = fwidth(BarCoord.xyz);
		vec3 a3 = smoothstep(vec3(0.0), dBarCoord*2.5, BarCoord.xyz);
		float Result = min(min(a3.x, a3.y), a3.z);
		return Result;
	}

	void main()
	{
		//vec3 EdgeColor = vec3(0.17, 0.5, 0.8);
		vec3 SelectEdgeColor = vec3(0.86f, 0.65, 0.2);
#if 1
		float MinD = min(min(BarCoord.x, BarCoord.y), BarCoord.z);
		float dMinD = fwidth(MinD);
		float Thickness = dMinD*2.5;
		float Factor = smoothstep(0, Thickness, MinD);
#else
		float Factor = edgeFactor();
#endif
		float InvFactor = 1.0 - Factor;
		
		float A = step(1.0 - dMinD, BarCoord.w);
		vec3 FinalEdgeColor = mix(EdgeColor, SelectEdgeColor, A);

		FragColor = vec4(mix(Color.xyz, EdgeColor, InvFactor), 1.0);
	}
	)FOO";

	OpenGL.ModelProgramID = OpenGLCreateProgram((GLchar *)HeaderCode, (GLchar *)ModelVertexCode, (GLchar *)ModelFragmentCode);

	glUseProgram(OpenGL.ModelProgramID);
	glGenVertexArrays(1, &OpenGL.ModelVAO);
	glGenBuffers(1, &OpenGL.ModelVBO);

	OpenGL.ModelColorID = glGetUniformLocation(OpenGL.ModelProgramID, "Color");
	OpenGL.ModelProjID = glGetUniformLocation(OpenGL.ModelProgramID, "Proj");
	OpenGL.ModelTransformID = glGetUniformLocation(OpenGL.ModelProgramID, "ModelTransform");
	
	// TODO: Use for debuging, delete later
	OpenGL.ModelEdgeColor = glGetUniformLocation(OpenGL.ModelProgramID, "EdgeColor");

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
				glUniformMatrix4fv(OpenGL.BitmapProjID, 1, GL_FALSE, &Commands->OrthoProj.Forward.E[0][0]);

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
				SetTranslation(&ModelTransform, FaceEntry->Offset);

				glUniform4f(OpenGL.ModelColorID,
					FaceEntry->Color.r, FaceEntry->Color.g, FaceEntry->Color.b, FaceEntry->Color.a);
				glUniformMatrix4fv(OpenGL.ModelProjID, 1, GL_FALSE, &Commands->PersProj.Forward.E[0][0]);
				glUniformMatrix4fv(OpenGL.ModelTransformID, 1, GL_FALSE, &ModelTransform.E[0][0]);

				// TODO: Delete later
				glUniform3f(OpenGL.ModelEdgeColor,
					FaceEntry->EdgeColor.r, FaceEntry->EdgeColor.g, FaceEntry->EdgeColor.b);

				glBindVertexArray(OpenGL.VertexBufferVAO);
				glBindBuffer(GL_ARRAY_BUFFER, OpenGL.VertexBufferVBO);

				u32 OffsetInBytes = FaceEntry->VertexBufferOffset * sizeof(f32);

				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(f32), (void*)0);
				glEnableVertexAttribArray(1);
				glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(f32), (void*)(sizeof(f32) * 3));

				glDrawArrays(GL_TRIANGLES, FaceEntry->VertexBufferOffset / 7, 6);
			} break;

			// TODO: Remove
			case RenderEntryType_render_entry_model:
			{
				glUseProgram(OpenGL.ModelProgramID);

				render_entry_model *ModelEntry = (render_entry_model *)(Commands->PushBufferBase + BufferOffset);
				BufferOffset += sizeof(render_entry_model);

				m4x4 ModelTransform = Identity();
				SetTranslation(&ModelTransform, ModelEntry->Offset);

				glUniform4f(OpenGL.ModelColorID,
					ModelEntry->Color.r, ModelEntry->Color.g, ModelEntry->Color.b, ModelEntry->Color.a);
				glUniformMatrix4fv(OpenGL.ModelProjID, 1, GL_FALSE, &Commands->PersProj.Forward.E[0][0]);
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