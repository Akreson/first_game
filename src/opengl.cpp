#include "opengl.h"

global opengl_renderer_info OpenGL;
global const char *SharedHeaderCode;

#include "opengl_program.cpp"

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
	CompileRotateToolProgram(&OpenGL.RotateTools);
	CompileTrinModelColorPassProgram(&OpenGL.TrinModelColorPassProg);

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
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(render_model_face_vertex), (void*)(sizeof(f32) * 6));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(render_model_face_vertex), (void*)(sizeof(f32) * 9));
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(render_model_face_vertex), (void*)(sizeof(f32) * 12));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	//-----

	// NOTE: Init triangle vertex buffer VBO
	glGenVertexArrays(1, &OpenGL.TrinBufferVAO);
	glGenBuffers(1, &OpenGL.TrinBufferVBO);

	glBindVertexArray(OpenGL.TrinBufferVAO);
	glBindBuffer(GL_ARRAY_BUFFER, OpenGL.TrinBufferVBO);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(render_triangle_vertex), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(render_triangle_vertex), (void*)(sizeof(f32) * 3));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(render_triangle_vertex), (void*)(sizeof(f32) * 6));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	//-----
	
	// NOTE: Set full screen VAO
	const float Vertices[] = {
		// first triangle
		-1.0f, -1.0f, 0.0f, 0.0f,// bottom left
		 1.0f, -1.0f, 1.0f, 0.0f,// bottom right
		 1.0f,  1.0f, 1.0f, 1.0f,// top right
		// second triangle
		-1.0f, -1.0f, 0.0f, 0.0f,// bottom left
		 1.0f,  1.0f, 1.0f, 1.0f,// top right
		-1.0f,  1.0f, 0.0f, 1.0f,// top left 
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
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	glBindVertexArray(OpenGL.VertexBufferVAO);
	glBindBuffer(GL_ARRAY_BUFFER, OpenGL.VertexBufferVBO);
	glBufferData(GL_ARRAY_BUFFER, Commands->VertexBufferSize, (GLvoid *)Commands->VertexBufferBase, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glBindVertexArray(OpenGL.TrinBufferVAO);
	glBindBuffer(GL_ARRAY_BUFFER, OpenGL.TrinBufferVBO);
	glBufferData(GL_ARRAY_BUFFER, Commands->TriangleBufferSize, (GLvoid *)Commands->TriangleBufferBase, GL_STREAM_DRAW);
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

				glBindVertexArray(OpenGL.TrinBufferVAO);
				glBindBuffer(GL_ARRAY_BUFFER, OpenGL.TrinBufferVBO);

				UseProgramBegin(&OpenGL.BitmapProg, BitmapEntry->Color, &Commands->OrthoProj.Forward);
				OpenGLBindTex(GL_TEXTURE_2D, GL_TEXTURE0, (GLuint)BitmapEntry->Texture.Handle);
				glDrawArrays(GL_TRIANGLES, BitmapEntry->TrinBuffOffset / sizeof(render_triangle_vertex), 6);

				glBindTexture(GL_TEXTURE_2D, 0);
				glBindBuffer(GL_ARRAY_BUFFER, 0);
				glBindVertexArray(0);

				UseProgramEnd(&OpenGL.BitmapProg);
			} break;

			case RenderEntryType_render_entry_model:
			{
				glEnable(GL_CULL_FACE);
				//glCullFace(GL_BACK);
				//glFrontFace(GL_CCW);

				render_entry_model *ModelEntry = (render_entry_model *)Data;
				BufferOffset += sizeof(render_entry_model);

				m4x4 ModelTransform = Identity();
				SetTranslation(&ModelTransform, ModelEntry->Offset);

				glBindVertexArray(OpenGL.VertexBufferVAO);
				glBindBuffer(GL_ARRAY_BUFFER, OpenGL.VertexBufferVBO);

				UseProgramBegin(&OpenGL.ModelProg, ModelEntry->Color, ModelEntry->EdgeColor,
					&Commands->ForwardPersCamera, &ModelTransform);

				glDrawArrays(GL_TRIANGLES,
					ModelEntry->StartOffset / sizeof(render_model_face_vertex),
					ModelEntry->ElementCount);

				glBindBuffer(GL_ARRAY_BUFFER, 0);
				glBindVertexArray(0);

				UseProgramEnd(&OpenGL.ModelProg);
				glDisable(GL_CULL_FACE);
			} break;

			case RenderEntryType_render_entry_trin_model:
			{
				render_entry_trin_model *TrinModel = (render_entry_trin_model *)Data;
				BufferOffset += sizeof(render_entry_trin_model);
			
				m4x4 ModelTransform = Identity();
				SetTranslation(&ModelTransform, TrinModel->Pos);

				glBindVertexArray(OpenGL.TrinBufferVAO);
				glBindBuffer(GL_ARRAY_BUFFER, OpenGL.TrinBufferVBO);

				UseProgramBegin(&OpenGL.TrinModelColorPassProg, &Commands->ForwardPersCamera, &ModelTransform);
				glDrawArrays(GL_TRIANGLES,
					TrinModel->StartOffset / sizeof(render_triangle_vertex),
					TrinModel->ElementCount);

				glBindBuffer(GL_ARRAY_BUFFER, 0);
				glBindVertexArray(0);

				UseProgramEnd(&OpenGL.BitmapProg);
			} break;

			case RenderEntryType_render_entry_tool_rotate:
			{
				glEnable(GL_CULL_FACE);
				//glDepthMask(false);

				render_entry_tool_rotate *RotateTool = (render_entry_tool_rotate *)Data;
				BufferOffset += sizeof(render_entry_tool_rotate);

				m4x4 Scale = ScaleMat(RotateTool->Scale);
				m4x4 Traslate = Identity();
				SetTranslation(&Traslate, RotateTool->Pos);

				m4x4 Transform = Scale * Traslate;
				UseProgramBegin(&OpenGL.RotateTools, &Commands->PersProj.Forward,
					&Commands->CameraTransform.Forward, &Transform, RotateTool);
				glBindVertexArray((GLuint)RotateTool->Mesh.Handle);
				glDrawElements(GL_TRIANGLES, RotateTool->Mesh.ElementCount, GL_UNSIGNED_INT, 0);
				UseProgramEnd(&OpenGL.StaticMeshProg);

				//glDepthMask(true);
				glDisable(GL_CULL_FACE);
			} break;

			case RenderEntryType_render_entry_static_mesh:
			{
				render_entry_static_mesh *MeshEntry = (render_entry_static_mesh *)Data;
				BufferOffset += sizeof(render_entry_static_mesh);

				m4x4 I = Identity();

				UseProgramBegin(&OpenGL.StaticMeshProg,
					&Commands->ForwardPersCamera, &I, MeshEntry->Color);

				glBindVertexArray((GLuint)MeshEntry->Mesh.Handle);
				glDrawElements(GL_TRIANGLES, MeshEntry->Mesh.ElementCount, GL_UNSIGNED_INT, 0);
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

				UseProgramBegin(&OpenGL.ModelColorPassProg, &Commands->ForwardPersCamera, &ModelTransform);
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

	// TODO: Add separate queue for post-process?
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