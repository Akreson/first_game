#pragma once

union v2
{
	f32 E[2];
	struct
	{
		f32 x, y;
	};
};

union v2i
{
	s32 E[2];
	struct
	{
		s32 x, y;
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
		f32 r, g, b;
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
		v2 xy;
		v2 zw;
	};

	struct
	{
		f32 r, g, b, a;
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

	struct
	{
		v4 Row0;
		v4 Row1;
		v4 Row2;
		v4 Row3;
	};
};

union m3x3
{
	f32 E[3][3];

	struct
	{
		v3 Row0;
		v3 Row1;
		v3 Row2;
	};

	struct
	{
		v3 Row[3];
	};

	struct
	{
		v3 X;
		v3 Y;
		v3 Z;
	};
};

struct m4x4_inv
{
	m4x4 Forward;
	m4x4 Inverse;
};
