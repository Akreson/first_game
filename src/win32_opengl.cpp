#include "win32_opengl.h"

void
Win32SetPixelFormat(HDC WindowDC)
{
	PIXELFORMATDESCRIPTOR SuggestedPixelFormat = {};
	int SuggestedPixelFormatIndex = 0;
	GLuint NumFormats = 0;

	if (wglChoosePixelFormatARB)
	{
		int AttribList[] =
		{
			WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
			WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
			WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
			WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
			WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
			WGL_RED_BITS_ARB, 8,
			WGL_GREEN_BITS_ARB, 8,
			WGL_BLUE_BITS_ARB, 8,
			0
		};

		wglChoosePixelFormatARB(WindowDC, AttribList, 0, 1,
			&SuggestedPixelFormatIndex, &NumFormats);
	}

	if (!NumFormats)
	{
		SuggestedPixelFormat.nSize = sizeof(SuggestedPixelFormat);
		SuggestedPixelFormat.nVersion = 1;
		SuggestedPixelFormat.dwFlags = PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW;
		SuggestedPixelFormat.iPixelType = PFD_TYPE_RGBA;
		SuggestedPixelFormat.iLayerType = PFD_MAIN_PLANE;
		SuggestedPixelFormat.cColorBits = 32;
		SuggestedPixelFormat.cAlphaBits = 8;
		SuggestedPixelFormat.cDepthBits = 24;

		SuggestedPixelFormatIndex = ChoosePixelFormat(WindowDC, &SuggestedPixelFormat);
	}

	DescribePixelFormat(WindowDC, SuggestedPixelFormatIndex, sizeof(SuggestedPixelFormat),
		&SuggestedPixelFormat);
	if (!SetPixelFormat(WindowDC, SuggestedPixelFormatIndex, &SuggestedPixelFormat))
	{
		Assert(0);
	}
}

internal void
Win32LoadOpenGLExtenssion(void)
{
	WNDCLASSA WindowClass = {};

	WindowClass.lpfnWndProc = DefWindowProcA;
	WindowClass.hInstance = GetModuleHandle(0);
	WindowClass.lpszClassName = "TempWindowClass";

	if (RegisterClass(&WindowClass))
	{
		HWND Window = CreateWindowEx(
			0,
			WindowClass.lpszClassName,
			"Cross Plane",
			0,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			0,
			0,
			WindowClass.hInstance,
			0);


		HDC WindowDC = GetDC(Window);
		Win32SetPixelFormat(WindowDC);
		HGLRC OpenGLRC = wglCreateContext(WindowDC);
		if (wglMakeCurrent(WindowDC, OpenGLRC))
		{
			wglCreateContextAttribsARB = (wgl_create_context_attribs_arb *)wglGetProcAddress("wglCreateContextAttribsARB");
			wglChoosePixelFormatARB = (wgl_choose_pixel_format_arb *)wglGetProcAddress("wglChoosePixelFormatARB");
			wglSwapIntervalEXT = (wgl_swap_interval_ext *)wglGetProcAddress("wglSwapIntervalEXT");
			wglGetExtensionStringEXT = (wgl_get_extension_string_ext *)wglGetProcAddress("wglGetExtensionStringEXT");
#if 0
			if (wglGetExtensionStringEXT)
			{
				char *Extension = (char *)wglGetExtensionStringEXT();
				char *At = Extension;
			}
#endif
			wglMakeCurrent(0, 0);
		}

		wglDeleteContext(OpenGLRC);
		ReleaseDC(Window, WindowDC);
		DestroyWindow(Window);
	}
}

int Win32OpenGLContextAttribs[] =
{
	WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
	WGL_CONTEXT_MINOR_VERSION_ARB, 4,
	WGL_CONTEXT_FLAGS_ARB, 0
#if DEVELOP_MODE
	| WGL_CONTEXT_DEBUG_BIT_ARB
#endif
	,
	WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
	0,
};

#define Win32LoadOpenGLFunction(Name) {Name = (type_##Name *)wglGetProcAddress(#Name); Assert(Name)}

