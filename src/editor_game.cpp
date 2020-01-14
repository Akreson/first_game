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

// TODO: Complete
void
GeneratingCube(page_memory_arena *Arena, model *Model, f32 HalfDim = 0.5f)
{	
	v3 Vertex[8];
	Vertex[0] = V3(-HalfDim, -HalfDim, HalfDim);
	Vertex[1] = V3(HalfDim, -HalfDim, HalfDim);
	Vertex[2] = V3(HalfDim, HalfDim, HalfDim);
	Vertex[3] = V3(-HalfDim, HalfDim, HalfDim);

	Vertex[4] = V3(HalfDim, -HalfDim, -HalfDim);
	Vertex[5] = V3(-HalfDim, -HalfDim, -HalfDim);
	Vertex[6] = V3(-HalfDim, HalfDim, -HalfDim);
	Vertex[7] = V3(HalfDim, HalfDim, -HalfDim);

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
AddCubeModel(game_editor_state *EditorState, v4 Color = V4(1.0f), v3 Offset = V3(0))
{
	model *Model = AddModel(EditorState, Color, Offset);
	GeneratingCube(&EditorState->EditorPageArena, Model);
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

		InitArena(&GameState->EditorState.EditorMainArena, Memory->EditorStorageSize, (u8 *)Memory->EditorStorage);
		InitPageArena(&GameState->EditorState.EditorMainArena, &GameState->EditorState.EditorPageArena, MiB(10));

		AddCubeModel(&GameState->EditorState, V4(0.5f, 0, 1.0f, 1.0f));
		AddCubeModel(&GameState->EditorState, V4(0.5f, 0, 0.5f, 1.0f), V3(-2.0f, 1.0f, 1.0f));

		EditorState->CameraOffset = V3(0, 0, 3);

		GameState->IsInit = true;
	}

	render_group RenderGroup = InitRenderGroup(RenderCommands, Input, GameState->FontAsset);

	v3 CameraOffset = EditorState->CameraOffset;

	if (Input)
	{
		v2 Mouse = {};
		Mouse.x = Input->MouseX;
		Mouse.y = Input->MouseY;
		
		if ((GameState->LastMouseP.x != 0) && (GameState->LastMouseP.y != 0))
		{
			v2 dMouse = Mouse - GameState->LastMouseP;

#if 1
			char Buffer[1024];
			sprintf(Buffer, "Mouse x:%f y:%f", Mouse.x, Mouse.y);
			OutputText(&RenderGroup, Buffer, V3(0.7f), 0, RenderGroup.ScreenDim.y, 0.3f);
#endif

			if (Input->AltDown && Input->MouseButtons[PlatformMouseButton_Left].EndedDown)
			{
				f32 RotationSpeed = Pi32 * 0.001f;
				EditorState->CameraOrbit += dMouse.x * RotationSpeed;
				EditorState->CameraPitch += dMouse.y * RotationSpeed;
			}

			if (Input->AltDown && Input->MouseButtons[PlatformMouseButton_Right].EndedDown)
			{
				f32 ZoomSpeed = (CameraOffset.z + EditorState->CameraDolly) * 0.005f;
				EditorState->CameraDolly -= dMouse.y*ZoomSpeed;
			}
		}

		GameState->LastMouseP = Mouse;
	}

	m4x4 CameraR = YRotation(EditorState->CameraOrbit) * XRotation(EditorState->CameraPitch);
	CameraOffset.z += EditorState->CameraDolly;

	m4x4 CameraTansform = CameraViewTransform(&CameraR, CameraOffset);
	SetCameraTrasform(&RenderGroup, 0.41f, &CameraTansform);

	for (u32 ModelIndex = 0;
		ModelIndex < EditorState->ModelsCount;
		++ModelIndex)
	{
		model *Model = EditorState->Models + ModelIndex;
		for (u32 FaceIndex = 0;
			FaceIndex < Model->FaceCount;
			++FaceIndex)
		{
			model_face *Face = Model->Faces + FaceIndex;
			PushModelFace(&RenderGroup, Model->Vertex, Face, Model->Color, Model->Offset);
		}
	}

	//OutputText(&RenderGroup, (char *)"helloygj world", V3(0.5f, 0.5f, 0.5f), 0, RenderGroup.ScreenDim.y, 0.45f);
}