#include "editor_game.h"
#include "render_group.cpp"
#include "model.cpp"
#include "asset.cpp"

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

inline ui_id
IDFromModel(u32 ModelIndex, u32 FaceIndex = 0, u32 EdgeIndex = 0)
{
	ui_id Result;
	Result.ID[0] = ModelIndex;
	Result.ID[1] = (FaceIndex << 32) | EdgeIndex;

	return Result;
}

inline b32
AreEqual(ui_id A, ui_id B)
{
	return ((A.ID[0] == B.ID[0]) && (A.ID[1] == B.ID[1]));
}

inline u32
SetIntrTypeID(u16 Target, u16 Type)
{
	u32 Result = (Target << 16) | Type;
	return Result;
}

inline ui_interaction
SetSelectInteraction(interact_model *IModel, u16 Target)
{
	ui_interaction Result;

	Result.TypeID = SetIntrTypeID(Target, UI_InteractionType_Select);
	Result.ID = IDFromModel(IModel->ID, IModel->Face.ID, IModel->Edge.ID);

	return Result;
}

inline ui_interaction
SetSelectInteraction(u32 ModelID, u32 FaceID = 0, u32 EdgeID = 0, u16 Target = 0)
{
	ui_interaction Result;

	Result.TypeID = SetIntrTypeID(Target, UI_InteractionType_Select);;
	Result.ID = IDFromModel(ModelID, FaceID, EdgeID);

	return Result;
}

inline b32
AreEqual(ui_interaction A, ui_interaction B)
{
	b32 Result = AreEqual(A.ID, B.ID) && (A.TypeID == B.TypeID);
	return Result;
}

inline rect3
AddRadiusTo(rect3 A, f32 B)
{
	rect3 Result;
	Result.Min = A.Min - B;
	Result.Max = A.Max + B;

	return Result;
}

internal void inline
UpdateUIInteractionTarget(game_editor_state *Editor, game_input *Input, render_group *RenderGroup)
{
	editor_world_ui *WorldUI = &Editor->WorldUI;
	interact_model *IModel = &WorldUI->IModel;

	switch (WorldUI->ITarget)
	{
		case UI_InteractionTarget_None:
		{
			*IModel = {};
			if (RayModelsIntersect(&Editor->MainArena, Editor->Models, Editor->ModelsCount,
				WorldUI->MouseRay, &IModel->ID, &IModel->Face))
			{
				WorldUI->NextHotInteraction = SetSelectInteraction(IModel->ID, 0, 0, WorldUI->ITarget);
			}
		} break;

		case UI_InteractionTarget_ModelFace:
		{
			model *Model = Editor->Models + WorldUI->IModel.ID;
			IModel->Face = {};

			if (RayAABBIntersect(WorldUI->MouseRay, Model->AABB, Model->Offset))
			{
				if (RayModelFaceIntersect(Model, WorldUI->MouseRay, &IModel->Face))
				{
					WorldUI->NextHotInteraction = SetSelectInteraction(IModel, WorldUI->ITarget);
				}
			}
		} break;

		case UI_InteractionTarget_ModelEdge:
		{
			model *Model = Editor->Models + WorldUI->IModel.ID;
			IModel->Edge = {};

			f32 EdgeIntersetRadius = 0.015f;
			if (RayAABBIntersect(WorldUI->MouseRay, AddRadiusTo(Model->AABB, EdgeIntersetRadius), Model->Offset))
			{
				// TODO: Add sphere capsule radius to aabb
				if (RayModelEdgeInterset(Model, WorldUI->MouseRay, &IModel->Edge, EdgeIntersetRadius))
				{	
					WorldUI->NextHotInteraction = SetSelectInteraction(IModel, WorldUI->ITarget);
				}
			}
		} break;
	}
}

internal void
BeginInteraction(game_editor_state *Editor, game_input *Input, render_group *RenderGroup)
{
	editor_world_ui *WorldUI = &Editor->WorldUI;

	if (WorldUI->HotInteraction.Type)
	{
		WorldUI->Interaction = WorldUI->HotInteraction;
	}
	else
	{
		WorldUI->Interaction = {};
	}
}

