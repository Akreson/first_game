union v2
{
	f32 E[2];
	struct
	{
		f32 x, y;
	};
};

union v3
{
	f32 E[3];

	struct
	{
		f32 x, y, z;
	};

	struct
	{
		v2 xy;
		f32 _Ignored0;
	};

	struct
	{
		f32 _Ignored1;
		v2 yz;
	};
};

// TODO: Add _m128
union v4
{
	f32 E[4];

	struct
	{
		f32 x, y, z, w;
	};

	struct
	{
		v3 xyz;
		f32 _Ignored0;
	};
};

// NOTE: row-major
// TODO: Add _m128
union m4x4
{
	f32 E[4][4];
};

// add inverse
inline m4x4
OrthographicProjection(f32 AspectWidthOverHeight, v2 ScreenDim)
{
	f32 Far = 100.0f;
	f32 Near = -100.0f;

	f32 a = 2.0f / ScreenDim.x;
	f32 b = 2.0f / ScreenDim.y;
	f32 c = -2.0f / (Far - Near);
	f32 d = (Near + Far) / (Near - Far);

	m4x4 Result =
	{{
		{    a,     0, 0, 0},
		{    0,     b, 0, 0},
		{    0,     0, c, 0},
		{-1.0f, -1.0f, d, 0}
	}};

	return Result;
}