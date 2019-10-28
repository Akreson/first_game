#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "..\src\platform.h"

#define STB_TRUETYPE_IMPLEMENTATION 
#include "stb_truetype.h"

#define BITMAP_BYTES_PER_PIXEL 4

struct entire_file
{
	 void *Content;
	 u32 Size;
};

inline void
MemSets16(s16 Value, u64 Size, void *Ptr)
{
	s16 *Memory = (s16 *)Ptr;

	while (Size--)
	{
		*Memory++ = Value;
	}
}

inline void
FromMonoToRGBA(bitmap_info *Glyph, u8 *DestMem, u32 *BitmapSize)
{
	u32 Pitch = Glyph->Width * BITMAP_BYTES_PER_PIXEL;

	*BitmapSize = Glyph->Height * Pitch;;

	u8 *Source = (u8 *)Glyph->Memory;
	u8 *DestRow = DestMem + (Glyph->Height - 1)*Pitch;
	for (int Y = 0;
		Y < Glyph->Height;
		++Y)
	{
		u32 *Dest = (u32 *)DestRow;
		for (int X = 0;
			X < Glyph->Width;
			++X)
		{
			u8 MonoValue = *Source++;
			*Dest++ =
				((MonoValue << 24) |
				(MonoValue << 16) |
				(MonoValue << 8) |
				(MonoValue << 0));
		}

		DestRow -= Pitch;
	}
}

