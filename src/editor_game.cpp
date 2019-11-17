#include "editor_game.h"
#include "render_group.cpp"
#include "asset.cpp"

float CubeVertices[] = {
	// positions       
	-0.5f, -0.5f, -0.5f,
	0.5f, -0.5f, -0.5f,
	0.5f, 0.5f, -0.5f,
	0.5f, 0.5f, -0.5f,
	-0.5f, 0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,

	-0.5f, -0.5f, 0.5f,
	0.5f, -0.5f, 0.5f,
	0.5f, 0.5f, 0.5f,
	0.5f, 0.5f, 0.5f,
	-0.5f, 0.5f, 0.5f,
	-0.5f, -0.5f, 0.5f,

	-0.5f, 0.5f, 0.5f,
	-0.5f, 0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,
	-0.5f, -0.5f, 0.5f,
	-0.5f, 0.5f, 0.5f,

	0.5f, 0.5f, 0.5f,
	0.5f, 0.5f, -0.5f,
	0.5f, -0.5f, -0.5f,
	0.5f, -0.5f, -0.5f,
	0.5f, -0.5f, 0.5f,
	0.5f, 0.5f, 0.5f,

	-0.5f, -0.5f, -0.5f,
	0.5f, -0.5f, -0.5f,
	0.5f, -0.5f, 0.5f,
	0.5f, -0.5f, 0.5f,
	-0.5f, -0.5f, 0.5f,
	-0.5f, -0.5f, -0.5f,

	-0.5f, 0.5f, -0.5f,
	0.5f, 0.5f, -0.5f,
	0.5f, 0.5f, 0.5f,
	0.5f, 0.5f, 0.5f,
	-0.5f, 0.5f, 0.5f,
	-0.5f, 0.5f, -0.5f
};

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
			f32 YPos = ScreenY - (FontAsset->VerticalAdjast[GlyphIndex] * (f32)Glyph->Height);

			PushFont(Group, Glyph, V2(XPos, YPos), V2(XPos + Width, YPos + Height), TextColor);
		}


		ScreenX += (f32)FontAsset->GlyphAdvance[GlyphIndex] * Scale;
		if (PrevGlyphIndex)
		{
			ScreenX += (f32)FontAsset->KerningTable[PrevGlyphIndex*FontAsset->GlyphCount + GlyphIndex];
		}

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

inline void
AddModel(game_editor_state *EditorState, f32 *VertexPtr, u32 VertexCount, v4 Color)
{
	model *Model = EditorState->Models + EditorState->ModelsCount++;
	Assert(EditorState->ModelsCount < ArrayCount(EditorState->Models));
	Model->Vertex = VertexPtr;
	Model->VertexCount = VertexCount;
	Model->Color = Color;
}

void
AddCubeModel(game_editor_state *EditorState, v4 Color = V4(1.0f))
{
	f32 *Base = PushArray(&EditorState->EditorArena, f32, ArrayCount(CubeVertices));
	AddModel(EditorState, Base, sizeof(CubeVertices) / sizeof(f32), Color);
	Copy(sizeof(CubeVertices), Base, CubeVertices);
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

		InitArena(&GameState->EditorState.EditorArena, Memory->EditorStorageSize, (u8 *)Memory->EditorStorage);
		AddCubeModel(&GameState->EditorState, V4(0.5f, 0, 1.0f, 1.0f));
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
		
		v2 dMouse = {};
		
		if ((GameState->LastMouseP.x != 0) && (GameState->LastMouseP.y != 0))
		{
			dMouse = Mouse - GameState->LastMouseP;

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

	PushModel(&RenderGroup, EditorState->Models);
	RenderText(&RenderGroup, (char *)"hellow world", V3(0.5f, 0.5f, 0.5f), 0, RenderGroup.ScreenDim.y, 0.5f);
}