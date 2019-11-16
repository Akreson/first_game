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

void *
PushSize_(memory_arena *Arena, memory_index Size, u32 Alignment = 2)
{
	u32 CurrentBasePtr = (memory_index)Arena->Base + Arena->Used;

	u32 AlignMask = Alignment - 1;
	u32 AlignOffset = Alignment - (CurrentBasePtr & AlignMask);

	u32 TotalAddedSize = Size + AlignOffset;

	if (!(Arena->Size >= (Arena->Used + TotalAddedSize))) { Assert(0); }

	void *Result = (void *)(Arena->Base + Arena->Used + AlignOffset);
	Arena->Used += TotalAddedSize;

	return Result;
}

#define PushArray(Arena, type, Count, ...) (type *)PushSize_(Arena, (Count)*sizeof(type), ##__VA_ARGS__)

struct game_world_state
{
};

struct model
{
	v4 Color;
	f32 *Vertex;
	u16 VertexCount;
};

struct game_editor_state
{
	memory_arena EditorArena;
	model Models[16];
	u16 ModelsCount;
};

struct game_state
{
	b32 IsInit;

	font_asset_info *FontAsset;

	game_mode GameMode;
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