#pragma once

struct model_edge
{
	union
	{
		u16 VertexID[2];
		struct
		{
			u16 V0, V1;
		};
	};

	union
	{
		u16 FaceID[2];

		struct
		{
			u16 Face0, Face1;
		};
	};

};

// TODO: Expand to u32??? For convenient simd usage?
// TODO: Model vertex count never be bigger than U16_MAX_VALUE?
struct model_face
{
	union
	{
		u16 VertexID[4];
		struct
		{
			u16 V0, V1, V2, V3;
		};
	};

	// NOTE: All models build from quads
	union
	{
		u16 EdgeID[4];

		struct
		{
			u16 Edge0, Edge1, Edge2, Edge3;
		};
	};
};

// NOTE: Triangle specifed in conter-clokwise order
struct model
{
	v3 *Vertex;
	model_face *Faces;
	model_edge *Edges;
	v4 Color;
	rect3 AABB;
	v3 Offset; // TODO: Store vertex in origin of model space or already in world space offset?
	u16 FaceCount;
	u16 VertexCount;
	u16 EdgeCount;
};

struct element_ray_result
{
	u32 ID;
	v3 P;
};