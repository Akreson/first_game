#pragma once

enum render_entry_type
{
	RenderEntryType_render_entry_bitmap,
	RenderEntryType_render_entry_model,
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

struct render_group
{
	game_render_commands *Commands;
	font_asset_info *FontAsset;
	u32 LastProj;
	f32 Width, Height;
};