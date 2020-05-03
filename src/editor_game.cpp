#include "editor_game.h"
#include "render_group.cpp"
#include "model.cpp"
#include "asset.cpp"
#include "ui.cpp"

// TODO: Delete
#include <cstdio>

void
RenderText(render_group *Group, font_asset_info *FontAsset, char *Text, v3 TextColor, f32 ScreenX, f32 ScreenY, f32 Scale)
{
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

			PushFont(Group, Glyph->Texture, V2(XPos, YPos), V2(XPos + Width, YPos + Height), TextColor);
		}

		ScreenX += GetHorizontalAdvance(FontAsset, PrevGlyphIndex, GlyphIndex, Scale);

		PrevGlyphIndex = GlyphIndex;
	}
}

struct faces_edge_match
{
	u16 Index;
	b16 Succes;
};

inline faces_edge_match
MatchFaceEdge(model_face *A, model_face *B)
{
	faces_edge_match Result;

	__m128i EdgesA = _mm_load_si128((__m128i *)A->EdgesID);
	__m128i EdgesB = _mm_load_si128((__m128i *)B->EdgesID);

	__m128i EdgeB0 = ShuffleU324x(EdgesB, 0);
	__m128i EdgeB1 = ShuffleU324x(EdgesB, 1);
	__m128i EdgeB2 = ShuffleU324x(EdgesB, 2);
	__m128i EdgeB3 = ShuffleU324x(EdgesB, 3);

	__m128i CmpMask0 = _mm_cmpeq_epi32(EdgesA, EdgeB0);
	__m128i CmpMask1 = _mm_cmpeq_epi32(EdgesA, EdgeB1);
	__m128i CmpMask2 = _mm_cmpeq_epi32(EdgesA, EdgeB2);
	__m128i CmpMask3 = _mm_cmpeq_epi32(EdgesA, EdgeB3);

	__m128i OrMask01 = _mm_or_si128(CmpMask0, CmpMask1);
	__m128i OrMask23 = _mm_or_si128(CmpMask2, CmpMask3);
	__m128i OrMask = _mm_or_si128(OrMask01, OrMask23);

	u32 Mask32 = _mm_movemask_ps(_mm_castsi128_ps(OrMask));
	Assert(CountOfSetBits(Mask32) <= 1);

	bit_scan_result MaskResult = FindLeastSignificantSetBit(Mask32);

	Result.Succes = MaskResult.Succes;
	Result.Index = A->EdgesID[MaskResult.Index];

	return Result;
}

inline b32
MatchFaceEdge(model_face *A, u32 B)
{
	__m128i EdgeA = _mm_load_si128((__m128i *)A->EdgesID);
	__m128i EdgeB0 = _mm_set1_epi32(B);

	__m128i CmpMask0 = _mm_cmpeq_epi32(EdgeA, EdgeB0);

	u32 Mask32 = _mm_movemask_ps(_mm_castsi128_ps(CmpMask0));
	Assert(CountOfSetBits(Mask32) <= 1);

	b32 Result = FindLeastSignificantSetBit(Mask32).Succes;
	return Result;
}

// TODO: Can be more then 2 vertex?
struct face_vertex_match
{
	u16 Index[2];
	b16 Succes;
};

inline face_vertex_match
MatchFaceVertex(model_face *A, model_edge *B)
{
	face_vertex_match Result = {};

	__m128i VertexA = _mm_load_si128((__m128i *)A->VertexID);

	__m128i EdgeB = _mm_load_si128((__m128i *)B);
	__m128i VertexB0 = ShuffleU324x(EdgeB, 0);
	__m128i VertexB1 = ShuffleU324x(EdgeB, 1);

	__m128i CmpMask0 = _mm_cmpeq_epi32(VertexA, VertexB0);
	__m128i CmpMask1 = _mm_cmpeq_epi32(VertexA, VertexB1);

	__m128i OrMask = _mm_or_si128(CmpMask0, CmpMask1);

	u32 Mask32 = _mm_movemask_ps(_mm_castsi128_ps(OrMask));
	
	u32 SetBits = CountOfSetBits(Mask32);
	Assert(SetBits <= 2);

	Result.Succes = SetBits == 2 ? true : false;
	Result.Index[0] = FindLeastSignificantSetBit(Mask32).Index;
	Result.Index[1] = FindMostSignificantSetBit(Mask32).Index;

	return Result;
}

