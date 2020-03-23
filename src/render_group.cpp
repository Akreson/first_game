
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

	BitmapEntry->TextureID = Glyph->TextureID;
	BitmapEntry->Min = Min;
	BitmapEntry->Max = Max;
	BitmapEntry->Color = Color;
}

inline render_model_face_vertex
ConstructFaceVertexInfo(v4 Vertex, v4 MetaInfo)
{
	render_model_face_vertex Result;

	Result.Vertex = Vertex;
	Result.MetaInfo = MetaInfo;

	return Result;
}

#define FaceSelectionType_Hot 1.0f
#define FaceSelectionType_Select 2.0f

struct face_render_param
{
	f32 SelectionType;
	b8 ActiveVert[4];
};

void
PushFace(render_group *Group, v3 *VertexStorage, model_face Face, face_render_param FaceParam = {})
{
	Assert(Group->GroupRenderElement);
	game_render_commands *Commands = Group->Commands;

	render_model_face_vertex *StartFaceVertex = 
		(render_model_face_vertex *)(Commands->VertexBufferBase + Commands->VertexBufferOffset);
	render_model_face_vertex *FaceVertex = StartFaceVertex;

	f32 ActiveVert[4];
	ActiveVert[0] = FaceParam.ActiveVert[0] ? 1.0f : 0;
	ActiveVert[1] = FaceParam.ActiveVert[1] ? 1.0f : 0;
	ActiveVert[2] = FaceParam.ActiveVert[2] ? 1.0f : 0;
	ActiveVert[3] = FaceParam.ActiveVert[3] ? 1.0f : 0;

	*FaceVertex++ = ConstructFaceVertexInfo(V4(VertexStorage[Face.V0], FaceParam.SelectionType), V4(1, 1, 0, ActiveVert[0]));
	*FaceVertex++ = ConstructFaceVertexInfo(V4(VertexStorage[Face.V1], FaceParam.SelectionType), V4(0, 1, 0, ActiveVert[1]));
	*FaceVertex++ = ConstructFaceVertexInfo(V4(VertexStorage[Face.V2], FaceParam.SelectionType), V4(0, 0, 1, ActiveVert[2]));

	*FaceVertex++ = ConstructFaceVertexInfo(V4(VertexStorage[Face.V0], FaceParam.SelectionType), V4(1, 0, 1, ActiveVert[0]));
	*FaceVertex++ = ConstructFaceVertexInfo(V4(VertexStorage[Face.V2], FaceParam.SelectionType), V4(0, 1, 0, ActiveVert[2]));
	*FaceVertex++ = ConstructFaceVertexInfo(V4(VertexStorage[Face.V3], FaceParam.SelectionType), V4(0, 0, 1, ActiveVert[3]));

	Commands->VertexBufferOffset += (u32)((FaceVertex - StartFaceVertex)) * sizeof(render_model_face_vertex);
}

void
BeginPushModel(render_group *Group, v4 Color, v3 Offset, v3 EdgeColor, v3 OutlineColor = V3(0), b32 SetOutline = false)
{
	game_render_commands *Commands = Group->Commands;
	render_entry_model *ModelEntry = (render_entry_model *)PushRenderElement(Group, render_entry_model);

	ModelEntry->StartOffset = Commands->VertexBufferOffset;
	ModelEntry->Offset = Offset;
	ModelEntry->Color = Color;
	ModelEntry->EdgeColor = EdgeColor;

	if (SetOutline)
	{
		render_entry_model_outline *ModelOutlineEntry =
			(render_entry_model_outline *)PushRenderElement(Group, render_entry_model_outline);
		
		ModelOutlineEntry->ModelEntry = ModelEntry;
		ModelOutlineEntry->OutlineColor = OutlineColor;
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