#pragma once

#define MIN(A, B) (((A) < (B)) ? (A) : (B))
#define MAX(A, B) (((A) > (B)) ? (A) : (B))

#include "intrinsics.cpp"
#include "math.cpp"
#include "string.h"
#include "memory_arena.h"
#include "model.h"
#include "render_group.h"
#include "asset.h"

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

struct model_highlight_params
{
	v3 EdgeColor;
	v3 OutlineColor;
	b32 OutlineIsSet;
};

enum ui_intercation_target
{
	UI_InteractionTarget_None,

	UI_InteractionTarget_Model,
	UI_InteractionTarget_ModelFace,
	UI_InteractionTarget_ModelEdge,

	UI_InteractionTarget_ModelCount,
};

enum ui_interaction_type
{
	UI_InteractionType_None,

	UI_InteractionType_Select,
};

enum select_element_type
{
	SelectElementType_None,

	SelectElementType_Edge,
	SelectElementType_Face
};

struct selected_elements_buffer
{
	u32 *Elements;
	u32 Count;
	u32 MaxCount;
	select_element_type Type;
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

struct ui_id
{
	u64 ID[2];
};

// TODO: Improve
struct ui_interaction
{
	union
	{
		u32 TypeID;
		struct
		{
			u16 Type;
			u16 Target;
		};
	};

	ui_id ID; //
};

struct interact_model
{
	u32 ID;

	element_ray_result Face;
	element_ray_result Edge;
};

#define ITargetType(ITarget, Type) ((ITarget) == UI_InteractionTarget_##Type)

struct editor_world_ui
{
	u32 UpdateITarget;
	u32 ITarget;

	ui_interaction Interaction;
	ui_interaction HotInteraction;
	ui_interaction NextHotInteraction;

	interact_model IModel;

	v2 MouseP;
	v2 dMouseP;
	v2 LastMouseP;
	ray_params MouseRay;
};

struct game_editor_state
{
	model Models[32];
	u16 ModelsCount;

	static_mesh StaticMesh[16];
	u16 StaticMeshCount;

	memory_arena MainArena;
	memory_arena TranArena;
	page_memory_arena PageArena;

	camera Camera;

	editor_world_ui WorldUI;
	selected_elements_buffer Selected;

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