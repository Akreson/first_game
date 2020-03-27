// TODO: Find way combine MatchEdgeToFace and MatchFaceToEdge?
inline void
MatchEdgeToFace(model_edge *Edges, u32 EdgeCount, model_face *Faces, u32 FaceCount)
{
	for (u32 EdgeIndex = 0;
		EdgeIndex < EdgeCount;
		++EdgeIndex)
	{
		model_edge *Edge = Edges + EdgeIndex;

		u32 FaceMatchedIndex = 0;
		for (u32 FaceIndex = 0;
			FaceIndex < FaceCount;
			++FaceIndex)
		{
			model_face *Face = Faces + FaceIndex;

			u32 MatchedCount = 0;
			for (u32 EdgeVertexID = 0;
				EdgeVertexID < ArrayCount(Edge->VertexID);
				++EdgeVertexID)
			{
				for (u32 FaceVertexID = 0;
					FaceVertexID < ArrayCount(Face->VertexID);
					++FaceVertexID)
				{
					if (Edge->VertexID[EdgeVertexID] == Face->VertexID[FaceVertexID])
					{
						MatchedCount++;
						break;
					}
				}
			}

			if (MatchedCount == ArrayCount(Edge->VertexID))
			{
				Edge->FaceID[FaceMatchedIndex] = FaceIndex;
				FaceMatchedIndex++;

				if (FaceMatchedIndex == ArrayCount(Edge->FaceID)) break;
			}

		}

		Assert(FaceMatchedIndex == ArrayCount(Edge->FaceID));
	}
}

inline void
MatchFaceToEdge(model_edge *Edges, u32 EdgeCount, model_face *Faces, u32 FaceCount)
{
	for (u32 FaceIndex = 0;
		FaceIndex < FaceCount;
		++FaceIndex)
	{
		model_face *Face = Faces + FaceIndex;

		u32 EdgeMatchedIndex = 0;
		for (u32 EdgeIndex = 0;
			EdgeIndex < EdgeCount;
			++EdgeIndex)
		{
			model_edge *Edge = Edges + EdgeIndex;

			u32 MatchedCount = 0;
			for (u32 EdgeVertexID = 0;
				EdgeVertexID < ArrayCount(Edge->VertexID);
				++EdgeVertexID)
			{
				for (u32 FaceVertexID = 0;
					FaceVertexID < ArrayCount(Face->VertexID);
					++FaceVertexID)
				{
					if (Edge->VertexID[EdgeVertexID] == Face->VertexID[FaceVertexID])
					{
						MatchedCount++;
						break;
					}
				}
			}

			if (MatchedCount == ArrayCount(Edge->VertexID))
			{
				Face->EdgeID[EdgeMatchedIndex] = EdgeIndex;
				EdgeMatchedIndex++;

				if (EdgeMatchedIndex == ArrayCount(Face->EdgeID)) break;
			}
		}

		Assert(EdgeMatchedIndex == ArrayCount(Face->EdgeID));
	}
}

rect3
ComputeMeshAABB(v3 *VertexArray, u32 VertexCount)
{
	rect3 Rect;
	Rect.Min = V3(FLOAT_MAX);
	Rect.Max = V3(FLOAT_MAX_NEG);

	for (u32 VertexIndex = 0;
		VertexIndex < VertexCount;
		++VertexIndex)
	{
		v3 Vertex = VertexArray[VertexIndex];

		if (Vertex.x < Rect.Min.x) Rect.Min.x = Vertex.x;
		if (Vertex.x > Rect.Max.x) Rect.Max.x = Vertex.x;

		if (Vertex.y < Rect.Min.y) Rect.Min.y = Vertex.y;
		if (Vertex.y > Rect.Max.y) Rect.Max.y = Vertex.y;

		if (Vertex.z < Rect.Min.z) Rect.Min.z = Vertex.z;
		if (Vertex.z > Rect.Max.z) Rect.Max.z = Vertex.z;
	}

	Assert(Rect.Max.x < 5.0f);
	Assert(Rect.Max.y < 5.0f);
	Assert(Rect.Max.z < 5.0f);

	return Rect;
}