internal void
EndInteraction(game_editor_state *Editor, game_input *Input, render_group *RenderGroup)
{
	editor_world_ui *WorldUI = &Editor->WorldUI;

	if (WorldUI->Interaction.Type)
	{
		switch (WorldUI->Interaction.Type)
		{
			case UI_InteractionType_Select:
			{
				switch (WorldUI->ITarget)
				{
					case UI_InteractionTarget_None:
					{
						if ((IsDown(Input->Ctrl) &&
							WasDown(Input->MouseButtons[PlatformMouseButton_Right])))
						{
							model *Model = Editor->Models + WorldUI->IModel.ID;
							Editor->Camera.Pos = Model->Offset;
						}
						else if (WasDown(Input->MouseButtons[PlatformMouseButton_Left]))
						{
							WorldUI->ITarget = UI_InteractionTarget_Model;
						}
					} break;
				}
			} break;
		}
	}

	WorldUI->Interaction = {};
}

inline void
ProcessWorldUIInput(editor_world_ui *WorldUI, game_input *Input)
{
	if (WasDown(Input->MouseButtons[PlatformMouseButton_Extended0]))
	{
		if (WorldUI->ITarget)
		{
			++WorldUI->ITarget;
			if (WorldUI->ITarget == UI_InteractionTarget_ModelCount)
			{
				WorldUI->ITarget = UI_InteractionTarget_Model;
			}
		}
	}

	if (IsKepDown(Input->Alt) && IsGoDown(Input->Shift))
	{
		WorldUI->ITarget = UI_InteractionTarget_None;
		WorldUI->Interaction = {};
	}
}

void
EditorUIInteraction(game_editor_state *Editor, game_input *Input, render_group *RenderGroup)
{
	editor_world_ui *WorldUI = &Editor->WorldUI;

	ProcessWorldUIInput(WorldUI, Input);

	if (WorldUI->UpdateITarget)
	{
		UpdateUIInteractionTarget(Editor, Input, RenderGroup);
	}

	// TODO: Set ui interaction in proper way
	if (!WorldUI->Interaction.Type)
	{
		WorldUI->HotInteraction = WorldUI->NextHotInteraction;

		if (IsDown(Input->MouseButtons[PlatformMouseButton_Left]) ||
			IsDown(Input->MouseButtons[PlatformMouseButton_Right]))
		{
			BeginInteraction(Editor, Input, RenderGroup);
		}
	}
	else
	{
		if (WasDown(Input->MouseButtons[PlatformMouseButton_Left]) ||
			WasDown(Input->MouseButtons[PlatformMouseButton_Right]))
		{
			EndInteraction(Editor, Input, RenderGroup);
		}
	}

	WorldUI->NextHotInteraction = {};
}

inline b32
IsActiveModel(editor_world_ui *UI, u32 ModelID)
{
	b32 Result = (UI->IModel.ID == ModelID) &&
		((UI->ITarget >= UI_InteractionTarget_Model) &&
		(UI->ITarget < UI_InteractionTarget_ModelCount));

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
					model_face IFace = Model->Faces[WorldUI->IModel.Face.ID];
					model_face CompFace = Model->Faces[FaceIndex];

					for (u32 SearchIndex = 0;
						SearchIndex < ArrayCount(IFace.EdgeID);
						++SearchIndex)
					{
						for (u32 CompIndex = 0;
							CompIndex < ArrayCount(CompFace.EdgeID);
							++CompIndex)
						{
							u32 CompEdgeIndex = CompFace.EdgeID[CompIndex];
							if (IFace.EdgeID[SearchIndex] == CompEdgeIndex)
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
											Result.ActiveVert[FaceVIndex] = FaceVertexParams_Active;
										}
									}
								}

								break;
							}
						}
					}
				}
			} break;

			case UI_InteractionTarget_ModelEdge:
			{
				u32 IEdgeIndex = WorldUI->IModel.Edge.ID;
				model_edge IEdge = Model->Edges[WorldUI->IModel.Edge.ID];

				model_face CompFace = Model->Faces[FaceIndex];
				for (u32 SearchIndex = 0;
					SearchIndex < ArrayCount(CompFace.EdgeID);
					++SearchIndex)
				{
					if (CompFace.EdgeID[SearchIndex] == IEdgeIndex)
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
									Result.ActiveVert[FaceVIndex] = FaceVertexParams_Active;
								}
							}
						}
					}
				}
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

		// TODO: Fix font rendering
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