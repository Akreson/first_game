
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
SetCameraTrasform(render_group *Group, f32 FocalLength, b32 Ortho)
{
	if (Ortho)
	{
		Group->Commands->Proj = OrthographicProjection(Group->Width, Group->Height);
	}
	else
	{
		Group->Commands->Proj = PerspectiveProjection(FocalLength, Group->Width / Group->Height);
	}
}

void *
PushRenderElement_(render_group *Group, u32 Size, render_entry_type Type)
{
	void *Result = nullptr;
	game_render_commands *Commands = Group->Commands;

	u32 NeedingPushBufferSize = Commands->PushBufferSize + sizeof(render_entry_header) + Size;
	if (NeedingPushBufferSize < Commands->MaxPushBufferSize)
	{
		render_entry_header *Header = (render_entry_header *)(Commands->PushBufferBase + Commands->PushBufferSize);
		Commands->PushBufferSize += sizeof(render_entry_header);

		Header->Type = Type;

		Result = (void *)(Header + 1);
		Commands->PushBufferSize += Size;
	}
	else
	{
		Assert(0);
	}

	return Result;
}

#define PushRenderElement(Group, Type) (Type *)PushRenderElement_(Group, sizeof(Type), RenderEntryType_##Type)

internal void
PushFont(render_group *Group, bitmap_info *Glyph, v2 Min, v2 Max, v3 Color)
{
	render_entry_bitmap *BitmapEntry = PushRenderElement(Group, render_entry_bitmap);
	BitmapEntry->Bitmap = Glyph;
	BitmapEntry->Min = Min;
	BitmapEntry->Max = Max;
	BitmapEntry->Color = Color;
}

struct render_entry_model
{
	v4 Color;
	f32 *Vertex;
	u16 VertexCount;
};

internal void
PushModel(render_group *Group, model *Model)
{
	render_entry_model *ModelEntry = PushRenderElement(Group, render_entry_model);
	ModelEntry->Color = Model->Color;
	ModelEntry->Vertex = Model->Vertex;
	ModelEntry->VertexCount = Model->VertexCount;
}