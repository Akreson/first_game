#pragma once

#define MIN(A, B) (((A) < (B)) ? (A) : (B))
#define MAX(A, B) (((A) > (B)) ? (A) : (B))

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

struct element_ray_result
{
	u32 ID;
	v3 P;
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

enum model_intercation_target
{
	ModelInteractionTarget_None,

	ModelInteractionTarget_Model,
	ModelInteractionTarget_Face,
	ModelInteractionTarget_Edge,

	ModelInteractionTarget_Count
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

enum interaction_type
{
	InteractionType_None,

	InteractionType_Select,
};

struct ui_interaction
{
	u32 Type;
	u32 ID;
};

struct interacted_model
{
	u32 ID;
	element_ray_result Face;
	element_ray_result Edge;
};

struct editor_ui
{
	u32 ITarget;

	ui_interaction HotInteraction;
	ui_interaction Interaction;

	interacted_model IModel;

	v2 MouseP;
	v2 dMouseP;
	v2 LastMouseP;
};

struct game_editor_state
{
	model Models[32];
	u16 ModelsCount;

	memory_arena MainArena;
	page_memory_arena PageArena;

	camera Camera;

	editor_ui WorldUI;
	selected_elements_buffer Selected;
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