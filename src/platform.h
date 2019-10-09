#pragma once

#include <stdint.h>
#include <stddef.h>

#define internal static
#define global_variable static

#define Pi32 3.14159265359f
#define Tau32 6.2831853071f

#define KiB(Value) (Value * 1024)
#define MiB(Value) (KiB(Value) * 1024))
#define GiB(Value) (MiB(Value) * 1024)

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t s8;

typedef float f32;
typedef double f64;

typedef uint32_t b32;

typedef uintptr_t umm;

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#if DEVELOP_MODE
#define Assert(Expression) if (!(Expression)) *((int *)0) = 0;
#else
#define Assert(Expression)
#endif

struct game_button_state
{
	b32 EndedDown;
};

// TODO: finilize
struct game_controller_input
{
	union
	{
		game_button_state Button[4];

		struct
		{
			
		};
	};
};

enum game_mouse_buttons
{
	PlatformMouseButton_Left,
	PlatformMouseButton_Right,
	PlatformMouseButton_Middle,
	PlatformMouseButton_Extended0,
	PlatformMouseButton_Extended1,

	PlatformMouseButton_Count,
};

struct game_input
{
	game_controller_input Controller;
	game_button_state MouseButtons[PlatformMouseButton_Count];
	f32 MouseX, MouseY, MouseZ;
};