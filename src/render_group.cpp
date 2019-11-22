
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
	ModelEntry->Offset = Model->Offset;
}

internal void
PushModelFace(render_group *Group, v3 *VertexStorage, model_face *Face, v4 Color, v3 Offset)
{
	game_render_commands *Commands = Group->Commands;
	render_entry_model_face *ModelFaceEntry = PushRenderElement(Group, render_entry_model_face);

	ModelFaceEntry->Color = Color;
	ModelFaceEntry->VertexBufferOffset = Commands->VertexCount;
	ModelFaceEntry->Offset = Offset;

	v3 *FaceVertex = (v3 *)(Commands->VertexBufferBase + Commands->VertexCount);
	FaceVertex[0] = VertexStorage[Face->V1];
	FaceVertex[1] = VertexStorage[Face->V2];
	FaceVertex[2] = VertexStorage[Face->V3];
	FaceVertex[3] = VertexStorage[Face->V1];
	FaceVertex[4] = VertexStorage[Face->V3];
	FaceVertex[5] = VertexStorage[Face->V4];

	Commands->VertexCount += 6 * 3;
}