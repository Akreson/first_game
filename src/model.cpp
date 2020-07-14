
// TODO: Return in face edge array index or absolute index?
inline face_edge_match
MatchFaceEdge(model_face *A, model_face *B)
{
	face_edge_match Result;

	__m128i EdgesA = _mm_load_si128((__m128i *)A->EdgesID);
	__m128i EdgesB = _mm_load_si128((__m128i *)B->EdgesID);

	__m128i EdgeB0 = ShuffleU32_4x(EdgesB, 0);
	__m128i EdgeB1 = ShuffleU32_4x(EdgesB, 1);
	__m128i EdgeB2 = ShuffleU32_4x(EdgesB, 2);
	__m128i EdgeB3 = ShuffleU32_4x(EdgesB, 3);

	__m128i CmpMask0 = _mm_cmpeq_epi32(EdgesA, EdgeB0);
	__m128i CmpMask1 = _mm_cmpeq_epi32(EdgesA, EdgeB1);
	__m128i CmpMask2 = _mm_cmpeq_epi32(EdgesA, EdgeB2);
	__m128i CmpMask3 = _mm_cmpeq_epi32(EdgesA, EdgeB3);

	__m128i OrMask01 = _mm_or_si128(CmpMask0, CmpMask1);
	__m128i OrMask23 = _mm_or_si128(CmpMask2, CmpMask3);
	__m128i OrMask = _mm_or_si128(OrMask01, OrMask23);

	u32 Mask32 = _mm_movemask_ps(_mm_castsi128_ps(OrMask));
	Assert(CountOfSetBits(Mask32) <= 1);

	bit_scan_result MaskResult = FindLeastSignificantSetBit(Mask32);

	Result.Succes = MaskResult.Succes;
	Result.Index = MaskResult.Index;

	return Result;
}

inline face_edge_match
MatchFaceEdge(model_face *A, u32 B)
{
	face_edge_match Result;

	__m128i EdgeA = _mm_load_si128((__m128i *)A->EdgesID);
	__m128i EdgeB0 = _mm_set1_epi32(B);

	__m128i CmpMask = _mm_cmpeq_epi32(EdgeA, EdgeB0);

	u32 Mask32 = _mm_movemask_ps(_mm_castsi128_ps(CmpMask));
	Assert(CountOfSetBits(Mask32) <= 1);

	bit_scan_result MaskResult = FindLeastSignificantSetBit(Mask32);

	Result.Succes = MaskResult.Succes;
	Result.Index = MaskResult.Index;

	return Result;
}

inline face_vertex_match
GetFaceVertexMatchResult(u32 Mask32)
{
	face_vertex_match Result;

	u32 SetBits = CountOfSetBits(Mask32);
	Assert(SetBits <= 2);

	Result.Succes = SetBits == 2 ? true : false;
	Result.Index[0] = FindLeastSignificantSetBit(Mask32).Index;
	Result.Index[1] = FindMostSignificantSetBit(Mask32).Index;

	return Result;
}

inline face_vertex_match
MatchFaceVertex(model_face *A, model_face *B)
{
	face_vertex_match Result;

	__m128i VerticesA = _mm_load_si128((__m128i *)A->VertexID);
	__m128i VerticesB = _mm_load_si128((__m128i *)B->VertexID);

	__m128i VertexB0 = ShuffleU32_4x(VerticesB, 0);
	__m128i VertexB1 = ShuffleU32_4x(VerticesB, 1);
	__m128i VertexB2 = ShuffleU32_4x(VerticesB, 2);
	__m128i VertexB3 = ShuffleU32_4x(VerticesB, 3);

	__m128i CmpMask0 = _mm_cmpeq_epi32(VerticesA, VertexB0);
	__m128i CmpMask1 = _mm_cmpeq_epi32(VerticesA, VertexB1);
	__m128i CmpMask2 = _mm_cmpeq_epi32(VerticesA, VertexB2);
	__m128i CmpMask3 = _mm_cmpeq_epi32(VerticesA, VertexB3);

	__m128i OrMask0 = _mm_or_si128(CmpMask0, CmpMask1);
	__m128i OrMask1 = _mm_or_si128(CmpMask2, CmpMask3);
	__m128i OrMask = _mm_or_si128(OrMask0, OrMask1);

	u32 Mask32 = _mm_movemask_ps(_mm_castsi128_ps(OrMask));
	Result = GetFaceVertexMatchResult(Mask32);

	return Result;
}

// TODO: Set _Mask..._ function for other function?
inline u32
MaskOfMatchFaceVertex(model_face *A, model_edge *B)
{
	// TODO: See if in optimize build with same _A_ value
	// compiler will propagate _VertexA_ to multimple sequential call
	__m128i VertexA = _mm_load_si128((__m128i *)A->VertexID);

	__m128i EdgeB = _mm_load_si128((__m128i *)B);
	__m128i VertexB0 = ShuffleU32_4x(EdgeB, 0);
	__m128i VertexB1 = ShuffleU32_4x(EdgeB, 1);

	__m128i CmpMask0 = _mm_cmpeq_epi32(VertexA, VertexB0);
	__m128i CmpMask1 = _mm_cmpeq_epi32(VertexA, VertexB1);

	__m128i OrMask = _mm_or_si128(CmpMask0, CmpMask1);

	u32 ResultMask = _mm_movemask_ps(_mm_castsi128_ps(OrMask));
	return ResultMask;
}

