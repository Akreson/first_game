
inline render_group
InitRenderGroup(game_render_commands *Commands, game_input *Input, font_asset_info *Font)
{
	render_group Result = {};

	Result.Commands = Commands;
	Result.ScreenDim = Commands->ScreenDim;
	Result.FontAsset = Font;

	return Result;
}

inline render_alloc_mesh_params
SetAllocMeshParams(void *VertexData, u32 *Tris, u32 VertexCount, u32 TrisCount, u32 Flags)
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
	m4x4_inv PersProj = PerspectiveProjection(FocalLength, Group->ScreenDim.x / Group->ScreenDim.y);
	m4x4 Pers = CameraViewTransform->Forward * PersProj.Forward;
	m4x4 InvPers = PersProj.Inverse * CameraViewTransform->Inverse;
	
	Group->Commands->PersProj = PersProj;
	Group->Commands->OrthoProj = OrthographicProjection(Group->ScreenDim.x, Group->ScreenDim.y);
	Group->Commands->CameraTransform = *CameraViewTransform;
	Group->Commands->ForwardPersCamera = Pers;
	Group->InvPerspective = PersProj.Inverse;
	Group->InvCamera = CameraViewTransform->Inverse;
	//Group->CameraZ = GetRow(CameraViewTransform->Forward, 2);
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
	v4 WorldRayDir = V4(CameraP.xy, -1.0f, 0) * Group->InvCamera;

	return Normalize(WorldRayDir.xyz);
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

inline render_triangle_vertex
CreateTrinVertex(v3 V, v2 UV, v4 Color = V4(1.0f))
{
	render_triangle_vertex Result;
	Result.V = V;
	Result.UV = UV;
	Result.Color = Color;

	return Result;
}

void
PushBitmap(render_group *Group, renderer_texture BitmapTex, v2 Min, v2 Max, v3 Color)
{
	game_render_commands *Commands = Group->Commands;
	render_entry_bitmap *BitmapEntry = PushRenderElement(Group, render_entry_bitmap);

	render_triangle_vertex *TrinBuff =
		(render_triangle_vertex *)(Commands->TriangleBufferBase + Commands->TriangleBufferSize);

	*TrinBuff++ = CreateTrinVertex(V3(Min.x, Min.y, 0.0f), V2(0.0f, 0.0f));// bottom left
	*TrinBuff++ = CreateTrinVertex(V3(Max.x, Min.y, 0.0f), V2(1.0f, 0.0f));// bottom right
	*TrinBuff++ = CreateTrinVertex(V3(Max.x, Max.y, 0.0f), V2(1.0f, 1.0f));// top right
	// second triangle
	*TrinBuff++ = CreateTrinVertex(V3(Min.x, Min.y, 0.0f), V2(0.0f, 0.0f));// bottom left
	*TrinBuff++ = CreateTrinVertex(V3(Max.x, Max.y, 0.0f), V2(1.0f, 1.0f));// top right
	*TrinBuff++ = CreateTrinVertex(V3(Min.x, Max.y, 0.0f), V2(0.0f, 1.0f));// top left 

	BitmapEntry->Texture = BitmapTex;
	BitmapEntry->Color = Color;
	BitmapEntry->TrinBuffOffset = Commands->TriangleBufferSize;

	Commands->TriangleBufferSize += 6 * sizeof(render_triangle_vertex);
}

void
BeginPushTrinModel(render_group *Group, v3 Pos)
{
	game_render_commands *Commands = Group->Commands;
	render_entry_trin_model *TrinModelEntry =
		(render_entry_trin_model *)PushRenderElement(Group, render_entry_trin_model);

	TrinModelEntry->Pos = Pos;
	TrinModelEntry->StartOffset = Commands->TriangleBufferSize;

	SetGuardPtr(&Commands->TBuffGroupGuard, (void *)TrinModelEntry);
}

void
EndPushTrinModel(render_group *Group)
{
	game_render_commands *Commands = Group->Commands;
	render_entry_trin_model *TrinModelEntry = (render_entry_trin_model *)Commands->TBuffGroupGuard;

	TrinModelEntry->ElementCount =
		(Commands->TriangleBufferSize - TrinModelEntry->StartOffset) / sizeof(render_triangle_vertex);

	ResetGuardPtr(&Commands->TBuffGroupGuard);
}

