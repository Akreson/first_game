#pragma once

#include <stdint.h>
#include <stddef.h>

#define internal static
#define global_variable static

#define Pi32 3.14159265359f
#define Tau32 6.2831853071f

#define KiB(Value) (Value * 1024)
#define MiB(Value) (KiB(Value) * 1024)
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
#define OffsetOf(Instance, Member) ((size_t)&(((Instance *)0)->Member))

#if DEVELOP_MODE
#define Assert(Expression) if (!(Expression)) *((int *)0) = 0;
#else
#define Assert(Expression)
#endif

inline void
ZeroSize(u64 Size, void *Ptr)
{
	u8 *Byte = (u8 *)Ptr;
	while (Size--)
	{
		*Byte++ = 0;
	}
}

#define ZeroStruct(Instance) ZeroSize(sizeof(Instance), &(Instance))

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

struct game_memory
{
	void *PermanentStorage;
	u64 PermanentStorageSize;
};

// TODO: Change location
struct bitmap_info
{
	void *TextureHandler;
	void *Memory;
	f32 WidthOverHeight;
	u16 Width;
	u16 Height;
};

// TODO: replace patching?
struct font_asset_info
{
	union
	{
		void *Refs;

		// NOTE: Only for pointer for letter patching
		struct
		{
			u16 *UnicodeMap; // NOTE: 0 mean for this unicode code glyph doesn't exist
			s16 *KerningTable;
			s16 *GlyphAdvance; // TODO: does this need?
			bitmap_info *Glyphs; // NOTE: Must be last
		};

	};

	u32 GlyphCount;
	u32 OnePastLastUnicodeCode;
	s16 AscenderHeight;
	s16 DescenderHeight;
	s16 LineGap;
};

#define MAX_REFS_METRICS_COUNT ((u32)OffsetOf(font_asset_info, Glyphs)/sizeof(void*))