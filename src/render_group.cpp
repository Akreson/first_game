
inline render_group
InitRenderGroup(game_render_commands *Commands, game_input *Input, font_asset_info *FontAsset)
{
	render_group Result = {};

	Result.Commands = Commands;
	Result.FontAsset = FontAsset;
	Result.ScreenDim = Commands->ScreenDim;

	return Result;
}

inline m4x4
CameraViewTransform(m4x4 *CameraR, v3 CameraO)
{
	m4x4 Result = *CameraR;
	Translate(&Result, CameraO);

	return Result;
}

void
SetCameraTrasform(render_group *Group, f32 FocalLength, m4x4 *CameraViewTransform)
{
	Group->Commands->OrthoProj = OrthographicProjection(Group->ScreenDim.x, Group->ScreenDim.y);

	Group->Commands->PersProj = PerspectiveProjection(FocalLength, Group->ScreenDim.x / Group->ScreenDim.y);
	Group->Commands->PersProj = *CameraViewTransform * Group->Commands->PersProj;
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

internal void
PushModel(render_group *Group, model *Model)
{
	render_entry_model *ModelEntry = PushRenderElement(Group, render_entry_model);
	ModelEntry->Color = Model->Color;
	ModelEntry->Vertex = Model->Vertex;
	ModelEntry->VertexCount = Model->VertexCount;
}