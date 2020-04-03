
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

	BitmapEntry->Texture = Glyph->Texture;
	BitmapEntry->Min = Min;
	BitmapEntry->Max = Max;
	BitmapEntry->Color = Color;
}

inline render_model_face_vertex
ConstructFaceVertex(v3 Vertex, v3 BarCoords, v3 MetaInfo)
{
	render_model_face_vertex Result;

	Result.Vertex = Vertex;
	Result.BarCoords = BarCoords;
	Result.MetaInfo = MetaInfo;

	return Result;
}

void
PushFace(render_group *Group, v3 *VertexStorage, model_face Face, face_render_params FaceParam = {})
{
	Assert(Group->GroupRenderElement);
	game_render_commands *Commands = Group->Commands;

	render_model_face_vertex *StartFaceVertex = 
		(render_model_face_vertex *)(Commands->VertexBufferBase + Commands->VertexBufferOffset);
	render_model_face_vertex *FaceVertex = StartFaceVertex;

	f32 ActiveArray[2] = {0, 1.0f};

	f32 ActiveVert0 = ActiveArray[FaceParam.ActiveVert[0]];
	f32 ActiveVert1 = ActiveArray[FaceParam.ActiveVert[1]];
	f32 ActiveVert2 = ActiveArray[FaceParam.ActiveVert[2]];
	f32 ActiveVert3 = ActiveArray[FaceParam.ActiveVert[3]];

	f32 HotVert0 = ActiveArray[FaceParam.HotVert[0]];
	f32 HotVert1 = ActiveArray[FaceParam.HotVert[1]];
	f32 HotVert2 = ActiveArray[FaceParam.HotVert[2]];
	f32 HotVert3 = ActiveArray[FaceParam.HotVert[3]];

	f32 SelectionType = 0;
	if (FaceParam.SelectionFlags & FaceSelectionType_Hot) SelectionType += 1.0f;
	if (FaceParam.SelectionFlags & FaceSelectionType_Select) SelectionType += 2.0f;

	*FaceVertex++ = ConstructFaceVertex(VertexStorage[Face.V0], V3(1, 1, 0), V3(SelectionType, ActiveVert0, HotVert0));
	*FaceVertex++ = ConstructFaceVertex(VertexStorage[Face.V1], V3(0, 1, 0), V3(SelectionType, ActiveVert1, HotVert1));
	*FaceVertex++ = ConstructFaceVertex(VertexStorage[Face.V2], V3(0, 0, 1), V3(SelectionType, ActiveVert2, HotVert2));

	*FaceVertex++ = ConstructFaceVertex(VertexStorage[Face.V0], V3(1, 0, 1), V3(SelectionType, ActiveVert0, HotVert0));
	*FaceVertex++ = ConstructFaceVertex(VertexStorage[Face.V2], V3(0, 1, 0), V3(SelectionType, ActiveVert2, HotVert2));
	*FaceVertex++ = ConstructFaceVertex(VertexStorage[Face.V3], V3(0, 0, 1), V3(SelectionType, ActiveVert3, HotVert3));

	Commands->VertexBufferOffset += (u32)((FaceVertex - StartFaceVertex)) * sizeof(render_model_face_vertex);
}

void
BeginPushModel(render_group *Group, v4 Color, v3 Offset, v3 EdgeColor, model_outline_params Outline = {})
{
	game_render_commands *Commands = Group->Commands;
	render_entry_model *ModelEntry = (render_entry_model *)PushRenderElement(Group, render_entry_model);

	ModelEntry->StartOffset = Commands->VertexBufferOffset;
	ModelEntry->Offset = Offset;
	ModelEntry->Color = Color;
	ModelEntry->EdgeColor = EdgeColor;

	if (Outline.IsSet)
	{
		render_entry_model_outline *ModelOutlineEntry =
			(render_entry_model_outline *)PushRenderElement(Group, render_entry_model_outline);
		
		ModelOutlineEntry->ModelEntry = ModelEntry;
		ModelOutlineEntry->OutlineColor = Outline.Color;
	}

	Assert(!Group->GroupRenderElement);
	Group->GroupRenderElement = (void *)ModelEntry;
}

void
EndPushModel(render_group *Group)
{
	game_render_commands *Commands = Group->Commands;
	render_entry_model *ModelEntry = (render_entry_model *)Group->GroupRenderElement;

	ModelEntry->ElementCount =
		(Commands->VertexBufferOffset - ModelEntry->StartOffset) / sizeof(render_model_face_vertex);

	Group->GroupRenderElement = 0;
}

inline void
PushModelFace(render_group *Group, v3 *VertexStorage, model_face Face,
	v4 Color = V4(1), v3 Offset = V3(0), v3 EdgeColor = V3(0))
{
	BeginPushModel(Group, Color, Offset, EdgeColor);
	PushFace(Group, VertexStorage, Face);
	EndPushModel(Group);
}