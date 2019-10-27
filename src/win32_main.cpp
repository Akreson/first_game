#include <Windows.h>
#include <gl/gl.h>

// NOTE: Incude for test. Must be remove later
#include <stdio.h>
#include <stdlib.h>

#include "platform.h"
#include "string.h"

// TODO: Set as different translation unit and compile as dll?
#include "editor_game.cpp"

#include "opengl.cpp"

// TODO: Delete form platform layer
#include "math.cpp"

typedef HGLRC WINAPI wgl_create_context_attribs_arb(HDC hDC, HGLRC hShareContext,
	const int *attribList);

typedef BOOL WINAPI wgl_get_pixel_format_attrib_iv_arb(HDC hdc,
	int iPixelFormat,
	int iLayerPlane,
	UINT nAttributes,
	const int *piAttributes,
	int *piValue);

typedef BOOL WINAPI wgl_get_pixel_format_attrib_fv_arb(HDC hdc,
	int iPixelFormat,
	int iLayerPlane,
	UINT nAttributes,
	const int *piAttributes,
	FLOAT *piValue);

typedef BOOL WINAPI wgl_choose_pixel_format_arb(HDC hdc,
	const int *piAttribList,
	const FLOAT *pfAttribFList,
	UINT nMaxFormats,
	int *piFormats,
	UINT *nNumFormats);

typedef BOOL WINAPI wgl_swap_interval_ext(int interval);
typedef const char * WINAPI wgl_get_extension_string_ext(void);

global_variable wgl_create_context_attribs_arb *wglCreateContextAttribsARB;
global_variable wgl_choose_pixel_format_arb *wglChoosePixelFormatARB;
global_variable wgl_swap_interval_ext *wglSwapIntervalEXT;
global_variable wgl_get_extension_string_ext *wglGetExtensionStringEXT;

global_variable b32 GlobalRunning;
global_variable WINDOWPLACEMENT GlobalWindowPosition = {sizeof(GlobalWindowPosition)};

LRESULT
Win32MainWindowCallback(HWND Window, UINT Message, WPARAM WParam, LPARAM  LParam)
{
	LRESULT Result = 0;

	switch (Message)
	{
		case WM_CLOSE:
		{
			GlobalRunning = false;
		} break;

		case WM_DESTROY:
		{
			GlobalRunning = false;
		} break;

		default:
		{
			Result = DefWindowProc(Window, Message, WParam, LParam);;
		} break;
	}

	return Result;
}

inline void
Win32ProcessButtonState(game_button_state *Button, b32 IsDown)
{
	if (Button->EndedDown != IsDown)
	{
		Button->EndedDown = IsDown;
	}
}

void
ToggleFullscreen(HWND Window)
{
	// NOTE: This foloow Raymond Chan's prescription
	// for fullscreen toggling, see:
	// https://blogs.msdn.microsoft.com/oldnewthing/20100412-00/?p=14353/

	DWORD Style = GetWindowLong(Window, GWL_STYLE);
	if (Style & WS_OVERLAPPEDWINDOW)
	{
		MONITORINFO MonitorInfo = {sizeof(MonitorInfo)};
		if (GetWindowPlacement(Window, &GlobalWindowPosition) &&
			GetMonitorInfo(MonitorFromWindow(Window, MONITOR_DEFAULTTOPRIMARY), &MonitorInfo))
		{
			SetWindowLong(Window, GWL_STYLE, Style & ~WS_OVERLAPPEDWINDOW);
			SetWindowPos(Window, HWND_TOP,
				MonitorInfo.rcMonitor.left, MonitorInfo.rcMonitor.top,
				MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left,
				MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top,
				SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		}
	}
	else
	{
		SetWindowLong(Window, GWL_STYLE, Style | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(Window, &GlobalWindowPosition);
		SetWindowPos(Window, NULL, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
			SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
	}
}

// TODO: Set Keys
void
Win32ProcessMessage(game_input *GameInput)
{
	MSG Message;

	while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
	{
		switch (Message.message)
		{
			/*case WM_QUIT:
			{
				GlobalRunning = false;
			} break;*/

			case WM_SYSKEYDOWN:
			case WM_SYSKEYUP:
			case WM_KEYDOWN:
			case WM_KEYUP:
			{
				u32 VKCode = (u32)Message.wParam;
				b32 AltIsDown = Message.lParam & (1 << 29);
				b32 IsDown = ((Message.lParam & (1 << 31)) == 0);
				b32 WasDown = ((Message.lParam & (1 << 30)) != 0);

				switch (VKCode)
				{
					case 'P':
					{
					} break;
				}
			} break;

			default:
			{
				TranslateMessage(&Message);
				DispatchMessage(&Message);
			} break;
		}
	}
}


// TODO: Cange memory location to pre-allocated memory

struct debug_read_file
{
	void *Content;
	u32 Size;
};

debug_read_file
Win32ReadFile(const char *FileName)
{
	debug_read_file Result;
	HANDLE FileHandle = CreateFile(FileName, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);

	if (FileHandle != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER FileSize;
		if (GetFileSizeEx(FileHandle, &FileSize))
		{
			Result.Size = (u32)FileSize.QuadPart;
			Result.Content = VirtualAlloc(0, FileSize.QuadPart, MEM_COMMIT, PAGE_READWRITE);
			if (Result.Content)
			{
				DWORD BytesRead;
				if (!ReadFile(FileHandle, Result.Content, Result.Size, &BytesRead, 0) &&
					(BytesRead != Result.Size))
				{
					VirtualFree(Result.Content, 0, MEM_RELEASE);
					// TODO: Handle error
				}
			}
			else
			{
				Assert(0);
				// TODO: Handle error
			}
		}
		else
		{
			Assert(0);
			// TODO: Handle error
		}
	}
	else
	{
		Assert(0);
		// TODO: Handle error
	}

	return Result;
}

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
			WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
			WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
			WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
			WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
			WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
			0
		};

		SuggestedPixelFormatIndex = wglChoosePixelFormatARB(WindowDC, AttribList, 0, 1,
			&SuggestedPixelFormatIndex, &NumFormats);
	}

	if (!NumFormats)
	{
		SuggestedPixelFormat.nSize = sizeof(SuggestedPixelFormat);
		SuggestedPixelFormat.nVersion = 1;
		SuggestedPixelFormat.dwFlags = PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW;
		SuggestedPixelFormat.iPixelType = PFD_TYPE_RGBA;
		SuggestedPixelFormat.cColorBits = 32;
		SuggestedPixelFormat.cAlphaBits = 8;

		SuggestedPixelFormatIndex = ChoosePixelFormat(WindowDC, &SuggestedPixelFormat);
	}

	DescribePixelFormat(WindowDC, SuggestedPixelFormatIndex, sizeof(SuggestedPixelFormat),
		&SuggestedPixelFormat);
	SetPixelFormat(WindowDC, SuggestedPixelFormatIndex, &SuggestedPixelFormat);
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
Win32InitOpenGL(HDC WindowDC)
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
		Win32LoadOpenGLFunction(glDeleteBuffers);

		Win32LoadOpenGLFunction(glEnableVertexAttribArray);
		Win32LoadOpenGLFunction(glDisableVertexAttribArray);
		Win32LoadOpenGLFunction(glVertexAttribPointer);

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

#ifdef DEVELOP_MODE
	glDebugMessageCallback(OpenGLMessageDebugCallback, 0);
#endif

	OpenGLInit();

	return OpenGLRC;
}

