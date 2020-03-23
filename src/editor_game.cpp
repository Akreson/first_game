#include "editor_game.h"
#include "render_group.cpp"
#include "model.cpp"
#include "asset.cpp"

// TODO: Delete
#include <cstdio>

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

			PushFont(Group, Glyph, V2(XPos, YPos), V2(XPos + Width, YPos + Height), TextColor);
		}

		ScreenX += GetHorizontalAdvance(FontAsset, PrevGlyphIndex, GlyphIndex, Scale);

		PrevGlyphIndex = GlyphIndex;
	}
}

// TODO: Implement
void
RayModelEdgeInterset(void)
{
#if 0
	if (HitTest)
	{
		// NOTE: Closest vertex match
		u32 VertexRelativeIndex[ArrayCount(GetMemberOf(model_edge, VertexID))];
		f32 LengthsToVertex[4];

		LengthsToVertex[0] = LengthSq(V0 - IntersetPoint);
		LengthsToVertex[1] = LengthSq(V1 - IntersetPoint);
		LengthsToVertex[2] = LengthSq(V2 - IntersetPoint);
		LengthsToVertex[3] = LengthSq(V3 - IntersetPoint);

		for (u32 MinLengthIndex = 0;
			MinLengthIndex < ArrayCount(VertexRelativeIndex);
			++MinLengthIndex)
		{
			f32 MinLength = FLOAT_MAX;
			u32 CurrentMinLengthIndex;

			for (u32 LengthIndex = 0;
				LengthIndex < ArrayCount(LengthsToVertex);
				++LengthIndex)
			{
				f32 CheckLength = LengthsToVertex[LengthIndex];
				if (CheckLength < MinLength)
				{
					CurrentMinLengthIndex = LengthIndex;
					MinLength = CheckLength;
				}
			}

			VertexRelativeIndex[MinLengthIndex] = CurrentMinLengthIndex;
			LengthsToVertex[CurrentMinLengthIndex] = FLOAT_MAX;
		}

		//NOTE: Edge match
		u32 VertexAbsoluteIndex[ArrayCount(GetMemberOf(model_edge, VertexID))] =
		{
			Face.VertexID[VertexRelativeIndex[0]],
			Face.VertexID[VertexRelativeIndex[1]]
		};

		model_edge MatchEdge;
		b32 SuccesMatch = false;
		for (u32 EdgeIndex = 0;
			EdgeIndex < ArrayCount(Face.EdgeID);
			++EdgeIndex)
		{
			u32 MatchCount = 0;
			model_edge Edge = Model->Edges[Face.EdgeID[EdgeIndex]];

			for (u32 EdgeVertexIndex = 0;
				EdgeVertexIndex < ArrayCount(Edge.VertexID);
				++EdgeVertexIndex)
			{
				for (u32 MatchVertIndex = 0;
					MatchVertIndex < ArrayCount(Edge.VertexID);
					++MatchVertIndex)
				{
					if (Edge.VertexID[EdgeVertexIndex] == VertexAbsoluteIndex[MatchVertIndex])
					{
						MatchCount++;
					}
				}
			}

			if (MatchCount == ArrayCount(Edge.VertexID))
			{
				MatchEdge = Edge;
				SuccesMatch = true;
				break;
			}
		}
		Assert(SuccesMatch);

		v3 EdgeV0 = Model->Vertex[MatchEdge.V0] + Model->Offset;
		v3 EdgeV1 = Model->Vertex[MatchEdge.V1] + Model->Offset;

		v3 NormalizeEdgeDir = Normalize(EdgeV1 - EdgeV0);
		v3 DistVector = IntersetPoint - EdgeV0;

		f32 LengthOnEdge = Dot(NormalizeEdgeDir, DistVector);
		LengthOnEdge = LengthOnEdge < 0 ? LengthOnEdge * -1.0f : LengthOnEdge;

		v3 PointOnEdge = EdgeV0 + (NormalizeEdgeDir * LengthOnEdge);
		f32 DistanceToEdge = Length(PointOnEdge - IntersetPoint);

		HitTest = DistanceToEdge < 0.03f ? true : false;
	}
#endif
}

