#pragma once

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

// TODO: Remove?
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
	// slect_element_type Type;
};

struct ui_id
{
	u64 ID[2];
};

// TODO: Improve?
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
	b32 ElementPick;
	element_ray_result Face;
	element_ray_result Edge;
};

#define ITargetType(ITarget, Type) ((ITarget) == UI_InteractionTarget_##Type)

struct editor_world_ui
{
	b32 UpdateITarget;
	u32 ITarget;

	ui_interaction Interaction;
	ui_interaction HotInteraction;
	ui_interaction NextHotInteraction;

	ui_interaction ToExecute;
	ui_interaction NextToExecute;

	interact_model IModel;
	selected_elements_buffer Selected;

	v2 MouseP;
	v2 dMouseP;
	v2 LastMouseP;
	ray_params MouseRay;
};