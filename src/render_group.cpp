
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
CreateTrinVertex(v3 V, v2 UV, v3 Color = V3(1.0f))
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

struct render_entry_trin_model
{

};

void
BeginPushTrinModel(render_group *Group)
{
	game_render_commands *Commands = Group->Commands;
}

void
EndPushTrinModel()
{

}

internal inline rect3
CalcScaleToolAxisRect()
{

}

void
PushScaleTool(render_group *Group, m3x3 Axis, f32 Scale, f32 EdgeLength, f32 EdgeHalfSize, f32 ArrowSize)
{
	v3 XMinStartP = Axis.X * (EdgeLength - (EdgeLength * 0.85f));
	v3 XMaxStartP = Axis.X * EdgeLength;

	rect3 XRect0 = {};
	XRect0.A = MovePointAlongDir(XRect0.A, Axis.Y, EdgeHalfSize);
	XRect0.A = MovePointAlongDir(XRect0.A, -Axis.Z, EdgeHalfSize);
	XRect0.B = MovePointAlongDir(XRect0.B, -Axis.Y, EdgeHalfSize);
	XRect0.B = MovePointAlongDir(XRect0.B, Axis.Z, EdgeHalfSize);
	rect3 XRect1 = {};
	XRect1.A = MovePointAlongDir(XRect0.A, -Axis.X, EdgeLength);
	XRect1.B = MovePointAlongDir(XRect0.B, -Axis.X, EdgeLength);
}

#define IsHaveMatch(A, B) \
	_mm_movemask_ps(_mm_castsi128_ps( \
		_mm_cmpeq_epi32(_mm_and_si128(A, B), B)))

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

	Result.Active01 = (IsHaveMatch(Active, Mask01)) ? true : false;
	Result.Active12 = (IsHaveMatch(Active, Mask12)) ? true : false;
	Result.Active03 = (IsHaveMatch(Active, Mask03)) ? true : false;
	Result.Active23 = (IsHaveMatch(Active, Mask23)) ? true : false;

	Result.Hot01 = (IsHaveMatch(Hot, Mask01)) ? true : false;
	Result.Hot12 = (IsHaveMatch(Hot, Mask12)) ? true : false;
	Result.Hot03 = (IsHaveMatch(Hot, Mask03)) ? true : false;
	Result.Hot23 = (IsHaveMatch(Hot, Mask23)) ? true : false;

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
	Assert(Group->GroupRenderElement);
	game_render_commands *Commands = Group->Commands;

	render_model_face_vertex *StartFaceVertex = 
		(render_model_face_vertex *)(Commands->VertexBufferBase + Commands->VertexBufferOffset);
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
	*FaceVertex++ = CreateFaceVertex(VertexStorage[Face.V2],
		V3(0, 1, 1), V3(Active12, 1, Active01), V3(Hot12, 1, Hot01), SelectionType);

	*FaceVertex++ = CreateFaceVertex(VertexStorage[Face.V0], V3(1, 0, 1));
	*FaceVertex++ = CreateFaceVertex(VertexStorage[Face.V2], V3(0, 1, 1));
	*FaceVertex++ = CreateFaceVertex(VertexStorage[Face.V3],
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

struct render_entry_static_mesh
{
	renderer_mesh Mesh;
	v3 Color;
};

void
PushSphere(render_group *Group, renderer_mesh Mesh, v3 Color = V3(1))
{
	game_render_commands *Commands = Group->Commands;
	render_entry_static_mesh *SphereEntry = (render_entry_static_mesh *)PushRenderElement(Group, render_entry_static_mesh);

	SphereEntry->Mesh = Mesh;
	SphereEntry->Color = Color;
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