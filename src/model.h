#pragma once

struct model_edge
{
	union
	{
		u32 VertexID[2];
		struct
		{
			u32 V0, V1;
		};
	};

	union
	{
		u32 FaceID[2];

		struct
		{
			u32 Face0, Face1;
		};
	};

};

// TODO: Expand to u32??? For convenient simd usage?
// TODO: Model vertex count never be bigger than U16_MAX_VALUE?
struct model_face
{
	union
	{
		u32 VertexID[4];
		struct
		{
			u32 V0, V1, V2, V3;
		};
	};

	// NOTE: All models build from quads
	union
	{
		u32 EdgesID[4];

		struct
		{
			u32 Edge0, Edge1, Edge2, Edge3;
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
	u32 FaceCount;
	u32 VertexCount;
	u32 EdgeCount;
};

struct element_ray_result
{
	u32 ID;
	v3 P;
};

struct faces_edge_match
{
	u16 Index;
	b16 Succes;
};

// TODO: Can be more then 2 vertex?
struct face_vertex_match
{
	u16 Index[2];
	b16 Succes;
};

struct edge_vertex_match
{
	u16 Index;
	u16 Succes;
};