b32
RayModelFaceIntersect(model *Model, ray_param Ray, element_ray_result *FaceResult)
{
	b32 Result = false;

	for (u32 FaceIndex = 0;
		FaceIndex < Model->FaceCount;
		++FaceIndex)
	{
		model_face Face = Model->Faces[FaceIndex];

		v3 V0 = Model->Vertex[Face.V0] + Model->Offset;
		v3 V1 = Model->Vertex[Face.V1] + Model->Offset;
		v3 V2 = Model->Vertex[Face.V2] + Model->Offset;
		v3 V3 = Model->Vertex[Face.V3] + Model->Offset;

		v3 Edge1 = V0 - V1;
		v3 Edge2 = V0 - V3;

		plane_param Plane;
		Plane.N = Normalize(Cross(Edge1, Edge2));
		Plane.D = Dot(Plane.N, V0);

		// Plane intersect
		f32 DotRayPlane = Dot(Ray.Dir, Plane.N);
		if (DotRayPlane < 0)
		{
			f32 tPlaneIntersect = ((Plane.D - Dot(Plane.N, Ray.Pos)) / DotRayPlane);

			if ((tPlaneIntersect != 0) && (tPlaneIntersect > 0))
			{
				v3 IntersetPoint = Ray.Pos + (Ray.Dir * tPlaneIntersect);

				b32 HitTest = IsPointInTriangle(V0, V1, V2, IntersetPoint);
				if (!HitTest)
				{
					HitTest = IsPointInTriangle(V0, V2, V3, IntersetPoint);
				}

				if (HitTest)
				{
					FaceResult->ID = FaceIndex;
					FaceResult->P = IntersetPoint;

					Result = true;
					break;
				}
			}
		}
	}

	return Result;
}

b32
RayModelsIntersect(memory_arena *Arena, model *Models, u32 ModelCount, ray_param Ray, interacted_model *Result)
{
	b32 Hit = false;
	temp_memory TempMem = BeginTempMemory(Arena);

	u32 ModelsHitCount = 0;
	u32 ModelsSortArraySize = 20;
	model_ray_sort *ModelsSortArray = (model_ray_sort *)PushArray(Arena, model_ray_sort, ModelsSortArraySize);

	// NOTE: Gather all intersect models
	for (u32 ModelIndex = 0;
		ModelIndex < ModelCount;
		++ModelIndex)
	{
		model *Model = Models + ModelIndex;

		b32 HitTest = RayAABBIntersect(Ray, Model->AABB, Model->Offset);

		if (HitTest)
		{
			model_ray_sort *SortEntry = ModelsSortArray + ModelsHitCount++;

			v3 CenterOfAABB = ((Model->AABB.Min + Model->Offset) + (Model->AABB.Max + Model->Offset)) / 2.0f;

			SortEntry->Index = ModelIndex;
			SortEntry->Length = LengthSq(CenterOfAABB - Ray.Pos);

			if (ModelsHitCount >= ModelsSortArraySize)
			{
				PushArray(Arena, u32, ModelsSortArraySize);
				ModelsSortArraySize += ModelsSortArraySize;
			}
		}
	}

	// NOTE: Sort by length from ray position to center of AABB
	for (u32 Outer = 0;
		Outer < ModelsHitCount;
		++Outer)
	{
		for (u32 Inner = 0;
			Inner < (ModelsHitCount - 1);
			++Inner)
		{
			model_ray_sort *A = ModelsSortArray + Inner;
			model_ray_sort *B = ModelsSortArray + Inner + 1;

			if (A->Length > B->Length)
			{
				model_ray_sort Temp = *B;
				*B = *A;
				*A = Temp;
			}
		}
	}

	// NOTE: Find intersetc model face for non convex case
	if (ModelsHitCount)
	{
		for (u32 SortIndex = 0;
			(SortIndex < ModelsHitCount);
			++SortIndex)
		{
			u32 ModelIndex = ModelsSortArray[SortIndex].Index;
			model *Model = Models + ModelIndex;

			if (RayModelFaceIntersect(Model, Ray, &Result->Face))
			{
				Hit = true;
				Result->ID = ModelIndex;
				break;
			}
		}
	}

	EndTempMemory(TempMem);

	return Hit;
}

