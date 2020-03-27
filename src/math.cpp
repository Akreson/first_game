
// TODO: Don't use std math lib
#include <math.h>

#define Pi32 3.14159265359f
#define Tau32 6.2831853071f

struct rect3
{
	v3 Min;
	v3 Max;
};

struct ray_params
{
	v3 Dir;
	v3 Pos;
};

struct plane_params
{
	v3 N;
	f32 D;
};

inline f32
Cos(f32 Angle)
{
	f32 Result = cosf(Angle);
	return Result;
}

inline f32
Sin(f32 Angle)
{
	f32 Result = sinf(Angle);
	return Result;
}

inline v2
V2(f32 x, f32 y)
{
	v2 Result = {x, y};
	return Result;
}

inline v2
V2(f32 Value)
{
	v2 Result = {Value, Value};
	return Result;
}

inline v3
V3(f32 x, f32 y, f32 z)
{
	v3 Result = {x, y, z};
	return Result;
}

inline v3
V3(v2 A, f32 z)
{
	v3 Result = {A.x, A.y, z};
	return Result;
}

inline v3
V3(f32 Value)
{
	v3 Result = {Value, Value, Value};
	return Result;
}

inline v4
V4(f32 x, f32 y, f32 z, f32 w)
{
	v4 Result = {x, y, z, w};
	return Result;
}

inline v4
V4(f32 Value)
{
	v4 Result = {Value, Value, Value, Value};
	return Result;
}

