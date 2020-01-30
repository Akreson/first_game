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
#include "render_group.h"

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

	// NOTE: All models build from quads
	union
	{
		u16 EdgeID[4];

		struct
		{
			u16 Edge0, Edge1, Edge2, Edge3;
		};
	};
};

struct model_edge
{
	union
	{
		u16 VertexID[2];
		struct
		{
			u16 V0, V1;
		};
	};

	union
	{
		u16 FaceID[2];

		struct
		{
			u16 Face0, Face1;
		};
	};

};

// NOTE: Triangle specifed in conter-clokwise order
struct model
{
	v3 *Vertex;
	model_face *Faces;
	model_edge *Edges;
	v4 Color;
	rect3 AABB;
	v3 Offset; // TODO: Store vertex in origin of model space or already in world space offset?
	u16 FaceCount;
	u16 VertexCount;
	u16 EdgeCount;
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
	v3 CameraPos;

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