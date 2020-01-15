#pragma once

#include <stdint.h>
#include <stddef.h>
#include <limits.h>

#if _MSC_VER
#include <intrin.h>
#else
#error "Unsuported compiler"
#endif

#define internal static
#define global_variable static

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

typedef uint32_t b32;

typedef uintptr_t umm;

#define X86_CACHE_LINE_SIZE 64

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))
#define OffsetOf(Instance, Member) ((size_t)&(((Instance *)0)->Member))
#define PointerFromU32(Type, Value) (Type *)((size_t)Value)
#define U32FromPointer(Pointer) (u32)((size_t)(Pointer))
#define SetBit(Value, BitNum) Value |= (1 << BitNum)
#define ResetBit(Value, BitNum) Value &= ~(1 << BitNum)
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

// TODO: See if need change in loop body
void
Copy128(memory_index Size, void *DestBase, void *SourceBase)
{
	Assert(Size >= X86_CACHE_LINE_SIZE);

	__m128 *Source = (__m128 *)SourceBase;
	__m128 *Dest = (__m128 *)DestBase;

	memory_index Offset = 0;
	while (Size)
	{
		Dest[0] = Source[0];
		Dest[1] = Source[1];
		Dest[2] = Source[2];
		Dest[3] = Source[3];

		Dest += 4;
		Source += 4;

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
	b32 AltDown, ShiftDown, CtrlDown, TabDown;
};

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

	f32 *VertexBufferBase;
	u32 VertexCount;
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

#define PLATFORM_ALLOCATE_TEXTURE(name) void *name(u32 Width, u32 Height, void *Data)
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