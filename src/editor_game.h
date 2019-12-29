#pragma once

#include "intrinsics.cpp"
#include "asset.h"
#include "render_group.h"

enum game_mode
{
	GameMode_None,

	GameMode_Game,
	GameMode_Editor,
};

struct memory_arena
{
	memory_index Size;
	memory_index Used;
	u8 *Base;
};

#define MAX_PAGE_USED SHRT_MAX
struct page_memory_arena
{
	void *Base;
	u32 *AllocStatus; // NOTE: One bit status: 0 - used, 1 - unused (for now)
	s16 *UsedStatus; // per page, must be signed
	u32 PageSize;
	u16 PageCount;
	u16 UnusedSpace;
};

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

inline u32
GetAlignmentOffsetForwad(memory_index Ptr, u32 Alignment)
{
	Assert(!((Alignment - 1) & Alignment));

	u32 AlignMask = Alignment - 1;
	u32 AlignOffset = Alignment - (Ptr & AlignMask);

	return AlignOffset;
}

inline u32
GetAlignmentOffsetBack(memory_index Ptr, u32 Alignment)
{
	Assert(!((Alignment - 1) & Alignment));

	u32 AlignMask = Alignment - 1;
	u32 AlignOffset = Ptr & AlignMask;

	return AlignOffset;
}

#define PushSize(Arena, Size, ...) PushSize_(Arena, Size, ##__VA_ARGS__)
#define PushStruct(Arena, type, ...) (type *)PushSize_(Arena, sizeof(type), ##__VA_ARGS__)
#define PushArray(Arena, type, Count, ...) (type *)PushSize_(Arena, (Count)*sizeof(type), ##__VA_ARGS__)

void *
PushSize_(memory_arena *Arena, u32 Size, u32 Alignment = 4)
{
	memory_index CurrentArenaPtr = (memory_index)Arena->Base + Arena->Used;

	u32 AlignOffset = GetAlignmentOffsetForwad(CurrentArenaPtr, Alignment);

	u32 TotalAddedSize = Size + AlignOffset;

	if (!(Arena->Size >= (Arena->Used + TotalAddedSize))) { Assert(0); }

	void *Result = (void *)(Arena->Base + Arena->Used + AlignOffset);
	Arena->Used += TotalAddedSize;

	return Result;
}

inline u32
GetPageIndex(void *Ptr, void *PageBase, u32 PageSize)
{
	Assert(IsPowerOf2(PageSize));
	Assert(((umm)PageBase & (PageSize - 1)));

	u32 AlignOffset = GetAlignmentOffsetBack((memory_index)Ptr, PageSize);
	u8 *AlignPtr = (u8 *)Ptr - AlignOffset;

	u32 ShiftValue = FindLeastSignificantSetBit(PageSize);
	
	u32 Result = (u8 *)AlignPtr - (u8 *)PageBase;

	return Result;
}

// TODO: Complete!!!
void *
PushSize_(page_memory_arena *Arena, memory_index Size, void *Dest, u8 *Source)
{
	u32 PageIndex = GetPageIndex(Dest, Arena->Base, Arena->PageSize);

	return nullptr;
}

inline void
InitArena(memory_arena *Arena, memory_index Size, u8 *Base)
{
	Arena->Base = Base;
	Arena->Size = Size;
	Arena->Used = 0;
}

// TODO: Use struct for holding page alloc or use pointer and calc needed metadata
// on allocation side?
inline void
InitPageArena(memory_arena *Arena, u32 PageArenaSize, u16 PageSize = KiB(4))
{
	// TODO: Should be power of 2?
	Assert((PageSize > KiB(1)) && (PageSize < SHRT_MAX) && IsPowerOf2(PageSize));

	page_memory_arena *PageArena = nullptr;
	
	u32 PageCount = PageArenaSize / PageSize;
	Assert(PageCount);

	u32 PagesPerAllocStatusBlock = sizeof(*PageArena->AllocStatus) * 8;
	u32 AllocStatusBlocksCount = PageCount / PagesPerAllocStatusBlock;

	if ((PageCount - (AllocStatusBlocksCount * PagesPerAllocStatusBlock))) AllocStatusBlocksCount++;

	PageArena = PushStruct(Arena, page_memory_arena);
	PageArena->AllocStatus = PushArray(Arena, u32, AllocStatusBlocksCount, sizeof(*PageArena->AllocStatus));
	PageArena->UsedStatus = PushArray(Arena, s16, PageCount, sizeof(*PageArena->UsedStatus));
	PageArena->Base = PushSize(Arena, PageArenaSize, PageSize);
	PageArena->PageSize = PageSize;
	PageArena->PageCount = PageCount;
	PageArena->UnusedSpace = (u16)((umm)PageArena->Base - (umm)PageArena->UsedStatus);

	MemSet(PageArena->AllocStatus, AllocStatusBlocksCount, ULONG_MAX);
}

// TODO: Model vertex count never be bigger than U16_MAX_VALUE?
struct model_face
{
	union
	{
		u16 VertexID[4];
		struct
		{
			u16 V0, V1, V2, V3;
		};
	};
};

// NOTE: Triangle specifed in conter-clokwise order
// TODO: Add edge info!!
struct model
{
	v3 *Vertex;
	model_face *Faces;
	v4 Color;
	v3 Offset; // TODO: Store in origin of model space or already in world space offset?
	u16 FaceCount;
	u16 VertexCount;
};

struct game_world_state
{

};

struct game_editor_state
{
	v3 CameraOffset;
	f32 CameraOrbit;
	f32 CameraPitch;
	f32 CameraDolly;

	memory_arena EditorMainArena;
	page_memory_arena ModelsDataArena;
	model Models[32];
	u16 ModelsCount;
};

struct game_state
{
	b32 IsInit;
	
	game_mode GameMode;

	v2 LastMouseP;

	font_asset_info *FontAsset;

	memory_arena GameArena;

	game_world_state WorldState;

#ifdef DEVELOP_MODE
	game_editor_state EditorState;
#endif
};

global_variable platform_api PlatformAPI;