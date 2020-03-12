#include "editor_game.h"
#include "render_group.cpp"
#include "asset.cpp"

// TODO: Delete
#include <cstdio>

void
OutputText(render_group *Group, char *Text, v3 TextColor, f32 ScreenX, f32 ScreenY, f32 Scale)
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

inline model *
AddModel(game_editor_state *EditorState, v4 Color, v3 Offset)
{
	model *Model = EditorState->Models + EditorState->ModelsCount++;
	Assert(EditorState->ModelsCount < ArrayCount(EditorState->Models));
	Model->Color = Color;
	Model->Offset = Offset;

	return Model;
}

// TODO: Find way combine MatchEdgeToFace and MatchFaceToEdge?
inline void
MatchEdgeToFace(model_edge *Edges, u32 EdgeCount, model_face *Faces, u32 FaceCount)
{
	for (u32 EdgeIndex = 0;
		EdgeIndex < EdgeCount;
		++EdgeIndex)
	{
		model_edge *Edge = Edges + EdgeIndex;

		u32 FaceMatchedIndex = 0;
		for (u32 FaceIndex = 0;
			FaceIndex < FaceCount;
			++FaceIndex)
		{
			model_face *Face = Faces + FaceIndex;

			u32 MatchedCount = 0;
			for (u32 EdgeVertexID = 0;
				EdgeVertexID < ArrayCount(Edge->VertexID);
				++EdgeVertexID)
			{
				for (u32 FaceVertexID = 0;
					FaceVertexID < ArrayCount(Face->VertexID);
					++FaceVertexID)
				{
					if (Edge->VertexID[EdgeVertexID] == Face->VertexID[FaceVertexID])
					{
						MatchedCount++;
						break;
					}
				}
			}

			if (MatchedCount == ArrayCount(Edge->VertexID))
			{
				Edge->FaceID[FaceMatchedIndex] = FaceIndex;
				FaceMatchedIndex++;
				
				if (FaceMatchedIndex == ArrayCount(Edge->FaceID)) break;
			}

		}

		Assert(FaceMatchedIndex == ArrayCount(Edge->FaceID));
	}
}

inline void
MatchFaceToEdge(model_edge *Edges, u32 EdgeCount, model_face *Faces, u32 FaceCount)
{
	for (u32 FaceIndex = 0;
		FaceIndex < FaceCount;
		++FaceIndex)
	{
		model_face *Face = Faces + FaceIndex;

		u32 EdgeMatchedIndex = 0;
		for (u32 EdgeIndex = 0;
			EdgeIndex < EdgeCount;
			++EdgeIndex)
		{
			model_edge *Edge = Edges + EdgeIndex;

			u32 MatchedCount = 0;
			for (u32 EdgeVertexID = 0;
				EdgeVertexID < ArrayCount(Edge->VertexID);
				++EdgeVertexID)
			{
				for (u32 FaceVertexID = 0;
					FaceVertexID < ArrayCount(Face->VertexID);
					++FaceVertexID)
				{
					if (Edge->VertexID[EdgeVertexID] == Face->VertexID[FaceVertexID])
					{
						MatchedCount++;
						break;
					}
				}
			}

			if (MatchedCount == ArrayCount(Edge->VertexID))
			{
				Face->EdgeID[EdgeMatchedIndex] = EdgeIndex;
				EdgeMatchedIndex++;

				if (EdgeMatchedIndex == ArrayCount(Face->EdgeID)) break;
			}
		}

		Assert(EdgeMatchedIndex == ArrayCount(Face->EdgeID));
	}
}

rect3
ComputeMeshAABB(v3 *VertexArray, u32 VertexCount)
{
	rect3 Rect = {V3(60000.0f), V3(-60000.0f)};

	for (u32 VertexIndex = 0;
		VertexIndex < VertexCount;
		++VertexIndex)
	{
		v3 Vertex = VertexArray[VertexIndex];
		
		if (Vertex.x < Rect.Min.x) Rect.Min.x = Vertex.x;
		if (Vertex.x > Rect.Max.x) Rect.Max.x = Vertex.x;

		if (Vertex.y < Rect.Min.y) Rect.Min.y = Vertex.y;
		if (Vertex.y > Rect.Max.y) Rect.Max.y = Vertex.y;

		if (Vertex.z < Rect.Min.z) Rect.Min.z = Vertex.z;
		if (Vertex.z > Rect.Max.z) Rect.Max.z = Vertex.z;
	}

	return Rect;
}

