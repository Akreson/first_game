#pragma once

enum render_entry_type
{
	RenderEntryType_render_entry_bitmap,
	RenderEntryType_render_entry_model,
	RenderEntryType_render_entry_model_face,
	RenderEntryType_render_entry_model_outline,
};

struct render_entry_header
{
	u16 Type;
};

struct render_entry_bitmap
{
	u32 TextureID;
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
	v4 Vertex;
	v4 MetaInfo;
};

struct render_group
{
	game_render_commands *Commands;
	font_asset_info *FontAsset;

	void *GroupRenderElement;

	v2 ScreenDim;
	
	v3 CameraZ;

	m4x4 InvCamera;
	m4x4 InvPerspective;
};