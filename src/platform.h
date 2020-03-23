#pragma once

#include <stdint.h>
#include <stddef.h>
#include <limits.h>

#define FLOAT_MAX 3.402823466e+38f
#define FLOAT_MIN 1.7976931348623158e+308f
#define FLOAT_MAX_NEG -3.402823466e+38f
#define FLOAT_MIN_NEG -1.7976931348623158e+308f

#if _MSC_VER
#include <intrin.h>
#else
#error "Unsuported compiler"
#endif

#define internal static
#define global static

#define KiB(Value) (Value * 1024)
#define MiB(Value) (KiB(Value) * 1024)
#define GiB(Value) (MiB(Value) * 1024)

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
typedef size_t memory_index;

typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t s8;

typedef float f32;
typedef double f64;

typedef u32 b32;
typedef u8 b8;

typedef uintptr_t umm;

#define X86_CACHE_LINE_SIZE 64

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))
#define OffsetOf(Instance, Member) ((size_t)&(((Instance *)0)->Member))
#define GetMemberOf(Instance, Member) ((((Instance *)0)->Member))
#define PointerFromU32(Type, Value) (Type *)((size_t)Value)
#define U32FromPointer(Pointer) (u32)((size_t)(Pointer))
#define SetBit(Value, BitNum) (Value | (1 << BitNum))
#define ResetBit(Value, BitNum) (Value & ~(1 << BitNum))
#define IsBitSet(Value, BitNum) (Value & (1 << BitNum))

union v2
{
	f32 E[2];
	struct
	{
		f32 x, y;
	};
};


union v3
{
	f32 E[3];

	struct
	{
		f32 x, y, z;
	};

	struct
	{
		f32 r, g, b;
	};

	struct
	{
		v2 xy;
		f32 _Ignored0;
	};

	struct
	{
		f32 _Ignored1;
		v2 yz;
	};
};

// TODO: Add _m128
union v4
{
	f32 E[4];

	struct
	{
		f32 x, y, z, w;
	};

	struct
	{
		v2 xy;
		v2 zw;
	};

	struct
	{
		f32 r, g, b, a;
	};

	struct
	{
		v3 xyz;
		f32 _Ignored0;
	};
};

// NOTE: row-major
// TODO: Add _m128
union m4x4
{
	f32 E[4][4];

	struct
	{
		v4 Row0;
		v4 Row1;
		v4 Row2;
		v4 Row3;
	};
};

struct m4x4_inv
{
	m4x4 Forward;
	m4x4 Inverse;
};

#if DEVELOP_MODE
#define Assert(Expression) if (!(Expression)) *((int *)0) = 0;
#else
#define Assert(Expression)
#endif

inline u32
GetAlignmentOffsetForwad(memory_index Ptr, u32 Alignment)
{
	Assert(!(Alignment & (Alignment - 1)));

	u32 AlignMask = Alignment - 1;
	u32 AlignOffset = Alignment - (Ptr & AlignMask);

	return AlignOffset;
}

inline u32
GetAlignmentOffsetBack(memory_index Ptr, u32 Alignment)
{
	Assert(!(Alignment & (Alignment - 1)));

	u32 AlignMask = Alignment - 1;
	u32 AlignOffset = Ptr & AlignMask;

	return AlignOffset;
}

inline b32
IsAligned(void *Ptr, u32 Alignment)
{
	b32 Result = (umm)Ptr & (Alignment - 1);
	return Result;
}

inline b32
IsPowerOf2(u32 Value)
{
	b32 Result = !(Value & (Value - 1));
	return Result;
}

inline void
Copy(memory_index Size, void *DestBase, void *SourceBase)
{
	u8 *Source = (u8 *)SourceBase;
	u8 *Dest = (u8 *)DestBase;

	while (Size--)
	{
		*Dest++ = *Source++;
	}
}

