#pragma once

struct bitmap_info
{
	void *TextureHandler;
	void *Data;
	f32 WidthOverHeight;
	u16 Width;
	u16 Height;
};

// TODO: replace patching?
struct font_asset_info
{
	union
	{
		void *Refs;

		// NOTE: Only for pointer for letter patching
		struct
		{
			u16 *UnicodeMap; // NOTE: 0 mean for this unicode code glyph doesn't exist
			s16 *KerningTable;
			s16 *GlyphAdvance; // TODO: does this need?
			f32 *VerticalAdjast;
			bitmap_info *Glyphs; // NOTE: Must be last
		};

	};

	u32 GlyphCount;
	u32 OnePastLastUnicodeCode;
	f32 AscenderHeight;
	f32 DescenderHeight;
	f32 LineGap;
};
#define MAX_REFS_METRICS_COUNT ((u32)OffsetOf(font_asset_info, Glyphs)/sizeof(void*))