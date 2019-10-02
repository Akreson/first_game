#pragma once

inline b32
IsEndOfLine(char A)
{
	b32 Result = (A == '\n') || (A == '\r');
	return Result;
}

inline b32
IsWhitespace(char A)
{
	b32 Result = (A == ' ') || (A == '\t') || IsEndOfLine(A);
	return Result;
}

b32
StringAreEqual(u32 ALength, char *A, char const *B)
{
	b32 Result = false;
	char const *At = B;

	for (u32 Index = 0;
		Index < ALength;
		++Index, ++At)
	{
		if ((*At == 0) || A[Index] != *At)
		{
			return false;
		}
	}

	Result = (*At == 0);

	return Result;
}