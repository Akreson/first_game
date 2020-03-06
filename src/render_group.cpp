
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
	m4x4 Pers = CameraViewTransform->Forward * PersProj.Forward;
	m4x4 InvPers = PersProj.Inverse * CameraViewTransform->Inverse;
	
	Group->Commands->PersProj.Forward = Pers;
	Group->Commands->PersProj.Inverse = InvPers;
	Group->InvPerspective = PersProj.Inverse;
	Group->InvCamera = CameraViewTransform->Inverse;
	Group->CameraZ = GetRow(CameraViewTransform->Forward, 2);
}

inline v2
FromScreenToClipSpace(render_group *Group, v2 Pos)
{
	v2 Result = ((Pos * 2.0f) / Group->ScreenDim) - 1.0f;
	return Result;
}

v3
Unproject(render_group *Group, v2 Mouse)
{
	v4 ClipPos;
	ClipPos.xy = FromScreenToClipSpace(Group, Mouse);
	ClipPos.zw = V2(-1.0f, 1.0f);

	v4 CameraP = ClipPos * Group->InvPerspective;
	v4 WorldRayDir = V4(CameraP.xyz, 0) * Group->InvCamera;

	return WorldRayDir.xyz;
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

struct rendre_model_face_vertex
{
	v3 Vertex;
	v4 MetaInfo;
};

inline rendre_model_face_vertex
ConstractFaceVertexInfo(v3 Vertex, v4 MetaInfo)
{
	rendre_model_face_vertex Result;

	Result.Vertex = Vertex;
	Result.MetaInfo = MetaInfo;

	return Result;
}

internal void
PushModelFace(render_group *Group, v3 *VertexStorage, model_face Face, v4 Color, v3 Offset, v3 EdgeColor = V3(0))
{
	game_render_commands *Commands = Group->Commands;
	render_entry_model_face *ModelFaceEntry = PushRenderElement(Group, render_entry_model_face);

	ModelFaceEntry->Color = Color;
	ModelFaceEntry->VertexBufferOffset = Commands->VertexCount;
	ModelFaceEntry->Offset = Offset;

	ModelFaceEntry->EdgeColor = EdgeColor;

	rendre_model_face_vertex *StartFaceVertex = (rendre_model_face_vertex *)(Commands->VertexBufferBase + Commands->VertexCount);
	rendre_model_face_vertex *FaceVertex = StartFaceVertex;

	*FaceVertex++ = ConstractFaceVertexInfo(VertexStorage[Face.V0], V4(1, 1, 0, 1));
	*FaceVertex++ = ConstractFaceVertexInfo(VertexStorage[Face.V1], V4(0, 1, 0, 1));
	*FaceVertex++ = ConstractFaceVertexInfo(VertexStorage[Face.V2], V4(0, 0, 1, 0));

	*FaceVertex++ = ConstractFaceVertexInfo(VertexStorage[Face.V0], V4(1, 0, 1, 0));
	*FaceVertex++ = ConstractFaceVertexInfo(VertexStorage[Face.V2], V4(0, 1, 0, 0));
	*FaceVertex++ = ConstractFaceVertexInfo(VertexStorage[Face.V3], V4(0, 0, 1, 0));

	Commands->VertexCount += (u32)((FaceVertex - StartFaceVertex)) * sizeof(rendre_model_face_vertex);
}