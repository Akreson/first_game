#pragma once

#include "intrinsics.cpp"
#include "memory_arena.h"
#include "asset.h"
#include "render_group.h"

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

enum game_mode
{
	GameMode_None,

	GameMode_Game,
	GameMode_Editor,
};

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
	v3 Offset; // TODO: Store vertex in origin of model space or already in world space offset?
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
	page_memory_arena EditorPageArena;
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