int CALLBACK
WinMain(HINSTANCE Instance,
	HINSTANCE PrevInstance,
	LPSTR commandLine,
	int ShowCode)
{
	WNDCLASSA WindowClass = {};

	WindowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	WindowClass.lpfnWndProc = Win32MainWindowCallback;
	WindowClass.hInstance = Instance;
	WindowClass.hCursor = LoadCursor(0, IDC_ARROW);
	WindowClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	WindowClass.lpszClassName = "CrossPlaneWindow";

	if (RegisterClass(&WindowClass))
	{
		HWND Window = CreateWindowEx(
			0,
			WindowClass.lpszClassName,
			"Cross Plane",
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			0,
			0,
			Instance,
			0);

		if (Window)
		{
			//ToggleFullscreen(Window);
			GlobalRunning = true;
			HDC WindowDC = GetDC(Window);
			HGLRC OpenGLRC = Win32InitOpenGL(WindowDC);
			ShowWindow(Window, SW_SHOW);
			
			game_input GameInput = {};

			//
			// NOTE: Some OpenGL init
			//
			// TODO: Remove
			//

			debug_read_file FontAssetFile = Win32ReadFile("data//font.edg");
			bitmap_info *FontBitmapInfo = (bitmap_info *)FontAssetFile.Content;

			GLuint VBO, VAO;
			glGenVertexArrays(1, &VAO);
			glGenBuffers(1, &VBO);

			glBindVertexArray(VAO);

			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);

			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(1);

			glBindVertexArray(0);

			GLuint FontTextureIndex;
			glGenTextures(1, &FontTextureIndex);
			glBindTexture(GL_TEXTURE_2D, FontTextureIndex);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FontBitmapInfo->Width, FontBitmapInfo->Height,
				0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *)((u8 *)FontAssetFile.Content + sizeof(bitmap_info)));

			glUseProgram(OpenGL.ProgramID);

			glUniform1i(glGetUniformLocation(OpenGL.ProgramID, "Texture1"), 0);

			while (GlobalRunning)
			{
				// NOTE: Input Process

				Win32ProcessMessage(&GameInput);

				POINT MouseP;
				GetCursorPos(&MouseP);
				ScreenToClient(Window, &MouseP);
				GameInput.MouseX = (f32)MouseP.x;
				GameInput.MouseY = (f32)(1920 - MouseP.y); // TODO: Change width be not hardcode
				GameInput.MouseZ = 0; // TODO: Support mousewheel


				DWORD Win32MappedMouseID[] =
				{
					VK_LBUTTON,
					VK_RBUTTON,
					VK_MBUTTON,
					VK_XBUTTON1,
					VK_XBUTTON2,
				};

				for (u32 ButtonIndex = 0;
					ButtonIndex < PlatformMouseButton_Count;
					++ButtonIndex)
				{
					Win32ProcessButtonState(&GameInput.MouseButtons[ButtonIndex],
						GetKeyState(Win32MappedMouseID[ButtonIndex]) & (1 << 15));
				}
				
				// NOTE: Render Test
				glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT);

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, FontTextureIndex);

				glUseProgram(OpenGL.ProgramID);

				glBindVertexArray(VAO);
				glDrawArrays(GL_TRIANGLES, 0, 6);

				HDC DeviceContext = GetDC(Window);
				SwapBuffers(DeviceContext);
			}
		}
	}
}