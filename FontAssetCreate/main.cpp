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

int
main(int a, char **b)
{
	FILE *FileHandler = fopen("DevData//liberation-mono.regular.ttf", "rb");

	if (FileHandler)
	{
		entire_file TTFFile = {};
		stbtt_fontinfo FontInfo = {};
		float Scale;
		int Width, Height, XOffset, YOffset;

		fseek(FileHandler, 0, SEEK_END);
		TTFFile.Size = ftell(FileHandler);
		fseek(FileHandler, 0, SEEK_SET);

		TTFFile.Content = malloc(TTFFile.Size);
		fread(TTFFile.Content, TTFFile.Size, 1, FileHandler);
		fclose(FileHandler);

		stbtt_InitFont(&FontInfo, (u8 *)TTFFile.Content, 0);
		Scale = stbtt_ScaleForPixelHeight(&FontInfo, 120.0f);
		u8 *MonoBitmap = stbtt_GetCodepointBitmap(&FontInfo, 0, Scale, 'C',
			&Width, &Height, &XOffset, &YOffset);
		int Pitch = Width*BITMAP_BYTES_PER_PIXEL;
		int DestBitmapSize = Height*Pitch;
		u8 *DestMem = (u8 *)malloc(DestBitmapSize);

		u8 *Source = MonoBitmap;
		u8 *DestRow = DestMem + (Height - 1)*Pitch;
		for (int Y = 0;
			Y < Height;
			++Y)
		{
			u32 *Dest = (u32 *)DestRow;
			for (int X = 0;
				X < Width;
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

		stbtt_FreeBitmap(MonoBitmap, 0);
		free(TTFFile.Content);

		FILE *DestFile = fopen("data//font.edg", "wb");

		bitmap_info FontBitmapInfo = {Width, Height};

		fwrite((void *)&FontBitmapInfo, sizeof(bitmap_info), 1, DestFile);
		fwrite(DestMem, DestBitmapSize, 1, DestFile);
		fclose(DestFile);
	}
	else
	{
		printf("ERROR: Font file can't be open!");
	}

	return 0;
}