// TODO: Complete
void
GeneratingCube(page_memory_arena *Arena, model *Model, f32 HalfDim = 0.5f)
{	
	v3 Vertex[12];
	Vertex[0] = V3(-HalfDim, -HalfDim, HalfDim);
	Vertex[1] = V3(HalfDim, -HalfDim, HalfDim);
	Vertex[2] = V3(HalfDim, HalfDim, HalfDim);
	Vertex[3] = V3(-HalfDim, HalfDim, HalfDim);

	Vertex[4] = V3(HalfDim, -HalfDim, -HalfDim);
	Vertex[5] = V3(-HalfDim, -HalfDim, -HalfDim);
	Vertex[6] = V3(-HalfDim, HalfDim, -HalfDim);
	Vertex[7] = V3(HalfDim, HalfDim, -HalfDim);
	
#if 1
	// NOTE: For debug
	f32 DebugHalfDim = HalfDim * 0.07f;
	Vertex[8] = V3(-DebugHalfDim, -DebugHalfDim, 0);
	Vertex[9] = V3(DebugHalfDim, -DebugHalfDim, 0);
	Vertex[10] = V3(DebugHalfDim, DebugHalfDim, 0);
	Vertex[11] = V3(-DebugHalfDim, DebugHalfDim, 0);
#endif

	model_face Faces[6];
	Faces[0] = {{0, 1, 2, 3}, {}};
	Faces[1] = {{4, 5, 6, 7}, {}};
	Faces[2] = {{1, 4, 7, 2}, {}};
	Faces[3] = {{5, 0, 3, 6}, {}};
	Faces[4] = {{5, 4, 1, 0}, {}};
	Faces[5] = {{3, 2, 7, 6}, {}};

	model_edge Edges[12];
	Edges[0] = {{3, 0}, {}};
	Edges[1] = {{0, 1}, {}};
	Edges[2] = {{1, 2}, {}};
	Edges[3] = {{2, 3}, {}};
	Edges[4] = {{5, 0}, {}};
	Edges[5] = {{4, 1}, {}};
	Edges[6] = {{7, 2}, {}};
	Edges[7] = {{6, 3}, {}};
	Edges[8] = {{6, 5}, {}};
	Edges[9] = {{5, 4}, {}};
	Edges[10] = {{7, 4}, {}};
	Edges[11] = {{7, 6}, {}};

	MatchEdgeToFace(Edges, ArrayCount(Edges), Faces, ArrayCount(Faces));
	MatchFaceToEdge(Edges, ArrayCount(Edges), Faces, ArrayCount(Faces));

	PagePushArray(Arena, v3, ArrayCount(Vertex), Model->Vertex, Vertex);
	PagePushArray(Arena, model_face, ArrayCount(Faces), Model->Faces, Faces);
	PagePushArray(Arena, model_edge, ArrayCount(Edges), Model->Edges, Edges);

	Model->VertexCount = ArrayCount(Vertex);
	Model->FaceCount = ArrayCount(Faces);
	Model->EdgeCount = ArrayCount(Edges);
}

void
AddCubeModel(game_editor_state *EditorState, v3 Offset = V3(0), v4 Color = V4(0, 0, 0, 1.0f))
{
	model *Model = AddModel(EditorState, Color, Offset);
	GeneratingCube(&EditorState->EditorPageArena, Model);
	Model->AABB = ComputeMeshAABB(Model->Vertex, Model->VertexCount);
}

struct model_ray_result
{
	b32 Hit;
	u32 ModelIndex;
	u32 FaceIndex;
	v3 IntersetPoint;
};

struct model_ray_sort
{
	u32 Index;
	f32 Length;
};

// TODO: Implemnt
void
ModelEdgeInterset(void)
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

model_ray_result
RayModelsIntersect(memory_arena *Arena, model *Models, u32 ModelCount, ray_param Ray)
{
	model_ray_result Result = {};
	temp_memory TempMem = BeginTempMemory(Arena);

	u32 ModelsHitCount = 0;
	u32 ModelsSortArraySize = 20;
	model_ray_sort *ModelsIndexArray = (model_ray_sort *)PushArray(Arena, model_ray_sort, ModelsSortArraySize);

	// NOTE: Gather all intersect models
	for (u32 ModelIndex = 0;
		ModelIndex < ModelCount;
		++ModelIndex)
	{
		model *Model = Models + ModelIndex;

		b32 HitTest = RayAABBIntersect(Ray, Model->AABB, Model->Offset);

		if (HitTest)
		{
			model_ray_sort *SortEntry = ModelsIndexArray + ModelsHitCount++;

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
			model_ray_sort *A = ModelsIndexArray + Inner;
			model_ray_sort *B = ModelsIndexArray + Inner + 1;

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
		b32 FaceHitTest = false;
		for (u32 SortIndex = 0;
			SortIndex < ModelsHitCount, !FaceHitTest;
			++SortIndex)
		{
			u32 ModelIndex = ModelsIndexArray[SortIndex].Index;
			model *Model = Models + ModelIndex;

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
							Result.Hit = true;
							Result.ModelIndex = ModelIndex;
							Result.FaceIndex = FaceIndex;
							Result.IntersetPoint = IntersetPoint;

							FaceHitTest = true;
							break;
						}
					}
				}
			}
		}
	}

	EndTempMemory(TempMem);

	return Result;
}

