#include "editor_game.h"
#include "render_group.cpp"
#include "asset.cpp"

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

inline void
InitArena(memory_arena *Arena, memory_index Size, u8 *Base)
{
	Arena->Base = Base;
	Arena->Size = Size;
	Arena->Used = 0;
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

void
GeneratingCube(memory_arena *Arena, model *Model, f32 HalfDim = 0.5f)
{
	Model->VertexCount = 8;
	Model->FaceCount = 6;
	v3 *Vertex = Model->Vertex = PushArray(Arena, v3, Model->VertexCount);

	Vertex[0] = V3(-HalfDim, -HalfDim, HalfDim);
	Vertex[1] = V3(HalfDim, -HalfDim, HalfDim);
	Vertex[2] = V3(HalfDim, HalfDim, HalfDim);
	Vertex[3] = V3(-HalfDim, HalfDim, HalfDim);

	Vertex[4] = V3(HalfDim, -HalfDim, -HalfDim);
	Vertex[5] = V3(-HalfDim, -HalfDim, -HalfDim);
	Vertex[6] = V3(-HalfDim, HalfDim, -HalfDim);
	Vertex[7] = V3(HalfDim, HalfDim, -HalfDim);

	model_face *Faces = Model->Faces = PushArray(Arena, model_face, Model->FaceCount);
	Faces[0] = {0, 1, 2, 3};
	Faces[1] = {4, 5, 6, 7};
	Faces[2] = {1, 4, 7, 2};
	Faces[3] = {5, 0, 3, 6};
	Faces[4] = {0, 1, 4, 5};
	Faces[5] = {3, 2, 7, 6};
}

void
AddCubeModel(game_editor_state *EditorState, v4 Color = V4(1.0f), v3 Offset = V3(0))
{
	model *Model = AddModel(EditorState, Color, Offset);
	GeneratingCube(&EditorState->EditorMainArena, Model);
}

void
UpdateAndRender(game_memory *Memory, game_input *Input, game_render_commands *RenderCommands)
{
	game_state *GameState = (game_state *)Memory->GameStorage;
	game_editor_state *EditorState = &GameState->EditorState;

	if (!GameState->IsInit)
	{
		GameState->FontAsset = (font_asset_info *)((u8 *)Memory->GameStorage + sizeof(game_state));
		LoadAsset((void *)GameState->FontAsset);
		PatchFontData(GameState->FontAsset);

		InitArena(&GameState->EditorState.EditorMainArena, Memory->EditorStorageSize, (u8 *)Memory->EditorStorage);

		AddCubeModel(&GameState->EditorState, V4(0.5f, 0.0f, 1.0f, 1.0f));
		AddCubeModel(&GameState->EditorState, V4(0.5f, 0, 0.5f, 1.0f), V3(-2.0f, 1.0f, 1.0f));
		EditorState->CameraOffset = V3(0, 0, -3);

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

	OutputText(&RenderGroup, (char *)"helloygj world", V3(0.5f, 0.5f, 0.5f), 0, RenderGroup.ScreenDim.y, 0.45f);
}