void
Copy128(memory_index Size, void *DestBase, void *SourceBase)
{
	Assert(Size >= X86_CACHE_LINE_SIZE);

	f32 *Source = (f32 *)SourceBase;
	f32 *Dest = (f32 *)DestBase;

	while (Size)
	{
		__m128 Source0 = _mm_load_ps(Source);
		__m128 Source1 = _mm_load_ps(Source + 4);
		__m128 Source2 = _mm_load_ps(Source + 8);
		__m128 Source3 = _mm_load_ps(Source + 16);

		_mm_store_ps(Dest, Source0);
		_mm_store_ps(Dest + 4, Source1);
		_mm_store_ps(Dest + 8, Source2);
		_mm_store_ps(Dest + 16, Source3);

		Dest += 20;
		Source += 20;

		Size -= X86_CACHE_LINE_SIZE;
	}
}

inline void
MemSet(u8 *Dest, u32 Size, u8 Value)
{
	while (Size--)
	{
		*Dest++ = Value;
	}
}

inline void
MemSet(u32 *Dest, u32 Size, u32 Value)
{
	while (Size--)
	{
		*Dest++ = Value;
	}
}

inline void
ZeroSize(void *Ptr, u32 Size)
{
	MemSet((u8 *)Ptr, Size, 0);
}

#define ZeroStruct(Instance) ZeroSize(sizeof(Instance), &(Instance))

struct game_button_state
{
	b8 EndedDown;
	b8 TransionState;
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
	game_button_state Alt, Shift, Ctrl, Tab;

	f32 PrevFrameTime;
};

inline void
PlatformProcessButtonState(game_button_state *Button, b32 IsDownState)
{
	b8 IsDown = IsDownState ? true : false;
	if (Button->EndedDown != IsDown)
	{
		Button->EndedDown = IsDown;
		Button->TransionState++;
	}
}

inline b32
IsDown(game_button_state Button)
{
	b32 Result = (Button.EndedDown);
	return Result;
}

inline b32
IsGoDown(game_button_state Button)
{
	b32 Result = (Button.EndedDown && (Button.TransionState >= 1));
	return Result;
}

inline b32
IsKepDown(game_button_state Button)
{
	b32 Result = (Button.EndedDown && (Button.TransionState == 0));
	return Result;
}

inline b32
WasDown(game_button_state Button)
{
	b32 Result = (!Button.EndedDown && (Button.TransionState >= 1));
	return Result;
}

struct game_memory
{
	void *GameStorage;
	u64 GameStorageSize;

	void *EditorStorage;
	u64 EditorStorageSize;
};

struct game_render_commands
{
	m4x4_inv PersProj;
	m4x4_inv OrthoProj;

	v2 ScreenDim;
	u8 *PushBufferBase;
	u32 PushBufferSize;
	u32 MaxPushBufferSize;

	u8 *VertexBufferBase;
	u32 VertexBufferOffset;
	u32 MaxVertexBufferSize;

	// TODO: store more mat
};

struct platform_file_handler
{
	b32 Errors;
	void *Handler;
};

enum file_type
{
	FileType_FontFile,
};

#define PLATFORM_GET_FILE_HANDLER_FOR_FILE(name) platform_file_handler name(file_type FileType)
typedef PLATFORM_GET_FILE_HANDLER_FOR_FILE(platform_get_file_handler_for_file);

#define PLATFORM_READ_FILE(name) void name(platform_file_handler *FileHandler, u32 Size, void *Dest)
typedef PLATFORM_READ_FILE(platform_read_file);

#define PLATFORM_GET_FILE_SIZE(name) u32 name(platform_file_handler *FileHandler)
typedef PLATFORM_GET_FILE_SIZE(platform_get_file_size);

#define PLATFORM_ALLOCATE_TEXTURE(name) u32 name(u32 Width, u32 Height, void *Data)
typedef PLATFORM_ALLOCATE_TEXTURE(platform_allocate_texture);

#define PLATFORM_DEALLOCATE_TEXTURE(name) void name(u32 TextureHandler)
typedef PLATFORM_DEALLOCATE_TEXTURE(platform_deallocate_texture);

struct platform_api
{
	platform_read_file *ReadFile;
	platform_get_file_size *GetFileSize;
	platform_allocate_texture *AllocateTexture;
	platform_deallocate_texture *DeallocateTexture;
	platform_get_file_handler_for_file *GetFileHandlerForFile;
};