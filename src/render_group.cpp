
inline render_group
InitRenderGroup(game_render_commands *Commands, game_input *Input, font_asset_info *FontAsset)
{
	render_group Result = {};

	Result.Commands = Commands;
	Result.FontAsset = FontAsset;
	Result.ScreenDim = Commands->ScreenDim;

	return Result;
}

void
SetCameraTrasform(render_group *Group, f32 FocalLength, m4x4_inv *CameraViewTransform)
{
	Group->Commands->OrthoProj = OrthographicProjection(Group->ScreenDim.x, Group->ScreenDim.y);

	m4x4_inv PersProj = PerspectiveProjection(FocalLength, Group->ScreenDim.x / Group->ScreenDim.y);
	Group->Commands->PersProj.Forward = CameraViewTransform->Forward * PersProj.Forward;
	Group->Commands->PersProj.Inverse = CameraViewTransform->Inverse * PersProj.Inverse;
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
	FaceVertex[0] = VertexStorage[Face->V0];
	FaceVertex[1] = V3(1, 1, 0);
	FaceVertex[2] = V3(1.0f);
	FaceVertex[3] = VertexStorage[Face->V1];
	FaceVertex[4] = V3(0, 1, 0);
	FaceVertex[5] = V3(1.0f);
	FaceVertex[6] = VertexStorage[Face->V2];
	FaceVertex[7] = V3(0, 1, 1);
	FaceVertex[8] = V3(0.0f);

	FaceVertex[9] = VertexStorage[Face->V0];
	FaceVertex[10] = V3(1, 0, 1);
	FaceVertex[11] = V3(1.0f);
	FaceVertex[12] = VertexStorage[Face->V2];
	FaceVertex[13] = V3(0, 1, 1);
	FaceVertex[14] = V3(0.0f);
	FaceVertex[15] = VertexStorage[Face->V3];
	FaceVertex[16] = V3(0, 0, 1);
	FaceVertex[17] = V3(0.0f);

	Commands->VertexCount += (18 * 3);
}