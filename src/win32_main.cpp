#include <Windows.h>
#include <gl/gl.h>

// NOTE: Incude for test. Must be remove later
#include <stdio.h>
#include <stdlib.h>

#include "platform.h"

// TODO: Set as different translation unit and compile as dll?
#include "editor_game.cpp"

#include "opengl.cpp"
#include "win32_opengl.cpp"

global b32 GlobalRunning;
global u64 GlobalPerfCountFrequency; 
global WINDOWPLACEMENT GlobalWindowPosition = {sizeof(GlobalWindowPosition)};

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

void *
Win32AllocateMemory(memory_index Size)
{
	void *Result = VirtualAlloc(0, Size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	return Result;
}

void
Win32DeallocateMemory(void *Ptr)
{
	if (Ptr)
	{
		VirtualFree(Ptr, 0, MEM_RELEASE);
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
			case WM_QUIT:
			{
				GlobalRunning = false;
			} break;

			case WM_SYSKEYDOWN:
			case WM_SYSKEYUP:
			case WM_KEYDOWN:
			case WM_KEYUP:
			{
				u32 VKCode = (u32)Message.wParam;
				b32 AltIsDown = Message.lParam & (1 << 29);
				b32 IsDown = !IsBitSet(Message.lParam, 31);
				b32 WasDown = IsBitSet(Message.lParam, 30);

				switch (VKCode)
				{
					case 'P':
					{
					} break;
				}

				if (IsDown)
				{
					if ((VKCode == VK_F4) && AltIsDown)
					{
						GlobalRunning = false;
					}
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
DEBUGWin32ReadFile(const char *FileName)
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

PLATFORM_GET_FILE_HANDLER_FOR_FILE(Win32GetFileHandlerForFile)
{
	platform_file_handler Result = {};
	char *FileName = nullptr;

	switch (FileType)
	{
		case FileType_FontFile:
		{
			FileName = (char *)"data//font.edg";
		} break;
	}

	if (FileName)
	{
		HANDLE Handle = CreateFile(FileName, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
		if (Handle != INVALID_HANDLE_VALUE)
		{
			Result.Handler = (void *)Handle;
		}
	}

	return Result;
}

PLATFORM_GET_FILE_SIZE(Win32GetFileSize)
{
	u32 Result = 0;

	if (!FileHandler->Errors)
	{
		HANDLE Handle = (HANDLE)FileHandler->Handler;

		LARGE_INTEGER FileSize;
		if (GetFileSizeEx(Handle, &FileSize))
		{
			Result = (u32)FileSize.QuadPart;
		}
		else
		{
			FileHandler->Errors = true;
		}
	}

	return Result;
}

PLATFORM_READ_FILE(Win32ReadFile)
{
	if (!FileHandler->Errors)
	{
		HANDLE Handle = (HANDLE)FileHandler->Handler;

		DWORD BytesRead;
		if (!ReadFile(Handle, Dest, Size, &BytesRead, 0) &&
			(BytesRead != Size))
		{
			FileHandler->Errors = true;
		}
	}
}

void
Win32DisplayRenderCommands(HWND Window, game_render_commands *RenderCommands)
{
	OpenGLRenderCommands(RenderCommands);

	HDC DeviceContext = GetDC(Window);
	SwapBuffers(DeviceContext);
}

inline void
Win32GetScreenDim(HWND Window, f32 *Width, f32 *Height)
{
	RECT ScreenDim;
	GetWindowRect(Window, &ScreenDim);
	u32 iWidth = ScreenDim.right - ScreenDim.left;
	u32 iHeight = ScreenDim.bottom - ScreenDim.top;

	*Width = (f32)iWidth;
	*Height = (f32)iHeight;
}

inline LARGE_INTEGER
Win32GetClock(void)
{
	LARGE_INTEGER Result;
	QueryPerformanceCounter(&Result);

	return(Result);
}

inline f32
Win32GetSecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End)
{
	f32 Result = (f32)(End.QuadPart - Start.QuadPart) / (f32)GlobalPerfCountFrequency;
	return Result;
}

int CALLBACK
WinMain(HINSTANCE Instance,
	HINSTANCE PrevInstance,
	LPSTR commandLine,
	int ShowCode)
{
	LARGE_INTEGER PerCountFrequencyResult;
	QueryPerformanceFrequency(&PerCountFrequencyResult);
	GlobalPerfCountFrequency = PerCountFrequencyResult.QuadPart;

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
			GlobalRunning = true;
			ToggleFullscreen(Window);
			HDC WindowDC = GetDC(Window);
			
			f32 ScreenWidth, ScreenHeight;
			Win32GetScreenDim(Window, &ScreenWidth, &ScreenHeight);

			HGLRC OpenGLRC = Win32InitOpenGL(WindowDC, ScreenWidth, ScreenHeight);

			ShowWindow(Window, SW_SHOW);
			
			game_memory GameMemory = {};
			game_input GameInput = {};

			u32 PushBufferSize = MiB(2);
			void *PushBufferBase = Win32AllocateMemory(PushBufferSize);
			u32 VertexBufferSize = MiB(5);
			void *VertexBuffer = Win32AllocateMemory(VertexBufferSize);

			GameMemory.GameStorageSize = MiB(20);
			GameMemory.GameStorage = Win32AllocateMemory(GameMemory.GameStorageSize);
			GameMemory.EditorStorageSize = MiB(50);
			GameMemory.EditorStorage = Win32AllocateMemory(GameMemory.EditorStorageSize);

			PlatformAPI.GetFileHandlerForFile = Win32GetFileHandlerForFile;
			PlatformAPI.ReadFile = Win32ReadFile;
			PlatformAPI.GetFileSize = Win32GetFileSize;
			PlatformAPI.AllocateTexture = OpenGLAllocateTexture;
			PlatformAPI.DeallocateTexture = OpenGLDeallocateTexture;

			LARGE_INTEGER LastCounter = Win32GetClock();

			while (GlobalRunning)
			{
				// NOTE: Input Process
				Win32ProcessMessage(&GameInput);

				POINT MouseP;
				GetCursorPos(&MouseP);
				ScreenToClient(Window, &MouseP);
				// TODO: Store in normalize form?
				GameInput.MouseX = (f32)MouseP.x;
				GameInput.MouseY = (f32)((ScreenHeight - 1) - MouseP.y);
				GameInput.MouseZ = 0; // TODO: Support mousewheel

				// TODO: Clean up
				GameInput.Alt.TransionState = 0;
				GameInput.Shift.TransionState = 0;
				GameInput.Ctrl.TransionState = 0;
				GameInput.Tab.TransionState = 0;
				PlatformProcessButtonState(&GameInput.Alt, IsBitSet(GetKeyState(VK_MENU), 15));
				PlatformProcessButtonState(&GameInput.Shift, IsBitSet(GetKeyState(VK_SHIFT), 15));
				PlatformProcessButtonState(&GameInput.Ctrl, IsBitSet(GetKeyState(VK_CONTROL), 15));
				PlatformProcessButtonState(&GameInput.Tab, IsBitSet(GetKeyState(VK_TAB), 15));

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
					GameInput.MouseButtons[ButtonIndex].TransionState = 0;
					PlatformProcessButtonState(&GameInput.MouseButtons[ButtonIndex],
						IsBitSet(GetKeyState(Win32MappedMouseID[ButtonIndex]), 15));
				}

				// NOTE: Update and render
				game_render_commands RenderCommands = {};
				RenderCommands.ScreenDim = V2(ScreenWidth, ScreenHeight);
				RenderCommands.PushBufferBase = (u8 *)PushBufferBase;
				RenderCommands.MaxPushBufferSize = PushBufferSize;
				RenderCommands.MaxVertexBufferSize = VertexBufferSize;
				RenderCommands.VertexBufferBase = (u8 *)VertexBuffer;

				UpdateAndRender(&GameMemory, &GameInput, &RenderCommands);

				Win32DisplayRenderCommands(Window, &RenderCommands);

				LARGE_INTEGER EndCounter = Win32GetClock();
				GameInput.PrevFrameTime = Win32GetSecondsElapsed(LastCounter, EndCounter);
				LastCounter = EndCounter;
			}
		}
	}
}