void
GeneratingCube(page_memory_arena *Arena, model *Model, f32 HalfDim = 0.5f)
{
	v3 Vertex[8];
	Vertex[0] = V3(-HalfDim, -HalfDim, HalfDim);
	Vertex[1] = V3(HalfDim, -HalfDim, HalfDim);
	Vertex[2] = V3(HalfDim, HalfDim, HalfDim);
	Vertex[3] = V3(-HalfDim, HalfDim, HalfDim);

	Vertex[4] = V3(HalfDim, -HalfDim, -HalfDim);
	Vertex[5] = V3(-HalfDim, -HalfDim, -HalfDim);
	Vertex[6] = V3(-HalfDim, HalfDim, -HalfDim);
	Vertex[7] = V3(HalfDim, HalfDim, -HalfDim);

#if 0
	// NOTE: For debug
	f32 DebugHalfDim = HalfDim * 0.07f;
	Vertex[8] = V3(-DebugHalfDim, -DebugHalfDim, 0);
	Vertex[9] = V3(DebugHalfDim, -DebugHalfDim, 0);
	Vertex[10] = V3(DebugHalfDim, DebugHalfDim, 0);
	Vertex[11] = V3(-DebugHalfDim, DebugHalfDim, 0);
#endif

	model_face Faces[6];
	Faces[0] = {{0, 1, 2, 3}, {}};
	Faces[1] = {{4, 5, 6, 7}, {}};
	Faces[2] = {{1, 4, 7, 2}, {}};
	Faces[3] = {{5, 0, 3, 6}, {}};
	Faces[4] = {{5, 4, 1, 0}, {}};
	Faces[5] = {{3, 2, 7, 6}, {}};

	model_edge Edges[12];
	Edges[0] = {{3, 0}, {}};
	Edges[1] = {{0, 1}, {}};
	Edges[2] = {{1, 2}, {}};
	Edges[3] = {{2, 3}, {}};
	Edges[4] = {{5, 0}, {}};
	Edges[5] = {{4, 1}, {}};
	Edges[6] = {{7, 2}, {}};
	Edges[7] = {{6, 3}, {}};
	Edges[8] = {{6, 5}, {}};
	Edges[9] = {{5, 4}, {}};
	Edges[10] = {{7, 4}, {}};
	Edges[11] = {{7, 6}, {}};

	MatchEdgeToFace(Edges, ArrayCount(Edges), Faces, ArrayCount(Faces));
	MatchFaceToEdge(Edges, ArrayCount(Edges), Faces, ArrayCount(Faces));

	PagePushArray(Arena, v3, ArrayCount(Vertex), Model->Vertex, Vertex);
	PagePushArray(Arena, model_face, ArrayCount(Faces), Model->Faces, Faces);
	PagePushArray(Arena, model_edge, ArrayCount(Edges), Model->Edges, Edges);

	Model->VertexCount = ArrayCount(Vertex);
	Model->FaceCount = ArrayCount(Faces);
	Model->EdgeCount = ArrayCount(Edges);
}


inline model *
AddModel(game_editor_state *EditorState, v4 Color, v3 Offset)
{
	model *Model = EditorState->Models + EditorState->ModelsCount++;
	Assert(EditorState->ModelsCount < ArrayCount(EditorState->Models));
	Model->Color = Color;
	Model->Offset = Offset;

	return Model;
}

void
AddCubeModel(game_editor_state *EditorState, v3 Offset = V3(0), v4 Color = V4(0, 0, 0, 1.0f))
{
	model *Model = AddModel(EditorState, Color, Offset);
	GeneratingCube(&EditorState->PageArena, Model);
	Model->AABB = ComputeMeshAABB(Model->Vertex, Model->VertexCount);
}