inline render_group
InitRenderGroup(game_render_commands *Commands, game_input *Input, font_asset_info *FontAsset)
{
	render_group Result = {};

	Result.Commands = Commands;
	Result.FontAsset = FontAsset;
	Result.Width = Input->ScreenWidth;
	Result.Height = Input->ScreenHeight;

	return Result;
}

void
SetCameraTrasform(render_group *Group, b32 Ortho)
{
	if (Ortho)
	{
		Group->Commands->Proj = OrthographicProjection(Group->Width, Group->Height);
	}
	else
	{
		// TODO: Implement perspective proj
		Assert(0)
	}
}


void
PushFont(render_group *Group, bitmap_info *Glyph, v2 Min, v2 Max, v3 Color)
{
	game_render_commands *Commands = Group->Commands;

	u32 NeedingPushBufferSize = Commands->PushBufferSize + sizeof(render_entry_header) + sizeof(render_entry_bitmap);
	if (NeedingPushBufferSize < Commands->MaxPushBufferSize)
	{
		render_entry_header *Header = (render_entry_header *)(Commands->PushBufferBase + Commands->PushBufferSize);
		Commands->PushBufferSize += sizeof(render_entry_header);
		Header->Type = RenderEntryType_render_entry_bitmap;

		render_entry_bitmap *BitmapEntry = (render_entry_bitmap *)((u8 *)Header + sizeof(render_entry_header));
		BitmapEntry->Bitmap = Glyph;
		BitmapEntry->Min = Min;
		BitmapEntry->Max = Max;
		BitmapEntry->Color = Color;
		Commands->PushBufferSize += sizeof(render_entry_bitmap);
	}
}