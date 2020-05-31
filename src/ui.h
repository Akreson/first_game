#pragma once

enum ui_intercation_target
{
	//UI_InteractionTarget_None, // TODO: Return?

	UI_InteractionTarget_Model,
	UI_InteractionTarget_Tools,
};

enum model_target_element
{
	ModelTargetElement_None,

	ModelTargetElement_Model,
	ModelTargetElement_Face,
	ModelTargetElement_Edge,

	ModelTargetElement_Count
};

enum tool_type
{
	//ToolType_None,

	ToolType_Rotate,
	ToolType_Translate,
	ToolType_Scale,

	ToolType_Count,
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
	u32 Target;
	u32 ID;
	element_ray_result Face;
	element_ray_result Edge;
};

#define AXIS_INTERACT_THRESHOLD 0.01f
struct rotate_tools
{
	v3 CenterPos;
	f32 Radius;
};

struct tools
{
	u32 Type;
	b32 IsInit;

	union
	{
		rotate_tools Rotate;
	};
};

#define IsITargetEq(ITarget, Type) ((ITarget) == UI_InteractionTarget_##Type)

struct editor_world_ui
{
	u32 ITarget;
	b32 UpdateModelInteraction;

	ui_interaction Interaction;
	ui_interaction HotInteraction;
	ui_interaction NextHotInteraction;

	ui_interaction ToExecute;
	ui_interaction NextToExecute;

	interact_model IModel;
	selected_elements_buffer Selected;
	tools Tools;

	v2 MouseP;
	v2 dMouseP;
	v2 LastMouseP;
	ray_params MouseRay;
};