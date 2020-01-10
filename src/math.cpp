
// TODO: Don't use std math lib
#include <math.h>

#define Pi32 3.14159265359f
#define Tau32 6.2831853071f

inline v2
V2(f32 x, f32 y)
{
	v2 Result = {x, y};
	return Result;
}

inline v2
V2(f32 Value)
{
	v2 Result = {Value};
	return Result;
}

inline v3
V3(f32 x, f32 y, f32 z)
{
	v3 Result = {x, y, z};
	return Result;
}

inline v3
V3(f32 ValueXY, f32 z)
{
	v3 Result = {ValueXY, ValueXY, z};
	return Result;
}

inline v3
V3(f32 Value)
{
	v3 Result = {Value};
	return Result;
}

inline v4
V4(f32 x, f32 y, f32 z, f32 w)
{
	v4 Result = {x, y, z, w};
	return Result;
}

inline v4
V4(f32 XYZW)
{
	v4 Result = {XYZW};
	return Result;
}

inline v2
operator-(v2 A, v2 B)
{
	v2 Result;
	Result.x = A.x - B.x;
	Result.y = A.y - B.y;
	return Result;
}

// TODO: use sse mul?
inline v4
operator*(v4 A, v4 B)
{
	v4 Result;
	Result.x = A.x * B.x;
	Result.y = A.y * B.y;
	Result.z = A.z * B.z;
	Result.w = A.w * B.w;

	return Result;
}

inline m4x4
operator*(m4x4 A, m4x4 B)
{
	m4x4 Result = {};

	// TODO: Optimize
	for (u32 r = 0; r <= 3; ++r)
	{
		for (u32 c = 0; c <= 3; ++c)
		{
			for (u32 i = 0; i <= 3; ++i)
			{
				Result.E[r][c] += A.E[r][i] * B.E[i][c];
			}
		}
	}

	return Result;
}

inline v3
operator*(v3 A, m4x4 B)
{
	v3 Result;
	Result.x = A.x*B.E[0][0] + A.y*B.E[1][0] + A.z*B.E[2][0];
	Result.y = A.x*B.E[0][1] + A.y*B.E[1][1] + A.z*B.E[2][1];
	Result.z = A.x*B.E[0][2] + A.y*B.E[1][2] + A.z*B.E[2][2];

	return Result;
}

inline v4
operator*(v4 A, m4x4 B)
{
	v4 Result;
	Result.x = A.x*B.E[0][0] + A.y*B.E[1][0] + A.z*B.E[2][0] + A.w*B.E[3][0];
	Result.y = A.x*B.E[0][1] + A.y*B.E[1][1] + A.z*B.E[2][1] + A.w*B.E[3][1];
	Result.z = A.x*B.E[0][2] + A.y*B.E[1][2] + A.z*B.E[2][2] + A.w*B.E[3][2];
	Result.w = A.x*B.E[0][3] + A.y*B.E[1][3] + A.z*B.E[2][3] + A.w*B.E[3][3];

	return Result;
}

inline m4x4
Identity(void)
{
	m4x4 Result =
	{{
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	}};

	return Result;
}

// add inverse
inline m4x4
OrthographicProjection(f32 Width, f32 Height)
{
	f32 Far = 100.0f;
	f32 Near = -100.0f;

	f32 a = 2.0f / Width;
	f32 b = 2.0f / Height;
	f32 c = -2.0f / (Far - Near);
	f32 d = (Near + Far) / (Near - Far);

	m4x4 Result =
	{{
		{ a,  0, 0, 0},
		{ 0,  b, 0, 0},
		{ 0,  0, c, 0},
		{-1, -1, d, 1}
	}};

	return Result;
}

inline m4x4
PerspectiveProjection(f32 FocalLength, f32 WidthOverHeight)
{
	
	f32 Near = 0.1f;
	f32 Far = 200.0f;

	f32 ZRange = Far - Near;

	f32 a = 1.0f / FocalLength;
	f32 b = 1.0f / FocalLength * WidthOverHeight;
	f32 c = -(Near + Far) / ZRange;
	f32 d = -(2.0f * Far * Near) / ZRange;

	m4x4 Result = 
	{{
		{a, 0, 0,  0},
		{0, b, 0,  0},
		{0, 0, c, -1},
		{0, 0, d,  0}
	}};

	return Result;
}

// TODO: Use quat for rotation?
inline m4x4
XRotation(f32 Angle)
{
	f32 c = cosf(Angle);
	f32 s = sinf(Angle);

	m4x4 Result =
	{{
		{ 1,  0, 0, 0},
		{ 0,  c, s, 0},
		{ 0, -s, c, 0},
		{ 0,  0, 0, 1}
	}};

	return Result;
}

inline m4x4
YRotation(f32 Angle)
{
	f32 c = cosf(Angle);
	f32 s = sinf(Angle);

	m4x4 Result =
	{{
		{c, 0, -s, 0},
		{0, 1,  0, 0},
		{s, 0,  c, 0},
		{0, 0,  0, 1}
	}};

	return Result;
}

inline m4x4
ZRotation(f32 Angle)
{
	f32 c = cosf(Angle);
	f32 s = sinf(Angle);

	m4x4 Result =
	{{
		{ c, s, 0, 0},
		{-s, c, 0, 0},
		{ 0, 0, 1, 0},
		{ 0, 0, 0, 1}
	}};

	return Result;
}

inline void
Translate(m4x4 *A, v3 B)
{
	v3 *TranslationPart = (v3 *)&A->E[3][0];
	*TranslationPart = B;
}