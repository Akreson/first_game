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

inline b32
IsInSelectedBuffer(selected_elements_buffer *Buffer, u32 ElementID)
{
	b32 Result = false;

	for (u32 Index = 0;
		Index < Buffer->Count;
		++Index)
	{
		u32 BufferElementID = Buffer->Elements[Index];
		if (BufferElementID == ElementID)
		{
			Result = true;
			break;
		}
	}

	return Result;
}

struct select_buffer_face_edges_match
{
	u32 Index[4];
	u32 Count;
};

#define Mi(a, i) (((u32 *)&(a))[i])

// TODO: Improve SIMD implementation?
inline select_buffer_face_edges_match
IsInSelectedBuffer(selected_elements_buffer *Buffer, model_face *Face)
{
	select_buffer_face_edges_match Result;

	__m128i OrMask = _mm_setzero_si128();
	__m128i EdgesID = _mm_load_si128((__m128i *)Face->EdgesID);

	for (u32 Index = 0;
		Index < Buffer->Count;
		++Index)
	{
		__m128i BufferElementID_4x = _mm_set1_epi32(Buffer->Elements[Index]);
		__m128i CmpMask = _mm_cmpeq_epi32(EdgesID, BufferElementID_4x);
		OrMask = _mm_or_si128(OrMask, CmpMask);
	}

	u32 ResultMask = _mm_movemask_ps(_mm_castsi128_ps(OrMask));
	u32 SetBitCount = CountOfSetBits(ResultMask);
	if (SetBitCount)
	{
		u32 CurrentResultIndex = 0;
		for (u32 Index = 0;
			Index < 4;
			++Index)
		{
			if (IsBitSet(ResultMask, Index))
			{
				Result.Index[CurrentResultIndex++] = Mi(EdgesID, Index);
			}
		}
	}

	Result.Count = SetBitCount;

	return Result;
}

// TODO: Debug _IsInSelectedBuffer_ part
internal face_render_params
SetFaceRenderParams(game_editor_state *Editor, model *Model, u32 FaceIndex)
{
	face_render_params Result = {};
	editor_world_ui *WorldUI = &Editor->WorldUI;

	switch (WorldUI->ITarget)
	{
		case UI_InteractionTarget_ModelFace:
		{
			if (IsInSelectedBuffer(&WorldUI->Selected, FaceIndex))
			{
				Result.SelectionFlags[FaceSelectionType_Select] = FaceElementParams_Mark;
				Result.Active = FaceElementParams_SetAll;
			}

			if (IsHotIntrType(WorldUI, UI_InteractionType_Select))
			{
				if (WorldUI->IModel.Face.ID == FaceIndex)
				{
					Result.SelectionFlags[FaceSelectionType_Hot] = FaceElementParams_Mark;
					Result.Active = FaceElementParams_SetAll;// TODO: Change to hot
				}
				else
				{
					model_face *IFace = Model->Faces + WorldUI->IModel.Face.ID;
					model_face *CompFace = Model->Faces + FaceIndex;

					face_edge_match EdgeMatch = MatchFaceEdge(CompFace, IFace);
					if (EdgeMatch.Succes)
					{
						model_edge *Edge = Model->Edges + CompFace->EdgesID[EdgeMatch.Index];
						Result.ActiveEdge[EdgeMatch.Index] = MaskMatchFaceVertex(CompFace, Edge);
					}
				}
			}
		} break;

		case UI_InteractionTarget_ModelEdge:
		{
			model_face *CompFace = Model->Faces + FaceIndex;

			select_buffer_face_edges_match MatchResult =
				IsInSelectedBuffer(&WorldUI->Selected, CompFace);
			if (MatchResult.Count)
			{
				for (u32 CheckIndex = 0;
					CheckIndex < MatchResult.Count;
					++CheckIndex)
				{
					u32 EdgeID = MatchResult.Index[CheckIndex];
					face_edge_match EdgeMatch = MatchFaceEdge(CompFace, EdgeID);
					if (EdgeMatch.Succes)
					{
						model_edge *Edge = Model->Edges + EdgeID;
						Result.ActiveEdge[EdgeMatch.Index] = MaskMatchFaceVertex(CompFace, Edge);
					}
				}
			}

			if (IsHotIntrType(WorldUI, UI_InteractionType_Select))
			{
				u32 IEdgeIndex = WorldUI->IModel.Edge.ID;
				model_edge *IEdge = Model->Edges + IEdgeIndex;

				face_edge_match EdgeMatch = MatchFaceEdge(CompFace, IEdgeIndex);
				if (EdgeMatch.Succes)
				{
					Result.ActiveEdge[EdgeMatch.Index] = MaskMatchFaceVertex(CompFace, IEdge);
				}
			}
		} break;
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
		Editor->WorldUI.Selected.Elements = (u32 *)PushSize(&Editor->MainArena, SelectedBufferSize);
		Editor->WorldUI.Selected.MaxCount = SelectedBufferSize / sizeof(u32);

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