inline face_vertex_match
MatchFaceVertex(model_face *A, model_edge *B)
{
	u32 Mask32 = MaskOfMatchFaceVertex(A, B);
	face_vertex_match Result = GetFaceVertexMatchResult(Mask32);

	return Result;
}

inline edge_vertex_match
MatchEdgeVertex(model_edge *A, model_edge *B)
{
	edge_vertex_match Result;

	__m128i EdgeA = _mm_load_si128((__m128i *)A);
	__m128i EdgeB = _mm_load_si128((__m128i *)B);

	EdgeA = ShuffleU32(EdgeB, 0, 1, 0, 1);
	EdgeB = ShuffleU32(EdgeB, 0, 1, 1, 0);

	__m128i CmpMask = _mm_cmpeq_epi32(EdgeA, EdgeB);
	u32 Mask32 = _mm_movemask_ps(_mm_castsi128_ps(CmpMask));

	bit_scan_result MaskResult = FindLeastSignificantSetBit(Mask32);

	Result.Succes = MaskResult.Succes;
	Result.Index = MaskResult.Index > 1 ? MaskResult.Index - 2 : MaskResult.Index;

	return Result;
}

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
			FaceIndex < FaceCount || (FaceMatchedIndex < ArrayCount(Edge->FaceID));
			++FaceIndex)
		{
			model_face *Face = Faces + FaceIndex;
			face_vertex_match MatchResult = MatchFaceVertex(Face, Edge);

			if (MatchResult.Succes)
			{
				Edge->FaceID[FaceMatchedIndex] = FaceIndex;
				FaceMatchedIndex++;
			}
		}
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
			(EdgeIndex < EdgeCount) || (EdgeMatchedIndex < ArrayCount(Face->EdgesID));
			++EdgeIndex)
		{
			model_edge *Edge = Edges + EdgeIndex;
			face_vertex_match MatchResult = MatchFaceVertex(Face, Edge);

			if (MatchResult.Succes)
			{
				Face->EdgesID[EdgeMatchedIndex] = EdgeIndex;
				EdgeMatchedIndex++;
			}
		}
	}
}