void
UpdateAndRender(game_memory *Memory, game_input *Input, game_render_commands *RenderCommands)
{
	game_state *GameState = (game_state *)Memory->GameStorage;
	game_editor_state *EditorState = &GameState->EditorState;

	if (!GameState->IsInit)
	{
		InitArena(&GameState->GameArena, Memory->GameStorageSize - sizeof(game_state),
			((u8 *)Memory->GameStorage + sizeof(game_state)));

		LoadAsset(GameState);

		InitArena(&EditorState->EditorMainArena, Memory->EditorStorageSize, (u8 *)Memory->EditorStorage);
		InitPageArena(&EditorState->EditorMainArena, &EditorState->EditorPageArena, MiB(10));

		AddCubeModel(EditorState);
		AddCubeModel(EditorState, V3(-2.0f, 1.0f, 1.0f));
		AddCubeModel(EditorState, V3(-2.0f, 4.0f, -1.0f));

		EditorState->CameraOffset = V3(0, 0, 3);
		EditorState->CameraPos = V3(0);

		GameState->IsInit = true;
	}

	render_group RenderGroup = InitRenderGroup(RenderCommands, Input, GameState->FontAsset);

	v3 CameraOffset = EditorState->CameraOffset;

	v2 Mouse = {};
	if (Input)
	{
		Mouse.x = Input->MouseX;
		Mouse.y = Input->MouseY;
		
		v2 dMouse = Mouse - GameState->LastMouseP;
#if 0
		char Buffer[1024];
		sprintf(Buffer, "Mouse x:%d y:%d", (u32)Mouse.x, (u32)Mouse.y);
		OutputText(&RenderGroup, Buffer, V3(0.7f), 0, RenderGroup.ScreenDim.y, 0.2f);
#endif
		if (Input->AltDown && Input->MouseButtons[PlatformMouseButton_Left].EndedDown)
		{
			f32 RotationSpeed = Pi32 * 0.0005f;
			EditorState->CameraOrbit -= dMouse.x * RotationSpeed;
			EditorState->CameraPitch += dMouse.y * RotationSpeed;
		}

		if (Input->AltDown && Input->MouseButtons[PlatformMouseButton_Right].EndedDown)
		{
			f32 ZoomSpeed = (CameraOffset.z + EditorState->CameraDolly) * 0.004f;
			EditorState->CameraDolly -= dMouse.y*ZoomSpeed;
		}

		/*if (Input->CtrlDown && Input->MouseButtons[PlatformMouseButton_Left].EndedDown)
		{
			f32 RotationSpeed = Pi32 * 0.0005f;
			EditorState->CameraOffset.x -= dMouse.x * RotationSpeed;
			EditorState->CameraOffset.y += dMouse.y * RotationSpeed;

			CameraOffset = EditorState->CameraOffset;
		}*/

		GameState->LastMouseP = Mouse;
	}

	m4x4 CameraR = XRotation(EditorState->CameraPitch) * YRotation(EditorState->CameraOrbit);
	v3 CameraOt = ((CameraOffset + V3(0, 0, EditorState->CameraDolly)) * CameraR) + EditorState->CameraPos;
	m4x4_inv CameraTansform = CameraViewTransform(CameraR, CameraOt, EditorState->CameraPos);
	SetCameraTrasform(&RenderGroup, 0.41f, &CameraTansform);

	ray_param Ray;
	Ray.Dir = Unproject(&RenderGroup, Mouse);
	Ray.Pos = CameraOt;

	model_ray_result ModelHit =
		RayModelsIntersect(&EditorState->EditorMainArena, EditorState->Models, EditorState->ModelsCount, Ray);

	b32 HitTest;
	for (u32 ModelIndex = 0;
		ModelIndex < EditorState->ModelsCount;
		++ModelIndex)
	{
		model *Model = EditorState->Models + ModelIndex;
		
		b32 HitTest = ModelHit.Hit && (ModelHit.ModelIndex == ModelIndex);

		if (HitTest && (Input->CtrlDown && IsDown(Input->MouseButtons[PlatformMouseButton_Left])))
		{
			EditorState->CameraPos = Model->Offset;
		}
		
		v3 EdgeColor = HitTest ? V3(0.86f, 0.70f, 0.2f) : V3(0.17f, 0.5f, 0.8f);

		BeginPushModel(&RenderGroup, Model->Color, Model->Offset, EdgeColor);
		
		for (u32 FaceIndex = 0;
			FaceIndex < Model->FaceCount;
			++FaceIndex)
		{
			model_face Face = Model->Faces[FaceIndex];
			face_render_param FaceParam = {};
			
			if (HitTest && (FaceIndex == ModelHit.FaceIndex))
			{
				FaceParam.SelectionType = FaceSelectionType_Hot;
			}

			PushFace(&RenderGroup, Model->Vertex, Face, FaceParam);
		}

		EndPushModel(&RenderGroup);
	}

	//OutputText(&RenderGroup, (char *)"helloygj world", V3(0.5f, 0.5f, 0.5f), 0, RenderGroup.ScreenDim.y, 0.45f);
}