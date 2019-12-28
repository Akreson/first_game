#pragma once

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

struct page_memory_arena
{
	void *Base;
	u32 *AllocStatus; // NOTE: One bit status 0 - used, 1 - unused (for now)
	s16 *UsedStatus; // per page, must be signed
	u16 PageCount;
	u16 UnusedSpace;
};

inline u32
GetAlignmentOffset(memory_index Ptr, u32 Alignment)
{
	Assert(!((Alignment - 1) & Alignment));

	u32 AlignMask = Alignment - 1;
	u32 AlignOffset = Alignment - (Ptr & AlignMask);

	return AlignOffset;
}

void *
PushSize_(memory_arena *Arena, memory_index Size, u32 Alignment = 4)
{
	memory_index CurrentArenaPtr = (memory_index)Arena->Base + Arena->Used;

	u32 AlignOffset = GetAlignmentOffset(CurrentArenaPtr, Alignment);

	u32 TotalAddedSize = Size + AlignOffset;

	if (!(Arena->Size >= (Arena->Used + TotalAddedSize))) { Assert(0); }

	void *Result = (void *)(Arena->Base + Arena->Used + AlignOffset);
	Arena->Used += TotalAddedSize;

	return Result;
}

#define PushSize(Arena, Size, ...) PushSize_(Arena, Size, ##__VA_ARGS__)
#define PushStruct(Arena, type, ...) (type *)PushSize_(Arena, sizeof(type), ##__VA_ARGS__)
#define PushArray(Arena, type, Count, ...) (type *)PushSize_(Arena, (Count)*sizeof(type), ##__VA_ARGS__)

inline void
InitPageArena(memory_arena *Arena, u32 PageArenaSize, u16 PageSize = KiB(2))
{
	Assert((PageSize > KiB(1)) && (PageSize < SHRT_MAX));

	page_memory_arena *PageArena = nullptr;
	
	u32 PageCount = PageArenaSize / PageSize;
	Assert(PageCount);

	u32 PagesPerAllocStatusBlock = sizeof(*PageArena->AllocStatus) * 8;
	u32 AllocStatusBlocksCount = PageCount / PagesPerAllocStatusBlock;// *sizeof(*PageArena->AllocStatus);

	if ((PageCount - (AllocStatusBlocksCount * PagesPerAllocStatusBlock))) AllocStatusBlocksCount++;

	PageArena = PushStruct(Arena, page_memory_arena);
	PageArena->AllocStatus = PushArray(Arena, u32, AllocStatusBlocksCount, sizeof(*PageArena->AllocStatus));
	PageArena->UsedStatus = PushArray(Arena, s16, PageCount, sizeof(*PageArena->UsedStatus));
	PageArena->Base = PushSize(Arena, PageArenaSize, PageSize);
	PageArena->PageCount = PageCount;
}

struct game_world_state
{
};

// TODO: Model vertex count never be bigger than U16_MAX_VALUE?
struct model_face
{
	union
	{
		u16 VertexID[4];
		struct
		{
			u16 V1, V2, V3, V4;
		};
	};
};

// NOTE: Triangle specifed in conter-clokwise order
// TODO: Add edge info!!
struct model
{
	v4 Color;
	v3 Offset;
	v3 *Vertex;
	model_face *Faces;
	u16 FaceCount;
	u16 VertexCount;
};

struct game_editor_state
{
	v3 CameraOffset;
	f32 CameraOrbit;
	f32 CameraPitch;
	f32 CameraDolly;

	memory_arena EditorMainArena;
	model Models[16];
	u16 ModelsCount;
};

struct game_state
{
	b32 IsInit;
	
	game_mode GameMode;

	v2 LastMouseP;

	font_asset_info *FontAsset;

	game_world_state WorldState;

#ifdef DEVELOP_MODE
	game_editor_state EditorState;
#endif
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

global_variable platform_api PlatformAPI;