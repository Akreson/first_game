
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
		GlyphBitmap->Memory = (void *)((u8 *)PtrToMetrics + (u64)GlyphBitmap->Memory);
	}
}

inline u32
GetGlyphFromCodePoint(font_asset_info *FontAsset, u32 CodePoint)
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

void
UpdateAndRender(game_input *GameInput)
{

}