inline v4
V4(v3 A, f32 w)
{
	v4 Result;
	Result.xyz = A;
	Result.w = w;

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

inline v2
operator/(v2 A, v2 B)
{
	v2 Result;
	Result.x = A.x / B.x;
	Result.y = A.y / B.y;
	return Result;
}

inline v2
operator*(v2 A, f32 B)
{
	v2 Result;
	Result.x = A.x * B;
	Result.y = A.y * B;
	return Result;
}

inline v2
operator-(v2 A, f32 B)
{
	v2 Result;
	Result.x = A.x - B;
	Result.y = A.y - B;
	return Result;
}

inline v3
operator-(v3 A)
{
	v3 Result = {-A.x, -A.y, -A.z};
	return Result;
}

inline v3
operator-(v3 A, v3 B)
{
	v3 Result;
	Result.x = A.x - B.x;
	Result.y = A.y - B.y;
	Result.z = A.z - B.z;

	return Result;
}

inline v3
operator+(v3 A, v3 B)
{
	v3 Result;
	Result.x = A.x + B.x;
	Result.y = A.y + B.y;
	Result.z = A.z + B.z;

	return Result;
}

inline v3
operator*(v3 A, v3 B)
{
	v3 Result;
	Result.x = A.x * B.x;
	Result.y = A.y * B.y;
	Result.z = A.z * B.z;

	return Result;
}

inline v3
operator/(v3 A, f32 B)
{
	v3 Result;
	Result.x = A.x / B;
	Result.y = A.y / B;
	Result.z = A.z / B;

	return Result;
}

inline v3
operator/(f32 B, v3 A)
{
	v3 Result;
	Result.x = B / A.x;
	Result.y = B / A.y;
	Result.z = B / A.z;

	return Result;
}


inline v3
operator*(v3 A, f32 B)
{
	v3 Result;
	Result.x = A.x * B;
	Result.y = A.y * B;
	Result.z = A.z * B;

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

inline f32
Dot(v3 A, v3 B)
{
	f32 Result = A.x*B.x + A.y*B.y + A.z*B.z;
	return Result;
}

inline f32
LengthSq(v3 A)
{
	f32 Result = Dot(A, A);
	return Result;
}

inline f32
Length(v3 A)
{
	f32 Result = SquareRoot(LengthSq(A));
	return Result;
}

inline v3
Normalize(v3 A)
{
	v3 Result = A * (1.0f / Length(A));
	return Result;
}

inline v3
Cross(v3 A, v3 B)
{
	v3 Result;

	Result.x = A.y*B.z - A.z*B.y;
	Result.y = A.z*B.x - A.x*B.z;
	Result.z = A.x*B.y - A.y*B.x;

	return Result;
}

// TODO: Optimize?
inline v4
Transform(v4 A, m4x4 B)
{
	v4 Result;
	Result.x = A.x*B.E[0][0] + A.y*B.E[1][0] + A.z*B.E[2][0] + A.w*B.E[3][0];
	Result.y = A.x*B.E[0][1] + A.y*B.E[1][1] + A.z*B.E[2][1] + A.w*B.E[3][1];
	Result.z = A.x*B.E[0][2] + A.y*B.E[1][2] + A.z*B.E[2][2] + A.w*B.E[3][2];
	Result.w = A.x*B.E[0][3] + A.y*B.E[1][3] + A.z*B.E[2][3] + A.w*B.E[3][3];

	return Result;
}

inline v3
operator*(v3 A, m4x4 B)
{
	v3 Result = Transform(V4(A, 1.0f), B).xyz;
	return Result;
}

inline v4
operator*(v4 A, m4x4 B)
{
	v4 Result = Transform(A, B);
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
GetRow(m4x4 A, u32 R)
{
	v3 Result = {A.E[R][0], A.E[R][1], A.E[R][2]};
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

inline m4x4_inv
OrthographicProjection(f32 Width, f32 Height)
{
	f32 Far = 100.0f;
	f32 Near = -100.0f;

	f32 a = 2.0f / Width;
	f32 b = 2.0f / Height;
	f32 c = -2.0f / (Far - Near);
	f32 d = (Near + Far) / (Near - Far);

	m4x4_inv Result;
	Result.Forward =
	{{
		{a,  0,  0, 0},
		{0,  b,  0, 0},
		{0,  0,  c, 0},
		{-1, -1, d, 1}
	}};
	Result.Inverse =
	{{
		{1/a,   0,    0, 0},
		{  0, 1/b,    0, 0},
		{  0,   0,  1/c, 0},
		{1/a, 1/b, -d/c, 1}
	}};

#if 0
	m4x4 I = Result.Forward * Result.Inverse;
#endif

	return Result;
}

inline m4x4_inv
PerspectiveProjection(f32 FocalLength, f32 WidthOverHeight)
{
	
	f32 Near = 0.1f;
	f32 Far = 200.0f;

	f32 ZRange = Far - Near;

	f32 a = 1.0f / FocalLength;
	f32 b = 1.0f / FocalLength * WidthOverHeight;
	f32 c = -(Near + Far) / ZRange;
	f32 d = -(2.0f * Far * Near) / ZRange;

	m4x4_inv Result;
	Result.Forward = 
	{{
		{a, 0, 0,  0},
		{0, b, 0,  0},
		{0, 0, c, -1},
		{0, 0, d,  0}
	}};
	Result.Inverse = 
	{{
		{1/a,   0,   0,   0},
		{  0, 1/b,   0,   0},
		{  0,   0,   0, 1/d},
		{  0,   0,  -1, c/d}
	}};

#if 1
	m4x4 I = Result.Forward * Result.Inverse;
	v4 Test0 = V4(0, 0, -Near, 1.0f) * Result.Forward;
	Test0.xyz = Test0.xyz / Test0.w;
	v4 Test1 = V4(0, 0, -Far, 1.0f) * Result.Forward;
	Test1.xyz = Test1.xyz / Test1.w;

	v4 Test2 = V4(0, 0, -1, 1.0f) * Result.Inverse;
	v4 Test3 = V4(0, 0, 1, 1.0f) * Result.Inverse;
#endif

	return Result;
}

// TODO: Use quaternion for rotation?
inline m4x4
XRotation(f32 Angle)
{
	f32 c = Cos(Angle);
	f32 s = Sin(Angle);

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
	f32 c = Cos(Angle);
	f32 s = Sin(Angle);

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
	f32 c = Cos(Angle);
	f32 s = Sin(Angle);

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
SetTranslation(m4x4 *A, v3 B)
{
	A->Row3.xyz = B;
}

inline m4x4
Transpose(m4x4 A)
{
	m4x4 Result;

	for (u32 i = 0; i <= 3; ++i)
	{
		for (u32 y = 0; y <= 3; ++y)
		{
			Result.E[y][i] = A.E[i][y];
		}
	}

	return Result;
}

inline m4x4
Row3x3(v3 X, v3 Y, v3 Z)
{
	m4x4 Result = {};
	Result.Row0.xyz = X;
	Result.Row1.xyz = Y;
	Result.Row2.xyz = Z;
	Result.E[3][3] = 1.0f;

	return Result;
}

inline m4x4_inv
CameraViewTransform(m4x4 R, v3 P, v3 O)
{
	m4x4_inv Result;

	m4x4 A = Transpose(R);
	v3 PA = -(P * A);
	SetTranslation(&A, PA);
	Result.Forward = A;

	v3 iX = GetRow(R, 0);
	v3 iY = GetRow(R, 1);
	v3 iZ = GetRow(R, 2);
	iX = iX / LengthSq(iX);
	iY = iY / LengthSq(iY);
	iZ = iZ / LengthSq(iZ);
	
	v3 iP = 
	{	
		PA.x*iX.x + PA.y*iY.x + PA.z*iZ.x,
		PA.x*iX.y + PA.y*iY.y + PA.z*iZ.y, 
		PA.x*iX.z + PA.y*iY.z + PA.z*iZ.z
	};

	m4x4 B = Row3x3(iX, iY, iZ);
	SetTranslation(&B, -iP);
	Result.Inverse = B;

#if 1
	m4x4 I = Result.Forward * Result.Inverse;
#endif
	return Result;
}

// TODO: Consider to optimize
inline b32
IsPointInTriangle(v3 A, v3 B, v3 C, v3 P)
{
	v3 AP = A - P;
	v3 BP = B - P;
	v3 CP = C - P;

	v3 U = Cross(BP, CP);
	v3 V = Cross(CP, AP);
	v3 W = Cross(AP, BP);

	f32 DirMesureUV = Dot(U, V);
	f32 DirMesureUW = Dot(U, W);

	b32 Result = (DirMesureUV > 0) && (DirMesureUW > 0);
	return Result;
}

// TODO: Consider to optimize
b32
RayAABBIntersect(ray_params Ray, rect3 AABB, v3 AABBOffset)
{
	b32 Result = true;

	v3 InvD = 1.0 / Ray.Dir;

	v3 Min = (AABB.Min + AABBOffset);
	v3 Max = (AABB.Max + AABBOffset);
	v3 tMinV = (Min - Ray.Pos) * InvD;
	v3 tMaxV = (Max - Ray.Pos) * InvD;

	f32 tMin = MAX(MAX(MIN(tMinV.x, tMaxV.x), MIN(tMinV.y, tMaxV.y)), MIN(tMinV.z, tMaxV.z));
	f32 tMax = MIN(MIN(MAX(tMinV.x, tMaxV.x), MAX(tMinV.y, tMaxV.y)), MAX(tMinV.z, tMaxV.z));

	if ((tMax < 0) || (tMin > tMax))
	{
		Result = false;
	}

	return Result;
}