// TODO: Multiple font?
// TODO: premultiply alpha?
// TODO: Use u16 for glyph index and set 0 index as unused?
int
main(int a, char **b)
{
	FILE *FileHandler = fopen("DevData//liberation-mono.regular.ttf", "rb");

	if (FileHandler)
	{
		entire_file TTFFile = {};
		stbtt_fontinfo FontInfo = {};

		fseek(FileHandler, 0, SEEK_END);
		TTFFile.Size = ftell(FileHandler);
		fseek(FileHandler, 0, SEEK_SET);

		TTFFile.Content = malloc(TTFFile.Size);
		fread(TTFFile.Content, TTFFile.Size, 1, FileHandler);
		fclose(FileHandler);

		u32 FirstUnicodeCode = '!';
		u32	LastUnicodeCode = '~';
		u32 GlyphCount = LastUnicodeCode - FirstUnicodeCode;

		u32 AllocateMemorySize = 
			(sizeof(font_asset_info) +
			(sizeof(s16)*LastUnicodeCode) +
			(sizeof(s16)*GlyphCount*GlyphCount) +
			(sizeof(u16)*GlyphCount) + 
			(sizeof(bitmap_info)*GlyphCount));
		
		font_asset_info *FontAsset = (font_asset_info *)malloc(AllocateMemorySize);
		ZeroSize(AllocateMemorySize, (void *)FontAsset);

		FontAsset->UnicodeMap = (s16 *)((u8 *)FontAsset + sizeof(font_asset_info));
		FontAsset->KerningTable = (s16 *)((u8 *)FontAsset->UnicodeMap + (sizeof(s16)*LastUnicodeCode));
		FontAsset->GlyphAdvance = (s16 *)((u8 *)FontAsset->KerningTable + (sizeof(s16)*GlyphCount*GlyphCount));
		FontAsset->Glyphs = (bitmap_info *)((u8 *)FontAsset->GlyphAdvance + (sizeof(u16)*GlyphCount));

		MemSets16(-1, sizeof(s16)*LastUnicodeCode, FontAsset->UnicodeMap);

		FontAsset->LastUnicodeCode = LastUnicodeCode;

		stbtt_InitFont(&FontInfo, (u8 *)TTFFile.Content, 0);
		// NOTE: Excpect vertical metric be less than s16
		stbtt_GetFontVMetrics(&FontInfo, (int *)&FontAsset->AscenderHeight,
			(int *)&FontAsset->DescenderHeight, (int *)&FontAsset->LineGap);
		f32 Scale = stbtt_ScaleForPixelHeight(&FontInfo, 120.0f);
		int lsb;

		for (u32 UnicodeIndex = FirstUnicodeCode;
			UnicodeIndex <= FontAsset->LastUnicodeCode;
			++UnicodeIndex)
		{
			u16 GlyphIndex = FontAsset->UnicodeMap[UnicodeIndex] = FontAsset->GlyphCount++;
			bitmap_info *GlyphBitmap = FontAsset->Glyphs + GlyphIndex;
			
			// NOTE: Excpect width and height be less than u16
			GlyphBitmap->Memory = 
				(void *)stbtt_GetCodepointBitmap(&FontInfo, 0, Scale, UnicodeIndex,
					(int *)&GlyphBitmap->Width, (int *)&GlyphBitmap->Height, 0, 0);

			GlyphBitmap->WidthOverHeight = (f32)GlyphBitmap->Width / (f32)GlyphBitmap->Height;

			// NOTE: Excpect advanceWith be less than u16
			stbtt_GetCodepointHMetrics(&FontInfo, UnicodeIndex, (int *)&FontAsset->GlyphAdvance[GlyphIndex], &lsb);
		}

		Assert(FontAsset->GlyphCount == GlyphCount);

		for (u32 UnicodeIndex = FirstUnicodeCode;
			 UnicodeIndex <= FontAsset->LastUnicodeCode;
			++UnicodeIndex)
		{
			s16 FirstKerningIndex = FontAsset->UnicodeMap[UnicodeIndex];
			if (FirstKerningIndex != -1)
			{
				for (u32 KerningPairIndex = FirstUnicodeCode;
					KerningPairIndex <= FontAsset->LastUnicodeCode;
					++KerningPairIndex)
				{
					s16 SecondKerningPairIndex = FontAsset->UnicodeMap[UnicodeIndex];
					if (SecondKerningPairIndex != -1)
					{
						FontAsset->KerningTable[SecondKerningPairIndex*FontAsset->GlyphCount + FirstKerningIndex] =
							stbtt_GetCodepointKernAdvance(&FontInfo, UnicodeIndex, KerningPairIndex);
					}
				}
			}
		}

		u32 AllocatedMemoryForBitmaps = 0;
		for (u32 GlyphIndex = 0;
			GlyphIndex < FontAsset->GlyphCount;
			++GlyphIndex)
		{
			bitmap_info *GlyphBitmap = FontAsset->Glyphs + GlyphIndex;
			AllocatedMemoryForBitmaps += (GlyphBitmap->Height * GlyphBitmap->Width * BITMAP_BYTES_PER_PIXEL);
		}

		u8 *TempMemoryForGlyphsBitmap = (u8 *)malloc(AllocatedMemoryForBitmaps);
		u8 *DestMemory = TempMemoryForGlyphsBitmap;
		u32 OffsetForBitmap = AllocateMemorySize;
		u32 NextOffsetForBitmap = 0;
		for (u32 Index = 0;
			Index < FontAsset->GlyphCount;
			++Index)
		{
			bitmap_info *Glyph = FontAsset->Glyphs + Index;
			
			FromMonoToRGBA(Glyph, DestMemory, &NextOffsetForBitmap);
			//stbtt_FreeBitmap((unsigned char *)Glyph->Memory, 0);

			Glyph->Memory = (void *)OffsetForBitmap;
			OffsetForBitmap += NextOffsetForBitmap;
			DestMemory += NextOffsetForBitmap;
		}

		free(TTFFile.Content);

		for (u32 RefsIndex = 0;
			RefsIndex < MAX_REFS_METRICS_COUNT;
			++RefsIndex)
		{
			FontAsset->Refs[RefsIndex] = (umm *)((u8 *)FontAsset->Refs[RefsIndex] - (u8 *)FontAsset);
		}

		FILE *DestFile = fopen("data//font.edg", "wb");

		fwrite((void *)FontAsset, AllocateMemorySize, 1, DestFile);
		fwrite((void *)TempMemoryForGlyphsBitmap, AllocatedMemoryForBitmaps, 1, DestFile);
		fclose(DestFile);
	}
	else
	{
		printf("ERROR: Font file can't be open!");
	}

	return 0;
}