// TODO: Optimize
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
	Model->XAxis = V3(1, 0, 0);
	Model->YAxis = V3(0, 1, 0);
	Model->ZAxis = V3(0, 0, 1);

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
	f32 Diameter, u32 StackCount, u32 SliceCount)
{
	Assert(StackCount >= 3);
	Assert(SliceCount >= 3);

	temp_memory TempMem = BeginTempMemory(TranArena);

	u32 VertexCount = 2 + ((StackCount - 2) * SliceCount);
	u32 VertexArrSize = VertexCount * sizeof(v3);

	f32 Theta = Pi32 / (StackCount - 1);
	f32 Phi = Tau32 / SliceCount;
	f32 Radius = Diameter / 2.0f;

	// NOTE: Generate vertecis

	v3 *VertArr = (v3 *)PushSize(TranArena, VertexArrSize);
	v3 *Vert = VertArr;
	*Vert++ = V3(0, -Radius, 0);
	for (u32 StackIndex = 1;
		StackIndex < (StackCount - 1);
		++StackIndex)
	{
		f32 StackRadius = Sin(Theta * (f32)StackIndex) * Radius;
		f32 YStackPos = -(Cos(Theta * StackIndex) * Radius);

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
	*Vert++ = V3(0, Radius, 0);

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

inline v3
GetPlaneNormal(model *Model, u32 FaceIndex)
{
	v3 Result;

	model_face Face = Model->Faces[FaceIndex];

	v3 Offset = Model->Offset;
	v3 V0 = Model->Vertex[Face.V0] + Offset;
	v3 V1 = Model->Vertex[Face.V1] + Offset;
	v3 V2 = Model->Vertex[Face.V2] + Offset;
	v3 V3 = Model->Vertex[Face.V3] + Offset;

	v3 Edge1 = V0 - V1;
	v3 Edge2 = V0 - V3;

	Result = Normalize(Cross(Edge1, Edge2));
	return Result;
}

// TODO: Optimize or change to another method
// Check if precompute and then check face visibility
// give benefit
// TODO: Fix hit test for deform face
b32
RayModelEdgeInterset(model *Model, ray_params Ray, element_ray_result *EdgeResult, f32 IntrRadius)
{
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

			Capsule.R = IntrRadius;
			Capsule.V0 = Model->Vertex[Edge.V0] + Model->Offset;
			Capsule.V1 = Model->Vertex[Edge.V1] + Model->Offset;
			Capsule.Dir = Capsule.V1 - Capsule.V0;

			f32 CapsuleRSquare = Square(Capsule.R);
			v3 NormCapDir = Normalize(Capsule.Dir);

			// NOTE: Colosest points between rays
			v3 R = Capsule.V0 - Ray.P;
			f32 CDotC = Dot(NormCapDir, NormCapDir);
			f32 CDotL = Dot(NormCapDir, Ray.Dir);
			f32 CDotR = Dot(NormCapDir, R);
			f32 LDotL = Dot(Ray.Dir, Ray.Dir);
			f32 LDotR = Dot(Ray.Dir, R);

			f32 Det = (CDotC * LDotL) - (CDotL * CDotL);

			f32 t0 = (CDotL * LDotR - CDotR * LDotL) / Det;
			f32 t1 = (CDotC * LDotR - CDotL * CDotR) / Det;
		
			v3 PointOnEdge = Capsule.V0 + (NormCapDir * t0);
			v3 PointOnRay = Ray.P + (Ray.Dir * t1);

			f32 CapRSquare = Capsule.R * Capsule.R;
			f32 Dist = LengthSq(PointOnEdge - PointOnRay);

			if ((t0 >= 0) && (t0 <= 1.0f))
			{
				if ((Dist <= CapsuleRSquare) && (Dist < ClosestRayP))
				{
					ClosestRayP = Dist;
					ClosestIndex = EdgeIndex;
					ResultPointOnEdge = PointOnEdge;
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
RayModelFaceIntersect(model *Model, ray_params Ray, element_ray_result *FaceResult, memory_arena *Arena)
{
	b32 Result = false;
	temp_memory TempMem = BeginTempMemory(Arena);
	element_ray_result *HitResult = 0;
	u32 HitResultCount = 0;

	v3 ModelOffset = Model->Offset;
	for (u32 FaceIndex = 0;
		FaceIndex < Model->FaceCount;
		++FaceIndex)
	{
		b32 HitTest = false;
		v3 IntersetPoint;
		plane_params Plane0;
		plane_params Plane1;

		model_face Face = Model->Faces[FaceIndex];

		v3 V0 = Model->Vertex[Face.V0] + ModelOffset;
		v3 V1 = Model->Vertex[Face.V1] + ModelOffset;
		v3 V2 = Model->Vertex[Face.V2] + ModelOffset;
		v3 V3 = Model->Vertex[Face.V3] + ModelOffset;

		v3 Edge01 = V2 - V1;
		v3 Edge02 = V0 - V1;
		Plane0.N = Normalize(Cross(Edge01, Edge02));
		Plane0.D = Dot(Plane0.N, V0);

		v3 Edge11 = V2 - V3;
		v3 Edge12 = V0 - V3;
		Plane1.N = Normalize(Cross(Edge12, Edge11));
		Plane1.D = Dot(Plane1.N, V0);

		f32 DotRayPlane0 = Dot(Ray.Dir, Plane0.N);
		f32 DotRayPlane1 = Dot(Ray.Dir, Plane1.N);

		if ((DotRayPlane0 < 0))
		{
			f32 tRay = RayPlaneIntersect(Ray, Plane0, DotRayPlane0);
			if (tRay >= 0)
			{
				IntersetPoint = PointOnRay(Ray, tRay);
				HitTest = IsPointInTriangle(V0, V1, V2, IntersetPoint);
			}
		}

		if (!HitTest && (DotRayPlane1 < 0))
		{
			f32 tRay = RayPlaneIntersect(Ray, Plane1, DotRayPlane1);
			if (tRay >= 0)
			{
				IntersetPoint = PointOnRay(Ray, tRay);
				HitTest = IsPointInTriangle(V0, V2, V3, IntersetPoint);
			}
		}

		if (HitTest)
		{
			HitResult = PushStruct(Arena, element_ray_result, 1);
			HitResult->ID = FaceIndex;
			HitResult->P = IntersetPoint;
			++HitResultCount;
		}
	}

	if (HitResult)
	{
		element_ray_result *StartHitResults = HitResult - (HitResultCount - 1);
		element_ray_result *ClosestElementHit = 0;
		f32 ClosestDistSq = FLOAT_MAX;
		for (u32 Index = 0;
			Index < HitResultCount;
			++Index)
		{
			element_ray_result *Hit = StartHitResults + Index;
			
			f32 HitDistSq = LengthSq(Hit->P - Ray.P);
			if (HitDistSq < ClosestDistSq)
			{
				ClosestElementHit = Hit;
			}
		}

		FaceResult->ID = ClosestElementHit->ID;
		FaceResult->P = ClosestElementHit->P;
		Result = true;
	}

	EndTempMemory(TempMem);

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
			SortEntry->Length = LengthSq(CenterOfAABB - Ray.P);

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

			if (RayModelFaceIntersect(Model, Ray, Face, Arena))
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