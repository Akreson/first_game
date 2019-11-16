#include "editor_game.h"
#include "render_group.cpp"
#include "asset.cpp"

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
	if (!GameState->IsInit)
	{
		GameState->FontAsset = (font_asset_info *)((u8 *)Memory->GameStorage + sizeof(game_state));
		LoadAsset((void *)GameState->FontAsset);
		PatchFontData(GameState->FontAsset);

		InitArena(&GameState->EditorState.EditorArena, Memory->EditorStorageSize, (u8 *)Memory->EditorStorage);
		AddCubeModel(&GameState->EditorState, V4(0.5f, 0, 1.0f, 1.0f));

		GameState->IsInit = true;
	}

	render_group RenderGroup = InitRenderGroup(RenderCommands, Input, GameState->FontAsset);

	SetCameraTrasform(&RenderGroup, 0.41f, false);

	//RenderText(&RenderGroup, (char *)"hellow world", V3(0.5f, 0.0f, 0.5f), 0, RenderGroup.Height, 0.5f);
	PushModel(&RenderGroup, GameState->EditorState.Models);
}