#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "..\src\platform.h"

#define STB_TRUETYPE_IMPLEMENTATION 
#include "stb_truetype.h"

struct entire_file
{
	 u32 Size;
	 void *Content;
};

int
main(int a, char **b)
{
	//stbtt_fontinfo font;

	FILE *FileHandler = fopen("DevData//liberation-mono.regular.ttf", "rb");

	if (FileHandler)
	{
		entire_file File;

		fseek(FileHandler, 0, SEEK_END);
		File.Size = ftell(FileHandler);
		fseek(FileHandler, 0, SEEK_SET);

		File.Content = malloc(File.Size);
		fread(File.Content, File.Size, 1, FileHandler);
		fclose(FileHandler);
	}
	else
	{
		printf("ERROR: Font file can't be open!");
	}

	return 0;
}