#define IsHaveMatch(A, B) \
	(_mm_movemask_ps(_mm_castsi128_ps( \
		_mm_cmpeq_epi32(_mm_and_si128(A, B), B))))

internal inline face_edge_params
GetEdgeFaceParams(face_render_params FaceParam)
{
	face_edge_params Result;

	__m128i Active = _mm_set_epi32(
			FaceParam.ActiveEdge[3], FaceParam.ActiveEdge[2],
			FaceParam.ActiveEdge[1], FaceParam.ActiveEdge[0]);

	__m128i Hot = _mm_set_epi32(
		FaceParam.HotEdge[3], FaceParam.HotEdge[2],
		FaceParam.HotEdge[1], FaceParam.HotEdge[0]);

	__m128i Mask01 = _mm_set1_epi32(MaskMatchVertex_01);
	__m128i Mask12 = _mm_set1_epi32(MaskMatchVertex_12);
	__m128i Mask03 = _mm_set1_epi32(MaskMatchVertex_03);
	__m128i Mask23 = _mm_set1_epi32(MaskMatchVertex_23);

	// TODO: Remove condition

	Result.Active01 = IsHaveMatch(Active, Mask01) ? true : false;
	Result.Active12 = IsHaveMatch(Active, Mask12) ? true : false;
	Result.Active03 = IsHaveMatch(Active, Mask03) ? true : false;
	Result.Active23 = IsHaveMatch(Active, Mask23) ? true : false;

	Result.Hot01 = IsHaveMatch(Hot, Mask01) ? true : false;
	Result.Hot12 = IsHaveMatch(Hot, Mask12) ? true : false;
	Result.Hot03 = IsHaveMatch(Hot, Mask03) ? true : false;
	Result.Hot23 = IsHaveMatch(Hot, Mask23) ? true : false;

	return Result;
}

#undef IsHaveMatch(A, B)

internal inline render_model_face_vertex
CreateFaceVertex(v3 Vertex, v3 BarCoords,
	v3 ActiveMask = V3(0), v3 HotMask = V3(0), v2 FaceSelParam = V2(0))
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
	game_render_commands *Commands = Group->Commands;

	Assert(Commands->VBuffGroupGuard);

	render_model_face_vertex *StartFaceVertex = 
		(render_model_face_vertex *)(Commands->VertexBufferBase + Commands->VertexBufferSize);
	render_model_face_vertex *FaceVertex = StartFaceVertex;

	f32 StateEdgeArray[2] = {0, 1.0f};
	face_edge_params EdgesParam = GetEdgeFaceParams(FaceParam);

	v2 SelectionType = V2(
		StateEdgeArray[FaceParam.SelectionFlags[FaceSelectionType_Select]],
		StateEdgeArray[FaceParam.SelectionFlags[FaceSelectionType_Hot]]);

	f32 Active01 = StateEdgeArray[EdgesParam.Active01];
	f32 Active12 = StateEdgeArray[EdgesParam.Active12];
	f32 Active03 = StateEdgeArray[EdgesParam.Active03];
	f32 Active23 = StateEdgeArray[EdgesParam.Active23];

	f32 Hot01 = StateEdgeArray[EdgesParam.Hot01];
	f32 Hot12 = StateEdgeArray[EdgesParam.Hot12];
	f32 Hot03 = StateEdgeArray[EdgesParam.Hot03];
	f32 Hot23 = StateEdgeArray[EdgesParam.Hot23];

	*FaceVertex++ = CreateFaceVertex(VertexStorage[Face.V0], V3(1, 1, 0));
	*FaceVertex++ = CreateFaceVertex(VertexStorage[Face.V1], V3(0, 1, 0));
	*FaceVertex++ = CreateFaceVertex(VertexStorage[Face.V2], V3(0, 1, 1),
		V3(Active12, 1, Active01), V3(Hot12, 1, Hot01), SelectionType);

	*FaceVertex++ = CreateFaceVertex(VertexStorage[Face.V0], V3(1, 0, 1));
	*FaceVertex++ = CreateFaceVertex(VertexStorage[Face.V2], V3(0, 1, 1));
	*FaceVertex++ = CreateFaceVertex(VertexStorage[Face.V3], V3(0, 0, 1),
		V3(Active23, Active03, 1), V3(Hot23, Hot03, 1), SelectionType);

	Commands->VertexBufferSize += (u32)((FaceVertex - StartFaceVertex)) * sizeof(render_model_face_vertex);
}

