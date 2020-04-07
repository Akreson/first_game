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
AddModel(game_editor_state *Editor, v4 Color, v3 Offset)
{
	model *Model = Editor->Models + Editor->ModelsCount++;
	Assert(Editor->ModelsCount < ArrayCount(Editor->Models));
	Model->Color = Color;
	Model->Offset = Offset;

	return Model;
}

void
AddCubeModel(game_editor_state *Editor, v3 Offset = V3(0), v4 Color = V4(0.3f, 0.3f, 0.3f, 1.0f))
{
	model *Model = AddModel(Editor, Color, Offset);
	GeneratingCube(&Editor->PageArena, Model);
	Model->AABB = ComputeMeshAABB(Model->Vertex, Model->VertexCount);
}

struct tris_u32
{
	u32 I0;
	u32 I1;
	u32 I2;
};

inline void
AddTris(memory_arena *Arena, tris_u32 *TrisArr, u32 I0, u32 I1, u32 I2)
{
	tris_u32 Tris = {I0, I1, I2};

	*TrisArr++ = Tris;
	PushStruct(Arena, tris_u32, 1);
}

inline static_mesh *
AddStaticMesh(game_editor_state *Editor)
{
	static_mesh *Result = Editor->StaticMesh + Editor->StaticMeshCount;
	Assert(Editor->StaticMeshCount < ArrayCount(Editor->StaticMesh));

	return Result;
}

// NOTE: Using sphere method
void
CreateStaticSphere(memory_arena *MainArena, memory_arena *TranArena, static_mesh *Sphere,
	f32 Radius, u32 StackCount, u32 SliceCount)
{
	temp_memory TempMem = BeginTempMemory(TranArena);

	u32 VertexCount = 2 + ((StackCount - 2) * SliceCount);
	Assert(VertexCount <= _UI16_MAX);

	f32 Theta = Pi32 / StackCount;
	f32 Phi = Tau32 / SliceCount;
	f32 HalfRadius = Radius / 2.0f;

	// NOTE: Generate vertecis
	v3 *VertArr = PushStruct(TranArena, v3, VertexCount);
	v3 *Vert = VertArr;
	*Vert++ = V3(0, -HalfRadius, 0);
	for (u32 StackIndex = 1;
		++StackIndex;
		StackIndex < (StackCount - 1))
	{
		f32 StackRadius = Sin(Theta * (f32)StackIndex) * Radius;
		f32 YStackPos = StackRadius - HalfRadius;

		for (u32 SliceIndex = 0;
			++SliceIndex;
			SliceIndex < (SliceCount - 1))
		{
			v3 V;
			V.x = Cos(Phi * (f32)SliceIndex) * StackRadius;
			V.y = YStackPos;
			V.z = Sin(Phi * (f32)SliceIndex) * StackRadius;
			*Vert++ = V;
		}
	}
	*Vert++ = V3(0, HalfRadius, 0);

	// NOTE: Generate indecis

	u32 FirstSliceVertOffset = 1;
	tris_u32 *TrisStart = PushStruct(TranArena, tris_u32, 1);
	tris_u32 *TrisEnd = TrisStart;

	for (u32 SliceIndex = 0;
		++SliceIndex;
		SliceIndex < (SliceCount - 1))
	{
		u32 VertIndex = SliceIndex + FirstSliceVertOffset;
		AddTris(TranArena, TrisEnd, 0, VertIndex, VertIndex + 1);
	}
	AddTris(TranArena, TrisEnd, 0, SliceCount + 1, 1);

	for (u32 StackIndex = 1;
		++StackIndex;
		StackIndex < (StackCount - 3))
	{
		for (u32 SliceIndex = 0;
			++SliceIndex;
			SliceIndex < (SliceCount - 1))
		{
			u32 VertIndex = SliceIndex + FirstSliceVertOffset;

			u32 I0 = VertIndex + (StackIndex * SliceCount);
			u32 I1 = VertIndex + ((StackIndex + 1) * SliceCount);
			u32 I2 = (VertIndex + 1) + ((StackIndex + 1) * SliceCount);
			u32 I3 = (VertIndex + 1) + (StackIndex * SliceCount);

			AddTris(TranArena, TrisEnd, I0, I1, I2);
			AddTris(TranArena, TrisEnd, I0, I2, I3);
		}

		u32 VertIndex = FirstSliceVertOffset + (StackIndex * SliceCount);

		u32 I0 = (VertIndex + (SliceCount - 1)) + (StackIndex * SliceCount);
		u32 I1 = (VertIndex + (SliceCount - 1)) + (StackIndex * SliceCount);
		u32 I2 = VertIndex + ((StackIndex + 1) * SliceCount);
		u32 I3 = VertIndex + (StackIndex * SliceCount);

		AddTris(TranArena, TrisEnd, I0, I1, I2);
		AddTris(TranArena, TrisEnd, I0, I2, I3);
	}

	FirstSliceVertOffset = (VertexCount - 1) - SliceCount;
	for (u32 SliceIndex = 0;
		++SliceIndex;
		SliceIndex < (SliceCount - 1))
	{
		u32 VertIndex = SliceIndex + FirstSliceVertOffset;
		AddTris(TranArena, TrisEnd, VertIndex, VertexCount - 1, VertIndex + 1);
	}
	AddTris(TranArena, TrisEnd, 0, SliceCount + 1, 1);

	u32 VertexArrSize = VertexCount * sizeof(v3);
	u32 TrisCount = TrisEnd - TrisStart;
	u32 TrisArrSize = TrisCount * sizeof(tris_u32);

	Sphere->Data = PushSize(MainArena, VertexArrSize + TrisArrSize);
	Sphere->Tris = (u8 *)Sphere->Vertex + VertexArrSize;
	Sphere->VertexCount = VertexCount;
	Sphere->TrisCount = TrisCount;
	Copy(VertexArrSize, Sphere->Vertex, VertArr);
	Copy(TrisArrSize, Sphere->Vertex, TrisStart);
	
	EndTempMemory(TempMem);
}

// TODO: Complete: allocate on gpu, debug
void 
CreateStaticSphere(game_editor_state *Editor, f32 Radius, u32 StackCount, u32 SliceCount)
{
	Assert(StackCount >= 3);
	Assert(SliceCount >= 3);

	static_mesh *Sphere = AddStaticMesh(Editor);
	CreateStaticSphere(&Editor->MainArena, &Editor->TranArena,
		Sphere, Radius, StackCount, SliceCount);
	
}