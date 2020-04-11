#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "..\src\platform.h"
#include "..\src\asset.h"

#define STB_TRUETYPE_IMPLEMENTATION 
#include "stb_truetype.h"

#define BITMAP_BYTES_PER_PIXEL 4

struct entire_file
{
	 void *Content;
	 u32 Size;
};

const char *FontFileName = "DevData//LiberationMono-Regular.ttf";
const char *OutputPath = "data//font.edg";

inline void
FromMonoToRGBA(bitmap_info *Glyph, u8 *DestMem, u32 *BitmapSize)
{
	u32 Pitch = Glyph->Width * BITMAP_BYTES_PER_PIXEL;

	*BitmapSize = Glyph->Height * Pitch;

	u8 *Source = (u8 *)Glyph->Data;
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
// TODO: Use u16 for glyph index and set 0 index as unused
// TODO: use 1 byte for bitmap?
int
main(int argc, char **argv)
{
	const char *FontPath = FontFileName;

	if (argv[1])
	{
		FontPath = argv[1];
	}

	FILE *FileHandler = fopen(FontPath, "rb");

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

		u32 FirstUnicodeCode = ' ';
		u32	LastUnicodeCode = '~';
		u32 GlyphCount = (LastUnicodeCode - FirstUnicodeCode) + 1;
		u32 OnePastLastUnicodeCode = LastUnicodeCode + 1;

		u32 AllocateMemorySize = 
			(sizeof(font_asset_info) +
			(sizeof(s16)*OnePastLastUnicodeCode) +
			(sizeof(s16)*GlyphCount*GlyphCount) +
			(sizeof(u16)*GlyphCount) + 
			(sizeof(f32)*GlyphCount) +
			(sizeof(bitmap_info)*GlyphCount));
		
		font_asset_info *FontAsset = (font_asset_info *)malloc(AllocateMemorySize);
		ZeroSize((void *)FontAsset, AllocateMemorySize);

		FontAsset->UnicodeMap = (u16 *)((u8 *)FontAsset + sizeof(font_asset_info));
		// TODO: Collate table and advance;
		FontAsset->KerningTable = (s16 *)((u8 *)FontAsset->UnicodeMap + (sizeof(s16)*OnePastLastUnicodeCode));
		FontAsset->GlyphAdvance = (s16 *)((u8 *)FontAsset->KerningTable + (sizeof(s16)*GlyphCount*GlyphCount));
		FontAsset->VerticalAdjast = (f32 *)((u8 *)FontAsset->GlyphAdvance + (sizeof(s16)*GlyphCount));
		FontAsset->Glyphs = (bitmap_info *)((u8 *)FontAsset->VerticalAdjast + (sizeof(f32)*GlyphCount));
		FontAsset->OnePastLastUnicodeCode = OnePastLastUnicodeCode;

		stbtt_InitFont(&FontInfo, (u8 *)TTFFile.Content, 0);
		f32 Scale = stbtt_ScaleForPixelHeight(&FontInfo, 100.0f);

		s32 AscenderHeight;
		s32 DescenderHeight;
		s32 LineGap;
		stbtt_GetFontVMetrics(&FontInfo, &AscenderHeight, &DescenderHeight, &LineGap);

		// NOTE: Store metrics in scale value
		FontAsset->AscenderHeight = (f32)AscenderHeight*Scale;
		FontAsset->DescenderHeight = (f32)DescenderHeight*Scale;
		FontAsset->LineGap = (f32)LineGap*Scale;

		for (u32 UnicodeIndex = FirstUnicodeCode;
			UnicodeIndex < FontAsset->OnePastLastUnicodeCode;
			++UnicodeIndex)
		{
			// TODO: Fix FirstUnicodeCode == 0, so it dont be used
			u16 GlyphIndex = FontAsset->UnicodeMap[UnicodeIndex] = FontAsset->GlyphCount++;
			bitmap_info *GlyphBitmap = FontAsset->Glyphs + GlyphIndex;

			// NOTE: Excpect width and height be less than u16
			GlyphBitmap->Data =
				(void *)stbtt_GetCodepointBitmap(&FontInfo, 0, Scale, UnicodeIndex,
					(int *)&GlyphBitmap->Width, (int *)&GlyphBitmap->Height, 0, 0);

			
			int x0, x1, y0, y1;
			stbtt_GetCodepointBitmapBox(&FontInfo, UnicodeIndex, Scale, Scale, &x0, &y0, &x1, &y1);
			FontAsset->VerticalAdjast[GlyphIndex] = (f32)y1 / (f32)GlyphBitmap->Height;

			GlyphBitmap->WidthOverHeight = (f32)GlyphBitmap->Width / (f32)GlyphBitmap->Height;

			// NOTE: Excpect advanceWith be less than s16
			int AdvanceWidth;
			stbtt_GetCodepointHMetrics(&FontInfo, UnicodeIndex, &AdvanceWidth, 0);
			FontAsset->GlyphAdvance[GlyphIndex] = (s16)((f32)AdvanceWidth * Scale);
		}

		Assert(FontAsset->GlyphCount == GlyphCount);

		for (u32 UnicodeIndex = FirstUnicodeCode;
			 UnicodeIndex < FontAsset->OnePastLastUnicodeCode;
			++UnicodeIndex)
		{
			s16 FirstKerningIndex = FontAsset->UnicodeMap[UnicodeIndex];
			if (FirstKerningIndex)
			{
				for (u32 KerningPairIndex = FirstUnicodeCode;
					KerningPairIndex < FontAsset->OnePastLastUnicodeCode;
					++KerningPairIndex)
				{
					s16 SecondKerningPairIndex = FontAsset->UnicodeMap[KerningPairIndex];
					if (SecondKerningPairIndex)
					{
						s32 KerningValue = stbtt_GetCodepointKernAdvance(&FontInfo, UnicodeIndex, KerningPairIndex);
						FontAsset->KerningTable[SecondKerningPairIndex*FontAsset->GlyphCount + FirstKerningIndex] =
							(s16)(Scale * (f32)KerningValue);
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
			bitmap_info *GlyphBitmap = FontAsset->Glyphs + Index;
			
			FromMonoToRGBA(GlyphBitmap, DestMemory, &NextOffsetForBitmap);
			stbtt_FreeBitmap((unsigned char *)GlyphBitmap->Data, 0);

			GlyphBitmap->Data = (void *)OffsetForBitmap;
			OffsetForBitmap += NextOffsetForBitmap;
			DestMemory += NextOffsetForBitmap;
		}

		// NOTE: Store offset for later patching
		umm **PtrToMetrics = (umm **)&FontAsset->Refs;
		for (u32 RefsIndex = 0;
			RefsIndex <= MAX_REFS_METRICS_COUNT;
			++RefsIndex)
		{
			PtrToMetrics[RefsIndex] = (umm *)((u8 *)PtrToMetrics[RefsIndex] - (u8 *)PtrToMetrics);
		}

 		free(TTFFile.Content);

		FILE *DestFile = fopen(OutputPath, "wb");

		fwrite((void *)FontAsset, AllocateMemorySize, 1, DestFile);
		fwrite((void *)TempMemoryForGlyphsBitmap, AllocatedMemoryForBitmaps, 1, DestFile);
		fclose(DestFile);
	}
	else
	{
		printf("ERROR: Cannot open file %s.\n", (char *)FontPath);
		Assert(0);
	}

	return 0;
}