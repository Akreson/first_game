
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
	f32 Far = 100.0f;

	f32 ZRange = Far - Near;

	f32 a = 1.0f / FocalLength * WidthOverHeight;
	f32 b = 1.0f / FocalLength;
	f32 c = -(Near + Far) / ZRange;
	f32 d = -(2.0f * Far * Near) / ZRange;

	m4x4 Result = 
	{{
			a, 0, 0,  0,
			0, b, 0,  0,
			0, 0, c, -1,
			0, 0, d,  0
	}};

	return Result;
}