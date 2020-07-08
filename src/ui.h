#pragma once

enum ui_intercation_target
{
	//UI_InteractionTarget_None, // TODO: Return?

	UI_InteractionTarget_Model,
	UI_InteractionTarget_Tools,

	UI_InteractionTarget_Count
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
	//ToolType_None, // TODO: Use?

	ToolType_Rotate,
	ToolType_Translate,
	ToolType_Scale,

	ToolType_Count,
};

enum ui_interaction_type
{
	UI_InteractionType_None,

	UI_InteractionType_Select,
	UI_InteractionType_Move,

	UI_InteractionType_Count,
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

#define RTOOLS_AXIS_INTERACT_THRESHOLD 0.03f
#define RTOOLS_PERP_AXIS_INTERACT_MIN_THRESHOLD 0.94f
enum tools_axis_id
{
	ToolsAxisID_None,

	ToolsAxisID_XAxis,
	ToolsAxisID_YAxis,
	ToolsAxisID_ZAxis,

	ToolsAxisID_Count,
};

struct rot_tool_perp_axis
{
	union
	{
		struct
		{
			u32 Index;
			u32 IsSet;
		};
		iv2 V;
	};
};

// TODO: Pack smarter?
struct rotate_tools
{
	m4x4 Transform;
	m3x3 Axis;
	
	plane_params InteractPlane;
	
	// NOTE: Persistent
	v3 CenterPos;
	f32 Radius;
	f32 PerpThreshold;

	v3 FromPosToRayP;

	v3 BeginVector;
	v3 PrevVector;
	v4 AxisMask;
	rot_tool_perp_axis PerpInfo; // 0 - index, 1 - is have perp axis 

	tools_axis_id InteractAxis;
	b8 EnterActiveState;
};

struct tools
{
	u16 Type;
	b16 IsInit;

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