void
BeginPushModel(render_group *Group, v4 Color, v3 Offset, model_highlight_params ModelHiLi)
{
	game_render_commands *Commands = Group->Commands;
	render_entry_model *ModelEntry = (render_entry_model *)PushRenderElement(Group, render_entry_model);

	ModelEntry->StartOffset = Commands->VertexBufferSize;
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

	SetGuardPtr(&Commands->VBuffGroupGuard, (void *)ModelEntry);
}

void
EndPushModel(render_group *Group)
{
	game_render_commands *Commands = Group->Commands;
	render_entry_model *ModelEntry = (render_entry_model *)Commands->VBuffGroupGuard;

	ModelEntry->ElementCount =
		(Commands->VertexBufferSize - ModelEntry->StartOffset) / sizeof(render_model_face_vertex);

	ResetGuardPtr(&Commands->VBuffGroupGuard);
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

void
PushStaticMesh(render_group *Group, renderer_mesh Mesh, v3 Pos, f32 Scale, v3 Color = V3(1))
{
	game_render_commands *Commands = Group->Commands;
	render_entry_static_mesh *MeshEntry = (render_entry_static_mesh *)PushRenderElement(Group, render_entry_static_mesh);

	MeshEntry->Mesh = Mesh;
	MeshEntry->Color = Color;
	MeshEntry->Pos = Pos;
	MeshEntry->Scale = Scale;
}

internal inline void
PushTrinRect(game_render_commands *Commands, plane_rect3 Rect, v4 Color)
{
	render_triangle_vertex *Buff =
		(render_triangle_vertex *)(Commands->TriangleBufferBase + Commands->TriangleBufferSize);

	Buff[0] = CreateTrinVertex(Rect.V0, V2(0), Color);
	Buff[1] = CreateTrinVertex(Rect.V1, V2(0), Color);
	Buff[2] = CreateTrinVertex(Rect.V2, V2(0), Color);
	Buff[3] = CreateTrinVertex(Rect.V0, V2(0), Color);
	Buff[4] = CreateTrinVertex(Rect.V2, V2(0), Color);
	Buff[5] = CreateTrinVertex(Rect.V3, V2(0), Color);

	Commands->TriangleBufferSize += sizeof(render_triangle_vertex) * 6;
}

internal inline void
PushTrinRect(render_triangle_vertex *Buff, v3 Vec0, v3 Vec1, v3 Vec2, v3 Vec3, v4 Color)
{
	// TODO: Delete debug code later
#if 1
	Buff[0] = CreateTrinVertex(Vec0, V2(0), Color);
	Buff[1] = CreateTrinVertex(Vec1, V2(0), Color);
	Buff[2] = CreateTrinVertex(Vec2, V2(0), Color);
#else
	Buff[0] = CreateTrinVertex(Vec0, V2(0), V3(1, 0, 0));
	Buff[1] = CreateTrinVertex(Vec2, V2(0), V3(0, 1, 0));
	Buff[2] = CreateTrinVertex(Vec3, V2(0), V3(0, 0, 1));
#endif

#if 1
	Buff[3] = CreateTrinVertex(Vec0, V2(0), Color);
	Buff[4] = CreateTrinVertex(Vec2, V2(0), Color);
	Buff[5] = CreateTrinVertex(Vec3, V2(0), Color);
#endif
}

internal void
PushUnalignRectAsTrin(game_render_commands *Commands, unalign_rect3 A, v4 Color)
{
	render_triangle_vertex *Buff =
		(render_triangle_vertex *)(Commands->TriangleBufferBase + Commands->TriangleBufferSize);
#if 1
	PushTrinRect(Buff, A.Rect0.V0, A.Rect0.V1, A.Rect0.V2, A.Rect0.V3, Color);// front
	PushTrinRect((Buff + 6), A.Rect1.V1, A.Rect1.V0, A.Rect1.V3, A.Rect1.V2, Color); // back
	PushTrinRect((Buff + 12), A.Rect1.V0, A.Rect0.V1, A.Rect0.V3, A.Rect1.V3, Color); // left
	PushTrinRect((Buff + 18), A.Rect0.V1, A.Rect1.V1, A.Rect1.V2, A.Rect0.V2, Color); // right
	PushTrinRect((Buff + 24), A.Rect0.V3, A.Rect0.V2, A.Rect1.V2, A.Rect1.V3, Color); // top
	PushTrinRect((Buff + 30), A.Rect0.V1, A.Rect0.V0, A.Rect1.V0, A.Rect1.V1, Color); // bottom
#else
	// NOTE: For debug
	PushTrinRect(Buff, A.Rect0.V0, A.Rect0.V1, A.Rect0.V2, A.Rect0.V3, V4(1)); // front
	PushTrinRect((Buff + 6), A.Rect1.V1, A.Rect1.V0, A.Rect1.V3, A.Rect1.V2, V4(1, 0, 0, 1)); // back
	PushTrinRect((Buff + 12), A.Rect1.V0, A.Rect0.V0, A.Rect0.V3, A.Rect1.V3, V4(0, 1, 0, 1)); // left
	PushTrinRect((Buff + 18), A.Rect0.V1, A.Rect1.V1, A.Rect1.V2, A.Rect0.V2, V4(0, 0, 1, 1)); // right
	PushTrinRect((Buff + 24), A.Rect0.V3, A.Rect0.V2, A.Rect1.V2, A.Rect1.V3, V4(0.5f , 0, 0.5f, 1)); // top
	PushTrinRect((Buff + 30), A.Rect0.V1, A.Rect0.V0, A.Rect1.V0, A.Rect1.V1, V4(1, 0.5f, 0, 1)); // bottom
#endif

	Commands->TriangleBufferSize += sizeof(render_triangle_vertex) * 36;
}

struct render_tool_elem_color
{
	v4 X, Y, Z;
};

internal inline render_tool_elem_color
GetRenderAxisColor(v4 Mask)
{
	render_tool_elem_color Result;
	v3 ActiveColor = V3(0.86f, 0.65f, 0.2f);

	Result.X.rgb = Lerp(V3(0, 0.6f, 0), Mask.x, V3(0, 1, 0));
	Result.X.rgb = Lerp(Result.X.rgb, Mask.x * Mask.w, ActiveColor);

	Result.Y.rgb = Lerp(V3(0.6f, 0, 0), Mask.y, V3(1, 0, 0));
	Result.Y.rgb = Lerp(Result.Y.rgb, Mask.y * Mask.w, ActiveColor);

	Result.Z.rgb = Lerp(V3(0, 0, 0.6f), Mask.z, V3(0, 0, 1));
	Result.Z.rgb = Lerp(Result.Z.rgb, Mask.z * Mask.w, ActiveColor);

	Result.X.a = Result.Y.a = Result.Z.a = 1.0f;

	return Result;
}

internal inline render_tool_elem_color
GetRenderAxisPlaneColor(v4 Mask)
{
	render_tool_elem_color Result;

	v3 ActiveColor = V3(0.86f, 0.65f, 0.2f);
	
	Result.X.a = Lerp(0.2f, Mask.x, 0.5f);
	Result.Y.a = Lerp(0.2f, Mask.y, 0.5f);
	Result.Z.a = Lerp(0.2f, Mask.z, 0.5f);

	Result.X.rgb = Lerp(V3(0, 0.8f, 0), Mask.x * Mask.w, ActiveColor);
	Result.Y.rgb = Lerp(V3(0.8f, 0, 0), Mask.y * Mask.w, ActiveColor);
	Result.Z.rgb = Lerp(V3(0, 0, 0.8f), Mask.z * Mask.w, ActiveColor);

	return Result;
}

void
PushScaleTool(render_group *Group,  v3 Pos, m3x3 Axis,
	scl_tool_display_params AxisParams, v4 AxisMask)
{
	game_render_commands *Commands = Group->Commands;
	render_tool_elem_color AxisColor = GetRenderAxisColor(AxisMask);

	f32 ArrowZLen = AxisParams.ArrowHalfSize * 2.0f;
	v3 ArrowHalfDim = V3(AxisParams.ArrowHalfSize);
	
	v3 XArrowCenter = Axis.X * AxisParams.X.Len;
	v3 YArrowCenter = Axis.Y * AxisParams.Y.Len;
	v3 ZArrowCenter = Axis.Z * AxisParams.Z.Len;

	v3 XEdgeCenter = Axis.X * AxisParams.X.EdgeCenter;
	v3 YEdgeCenter = Axis.Y * AxisParams.Y.EdgeCenter;
	v3 ZEdgeCenter = Axis.Z * AxisParams.Z.EdgeCenter;
	
	v3 XEdgeHalfDim = V3(AxisParams.EdgeXYHalfSize, AxisParams.EdgeXYHalfSize, AxisParams.X.EdgeLenHalfSize);
	v3 YEdgeHalfDim = V3(AxisParams.EdgeXYHalfSize, AxisParams.EdgeXYHalfSize, AxisParams.Y.EdgeLenHalfSize);
	v3 ZEdgeHalfDim = V3(AxisParams.EdgeXYHalfSize, AxisParams.EdgeXYHalfSize, AxisParams.Z.EdgeLenHalfSize);
	
	unalign_rect3 XArrow = CreateRect(XArrowCenter, -Axis.Z, Axis.Y, Axis.X, ArrowHalfDim);
	unalign_rect3 XEdge = CreateRect(XEdgeCenter, -Axis.Z, Axis.Y, Axis.X, XEdgeHalfDim);

	unalign_rect3 YArrow = CreateRect(YArrowCenter, Axis.Z, Axis.X, Axis.Y, ArrowHalfDim);
	unalign_rect3 YEdge = CreateRect(YEdgeCenter, Axis.Z, Axis.X, Axis.Y, YEdgeHalfDim);

	unalign_rect3 ZArrow = CreateRect(ZArrowCenter, Axis.X, Axis.Y, Axis.Z, ArrowHalfDim);
	unalign_rect3 ZEdge = CreateRect(ZEdgeCenter, Axis.X, Axis.Y, Axis.Z, ZEdgeHalfDim);

	BeginPushTrinModel(Group, Pos);

	PushUnalignRectAsTrin(Commands, XEdge, AxisColor.X);
	PushUnalignRectAsTrin(Commands, XArrow, AxisColor.X);

	PushUnalignRectAsTrin(Commands, YEdge, AxisColor.Y);
	PushUnalignRectAsTrin(Commands, YArrow, AxisColor.Y);

	PushUnalignRectAsTrin(Commands, ZEdge, AxisColor.Z);
	PushUnalignRectAsTrin(Commands, ZArrow, AxisColor.Z);
	
	EndPushTrinModel(Group);
}

internal inline plane_rect3
CreatePlaneRectFromAxis(v3 P, v3 XAxis, v3 YAxis, v2 Dim)
{
	plane_rect3 Result;

	Result.V0 = V3(0);
	Result.V3 = (YAxis * Dim.y);
	Result.V1 = (XAxis * Dim.x);
	Result.V2 = Result.V3 + Result.V1;

	return Result;
}

void
PushTranslateTool(render_group *Group, trans_tool_axis_params AxisParams, m3x3 Axis,
	v4 AxisMask, v4 PlaneMask, v3 Pos, f32 Scale, renderer_mesh ArrowMesh)
{
	game_render_commands *Commands = Group->Commands;
	render_tool_elem_color AxisColor = GetRenderAxisColor(AxisMask);
	render_tool_elem_color PalneColor = GetRenderAxisPlaneColor(PlaneMask);

	/*v3 XEdgeCenter = Axis.X * AxisParams.Axis.EdgeCenter;
	v3 YEdgeCenter = Axis.Y * AxisParams.Axis.EdgeCenter;
	v3 ZEdgeCenter = Axis.Z * AxisParams.Axis.EdgeCenter;*/

	v3 EdgeHalfDim = V3(AxisParams.EdgeXYHalfSize, AxisParams.EdgeXYHalfSize, AxisParams.Axis.EdgeLenHalfSize);
	unalign_rect3 XEdge = CreateRect(Axis.X * AxisParams.Axis.EdgeCenter, -Axis.Z, Axis.Y, Axis.X, EdgeHalfDim);
	unalign_rect3 YEdge = CreateRect(Axis.Y * AxisParams.Axis.EdgeCenter, Axis.Z, Axis.X, Axis.Y, EdgeHalfDim);
	unalign_rect3 ZEdge = CreateRect(Axis.Z * AxisParams.Axis.EdgeCenter, Axis.X, Axis.Y, Axis.Z, EdgeHalfDim);

	v2 PlaneDim = V2(AxisParams.PlaneRelDim);
	plane_rect3 XPlane = CreatePlaneRectFromAxis(Pos, Axis.Y, Axis.Z, PlaneDim);
	plane_rect3 YPlane = CreatePlaneRectFromAxis(Pos, Axis.Z, Axis.X, PlaneDim);
	plane_rect3 ZPlane = CreatePlaneRectFromAxis(Pos, Axis.X, Axis.Y, PlaneDim);

	BeginPushTrinModel(Group, Pos);

	PushUnalignRectAsTrin(Commands, XEdge, AxisColor.X);
	PushUnalignRectAsTrin(Commands, YEdge, AxisColor.Y);
	PushUnalignRectAsTrin(Commands, ZEdge, AxisColor.Z);

	PushTrinRect(Commands, XPlane, PalneColor.X);
	PushTrinRect(Commands, YPlane, PalneColor.Y);
	PushTrinRect(Commands, ZPlane, PalneColor.Z);

	EndPushTrinModel(Group);

	v3 XArrowCenter = Pos + (Axis.X * AxisParams.Axis.Len);
	v3 YArrowCenter = Pos + (Axis.Y * AxisParams.Axis.Len);
	v3 ZArrowCenter = Pos + (Axis.Z * AxisParams.Axis.Len);
	
	PushStaticMesh(Group, ArrowMesh, XArrowCenter, Scale, AxisColor.X.rgb);
	PushStaticMesh(Group, ArrowMesh, YArrowCenter, Scale, AxisColor.Y.rgb);
	PushStaticMesh(Group, ArrowMesh, ZArrowCenter, Scale, AxisColor.Z.rgb);
}

void
PushRotateTool(render_group *Group, renderer_mesh Mesh, v3 Pos, f32 Scale,
	m3x3 Axis, v4 AxisMask, v2i PerpInfo, v3 ViewDir)
{
	game_render_commands *Commands = Group->Commands;
	render_entry_tool_rotate *RotateToolEntry = (render_entry_tool_rotate *)PushRenderElement(Group, render_entry_tool_rotate);

	RotateToolEntry->Mesh = Mesh;
	RotateToolEntry->XAxis = Axis.X;
	RotateToolEntry->YAxis = Axis.Y;
	RotateToolEntry->ZAxis = Axis.Z;
	RotateToolEntry->Scale = Scale;
	RotateToolEntry->AxisActivityState = AxisMask;
	RotateToolEntry->Pos = Pos;
	RotateToolEntry->ViewDir = ViewDir;
	RotateToolEntry->PerpInfo = PerpInfo;
}

void
RenderText(render_group *Group, char *Text, v3 TextColor, f32 ScreenX, f32 ScreenY, f32 Scale)
{
	font_asset_info *FontAsset = Group->FontAsset;
	ScreenY -= FontAsset->AscenderHeight*Scale;

	// TODO: Use codepoint?
	u32 PrevGlyphIndex = 0;
	for (;
		*Text;
		++Text)
	{
		u32 GlyphIndex = GetGlyphIndexFromCodePoint(FontAsset, *Text);

		if (*Text != ' ')
		{
			bitmap_info *Glyph = GetGlyphBitmap(FontAsset, GlyphIndex);

			f32 Width = (f32)Glyph->Width * Scale;
			f32 Height = (f32)Glyph->Height * Scale;

			f32 XPos = ScreenX;
			f32 YPos = ScreenY - (FontAsset->VerticalAdjast[GlyphIndex] * (f32)Glyph->Height * Scale);

			PushBitmap(Group, Glyph->Texture, V2(XPos, YPos), V2(XPos + Width, YPos + Height), TextColor);
		}

		ScreenX += GetHorizontalAdvance(FontAsset, PrevGlyphIndex, GlyphIndex, Scale);

		PrevGlyphIndex = GlyphIndex;
	}
}