// TODO: Test code, replace in future
internal face_render_params
SetFaceRenderParams(game_editor_state *Editor, model *Model, u32 FaceIndex)
{
	face_render_params Result = {};
	editor_world_ui *WorldUI = &Editor->WorldUI;

	if (WorldUI->HotInteraction.Type == UI_InteractionType_Select)
	{
		switch (WorldUI->ITarget)
		{
			case UI_InteractionTarget_ModelFace:
			{
				if (WorldUI->IModel.Face.ID == FaceIndex)
				{
					Result.SelectionFlags |= FaceSelectionType_Hot;
					Result.Active = FaceVertexParams_SetAll;
				}
				else
				{
					model_face *IFace = Model->Faces + WorldUI->IModel.Face.ID;
					model_face *CompFace = Model->Faces + FaceIndex;

					faces_edge_match EdgeMatch = MatchFaceEdge(IFace, CompFace);
#if 1
					if (EdgeMatch.Succes)
					{
						model_edge *Edge = Model->Edges + EdgeMatch.Index;
						face_vertex_match VertexMatch = MatchFaceVertex(CompFace, Edge);
						if (VertexMatch.Succes)
						{
							Result.ActiveVert[VertexMatch.Index[0]] = FaceVertexParams_Mark;
							Result.ActiveVert[VertexMatch.Index[1]] = FaceVertexParams_Mark;
						}
					}
#else
					for (u32 SearchIndex = 0;
						SearchIndex < ArrayCount(IFace.EdgesID);
						++SearchIndex)
					{
						for (u32 CompIndex = 0;
							CompIndex < ArrayCount(CompFace.EdgesID);
							++CompIndex)
						{
							u32 CompEdgeIndex = CompFace.EdgesID[CompIndex];
							if (IFace.EdgesID[SearchIndex] == CompEdgeIndex)
							{
								model_edge Edge = Model->Edges[CompEdgeIndex];
								
								for (u32 EdgeVIndex = 0;
									EdgeVIndex < ArrayCount(Edge.VertexID);
									++EdgeVIndex)
								{		
									u32 EdgeVertexID = Edge.VertexID[EdgeVIndex];
									for (u32 FaceVIndex = 0;
										FaceVIndex < ArrayCount(CompFace.VertexID);
										++FaceVIndex)
									{
										u32 FaceVertexID = CompFace.VertexID[FaceVIndex];
										if (EdgeVertexID == FaceVertexID)
										{
											Result.ActiveVert[FaceVIndex] = FaceVertexParams_Mark;
										}
									}
								}

								break;
							}
						}
					}
#endif
				}
			} break;

			case UI_InteractionTarget_ModelEdge:
			{
				u32 IEdgeIndex = WorldUI->IModel.Edge.ID;
				model_edge IEdge = Model->Edges[WorldUI->IModel.Edge.ID];
				model_face CompFace = Model->Faces[FaceIndex];
#if 1
				if (MatchFaceEdge(&CompFace, IEdgeIndex))
				{
					face_vertex_match VertexMatch = MatchFaceVertex(&CompFace, &IEdge);
					if (VertexMatch.Succes)
					{
						Result.ActiveVert[VertexMatch.Index[0]] = FaceVertexParams_Mark;
						Result.ActiveVert[VertexMatch.Index[1]] = FaceVertexParams_Mark;
					}
				}
#else
				for (u32 SearchIndex = 0;
					SearchIndex < ArrayCount(CompFace.EdgesID);
					++SearchIndex)
				{
					if (CompFace.EdgesID[SearchIndex] == IEdgeIndex)
					{
						for (u32 EdgeVIndex = 0;
							EdgeVIndex < ArrayCount(IEdge.VertexID);
							++EdgeVIndex)
						{
							u32 EdgeVertexID = IEdge.VertexID[EdgeVIndex];
							for (u32 FaceVIndex = 0;
								FaceVIndex < ArrayCount(CompFace.VertexID);
								++FaceVIndex)
							{
								u32 FaceVertexID = CompFace.VertexID[FaceVIndex];
								if (EdgeVertexID == FaceVertexID)
								{
									Result.ActiveVert[FaceVIndex] = FaceVertexParams_Mark;
								}
							}
						}
					}
				}
#endif
			} break;
		}
	}

	return Result;
}