void
EditorUIInteraction(game_editor_state *Editor, game_input *Input, render_group *RenderGroup, ray_param Ray)
{
	if (WasDown(Input->Tab))
	{
		if (Editor->WorldUI.ITarget)
		{
			++Editor->WorldUI.ITarget;
			if (Editor->WorldUI.ITarget == ModelInteractionTarget_Count)
			{
				Editor->WorldUI.ITarget = 1;
			}
		}
	}

	// TODO: Delete leter
	if (IsKepDown(Input->Alt) && IsGoDown(Input->Shift))
	{
		Editor->WorldUI.ITarget = ModelInteractionTarget_None;
		Editor->WorldUI.Interaction = {};
	}

	// TODO: Set ui interaction in proper way
	if (!Editor->WorldUI.Interaction.Type)
	{
		switch (Editor->WorldUI.ITarget)
		{
			case ModelInteractionTarget_None:
			{
				if (RayModelsIntersect(&Editor->MainArena, Editor->Models, Editor->ModelsCount,
						Ray, &Editor->WorldUI.IModel))
				{
					Editor->WorldUI.HotInteraction.ID = Editor->WorldUI.IModel.ID;
					Editor->WorldUI.HotInteraction.Type = InteractionType_Select;
				}
				else
				{
					Editor->WorldUI.HotInteraction = {};
				}
			} break;

			case ModelInteractionTarget_Face:
			case ModelInteractionTarget_Edge:
			{
				model *Model = Editor->Models + Editor->WorldUI.IModel.ID;
				if (RayAABBIntersect(Ray, Model->AABB, Model->Offset))
				{
					if (RayModelFaceIntersect(Model, Ray, &Editor->WorldUI.IModel.Face))
					{
						Editor->WorldUI.HotInteraction.ID = Editor->WorldUI.IModel.Face.ID;
						Editor->WorldUI.HotInteraction.Type = InteractionType_Select;

						if (Editor->WorldUI.ITarget == ModelInteractionTarget_Edge)
						{
							RayModelEdgeInterset();
						}
					}
				}
				else
				{
					Editor->WorldUI.HotInteraction = {};
				}
			} break;
		}
	}
	else
	{

	}

	if (WasDown(Input->MouseButtons[PlatformMouseButton_Left]))
	{
		switch (Editor->WorldUI.HotInteraction.Type)
		{
			case InteractionType_Select:
			{
				if (Editor->WorldUI.ITarget == ModelInteractionTarget_None)
				{
					Editor->WorldUI.ITarget = ModelInteractionTarget_Model;

					// TODO: Change later
					//Editor->WorldUI.Interaction = Editor->WorldUI.HotInteraction;
				}
			} break;
		}
	}
}

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
		
		u32 SelectedBufferSize = MiB(1);
		Editor->Selected.Elements = (u32 *)PushSize(&Editor->MainArena, SelectedBufferSize);
		Editor->Selected.MaxCount = SelectedBufferSize / sizeof(u32);

		InitPageArena(&Editor->MainArena, &Editor->PageArena, MiB(10));

		AddCubeModel(Editor);
		AddCubeModel(Editor, V3(-2.0f, 1.0f, 1.0f));
		AddCubeModel(Editor, V3(-2.0f, 4.0f, -1.0f));

		Editor->Camera.Offset = V3(0, 0, 3);
		Editor->Camera.Pos = V3(0);

		GameState->IsInit = true;
	}

	render_group RenderGroup = InitRenderGroup(RenderCommands, Input, GameState->FontAsset);

	v3 CameraOffset = Editor->Camera.Offset;

	if (Input)
	{
		Editor->WorldUI.MouseP = V2(Input->MouseX, Input->MouseY);
		Editor->WorldUI.dMouseP = Editor->WorldUI.MouseP - Editor->WorldUI.LastMouseP;

#if 0
		char Buffer[1024];
		sprintf(Buffer, "Mouse x:%d y:%d", (u32)Editor->WorldUI.MouseP.x, (u32)Editor->WorldUI.MouseP.y);
		RenderText(&RenderGroup, Buffer, V3(0.7f), 0, RenderGroup.ScreenDim.y, 0.2f);
#endif
		if (IsKepDown(Input->Alt) && IsKepDown(Input->MouseButtons[PlatformMouseButton_Left]))
		{
			f32 RotationSpeed = Pi32 * 0.0005f;
			Editor->Camera.Orbit -= Editor->WorldUI.dMouseP.x * RotationSpeed;
			Editor->Camera.Pitch += Editor->WorldUI.dMouseP.y * RotationSpeed;
		}

		if (IsKepDown(Input->Alt) && IsKepDown(Input->MouseButtons[PlatformMouseButton_Right]))
		{
			f32 ZoomSpeed = (CameraOffset.z + Editor->Camera.Dolly) * 0.004f;
			Editor->Camera.Dolly -= Editor->WorldUI.dMouseP.y*ZoomSpeed;
		}

		/*if (Input->CtrlDown && Input->MouseButtons[PlatformMouseButton_Left].EndedDown)
		{
			f32 RotationSpeed = Pi32 * 0.0005f;
			Editor->Camera.Offset.x -= Editor->WorldUI.dMouseP.x * RotationSpeed;
			Editor->Camera.Offset.y += Editor->WorldUI.dMouseP.y * RotationSpeed;

			CameraOffset = Editor->Camera.Offset;
		}*/

		Editor->WorldUI.LastMouseP = Editor->WorldUI.MouseP;
	}

	m4x4 CameraR = XRotation(Editor->Camera.Pitch) * YRotation(Editor->Camera.Orbit);
	v3 CameraOt = ((CameraOffset + V3(0, 0, Editor->Camera.Dolly)) * CameraR) + Editor->Camera.Pos;
	m4x4_inv CameraTansform = CameraViewTransform(CameraR, CameraOt, Editor->Camera.Pos);
	SetCameraTrasform(&RenderGroup, 0.41f, &CameraTansform);

	ray_param Ray;
	Ray.Dir = Unproject(&RenderGroup, Editor->WorldUI.MouseP);
	Ray.Pos = CameraOt;

	EditorUIInteraction(Editor, Input, &RenderGroup, Ray);

	// TODO: Chenge later

	b32 IsHotModelSet = !Editor->WorldUI.ITarget && (Editor->WorldUI.HotInteraction.Type == InteractionType_Select);
	b32 IsActiveModelSet = Editor->WorldUI.ITarget;
	b32 HitModelTest = IsActiveModelSet || IsHotModelSet;

	for (u32 ModelIndex = 0;
		ModelIndex < Editor->ModelsCount;
		++ModelIndex)
	{
		model *Model = Editor->Models + ModelIndex;
		
		v3 EdgeColor = V3(0.17f, 0.5f, 0.8f);
		v3 OutlineColor = V3(0);
		b32 IsInteractedModel = HitModelTest && (Editor->WorldUI.IModel.ID == ModelIndex);
		b32 IsActiveModel = IsInteractedModel && IsActiveModelSet;
		b32 IsHotModel = IsInteractedModel && IsHotModelSet;
		b32 IsSetOutline = false;

		if (IsInteractedModel && (IsDown(Input->Ctrl) && IsDown(Input->MouseButtons[PlatformMouseButton_Right])))
		{
			Editor->Camera.Pos = Model->Offset;
		}

		if (IsActiveModel)
		{
			EdgeColor = V3(0.86f, 0.70f, 0.2f);

			if (Editor->WorldUI.ITarget == ModelInteractionTarget_Model)
			{
				IsSetOutline = true;
				OutlineColor = V3(0.86f, 0.70f, 0.2f);
			}
		}
		else if (IsHotModel)
		{
			IsSetOutline = true;
			OutlineColor = V3(0, 1, 0);
		}

		BeginPushModel(&RenderGroup, Model->Color, Model->Offset, EdgeColor, OutlineColor, IsSetOutline);
		
		for (u32 FaceIndex = 0;
			FaceIndex < Model->FaceCount;
			++FaceIndex)
		{
			model_face Face = Model->Faces[FaceIndex];
			face_render_param FaceParam = {};
			
			if (IsActiveModel)
			{	
				switch (Editor->WorldUI.ITarget)
				{
					case ModelInteractionTarget_Face:
					{
						if (Editor->WorldUI.HotInteraction.Type == InteractionType_Select)
						{
							if (Editor->WorldUI.IModel.Face.ID == FaceIndex)
							{
								FaceParam.SelectionType = FaceSelectionType_Hot;
								FaceParam.ActiveVert[0] = true;
								FaceParam.ActiveVert[1] = true;
								FaceParam.ActiveVert[2] = true;
								FaceParam.ActiveVert[3] = true;
							}
						}
					} break;

					case ModelInteractionTarget_Edge:
					{

					} break;
				}
			}

			PushFace(&RenderGroup, Model->Vertex, Face, FaceParam);
		}

		EndPushModel(&RenderGroup);
	}

	//RenderText(&RenderGroup, (char *)"helloygj world", V3(0.5f, 0.5f, 0.5f), 0, RenderGroup.ScreenDim.y, 0.45f);
}