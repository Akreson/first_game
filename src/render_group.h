#pragma once

enum render_entry_type
{
	RenderEntryType_render_entry_bitmap,
	RenderEntryType_render_entry_model,
	RenderEntryType_render_entry_model_face
};

struct render_entry_header
{
	u16 Type;
};

struct render_entry_bitmap
{
	bitmap_info *Bitmap;
	// TODO: Change to Position, XAxis, YAxis?
	v2 Min;
	v2 Max;
	v3 Color;
};

struct render_entry_model
{
	v4 Color;
	v3 Offset;
	v3 *Vertex;
	u16 VertexCount;
};

struct render_entry_model_face
{
	v4 Color;
	v3 Offset; // TODO: Remove?
	u32 VertexBufferOffset;
	//v3 BaricentricCoord;
};

struct render_group
{
	game_render_commands *Commands;
	font_asset_info *FontAsset;
	v2 ScreenDim;

	v3 CameraZ;

	m4x4 InvCamera;
	m4x4 InvPerspective;
};