#pragma once

#define MIN(A, B) (((A) < (B)) ? (A) : (B))
#define MAX(A, B) (((A) > (B)) ? (A) : (B))

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

#include "intrinsics.cpp"
#include "math.cpp"
#include "string.h"
#include "memory_arena.h"
#include "asset.h"
#include "model.h"
#include "render_group.h"

enum game_mode
{
	GameMode_None,

	GameMode_Game,
	GameMode_Editor,
};

struct camera
{
	v3 Pos;
	v3 Offset;
	f32 Orbit;
	f32 Pitch;
	f32 Dolly;
};

struct game_world_state
{

};

struct model_ray_result
{
	b32 Hit;
	u32 ModelIndex;
	u32 FaceIndex;
	v3 IntersetPoint;
};

struct model_ray_sort
{
	u32 Index;
	f32 Length;
};

struct game_editor_state
{
	model Models[32];
	u16 ModelsCount;

	memory_arena MainArena;
	page_memory_arena PageArena;

	camera Camera;

	b32 ActiveModel;
	u32 ActiveModelID;
	b32 HotModel;
	u32 HotMOdelID;
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

global platform_api PlatformAPI;