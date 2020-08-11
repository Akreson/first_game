#pragma once

struct bitmap_info
{
	void *Data;
	f32 WidthOverHeight;
	u16 Width;
	u16 Height;
	renderer_texture Texture;
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
			u16 *UnicodeMap;
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

struct tris_u32
{
	u32 I0;
	u32 I1;
	u32 I2;
};

struct static_mesh
{
	union
	{
		void *Data;

		struct
		{
			v3 *Vertex;
			void *Tris;
		};
	};
	u32 VertexCount;
	u32 TrisCount;

	renderer_mesh Mesh;
};

#define MAX_FONT_REFS_METRICS_COUNT ((u32)OffsetOf(font_asset_info, Glyphs) / sizeof(void*))