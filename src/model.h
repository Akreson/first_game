#pragma once

struct model_edge_faces
{
	u32 ID[2];
};

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

		model_edge_faces Faces;
	};

};

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

struct model_data_vertex
{
	v3 *E;
	u32 Count;
	u32 MaxCount;
};

struct model_data_face
{
	model_face *E;
	u32 Count;
	u32 MaxCount;
};

struct model_data_edge
{
	model_edge *E;
	u32 Count;
	u32 MaxCount;
};

// TODO: Separate?
// Choose how to store rotation (for now m4x4)
// Is need at all?!
// TODO: See if will need be opimize!!
struct vertex_transform_state
{
	// TODO: Get rid of rotation?
	m3x3 S;
	quat R;
	v3 T;
};

struct model_data
{
	model_data_vertex Vertices;
	model_data_edge Edges;
	model_data_face Faces;

	model_data_vertex SourceV;

	vertex_transform_state *VertexTrans;
};

struct work_model
{
	model_data Data;

	m3x3 Axis;
	rect3 AABB;
	v4 Color;
	v3 Scale;
	v3 Offset; // TODO: Store vertex in origin of model space or already in world space offset?
};

struct element_ray_result
{
	u32 ID;
	v3 P;
};

enum MaskMatchVertex
{
	MaskMatchVertex_01 = (1 << 1) | (1 << 0),
	MaskMatchVertex_12 = (1 << 2) | (1 << 1),
	MaskMatchVertex_03 = (1 << 3) | (1 << 0),
	MaskMatchVertex_23 = (1 << 3) | (1 << 2),
};

struct face_edge_match
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

struct face_vertex
{
	v3 V0, V1, V2, V3;
};

struct face_normals
{
	v3 N0, N1;
};

struct face_plane
{
	union
	{
		plane_params Plane[2];
		struct
		{
			plane_params P0;
			plane_params P1;
		};
	};
};

struct edge_faces_norm
{
	v3 N0, N1;
};

struct model_ray_result
{
	u32 ModelIndex;
	element_ray_result Face;
};

struct model_ray_sort
{
	u32 Index;
	f32 Length;
};

struct point_to_edge_proj
{
	model_edge *Edge;
	u32 ID;
	f32 t;
};