inline model_highlight_params
SetModelHighlight(game_editor_state *Editor, b32 IsActive, b32 IsHot, u32 ITarget)
{
	model_highlight_params Result = {};

	if (IsActive)
	{
		switch (ITarget)
		{
			case UI_InteractionTarget_Model:
			{
				Result.OutlineIsSet = true;
				Result.OutlineColor = Editor->ActiveOutlineColor;
			} break;

			case UI_InteractionTarget_ModelEdge:
			{
				Result.EdgeColor = Editor->EdgeColor;
			} break;
		}
	}
	else if (IsHot)
	{
		Result.OutlineIsSet = true;
		Result.OutlineColor = Editor->HotOutlineColor;
	}

	return Result;
}

// TODO: Support ui interaction in lower screen resolution
void
UpdateAndRender(game_memory *Memory, game_input *Input, game_render_commands *RenderCommands)
{
	game_state *GameState = (game_state *)Memory->GameStorage;
	game_editor_state *Editor = &GameState->EditorState;

	if (!GameState->IsInit)
	{
		InitArena(&GameState->GameArena, Memory->GameStorageSize - sizeof(game_state),
			((u8 *)Memory->GameStorage + sizeof(game_state)));

		LoadAsset(GameState);

		InitArena(&Editor->MainArena, Memory->EditorStorageSize, (u8 *)Memory->EditorStorage);
		
		// TODO: Create TranArena and PageArena as separate arena?
		Editor->TranArena = SubArena(&Editor->MainArena, MiB(5));
		
		u32 SelectedBufferSize = MiB(1);
		Editor->Selected.Elements = (u32 *)PushSize(&Editor->MainArena, SelectedBufferSize);
		Editor->Selected.MaxCount = SelectedBufferSize / sizeof(u32);

		InitPageArena(&Editor->MainArena, &Editor->PageArena, MiB(10));

		// NOTE: For Test
		AddCubeModel(Editor);
		AddCubeModel(Editor, V3(-2.0f, 1.0f, 1.0f));
		AddCubeModel(Editor, V3(-2.0f, 4.0f, -1.0f));

		Editor->Camera.Offset = V3(0, 0, 3);
		Editor->Camera.Pos = V3(0);

		Editor->EdgeColor = V3(0.17f, 0.5f, 0.8f);
		Editor->SelectColor = V3(0.86f, 0.70f, 0.2f);
		Editor->ActiveOutlineColor = V3(0.86f, 0.70f, 0.2f);
		Editor->HotOutlineColor = V3(0, 1, 0);

		Editor->WorldUI.UpdateITarget = true;

		CreateStaticSphere(Editor, 0.6f, 20, 20);

		GameState->IsInit = true;
	}

	render_group RenderGroup = InitRenderGroup(RenderCommands, Input);

	v3 CameraOffset = Editor->Camera.Offset;
	editor_world_ui *WorldUI = &Editor->WorldUI;

	//PushSphere(&RenderGroup, Editor->StaticMesh[0].Mesh);

	if (Input)
	{
		WorldUI->MouseP = V2(Input->MouseX, Input->MouseY);
		WorldUI->dMouseP = WorldUI->MouseP - WorldUI->LastMouseP;

#if 0
		char Buffer[1024];
		sprintf(Buffer, "Mouse x:%d y:%d", (u32)WorldUI->MouseP.x, (u32)WorldUI->MouseP.y);
		RenderText(&RenderGroup, GameState->FontAsset, Buffer, V3(0.7f), 0, RenderGroup.ScreenDim.y, 0.2f);
#endif
		if (IsKepDown(Input->Alt) && IsKepDown(Input->MouseButtons[PlatformMouseButton_Left]))
		{
			f32 RotationSpeed = Pi32 * 0.0005f;
			Editor->Camera.Orbit -= WorldUI->dMouseP.x * RotationSpeed;
			Editor->Camera.Pitch += WorldUI->dMouseP.y * RotationSpeed;
		}

		if (IsKepDown(Input->Alt) && IsKepDown(Input->MouseButtons[PlatformMouseButton_Right]))
		{
			f32 ZoomSpeed = (CameraOffset.z + Editor->Camera.Dolly) * 0.004f;
			Editor->Camera.Dolly -= WorldUI->dMouseP.y*ZoomSpeed;
		}

		/*if (Input->CtrlDown && Input->MouseButtons[PlatformMouseButton_Left].EndedDown)
		{
			f32 RotationSpeed = Pi32 * 0.0005f;
			Editor->Camera.Offset.x -= WorldUI->dMouseP.x * RotationSpeed;
			Editor->Camera.Offset.y += WorldUI->dMouseP.y * RotationSpeed;

			CameraOffset = Editor->Camera.Offset;
		}*/

		WorldUI->LastMouseP = WorldUI->MouseP;
	}

	m4x4 CameraR = XRotation(Editor->Camera.Pitch) * YRotation(Editor->Camera.Orbit);
	v3 CameraOt = ((CameraOffset + V3(0, 0, Editor->Camera.Dolly)) * CameraR) + Editor->Camera.Pos;
	m4x4_inv CameraTansform = CameraViewTransform(CameraR, CameraOt);
	SetCameraTrasform(&RenderGroup, 0.41f, &CameraTansform);

	WorldUI->MouseRay.Dir = Unproject(&RenderGroup, WorldUI->MouseP);
	WorldUI->MouseRay.Pos = CameraOt;

	EditorUIInteraction(Editor, Input, &RenderGroup);

	for (u32 ModelIndex = 0;
		ModelIndex < Editor->ModelsCount;
		++ModelIndex)
	{
		model *Model = Editor->Models + ModelIndex;
		ui_interaction SelectInteraction = SetSelectInteraction(ModelIndex);
		b32 IsHot = AreEqual(SelectInteraction, WorldUI->HotInteraction);
		b32 IsActive = IsActiveModel(WorldUI, ModelIndex);

		model_highlight_params ModelHiLi = SetModelHighlight(Editor, IsActive, IsHot, WorldUI->ITarget);

		BeginPushModel(&RenderGroup, Model->Color, Model->Offset, ModelHiLi);
		
		for (u32 FaceIndex = 0;
			FaceIndex < Model->FaceCount;
			++FaceIndex)
		{
			model_face Face = Model->Faces[FaceIndex];
			face_render_params FaceParam = {};
			
			if (IsActive)
			{	
				FaceParam = SetFaceRenderParams(Editor, Model, FaceIndex);
			}

			PushFace(&RenderGroup, Model->Vertex, Face, FaceParam);
		}

		EndPushModel(&RenderGroup);
	}

	RenderText(&RenderGroup, GameState->FontAsset,
		(char *)"helloygj world", V3(0.5f, 0.5f, 0.5f), 0,
		RenderGroup.ScreenDim.y, 0.45f);
}