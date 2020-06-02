#pragma once

struct renderer_texture
{
	u64 Handle;
};

struct renderer_mesh
{
	u64 Handle;
	u32 ElementCount;
};

struct render_alloc_mesh_params
{
	void *VertexData;
	u32 *Tris;
	u32 VertexCount;
	u32 TrisCount;
	u32 Flags;
};

enum alloc_mesh_params
{
	AllocMeshParam_NormalSet = (1 << 0),
	AllocMeshParam_UVSet = (1 << 1),
};

enum render_entry_type
{
	RenderEntryType_render_entry_bitmap,
	RenderEntryType_render_entry_model,
	RenderEntryType_render_entry_model_face,
	RenderEntryType_render_entry_model_outline,
	RenderEntryType_render_entry_tool_rotate,

	RenderEntryType_render_entry_sphere,
};

enum face_selection_type
{
	FaceSelectionType_Select,
	FaceSelectionType_Hot,
};

enum face_element_renderer_params
{
	FaceElementParams_Mark = 0x1,
	FaceElementParams_SetAll = 0xFFFFFFFF,
};

struct face_render_params
{
	union
	{
		struct
		{
			b8 ActiveEdge[4];
			b8 HotEdge[4];
		};

		struct
		{
			u32 Active;
			u32 Hot;
		};
	};

	u8 SelectionFlags[2];
};

struct face_edge_params
{
	b8 Active01;
	b8 Active12;
	b8 Active03;
	b8 Active23;

	b8 Hot01;
	b8 Hot12;
	b8 Hot03;
	b8 Hot23;
};

struct render_entry_header
{
	u16 Type;
};

struct render_entry_bitmap
{
	renderer_texture Texture;
	// TODO: Change to Position, XAxis, YAxis?
	v2 Min;
	v2 Max;
	v3 Color;
};

struct render_entry_model
{
	v4 Color;
	v3 EdgeColor;
	v3 Offset;
	u32 StartOffset;
	u32 ElementCount;
};

struct render_entry_model_outline
{
	render_entry_model *ModelEntry;
	v3 OutlineColor;
};

struct render_entry_model_face
{
	v4 Color;
	v3 Offset; // TODO: Remove?
	u32 VertexBufferOffset;

	//TODO:Delete later
	v3 EdgeColor;
	//v3 BaricentricCoord;
};

struct render_model_face_vertex
{
	v3 Vertex;
	v3 BarCoords;
	v3 ActiveMask;
	v3 HotMask;
	v2 FaceSelParam;
};

struct render_entry_tool_rotate
{
	renderer_mesh Mesh;
	v3 XAxis;
	v3 YAxis;
	v3 ZAxis;
	v3 Pos;
	v4 AxisActivityState;
	// TODO: Add Selected param and rotate degre
};

struct render_group
{
	game_render_commands *Commands;

	void *GroupRenderElement;

	v2 ScreenDim;
	
	v3 CameraZ;

	m4x4 InvCamera;
	m4x4 InvPerspective;
};