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

inline void
AddTris(memory_arena *Arena, tris_u32 **TrisArrPtr, u32 I0, u32 I1, u32 I2)
{
	tris_u32 Tris = {I0, I1, I2};
	*(*TrisArrPtr)++ = Tris;

	PushStruct(Arena, tris_u32);
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
	Assert(StackCount >= 3);
	Assert(SliceCount >= 3);

	temp_memory TempMem = BeginTempMemory(TranArena);

	u32 VertexCount = 2 + ((StackCount - 2) * SliceCount);
	u32 VertexArrSize = VertexCount * sizeof(v3);

	f32 Theta = Pi32 / (StackCount - 1);
	f32 Phi = Tau32 / SliceCount;
	f32 HalfRadius = Radius / 2.0f;

	// NOTE: Generate vertecis

	v3 *VertArr = (v3 *)PushSize(TranArena, VertexArrSize);
	v3 *Vert = VertArr;
	*Vert++ = V3(0, -HalfRadius, 0);
	for (u32 StackIndex = 1;
		StackIndex < (StackCount - 1);
		++StackIndex)
	{
		f32 StackRadius = Sin(Theta * (f32)StackIndex) * HalfRadius;
		f32 YStackPos = -(Cos(Theta * StackIndex) * HalfRadius);

		for (u32 SliceIndex = 0;
			SliceIndex < SliceCount;
			++SliceIndex)
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
	tris_u32 *TrisStart = PushStruct(TranArena, tris_u32);
	tris_u32 *TrisEnd = TrisStart;

	for (u32 SliceIndex = 0;
		SliceIndex < (SliceCount - 1);
		++SliceIndex)
	{
		u32 VertIndex = SliceIndex + FirstSliceVertOffset;
		AddTris(TranArena, &TrisEnd, 0, VertIndex, VertIndex + 1);
	}
	AddTris(TranArena, &TrisEnd, 0, SliceCount, 1);

	for (u32 StackIndex = 0;
		StackIndex < (StackCount - 3);
		++StackIndex)
	{
		for (u32 SliceIndex = 0;
			SliceIndex < (SliceCount - 1);
			++SliceIndex)
		{
			u32 VertIndex = SliceIndex + FirstSliceVertOffset;

			u32 I0 = VertIndex + (StackIndex * SliceCount);
			u32 I1 = VertIndex + ((StackIndex + 1) * SliceCount);
			u32 I2 = (VertIndex + 1) + ((StackIndex + 1) * SliceCount);
			u32 I3 = (VertIndex + 1) + (StackIndex * SliceCount);

			AddTris(TranArena, &TrisEnd, I0, I1, I2);
			AddTris(TranArena, &TrisEnd, I0, I2, I3);
		}

		u32 VertIndex = FirstSliceVertOffset + (StackIndex * SliceCount);

		u32 I0 = VertIndex;
		u32 I1 = VertIndex + SliceCount;
		u32 I2 = (VertIndex + (SliceCount - 1)) + SliceCount;
		u32 I3 = VertIndex + (SliceCount - 1);

		AddTris(TranArena, &TrisEnd, I0, I2, I1);
		AddTris(TranArena, &TrisEnd, I0, I3, I2);
	}

	FirstSliceVertOffset = (VertexCount - 1) - SliceCount;
	for (u32 SliceIndex = 0;
		SliceIndex < (SliceCount - 1);
		++SliceIndex)
	{
		u32 VertIndex = SliceIndex + FirstSliceVertOffset;
		AddTris(TranArena, &TrisEnd, VertexCount - 1, VertIndex + 1, VertIndex);
	}
	AddTris(TranArena, &TrisEnd, VertexCount - 1,
		FirstSliceVertOffset, FirstSliceVertOffset + (SliceCount - 1));

	u32 TrisCount = TrisEnd - TrisStart;
	u32 TrisArrSize = TrisCount * sizeof(tris_u32);

	Sphere->Data = PushSize(MainArena, VertexArrSize + TrisArrSize);
	Sphere->Tris = (u8 *)Sphere->Vertex + VertexArrSize;
	Sphere->VertexCount = VertexCount;
	Sphere->TrisCount = TrisCount;
	Copy(VertexArrSize, Sphere->Vertex, VertArr);
	Copy(TrisArrSize, Sphere->Tris, TrisStart);
	
	EndTempMemory(TempMem);
}

static_mesh *
CreateStaticSphere(game_editor_state *Editor, f32 Radius, u32 StackCount, u32 SliceCount)
{
	static_mesh *Sphere = AddStaticMesh(Editor);

	CreateStaticSphere(
		&Editor->MainArena, &Editor->TranArena,
		Sphere, Radius, StackCount, SliceCount);

	Sphere->Mesh = PlatformAPI.AllocateMesh(
		SetAllocMeshParams(Sphere->Vertex, (u32 *)Sphere->Tris, Sphere->VertexCount, Sphere->TrisCount));

	return Sphere;
}

internal plane_params
GetPlaneFromFace(model *Model, u32 FaceIndex)
{
	plane_params Result;

	model_face Face = Model->Faces[FaceIndex];

	v3 V0 = Model->Vertex[Face.V0] + Model->Offset;
	v3 V1 = Model->Vertex[Face.V1] + Model->Offset;
	v3 V2 = Model->Vertex[Face.V2] + Model->Offset;
	v3 V3 = Model->Vertex[Face.V3] + Model->Offset;

	v3 Edge1 = V0 - V1;
	v3 Edge2 = V0 - V3;

	Result.N = Normalize(Cross(Edge1, Edge2));
	Result.D = Dot(Result.N, V0);

	return Result;
}

struct capsule_params
{
	v3 V0, V1;
	v3 Dir;
	f32 R;
};

v3
GetPlaneNormal(model *Model, u32 FaceIndex)
{
	v3 Result;

	model_face Face = Model->Faces[FaceIndex];

	v3 V0 = Model->Vertex[Face.V0] + Model->Offset;
	v3 V1 = Model->Vertex[Face.V1] + Model->Offset;
	v3 V2 = Model->Vertex[Face.V2] + Model->Offset;
	v3 V3 = Model->Vertex[Face.V3] + Model->Offset;

	v3 Edge1 = V0 - V1;
	v3 Edge2 = V0 - V3;

	Result = Normalize(Cross(Edge1, Edge2));
	return Result;
}

b32
RayModelEdgeInterset(model *Model, ray_params Ray, element_ray_result *EdgeResult, f32 IntrRadius)
{
#if 0
	model_face Face = Model->Faces[FaceResult->ID];
	v3 IntersectP = FaceResult->P;
	v3 _IntersectP = FaceResult->P - Model->Offset;
	
	// NOTE: Closest vertex match
	u32 VertexRelativeIndex[2];
	f32 LengthsToVertex[4];

	LengthsToVertex[0] = LengthSq((Model->Vertex[Face.V0] + Model->Offset) - IntersectP);
	LengthsToVertex[1] = LengthSq((Model->Vertex[Face.V1] + Model->Offset) - IntersectP);
	LengthsToVertex[2] = LengthSq((Model->Vertex[Face.V2] + Model->Offset) - IntersectP);
	LengthsToVertex[3] = LengthSq((Model->Vertex[Face.V3] + Model->Offset) - IntersectP);

	// NOTE: Get 2 smallest length
	for (u32 MinLengthIndex = 0;
		MinLengthIndex < ArrayCount(VertexRelativeIndex);
		++MinLengthIndex)
	{
		f32 MinLength = FLOAT_MAX;
		u32 CurrentMinLengthIndex;

		for (u32 LengthIndex = 0;
			LengthIndex < ArrayCount(LengthsToVertex);
			++LengthIndex)
		{
			f32 CheckLength = LengthsToVertex[LengthIndex];
			if (CheckLength < MinLength)
			{
				CurrentMinLengthIndex = LengthIndex;
				MinLength = CheckLength;
			}
		}

		VertexRelativeIndex[MinLengthIndex] = CurrentMinLengthIndex;
		LengthsToVertex[CurrentMinLengthIndex] = FLOAT_MAX;
	}

	//NOTE: Edge match
	u32 AbsIndexV0 = Face.VertexID[VertexRelativeIndex[0]];
	u32 AbsIndexV1 = Face.VertexID[VertexRelativeIndex[1]];

	model_edge MatchEdge;
	b32 SuccesMatch = false;
	for (u32 EdgeIndex = 0;
		EdgeIndex < ArrayCount(Face.EdgeID);
		++EdgeIndex)
	{
		u32 MatchCount = 0;
		u32 EdgeAbsIndex = Face.EdgeID[EdgeIndex];
		model_edge Edge = Model->Edges[EdgeAbsIndex];

		b32 MatchV0 = ((Edge.V0 == AbsIndexV0) || (Edge.V0 == AbsIndexV1));
		b32 MatchV1 = ((Edge.V1 == AbsIndexV0) || (Edge.V1 == AbsIndexV1));

		if (MatchV0 && MatchV1)
		{
			MatchEdge = Edge;
			SuccesMatch = true;
			EdgeResult->ID = EdgeAbsIndex;
			break;
		}
	}
	Assert(SuccesMatch);

	v3 EdgeV0 = Model->Vertex[MatchEdge.V0] + Model->Offset;
	v3 EdgeV1 = Model->Vertex[MatchEdge.V1] + Model->Offset;

	v3 NormalizeEdgeDir = Normalize(EdgeV1 - EdgeV0);
	v3 DistVector = IntersectP - EdgeV0;

	f32 LengthOnEdge = Dot(NormalizeEdgeDir, DistVector);
	LengthOnEdge = LengthOnEdge < 0 ? LengthOnEdge * -1.0f : LengthOnEdge;

	v3 PointOnEdge = EdgeV0 + (NormalizeEdgeDir * LengthOnEdge);
	v3 Diff = PointOnEdge - IntersectP;
	f32 DistanceToEdge = MAX(MAX(Diff.x, Diff.y), Diff.z);

	EdgeResult->P = PointOnEdge;

	return DistanceToEdge < 0.03f ? true : false;
#else
	b32 Result = false;
	v3 ResultPointOnEdge;
	u32 ClosestIndex = 0;
	f32 ClosestRayP = FLOAT_MAX;

	// NOTE: 0 - edge ray, 1 - mouse ray
	for (u32 EdgeIndex = 0;
		EdgeIndex < Model->EdgeCount;
		++EdgeIndex)
	{
		model_edge Edge = Model->Edges[EdgeIndex];
		v3 PlaneN0 = GetPlaneNormal(Model, Edge.Face0);
		v3 PlaneN1 = GetPlaneNormal(Model, Edge.Face1);

		if ((Dot(PlaneN0, Ray.Dir) < 0) || (Dot(PlaneN1, Ray.Dir) < 0))
		{
			capsule_params Capsule;
			// TODO: Adjust radius based on distance
			Capsule.R = IntrRadius;
			Capsule.V0 = Model->Vertex[Edge.V0] + Model->Offset;
			Capsule.V1 = Model->Vertex[Edge.V1] + Model->Offset;
			Capsule.Dir = Capsule.V0 - Capsule.V1;
			v3 NormCapDir = Normalize(Capsule.Dir);

			v3 R = Capsule.V0 - Ray.Pos;
			f32 CDotC = Dot(NormCapDir, NormCapDir);
			f32 CDotL = Dot(NormCapDir, Ray.Dir);
			f32 CDotR = Dot(NormCapDir, R);
			f32 LDotL = Dot(Ray.Dir, Ray.Dir);
			f32 LDotR = Dot(Ray.Dir, R);

			f32 Det = (CDotC * LDotL) - (CDotL * CDotL);

			f32 t0 = (CDotL * LDotR - CDotR * LDotL) / Det;
			f32 t1 = (CDotC * LDotR - CDotL * CDotR) / Det;
		
			v3 P0 = Capsule.V0 + (NormCapDir * t0);
			v3 P1 = Ray.Pos + (Ray.Dir * t1);

			f32 CapRSquare = Capsule.R * Capsule.R;
			f32 Dist = Length(P0 - P1);

			// TODO: Add check for length bounds overflow
			if (Dist <= Capsule.R)
			{
				if (Dist < ClosestRayP)
				{
					ClosestRayP = Dist;
					ClosestIndex = EdgeIndex;
					ResultPointOnEdge = P1;
					Result = true;
				}
			}
		}
	}

	if (Result)
	{
		EdgeResult->ID = ClosestIndex;
		EdgeResult->P = ResultPointOnEdge;
	}

	return Result;
}

b32
RayModelFaceIntersect(model *Model, ray_params Ray, element_ray_result *FaceResult)
{
	b32 Result = false;

	for (u32 FaceIndex = 0;
		FaceIndex < Model->FaceCount;
		++FaceIndex)
	{
		model_face Face = Model->Faces[FaceIndex];

		v3 V0 = Model->Vertex[Face.V0] + Model->Offset;
		v3 V1 = Model->Vertex[Face.V1] + Model->Offset;
		v3 V2 = Model->Vertex[Face.V2] + Model->Offset;
		v3 V3 = Model->Vertex[Face.V3] + Model->Offset;

		v3 Edge1 = V0 - V1;
		v3 Edge2 = V0 - V3;

		plane_params Plane;
		Plane.N = Normalize(Cross(Edge1, Edge2));
		Plane.D = Dot(Plane.N, V0);

		// Plane intersect
		f32 DotRayPlane = Dot(Ray.Dir, Plane.N);
		if (DotRayPlane < 0)
		{
			f32 tPlaneIntersect = ((Plane.D - Dot(Plane.N, Ray.Pos)) / DotRayPlane);

			if ((tPlaneIntersect != 0) && (tPlaneIntersect > 0))
			{
				v3 IntersetPoint = Ray.Pos + (Ray.Dir * tPlaneIntersect);

				b32 HitTest = IsPointInTriangle(V0, V1, V2, IntersetPoint);
				if (!HitTest)
				{
					HitTest = IsPointInTriangle(V0, V2, V3, IntersetPoint);
				}

				if (HitTest)
				{
					FaceResult->ID = FaceIndex;
					FaceResult->P = IntersetPoint;

					Result = true;
					break;
				}
			}
		}
	}

	return Result;
}

b32
RayModelsIntersect(memory_arena *Arena, model *Models, u32 ModelCount, ray_params Ray, u32 *ModelID,
	element_ray_result *Face)
{
	b32 Result = false;
	temp_memory TempMem = BeginTempMemory(Arena);

	u32 ModelsHitCount = 0;
	u32 ModelsSortArraySize = 20;
	model_ray_sort *ModelsSortArray = (model_ray_sort *)PushArray(Arena, model_ray_sort, ModelsSortArraySize);

	// NOTE: Gather all intersect models
	for (u32 ModelIndex = 0;
		ModelIndex < ModelCount;
		++ModelIndex)
	{
		model *Model = Models + ModelIndex;

		b32 HitTest = RayAABBIntersect(Ray, Model->AABB, Model->Offset);

		if (HitTest)
		{
			model_ray_sort *SortEntry = ModelsSortArray + ModelsHitCount++;

			v3 CenterOfAABB = ((Model->AABB.Min + Model->Offset) + (Model->AABB.Max + Model->Offset)) / 2.0f;

			SortEntry->Index = ModelIndex;
			SortEntry->Length = LengthSq(CenterOfAABB - Ray.Pos);

			if (ModelsHitCount >= ModelsSortArraySize)
			{
				PushArray(Arena, u32, ModelsSortArraySize);
				ModelsSortArraySize += ModelsSortArraySize;
			}
		}
	}

	// NOTE: Sort by length from ray position to center of AABB
	for (u32 Outer = 0;
		Outer < ModelsHitCount;
		++Outer)
	{
		for (u32 Inner = 0;
			Inner < (ModelsHitCount - 1);
			++Inner)
		{
			model_ray_sort *A = ModelsSortArray + Inner;
			model_ray_sort *B = ModelsSortArray + Inner + 1;

			if (A->Length > B->Length)
			{
				model_ray_sort Temp = *B;
				*B = *A;
				*A = Temp;
			}
		}
	}

	// NOTE: Find intersect model face for non convex case
	if (ModelsHitCount)
	{
		for (u32 SortIndex = 0;
			(SortIndex < ModelsHitCount);
			++SortIndex)
		{
			u32 ModelIndex = ModelsSortArray[SortIndex].Index;
			model *Model = Models + ModelIndex;

			if (RayModelFaceIntersect(Model, Ray, Face))
			{
				Result = true;
				*ModelID = ModelIndex;
				break;
			}
		}
	}

	EndTempMemory(TempMem);

	return Result;
}