internal HGLRC
Win32InitOpenGL(HDC WindowDC, f32 ScreenWidth, f32 ScreenHeight)
{
	HGLRC OpenGLRC = 0;

	Win32LoadOpenGLExtenssion();

	if (wglCreateContextAttribsARB)
	{
		Win32SetPixelFormat(WindowDC);
		OpenGLRC = wglCreateContextAttribsARB(WindowDC, 0, Win32OpenGLContextAttribs);
	}

	// TODO: Maintain
	if (!OpenGLRC)
	{
		Assert(!"Not maintained");
	}

	if (wglMakeCurrent(WindowDC, OpenGLRC))
	{
		opengl_info OpenGLInfo = OpenGLGetInfo();

		if (OpenGLInfo.GL_EXT_framebuffer_object)
		{
			Win32LoadOpenGLFunction(glBindFramebuffer);
			Win32LoadOpenGLFunction(glGenFramebuffers);
			Win32LoadOpenGLFunction(glFramebufferTexture2D);
			Win32LoadOpenGLFunction(glDeleteFramebuffers);
			Win32LoadOpenGLFunction(glCheckFramebufferStatus);

			Win32LoadOpenGLFunction(glGenRenderbuffers);
			Win32LoadOpenGLFunction(glBindRenderbuffer);
			Win32LoadOpenGLFunction(glRenderbufferStorage);
			Win32LoadOpenGLFunction(glFramebufferRenderbuffer);
			Win32LoadOpenGLFunction(glBlitFramebuffer);
		}
		else
		{
			Assert(0);
		}

		Win32LoadOpenGLFunction(glCreateProgram);
		Win32LoadOpenGLFunction(glDeleteProgram);
		Win32LoadOpenGLFunction(glLinkProgram);
		Win32LoadOpenGLFunction(glCreateShader);
		Win32LoadOpenGLFunction(glCompileShader);
		Win32LoadOpenGLFunction(glShaderSource);
		Win32LoadOpenGLFunction(glDeleteShader);
		Win32LoadOpenGLFunction(glAttachShader);
		Win32LoadOpenGLFunction(glDetachShader);
		Win32LoadOpenGLFunction(glUseProgram);

		Win32LoadOpenGLFunction(glActiveTexture);

		Win32LoadOpenGLFunction(glUniform1i);
		Win32LoadOpenGLFunction(glUniform2i);
		Win32LoadOpenGLFunction(glUniform3i);
		Win32LoadOpenGLFunction(glUniform4i);
		Win32LoadOpenGLFunction(glUniform1f);
		Win32LoadOpenGLFunction(glUniform2f);
		Win32LoadOpenGLFunction(glUniform3f);
		Win32LoadOpenGLFunction(glUniform4f);
		Win32LoadOpenGLFunction(glUniform2fv);
		Win32LoadOpenGLFunction(glUniform3fv);
		Win32LoadOpenGLFunction(glUniform4fv);
		Win32LoadOpenGLFunction(glUniformMatrix2fv);
		Win32LoadOpenGLFunction(glUniformMatrix3fv);
		Win32LoadOpenGLFunction(glUniformMatrix4fv);

		Win32LoadOpenGLFunction(glGetUniformLocation);

		Win32LoadOpenGLFunction(glBindVertexArray);
		Win32LoadOpenGLFunction(glDeleteVertexArrays);
		Win32LoadOpenGLFunction(glGenVertexArrays);
		Win32LoadOpenGLFunction(glGenBuffers);
		Win32LoadOpenGLFunction(glBindBuffer);
		Win32LoadOpenGLFunction(glBufferData);
		Win32LoadOpenGLFunction(glBufferSubData);
		Win32LoadOpenGLFunction(glDeleteBuffers);

		Win32LoadOpenGLFunction(glEnableVertexAttribArray);
		Win32LoadOpenGLFunction(glDisableVertexAttribArray);
		Win32LoadOpenGLFunction(glVertexAttribPointer);
		Win32LoadOpenGLFunction(glVertexAttribIPointer);

		Win32LoadOpenGLFunction(glGetShaderiv);
		Win32LoadOpenGLFunction(glGetProgramiv);
		Win32LoadOpenGLFunction(glGetProgramInfoLog);
		Win32LoadOpenGLFunction(glGetShaderInfoLog);
		Win32LoadOpenGLFunction(glValidateProgram);

		Win32LoadOpenGLFunction(glDebugMessageCallback);

		if (wglSwapIntervalEXT)
		{
			wglSwapIntervalEXT(1);
		}
	}

	OpenGLInit(ScreenWidth, ScreenHeight);

	return OpenGLRC;
}