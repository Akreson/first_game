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

// TODO: Add _m128
union m4x4
{
	f32 E[4][4];
};