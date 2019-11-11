
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
		{    a,     0, 0,    0},
		{    0,     b, 0,    0},
		{    0,     0, c,    0},
		{-1.0f, -1.0f, d, 1.0f}
	}};

	return Result;
}