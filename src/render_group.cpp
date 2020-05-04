
inline render_group
InitRenderGroup(game_render_commands *Commands, game_input *Input)
{
	render_group Result = {};

	Result.Commands = Commands;
	Result.ScreenDim = Commands->ScreenDim;

	return Result;
}

inline render_alloc_mesh_params
SetAllocMeshParams(void *VertexData, u32 *Tris, u32 VertexCount, u32 TrisCount, u32 Flags = 0)
{
	render_alloc_mesh_params Result;
	Result.VertexData = VertexData;
	Result.VertexCount = VertexCount;
	Result.Tris = Tris;
	Result.TrisCount = TrisCount;
	Result.Flags = Flags;

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
PushFont(render_group *Group, renderer_texture Glyph, v2 Min, v2 Max, v3 Color)
{
	render_entry_bitmap *BitmapEntry = PushRenderElement(Group, render_entry_bitmap);

	BitmapEntry->Texture = Glyph;
	BitmapEntry->Min = Min;
	BitmapEntry->Max = Max;
	BitmapEntry->Color = Color;
}

struct face_edge_params
{
	b8 Active01;
	b8 Active12;
	b8 Active03;
	b8 Active23;

	b8 Hot01;
	b8 Hot12;
	b8 Hot03;
	b8 Hot23;
};

internal inline face_edge_params
GetEdgeFaceParams(face_render_params FaceParam)
{
	face_edge_params Result;

	Result.Active01 = FaceParam.ActiveVert[0] & FaceParam.ActiveVert[1];
	Result.Active12 = FaceParam.ActiveVert[1] & FaceParam.ActiveVert[2];
	Result.Active03 = FaceParam.ActiveVert[0] & FaceParam.ActiveVert[3];
	Result.Active23 = FaceParam.ActiveVert[2] & FaceParam.ActiveVert[3];

	Result.Hot01 = FaceParam.HotVert[0] & FaceParam.HotVert[1];
	Result.Hot12 = FaceParam.HotVert[1] & FaceParam.HotVert[2];
	Result.Hot03 = FaceParam.HotVert[0] & FaceParam.HotVert[3];
	Result.Hot23 = FaceParam.HotVert[2] & FaceParam.HotVert[3];

	return Result;
}

internal inline render_model_face_vertex
ConstructFaceVertex(v3 Vertex, v3 BarCoords, v3 ActiveMask = V3(0), v3 HotMask = V3(0), v2 FaceSelParam = V2(0))
{
	render_model_face_vertex Result;

	Result.Vertex = Vertex;
	Result.BarCoords = BarCoords;
	Result.ActiveMask = ActiveMask;
	Result.HotMask = HotMask;
	Result.FaceSelParam = FaceSelParam;

	return Result;
}

// TODO: Fix select edge stretching on small engel to adjacent face
// TODO: Compress face vertex data?
void
PushFace(render_group *Group, v3 *VertexStorage, model_face Face, face_render_params FaceParam = {})
{
	Assert(Group->GroupRenderElement);
	game_render_commands *Commands = Group->Commands;

	render_model_face_vertex *StartFaceVertex = 
		(render_model_face_vertex *)(Commands->VertexBufferBase + Commands->VertexBufferOffset);
	render_model_face_vertex *FaceVertex = StartFaceVertex;

	f32 StateEdgeArray[2] = {0, 1.0f};
	face_edge_params Edges = GetEdgeFaceParams(FaceParam);

	v2 SelectionType = V2(
		StateEdgeArray[FaceParam.SelectionFlags[FaceSelectionType_Select]],
		StateEdgeArray[FaceParam.SelectionFlags[FaceSelectionType_Hot]]);

	f32 Active01 = StateEdgeArray[Edges.Active01];
	f32 Active12 = StateEdgeArray[Edges.Active12];
	f32 Active03 = StateEdgeArray[Edges.Active03];
	f32 Active23 = StateEdgeArray[Edges.Active23];

	f32 Hot01 = StateEdgeArray[Edges.Hot01];
	f32 Hot12 = StateEdgeArray[Edges.Hot12];
	f32 Hot03 = StateEdgeArray[Edges.Hot03];
	f32 Hot23 = StateEdgeArray[Edges.Hot23];

	*FaceVertex++ = ConstructFaceVertex(VertexStorage[Face.V0],	V3(1, 1, 0));
	*FaceVertex++ = ConstructFaceVertex(VertexStorage[Face.V1], V3(0, 1, 0));
	*FaceVertex++ = ConstructFaceVertex(VertexStorage[Face.V2],
		V3(0, 1, 1), V3(Active12, 1, Active01), V3(Hot12, 1, Hot01), SelectionType);

	*FaceVertex++ = ConstructFaceVertex(VertexStorage[Face.V0], V3(1, 0, 1));
	*FaceVertex++ = ConstructFaceVertex(VertexStorage[Face.V2], V3(0, 1, 1));
	*FaceVertex++ = ConstructFaceVertex(VertexStorage[Face.V3],
		V3(0, 0, 1), V3(Active23, Active03, 1), V3(Hot23, Hot03, 1), SelectionType);

	Commands->VertexBufferOffset += (u32)((FaceVertex - StartFaceVertex)) * sizeof(render_model_face_vertex);
}

void
BeginPushModel(render_group *Group, v4 Color, v3 Offset, model_highlight_params ModelHiLi)
{
	game_render_commands *Commands = Group->Commands;
	render_entry_model *ModelEntry = (render_entry_model *)PushRenderElement(Group, render_entry_model);

	ModelEntry->StartOffset = Commands->VertexBufferOffset;
	ModelEntry->Offset = Offset;
	ModelEntry->Color = Color;
	ModelEntry->EdgeColor = ModelHiLi.EdgeColor;

	if (ModelHiLi.OutlineIsSet)
	{
		render_entry_model_outline *ModelOutlineEntry =
			(render_entry_model_outline *)PushRenderElement(Group, render_entry_model_outline);
		
		ModelOutlineEntry->ModelEntry = ModelEntry;
		ModelOutlineEntry->OutlineColor = ModelHiLi.OutlineColor;
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
	model_highlight_params ModelHiLi = {};
	ModelHiLi.EdgeColor = EdgeColor;

	BeginPushModel(Group, Color, Offset, ModelHiLi);
	PushFace(Group, VertexStorage, Face);
	EndPushModel(Group);
}

struct render_entry_sphere
{
	renderer_mesh Mesh;
	v3 Color;
};

void
PushSphere(render_group *Group, renderer_mesh Mesh, v3 Color = V3(1))
{
	game_render_commands *Commands = Group->Commands;
	render_entry_sphere *SphereEntry = (render_entry_sphere *)PushRenderElement(Group, render_entry_sphere);

	SphereEntry->Mesh = Mesh;
	SphereEntry->Color = Color;
}