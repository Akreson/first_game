#pragma once

#define MIN(A, B) (((A) < (B)) ? (A) : (B))
#define MAX(A, B) (((A) > (B)) ? (A) : (B))

enum builtin_static_mesh
{
	BuiltInMesh_RotateToolSphere,
	BuiltInMesh_TranslateArrow,
	BuiltInMesh_Cube,
	BuiltInMesh_Low,
};

#include "intrinsics.cpp"
#include "math.cpp"
#include "string.h"
#include "memory_arena.h"
#include "render_group.h"
#include "asset.h"
#include "model.h"
#include "ui.h"

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

#define _EDITOR_TOOLS_SIZE 1.5f
#define ROTATE_TOOL_DIAMETER _EDITOR_TOOLS_SIZE
#define SCALE_TOOL_SIZE (_EDITOR_TOOLS_SIZE * 0.5f)
#define TRANSLATE_TOOL_SIZE (_EDITOR_TOOLS_SIZE * 0.5f)
#define TRANSLATE_TOOL_ARROW_R (_EDITOR_TOOLS_SIZE * 0.04)

struct model_highlight_params
{
	v3 EdgeColor;
	v3 OutlineColor;
	b32 OutlineIsSet;
};

struct model_ray_result
{
	u32 ModelIndex;
	element_ray_result Face;
};

struct model_ray_sort
{
	u32 Index;
	f32 Length;
};

struct game_editor_state
{
	model_data SourceModels[32];
	u16 SourceModelsCount;

	work_model WorkModels[32];
	u16 WorkModelsCount;

	model_transform_cache TransCache[32];
	u16 ModelTransCacheCount;

	static_mesh StaticMesh[16];
	u16 StaticMeshCount;

	memory_arena MainArena;
	memory_arena TranArena;
	page_memory_arena PageArena;

	camera Camera;

	editor_world_ui WorldUI;

	v3 EdgeColor;
	v3 SelectColor;
	v3 ActiveOutlineColor;
	v3 HotOutlineColor;
};

struct game_state
{
	b32 IsInit;
	
	game_mode GameMode;

	font_asset_info *FontAsset;

	memory_arena GameArena;

	game_world_state WorldState;

#ifdef DEVELOP_MODE
	game_editor_state EditorState;
#endif
};

global platform_api PlatformAPI;