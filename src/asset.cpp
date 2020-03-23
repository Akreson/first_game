inline void
PatchFontData(font_asset_info *FontAsset)
{
	umm **PtrToMetrics = (umm **)&FontAsset->Refs;
	for (u32 RefsIndex = 0;
		RefsIndex <= MAX_REFS_METRICS_COUNT;
		++RefsIndex)
	{
		PtrToMetrics[RefsIndex] = (umm *)((u8 *)PtrToMetrics + (u64)PtrToMetrics[RefsIndex]);
	}

	for (u32 GlyphIndex = 0;
		GlyphIndex < FontAsset->GlyphCount;
		++GlyphIndex)
	{
		bitmap_info *GlyphBitmap = FontAsset->Glyphs + GlyphIndex;
		GlyphBitmap->Data = (void *)((u8 *)PtrToMetrics + (u64)GlyphBitmap->Data);
		// TODO: Allocate in other place?
		GlyphBitmap->TextureID = PlatformAPI.AllocateTexture(GlyphBitmap->Width, GlyphBitmap->Height, GlyphBitmap->Data);
	}
}

inline u32
GetGlyphIndexFromCodePoint(font_asset_info *FontAsset, u32 CodePoint)
{
	u32 Result = 0;
	if (CodePoint && CodePoint < FontAsset->OnePastLastUnicodeCode)
	{
		Result = FontAsset->UnicodeMap[CodePoint];
	}

	return Result;
}

inline bitmap_info *
GetGlyphBitmap(font_asset_info *FontAsset, u32 GlyphIndex)
{
	bitmap_info *Result = 0;
	if (GlyphIndex < FontAsset->GlyphCount)
	{
		Result = FontAsset->Glyphs + GlyphIndex;
	}

	return Result;
}

inline f32
GetHorizontalAdvance(font_asset_info *FontAsset, u32 PrevGlyphIndex, u32 GlyphIndex, f32 Scale)
{
	u16 Advance = FontAsset->GlyphAdvance[GlyphIndex];

	if (PrevGlyphIndex)
	{
		Advance += FontAsset->KerningTable[PrevGlyphIndex*FontAsset->GlyphCount + GlyphIndex];
	}

	f32 Result = (f32)Advance * Scale;
	return Result;
}

void
LoadAsset(game_state *GameState)
{
	platform_file_handler FileHandler = PlatformAPI.GetFileHandlerForFile(FileType_FontFile);
	u32 ReadSize = PlatformAPI.GetFileSize(&FileHandler);

	GameState->FontAsset = (font_asset_info *)PushSize(&GameState->GameArena, ReadSize);
	PlatformAPI.ReadFile(&FileHandler, ReadSize, GameState->FontAsset);
	PatchFontData(GameState->FontAsset);
}