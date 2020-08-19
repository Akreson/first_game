
inline ui_id
IDFromModel(u16 Target, u32 ModelIndex, u32 FaceIndex = 0, u32 EdgeIndex = 0)
{
	ui_id Result;
	Result.ID[0] = ((u64)Target << 32) | ModelIndex;
	Result.ID[1] = ((u64)FaceIndex << 32) | EdgeIndex;

	return Result;
}

inline b32
AreEqual(ui_id A, ui_id B)
{
	return ((A.ID[0] == B.ID[0]) && (A.ID[1] == B.ID[1]));
}

inline u32
SetIntrTypeID(u16 Target, u16 Type)
{
	u32 Result = (Target << 16) | Type;
	return Result;
}

inline b32
IsHotIntrType(editor_world_ui *WorldUI, u32 Type)
{
	b32 Result = (WorldUI->HotInteraction.Type == Type);
	return Result;
}

inline b32
IsHotIntrTypeID(editor_world_ui *WorldUI, u32 TypeID)
{
	b32 Result = (WorldUI->HotInteraction.TypeID == TypeID);
	return Result;
}

inline b32
IsActiveModel(editor_world_ui *UI, u32 ModelID)
{
	b32 Result = UI->IModel.Target && (UI->IModel.ID == ModelID);

	return Result;
}

inline ui_interaction
SetModelSelectInteraction(interact_model *IModel, u16 Target)
{
	ui_interaction Result;

	Result.TypeID = SetIntrTypeID(UI_InteractionTarget_Model, UI_InteractionType_Select);
	Result.ID = IDFromModel(Target, IModel->ID, IModel->Face.ID, IModel->Edge.ID);

	return Result;
}

// TODO: Make general
inline ui_interaction
SetModelSelectInteraction(u32 ModelID, u32 FaceID = 0, u32 EdgeID = 0, u16 Target = 0)
{
	ui_interaction Result;

	Result.TypeID = SetIntrTypeID(UI_InteractionTarget_Model, UI_InteractionType_Select);
	Result.ID = IDFromModel(Target, ModelID, FaceID, EdgeID);

	return Result;
}

inline b32
AreEqual(ui_interaction A, ui_interaction B)
{
	b32 Result = AreEqual(A.ID, B.ID) && (A.TypeID == B.TypeID);
	return Result;
}

inline void
ProcessWorldUIInput(editor_world_ui *WorldUI, game_input *Input)
{
	// TODO: Return later
	/*if (IsDown(Input->Alt))
	{
		WorldUI->UpdateModelInteraction = false;
	}
	else if (WasDown(Input->Alt))
	{
		WorldUI->UpdateModelInteraction = true;
	}*/

	if (WasDown(Input->MouseButtons[PlatformMouseButton_Extended0]))
	{
		if (IsITargetEq(WorldUI->ITarget, Model) && WorldUI->IModel.Target)
		{
			if (WorldUI->UpdateModelInteraction && !WorldUI->Selected.Count)
			{
				++WorldUI->IModel.Target;
				ZeroStruct(WorldUI->IModel.Face);
				ZeroStruct(WorldUI->IModel.Edge);

				if (WorldUI->IModel.Target == ModelTargetElement_Count)
				{
					WorldUI->IModel.Target = ModelTargetElement_Model;
				}
			}
		}
		else if (IsITargetEq(WorldUI->ITarget, Tools))
		{
			++WorldUI->Tools.Type;
			if (WorldUI->Tools.Type == ToolType_Count)
			{
				WorldUI->Tools.Type = 0;
			}
			WorldUI->Tools.IsInit = false;
		}
	}

	if (IsKepDown(Input->Alt) && WasUp(Input->Shift))
	{
		if (IsITargetEq(WorldUI->ITarget, Tools))
		{
			WorldUI->ITarget = UI_InteractionTarget_Model;
			WorldUI->Tools.IsInit = false;
			WorldUI->UpdateModelInteraction = true;
		}
		else if (WorldUI->Selected.Count)
		{
			WorldUI->Selected.Count = 0;
		}
		else
		{
			WorldUI->IModel.Target = ModelTargetElement_None;
			ZeroStruct(WorldUI->Interaction);
		}
	}
}

void
AddToSelectedBuffer(element_id_buffer *Selected,
	model *Model, u32 ElementID, u32 ITarget, b32 ShiftDown)
{
	if (Model && ShiftDown && Selected->Count)
	{
		switch (ITarget)
		{
			case ModelTargetElement_Edge:
			case ModelTargetElement_Face:
			{
				b32 AddToBuff = true;
				u32 ElementCount = Selected->Count;

				for (u32 Index = 0;
					Index < ElementCount;
					++Index)
				{
					u32 BufferElementID = Selected->Elements[Index];
					if (BufferElementID == ElementID)
					{
						Selected->Elements[Index] = Selected->Elements[--Selected->Count];
						AddToBuff = false;
						break;
					}
				}

				if (AddToBuff)
				{
					if (Selected->Count < Selected->MaxCount)
						Selected->Elements[Selected->Count++] = ElementID;
					else
						Assert(0);
				}
			} break;

			InvalidDefaultCase;
		}
	}
	else
	{
		if (!Selected->Count || (Selected->Count > 1))
		{
			Selected->Count = 1;
			Assert(Selected->Count < Selected->MaxCount);
			*Selected->Elements = ElementID;
		}
		else
		{
			if (Selected->Elements[0] == ElementID)
			{
				Selected->Count = 0;
			}

			*Selected->Elements = ElementID;
		}
	}
}

// TODO: Make selection as tool?
internal void inline
UpdateModelInteractionElement(game_editor_state *Editor, game_input *Input, render_group *RenderGroup)
{
	ui_interaction Interaction = {};

	editor_world_ui *WorldUI = &Editor->WorldUI;
	interact_model *IModel = &WorldUI->IModel;

	switch (IModel->Target)
	{
		case ModelTargetElement_None:
		{
			*IModel = {};
			if (RayModelsIntersect(&Editor->MainArena, Editor->Models, Editor->ModelsCount,
				WorldUI->MouseRay, &IModel->ID, &IModel->Face))
			{
				Interaction = SetModelSelectInteraction(IModel->ID, 0, 0, IModel->Target);

				if (AreEqual(Interaction, WorldUI->ToExecute))
				{
					if (IsDown(Input->Ctrl))
					{
						model *Model = Editor->Models + WorldUI->IModel.ID;
						Editor->Camera.Pos = Model->Offset;
					}
					else
					{
						IModel->Target = ModelTargetElement_Model;
						/*AddToSelectedBuffer(&WorldUI->Selected, 0,
							IModel->ID, IModel->Target, IsDown(Input->Shift));*/
					}
				}
			}
		} break;

		case ModelTargetElement_Model:
		{

		} break;

		case ModelTargetElement_Face:
		{
			model *Model = Editor->Models + WorldUI->IModel.ID;
			IModel->Face = {};

			if (RayAABBIntersect(WorldUI->MouseRay, Model->AABB, Model->Offset))
			{
				if (RayModelFacesIntersect(Model, WorldUI->MouseRay, &IModel->Face))
				{
					Interaction = SetModelSelectInteraction(IModel, IModel->Target);

					if (AreEqual(Interaction, WorldUI->ToExecute))
					{
						AddToSelectedBuffer(&WorldUI->Selected,	Model,
							IModel->Face.ID, IModel->Target, IsDown(Input->Shift));
					}
				}
			}
		} break;

		case ModelTargetElement_Edge:
		{
			model *Model = Editor->Models + WorldUI->IModel.ID;
			IModel->Edge = {};

			// TODO: Adjust dynamically?
			f32 EdgeIntersetRadius = 0.015f;
			if (RayAABBIntersect(WorldUI->MouseRay, AddRadiusTo(Model->AABB, EdgeIntersetRadius), Model->Offset))
			{
				if (RayModelEdgesIntersect(Model, WorldUI->MouseRay, &IModel->Edge, EdgeIntersetRadius))
				{
					Interaction = SetModelSelectInteraction(IModel, IModel->Target);

					if (AreEqual(Interaction, WorldUI->ToExecute))
					{
						AddToSelectedBuffer(&WorldUI->Selected, Model,
							IModel->Edge.ID, IModel->Target, IsDown(Input->Shift));
					}
				}

			}
		} break;
	}

	WorldUI->NextHotInteraction = Interaction;
}

// TODO: Delete or change implementation
v3
ComputeToolPos(model *Model, element_id_buffer *UniqIndeces,
	element_id_buffer *Selected, u32 ElementTarget)
{
	v3 Result = {};
	UniqIndeces->Count = 0;

	v3 ModelOffset = Model->Offset;
	switch (ElementTarget)
	{
		case ModelTargetElement_Model:
		{
			Result = ((Model->AABB.Min + ModelOffset) + (Model->AABB.Max + ModelOffset)) / 2.0f;
		} break;

		// TODO: Optimize for face and edge
		case ModelTargetElement_Face:
		{
			for (u32 Index = 0;
				Index < Selected->Count;
				++Index)
			{
				u32 FaceIndex = Selected->Elements[Index];
				model_face *Face = Model->Faces + FaceIndex;

				for (u32 InFaceIndex = 0;
					InFaceIndex < ArrayCount(Face->VertexID);
					++InFaceIndex)
				{
					b32 IsAddToUniqBuff = true;
					u32 VertexIndex = Face->VertexID[InFaceIndex];

					if (UniqIndeces->Count != 0)
					{
						for (u32 UniqIndex = 0;
							UniqIndex < UniqIndeces->Count;
							++UniqIndex)
						{
							if (UniqIndeces->Elements[UniqIndex] == VertexIndex)
							{
								IsAddToUniqBuff = false;
								break;
							}
						}
					}
					
					if (IsAddToUniqBuff)
					{
						UniqIndeces->Elements[UniqIndeces->Count++] = VertexIndex;
					}
				}
			}

			// TODO: See if it don't get enough precision
			for (u32 Index = 0;
				Index < UniqIndeces->Count;
				++Index)
			{
				u32 VertexIndex = UniqIndeces->Elements[Index];

				v3 V = Model->Vertex[VertexIndex] + ModelOffset;
				Result += V;
			}

			Result /= (f32)UniqIndeces->Count;
		} break;

		case ModelTargetElement_Edge:
		{
			for (u32 Index = 0;
				Index < Selected->Count;
				++Index)
			{
				u32 EdgeIndex = Selected->Elements[Index];
				model_edge *Edge = Model->Edges + EdgeIndex;

				for (u32 InEdgeIndex = 0;
					InEdgeIndex < ArrayCount(Edge->VertexID);
					++InEdgeIndex)
				{
					b32 IsAddToUniqBuff = true;
					u32 VertexIndex = Edge->VertexID[InEdgeIndex];

					if (UniqIndeces->Count != 0)
					{
						for (u32 UniqIndex = 0;
							UniqIndex < UniqIndeces->Count;
							++UniqIndex)
						{
							if (UniqIndeces->Elements[UniqIndex] == VertexIndex)
							{
								IsAddToUniqBuff = false;
								break;
							}
						}
					}

					if (IsAddToUniqBuff)
					{
						UniqIndeces->Elements[UniqIndeces->Count++] = VertexIndex;
					}
				}
			}

			// TODO: See if it don't get enough precision
			for (u32 Index = 0;
				Index < UniqIndeces->Count;
				++Index)
			{
				u32 VertexIndex = UniqIndeces->Elements[Index];

				v3 V = Model->Vertex[VertexIndex] + ModelOffset;
				Result += V;
			}

			Result /= (f32)UniqIndeces->Count;
		} break;
	}

	return Result;
}

// TODO: Optimize
// NOTE: Transform happens in model space
void
ApplyRotation(model *Model, element_id_buffer *UniqIndeces,
	model_target_element ElementTarget, v3 RotationOrigin, m4x4 Transform)
{
	v3 ModelSpaleRotOrigin = RotationOrigin - Model->Offset;

	switch (ElementTarget)
	{
		case ModelTargetElement_Model:
		{
			for (u32 Index = 0;
				Index < Model->VertexCount;
				++Index)
			{
				v3 V = Model->Vertex[Index];
				V = V * Transform;
				Model->Vertex[Index] = V;
			}
		} break;
		
		case ModelTargetElement_Edge:
		case ModelTargetElement_Face:
		{
			for (u32 Index = 0;
				Index < UniqIndeces->Count;
				++Index)
			{
				u32 VertexIndex = UniqIndeces->Elements[Index];
				v3 V = Model->Vertex[VertexIndex];
				
				V -= ModelSpaleRotOrigin;
				V = V * Transform;
				V += ModelSpaleRotOrigin;

				Model->Vertex[VertexIndex] = V;
			}
		} break;
	}
}

internal inline m4x4
GetRotateMatrixFromAxisID(f32 Angle, tools_axis_id ID)
{
	m4x4 Result;

	switch (ID)
	{
		case ToolsAxisID_X:
		{
			Result = XRotation(Angle);
		} break;
		case ToolsAxisID_Y:
		{
			Result = YRotation(Angle);
		} break;
		case ToolsAxisID_Z:
		{
			Result = ZRotation(Angle);
		} break;
	}

	return Result;
}

internal inline b32
SetCurrentDirVector(rotate_tools *Tool, ray_params Ray, v3 *ResultVector)
{
	b32 Result = false;
	
	f32 RPosDotPlaneN = Dot(Tool->FromPosToRayP, Tool->InteractPlane.N);

	f32 DotRayPlane = Dot(Ray.Dir, Tool->InteractPlane.N);
	f32 tRay = RayPlaneIntersect(Ray, Tool->InteractPlane, DotRayPlane);

	v3 CurrentVector;
	if (Abs(RPosDotPlaneN) <= RTOOLS_AXIS_INTERACT_THRESHOLD)
	{
		v3 PointOnSphere;
		if (RaySphereIntersect(Ray, Tool->CenterP, Tool->Radius, &PointOnSphere))
		{
			v3 DirFromCenter = Normalize(PointOnSphere - Tool->CenterP);
			f32 PDotD = Dot(Tool->InteractPlane.N, DirFromCenter);

			CurrentVector = Normalize(DirFromCenter - (Tool->InteractPlane.N * PDotD));
#if 1
			f32 CheckDot = Dot(CurrentVector, Tool->InteractPlane.N);
			Assert(CheckDot < 0.00001f);
#endif
			*ResultVector = CurrentVector;
			Result = true;
		}
	}
	else if (tRay >= 0)
	{
		CurrentVector = PointOnRay(Ray, tRay);;
		CurrentVector = Normalize(CurrentVector - Tool->CenterP);

		*ResultVector = CurrentVector;
		Result = true;
	}

	return Result;
}
internal b32
ProcessRotateToolTransform(rotate_tools *Tool, ray_params Ray, m3x3 Axis)
{
	b32 Result = false;

	v3 CurrentVector;
	if (SetCurrentDirVector(Tool, Ray, &CurrentVector))
	{
		// TODO: Move to set _move_ interaction?
		if (!Tool->EnterActiveState)
		{
			Tool->BeginVector = CurrentVector;
			Tool->PrevVector = CurrentVector;
			Tool->EnterActiveState = true;
		}

		if (Tool->PrevVector != CurrentVector)
		{
			v3 PerpVector = Cross(Tool->PrevVector, CurrentVector);
			f32 RotateDir = Dot(PerpVector, Tool->InteractPlane.N);
			RotateDir = Sign(RotateDir);
			// TODO: Catch and fix NaN bug
			Assert(!isnan(RotateDir));

			f32 DotAngle = Dot(CurrentVector, Tool->PrevVector);
			DotAngle = Clamp(-1.0f, DotAngle, 1.0f);
			f32 AngleBetween = ACos(DotAngle) * RotateDir;

			if (AngleBetween != 0)
			{
				m4x4 Rotate = GetRotateMatrixFromAxisID(AngleBetween, Tool->InteractAxis);
				m4x4 CurrentRotateAxis = Row3x3(Axis.X, Axis.Y, Axis.Z);
				m4x4 ResultAxis = Rotate * CurrentRotateAxis;

				m4x4 InvCurrentAxis = Transpose(CurrentRotateAxis);
				m4x4 ResultTransform = InvCurrentAxis * ResultAxis;

				m4x4 ToolCurrentAxis;
				if (Tool->DefaultAxisSet)
				{
					ToolCurrentAxis = Row3x3(Tool->Axis.X, Tool->Axis.Y, Tool->Axis.Z);
					ToolCurrentAxis = ToolCurrentAxis * Rotate;
				}
				else
				{
					ToolCurrentAxis = ResultAxis;
				}

				Tool->Transform = ResultTransform;
				Tool->Axis.X = GetRow(ToolCurrentAxis, 0);
				Tool->Axis.Y = GetRow(ToolCurrentAxis, 1);
				Tool->Axis.Z = GetRow(ToolCurrentAxis, 2);
				Tool->PrevVector = CurrentVector;
				
				// TODO: Catch and fix NaN bug
				Assert(!isnan(Tool->Axis.X.x) && !isnan(Tool->Axis.X.y) && !isnan(Tool->Axis.X.z));
				Assert(!isnan(Tool->Axis.Y.x) && !isnan(Tool->Axis.Y.y) && !isnan(Tool->Axis.Y.z));
				Assert(!isnan(Tool->Axis.Z.x) && !isnan(Tool->Axis.Z.y) && !isnan(Tool->Axis.Z.z));

				Result = true;
			}
		}
	}

	return Result;
}

internal inline b32
IsRotateToolPerpAxisIntreract(rotate_tools *Tool, ray_params Ray, v3 Axis)
{
	b32 Result = false;

	plane_params Plane = {};
	Plane.N = Axis;
	Plane.D = Dot(Axis, Tool->CenterP);

	f32 ADotR = Dot(Axis, Ray.Dir);
	f32 tRay = RayPlaneIntersect(Ray, Plane, ADotR);
	v3 P = PointOnRay(Ray, tRay);

	f32 RadiusSq = Square(Tool->Radius);
	f32 FromCenterToP = LengthSq(P - Tool->CenterP);
	f32 NormLength = FromCenterToP / RadiusSq;

	if (NormLength <= 1.0f &&
		NormLength >= RTOOLS_PERP_AXIS_INTERACT_MIN_THRESHOLD)
	{
		Result = true;
	}

	return Result;
}

internal inline b32
IsRotateToolAxisPerp(rotate_tools *Tool, v3 Axis, v3 CameraZ)
{
	b32 Result = false;

	f32 ADotC = Abs(Dot(Axis, CameraZ));
	if (Abs(ADotC) >= Tool->PerpThreshold)
	{
		Result = true;
	}

	return Result;
}

internal inline scale_tool_axis_params
ModScaleToolAxisParams(scale_tool_axis_params Params, f32 Scale)
{
	scale_tool_axis_params Result;

	Result.EdgeLen = Params.EdgeLen * Scale;
	Result.AdjustEdgeLen = Params.AdjustEdgeLen * Scale;
	Result.EdgeHalfSize = Params.EdgeHalfSize * Scale;
	Result.ArrowHalfSize = Params.ArrowHalfSize * Scale;

	return Result;
}

// TODO: FINISH!!!!
internal inline tools_axis_id
RayScaleToolAxisTest(ray_params Ray, scale_tool_axis_params AxisParams,
	m3x3 Axis, v3 AxisOffset, f32 ZSignMod)
{
	tools_axis_id Result = ToolsAxisID_None;

	// NOTE: Test code, not working on face and edge
	m4x4 MAxis = ToM4x4(Axis);
	m4x4 InvAxis = Transpose(MAxis);
	v4 _RayD = V4(Ray.Dir, 1.0f) * InvAxis;
	v4 _RayP = V4(Ray.P, 1.0f) * InvAxis;
	ray_params InvRay = CreateRay(_RayP.xyz, Normalize(_RayD.xyz));
	
	m3x3 DefaultAxis = Identity3x3();
	DefaultAxis.Z *= ZSignMod;

	v3 ArrowDim = V3(AxisParams.ArrowHalfSize);
	f32 HalfAdjustEdgeLen = AxisParams.AdjustEdgeLen * 0.5f;
	f32 ArrowCenterOffset = AxisParams.EdgeLen - AxisParams.ArrowHalfSize;
	f32 EdgeCenterOffset = AxisParams.EdgeLen - HalfAdjustEdgeLen;

	//v3 YEdgeDim = V3(AxisParams.EdgeHalfSize, HalfAdjustEdgeLen, AxisParams.EdgeHalfSize);
	//v3 ZEdgeDim = V3(AxisParams.EdgeHalfSize, AxisParams.EdgeHalfSize, HalfAdjustEdgeLen);

	v3 XHalfEdgeDim = V3(HalfAdjustEdgeLen, AxisParams.EdgeHalfSize, AxisParams.EdgeHalfSize);
	rect3 XArrowAABB = CreateRect(ArrowDim, DefaultAxis.X*ArrowCenterOffset);
	rect3 XEdgeAABB = CreateRect(XHalfEdgeDim, DefaultAxis.X*EdgeCenterOffset);

	b32 IsHitXArrow = RayAABBIntersect(InvRay, XArrowAABB, AxisOffset);
	b32 IsHitXEdge = RayAABBIntersect(InvRay, XEdgeAABB, AxisOffset);

	if (IsHitXArrow || IsHitXEdge) Result = ToolsAxisID_X;

	return Result;
}

m3x3
SetAxisForTool(model *Model, element_id_buffer *Selected, u32 ElementTarget)
{
	m3x3 Result = {};

	if (ElementTarget == ModelTargetElement_Model)
	{
		Result = Model->Axis;
	}
	else if (Selected->Count > 1)
	{
		Result = Identity3x3();
	}
	else if (Selected->Count == 1)
	{
		u32 ElementID = Selected->Elements[0];
		switch (ElementTarget)
		{
			case ModelTargetElement_Edge:
			{
				model_edge *Edge = Model->Edges + ElementID;
				edge_faces_norm RelatedNorm = GetEdgeFacesRelatedNormals(Model, Edge);
				v3 V0 = Model->Vertex[Edge->V0];
				v3 V1 = Model->Vertex[Edge->V1];

				Result.Z = Normalize(V1 - V0);
				Result.Y = NLerp(RelatedNorm.N0, 0.5f, RelatedNorm.N1);
				Result.X = Cross(Result.Y, Result.Z);
			} break;

			case ModelTargetElement_Face:
			{
				model_face *Face = Model->Faces + ElementID;
				face_vertex Vertex = GetFaceVertex(Model, Face);

				v3 OriginZ = V3(0, 0, 1);
				Result.Z = GetPlaneAvgNormal(Vertex);

				f32 ZDotResult = Abs(Dot(Result.Z, OriginZ));
				if (ZDotResult == 1.0f)
					OriginZ = V3(-1, 0, 0);

				Result.Y = Normalize(Cross(OriginZ, Result.Z));
				Result.X = Cross(Result.Y, Result.Z);

				Assert(Length(Result.Y) >= 0.98f);
				Assert(Length(Result.X) >= 0.98f);
				Assert(Length(Result.Z) >= 0.98f);
			}
		}
	}

	return Result;
}

// TODO: Add more tools!!!
void
InitTools(editor_world_ui *WorldUI, tools *Tools, model *ModelsArr, memory_arena *TranArena)
{
	element_id_buffer *Selected = &WorldUI->Selected;
	interact_model *IModel = &WorldUI->IModel;
	model *Model = ModelsArr + IModel->ID;

	// TODO: Define pos. as common
	tool_type ToolType = (tool_type)Tools->Type;
	switch (ToolType)
	{
		case ToolType_Translate:
		case ToolType_Scale:
		{
			Tools->Scale = {};
			
			Tools->Scale.P = ComputeToolPos(Model, &Tools->UniqIndeces, Selected, IModel->Target);
			Tools->Scale.InitAxisParams.EdgeLen = SCALE_TOOL_SIZE;
			Tools->Scale.InitAxisParams.AdjustEdgeLen = SCALE_TOOL_SIZE * 0.85f;
			Tools->Scale.InitAxisParams.EdgeHalfSize = 0.005f;
			Tools->Scale.InitAxisParams.ArrowHalfSize = SCALE_TOOL_SIZE * 0.04f;
		} break;

		case ToolType_Rotate:
		{
			Tools->Rotate = {};

			Tools->Rotate.CenterP = ComputeToolPos(Model, &Tools->UniqIndeces, Selected, IModel->Target);
			Tools->Rotate.InitRadius = ROTATE_TOOL_DIAMETER * 0.5f;
			Tools->Rotate.PerpThreshold = 0.95f;
		} break;

		InvalidDefaultCase;
	}

	Tools->IsInit = true;
}

// TODO: Make tools sizeble or on same distance to the camera
internal void inline
UpdateModelInteractionTools(game_editor_state *Editor, game_input *Input, render_group *RenderGroup)
{
	ui_interaction Interaction = {};
	editor_world_ui *WorldUI = &Editor->WorldUI;
	tools *Tools = &WorldUI->Tools;
	model *Model = Editor->Models + WorldUI->IModel.ID;

	// TODO: Extend
	if (!Tools->IsInit)
	{
		InitTools(WorldUI, Tools, Editor->Models, &Editor->TranArena);
	}

	ray_params Ray = WorldUI->MouseRay;
	switch (Tools->Type)
	{
		// TODO: Implement drawing progres angle
		case ToolType_Rotate:
		{
			rotate_tools *RotateTool = &Tools->Rotate;

			v3 RayPCenterP = Ray.P - RotateTool->CenterP;
			f32 LengthRayPCenterP = Length(RayPCenterP);
			f32 Scale = LengthRayPCenterP / Tools->AdjustScaleDist;

			RotateTool->Radius = RotateTool->InitRadius * Scale;
			RotateTool->FromPosToRayP = Normalize(RayPCenterP, LengthRayPCenterP);

			m3x3 Axis;
			if (IsDown(Input->Ctrl))
			{
				Axis = Identity3x3();
				RotateTool->DefaultAxisSet = true;
			}
			else
			{
				if (!RotateTool->AxisSet)
				{
					RotateTool->Axis = SetAxisForTool(Model, &WorldUI->Selected, WorldUI->IModel.Target);
					RotateTool->AxisSet = true;
				}
				Axis = RotateTool->Axis;
				RotateTool->DefaultAxisSet = false;
			}
			
			
			RotateTool->PerpInfo = {};
			b32 IsXPerp = IsRotateToolAxisPerp(RotateTool, Axis.X, RotateTool->FromPosToRayP);
			b32 IsYPerp = IsRotateToolAxisPerp(RotateTool, Axis.Y, RotateTool->FromPosToRayP);
			b32 IsZPerp = IsRotateToolAxisPerp(RotateTool, Axis.Z, RotateTool->FromPosToRayP);
			b32 IsHavePerpAxis = IsXPerp | IsYPerp | IsZPerp;

			u32 PerpAxisIndex;
			if (IsHavePerpAxis)
			{
				RotateTool->PerpInfo.IsSet = 1;

				if (IsXPerp)
					RotateTool->PerpInfo.Index = 0;
				else if (IsYPerp)
					RotateTool->PerpInfo.Index = 1;
				else if (IsZPerp)
					RotateTool->PerpInfo.Index = 2;
			}

			if (RotateTool->InteractAxis == ToolsAxisID_None)
			{
				// TODO: Move this code to separate function?
				RotateTool->AxisMask = {};

				b32 PerpAxisIntr[3] = {};
				if (IsHavePerpAxis)
				{
					u32 PerpAxisIndex = RotateTool->PerpInfo.Index;
					PerpAxisIntr[PerpAxisIndex] =
						IsRotateToolPerpAxisIntreract(RotateTool, Ray, Axis.Row[PerpAxisIndex]);
				}

				// TODO: Make posible interaction with perp axis beyond sphere radius?
				// (set IsHavePerpIntr and IsHaveSphereIntr?)
				v3 PointOnSphere;
				if (RaySphereIntersect(Ray, RotateTool->CenterP, RotateTool->Radius, &PointOnSphere))
				{
					// TODO: Move this code to separate function?

					v3 DirFromCenter = Normalize(PointOnSphere - RotateTool->CenterP);
					f32 XDotP = Abs(Dot(Axis.X, DirFromCenter));
					f32 YDotP = Abs(Dot(Axis.Y, DirFromCenter));
					f32 ZDotP = Abs(Dot(Axis.Z, DirFromCenter));
					
					tools_axis_id InteractAxis = ToolsAxisID_None;
					if ((ZDotP <= RTOOLS_AXIS_INTERACT_THRESHOLD) || PerpAxisIntr[2])
					{
						InteractAxis = ToolsAxisID_Z;
						RotateTool->AxisMask.z = 1.0f;
						RotateTool->InteractPlane.N = Axis.Z;
					}
					else if ((YDotP <= RTOOLS_AXIS_INTERACT_THRESHOLD) || PerpAxisIntr[1])
					{
						InteractAxis = ToolsAxisID_Y;
						RotateTool->AxisMask.y = 1.0f;
						RotateTool->InteractPlane.N = Axis.Y;
					}
					else if ((XDotP <= RTOOLS_AXIS_INTERACT_THRESHOLD) || PerpAxisIntr[0])
					{
						InteractAxis = ToolsAxisID_X;
						RotateTool->AxisMask.x = 1.0f;
						RotateTool->InteractPlane.N = Axis.X;
					}

					Interaction.TypeID = SetIntrTypeID(UI_InteractionTarget_Tools, UI_InteractionType_Select);
					Interaction.ID.ID[0] = ToolType_Rotate;
					Interaction.ID.ID[1] = InteractAxis;

					if ((InteractAxis != ToolsAxisID_None) && AreEqual(WorldUI->Interaction, Interaction))
					{
						RotateTool->InteractAxis = InteractAxis;
						RotateTool->InteractPlane.D = Dot(RotateTool->InteractPlane.N, RotateTool->CenterP);
						RotateTool->AxisMask.w = 1.0f;

						Interaction.TypeID = SetIntrTypeID(UI_InteractionTarget_Tools, UI_InteractionType_Move);
						WorldUI->Interaction = Interaction;
					}
				}
			}
			else
			{
				Interaction.TypeID = SetIntrTypeID(UI_InteractionTarget_Tools, UI_InteractionType_Move);
				Interaction.ID.ID[0] = ToolType_Rotate;
				Interaction.ID.ID[1] = RotateTool->InteractAxis;

				if (AreEqual(WorldUI->Interaction, Interaction))
				{
					if (ProcessRotateToolTransform(RotateTool, Ray, Axis))
					{
						model_target_element TargetElement = (model_target_element)WorldUI->IModel.Target;
						if (TargetElement == ModelTargetElement_Model)
						{
							Model->Axis = RotateTool->Axis;
						}

						if (!RotateTool->DefaultAxisSet)
						{
							Axis = RotateTool->Axis;
						}

						ApplyRotation(Model, &Tools->UniqIndeces, TargetElement,
							RotateTool->CenterP, RotateTool->Transform);
						Model->AABB = ComputeMeshAABB(Model->Vertex, Model->VertexCount);
					}
				}
				else
				{
					//ZeroStruct(RotateTool->PrevVector);
					ZeroStruct(RotateTool->InteractPlane);
					ZeroStruct(RotateTool->BeginVector);
					ZeroStruct(RotateTool->AxisMask);
					RotateTool->InteractAxis = ToolsAxisID_None;
					RotateTool->EnterActiveState = false;
				}
			}

			PushRotateTool(RenderGroup, Editor->StaticMesh[0].Mesh, RotateTool->CenterP,
				Scale, Axis, RotateTool->AxisMask, RotateTool->PerpInfo.V,
				RotateTool->FromPosToRayP);
		} break;
		case ToolType_Scale:
		{
			scale_tools *ScaleTool = &Tools->Scale;
			v4 AxisMask = {};

			v3 RayPCenterP = Ray.P - ScaleTool->P;
			f32 LengthRayPCenterP = Length(RayPCenterP);
			f32 Scale = LengthRayPCenterP / Tools->AdjustScaleDist;
			
			scale_tool_axis_params ScaleAxisParams =
				ModScaleToolAxisParams(ScaleTool->InitAxisParams, Scale);
			
			//if (ScaleTool->InteractAxis == ToolsAxisID_None)
			//{
				m3x3 Axis;
				if (IsDown(Input->Ctrl))
				{
					Axis = Identity3x3();
					ScaleTool->DefaultAxisSet = true;
				}
				else
				{
					Axis = SetAxisForTool(Model, &WorldUI->Selected, WorldUI->IModel.Target);
				}

				f32 ZSignMod = 1.0f; //Sign(Dot(Axis.Z, RayPCenterP));
				//Axis.Z *= ZSignMod;


				tools_axis_id InteractAxis =
					RayScaleToolAxisTest(Ray, ScaleAxisParams, Axis, ScaleTool->P, ZSignMod);

				switch (InteractAxis)
				{
					case ToolsAxisID_X:
					{
						AxisMask.x = 1.0f;
					} break;
				}

			//}
			//else
			//{

			//}

			PushScaleTool(RenderGroup, ScaleTool->P, Axis, ScaleAxisParams, AxisMask);
		} break;
		case ToolType_Translate:
		{
			PushSphere(RenderGroup, Editor->StaticMesh[0].Mesh, V3(0, 0, 1));
		} break;
	}

	WorldUI->NextHotInteraction = Interaction;
}

inline b32
IsValid(interact_model IModel)
{
}

void
EditorUIInteraction(game_editor_state *Editor, game_input *Input, render_group *RenderGroup)
{
	editor_world_ui *WorldUI = &Editor->WorldUI;

	ProcessWorldUIInput(WorldUI, Input);

	// TODO: Remove UpdateITarget conception?
	
	if (IsITargetEq(WorldUI->ITarget, Model) &&	WorldUI->UpdateModelInteraction)
	{
		UpdateModelInteractionElement(Editor, Input, RenderGroup);
	}
	else if (IsITargetEq(WorldUI->ITarget, Tools))
	{
		UpdateModelInteractionTools(Editor, Input, RenderGroup);
	}

	// TODO: Split to anouther function?
	// TODO: Set ui interaction in proper way
	switch (WorldUI->Interaction.Type)
	{
		case UI_InteractionType_None:
		{
			WorldUI->HotInteraction = WorldUI->NextHotInteraction;
			if (IsDown(Input->MouseButtons[PlatformMouseButton_Left]))
			{
				WorldUI->UpdateModelInteraction = false;
				WorldUI->Interaction = WorldUI->HotInteraction;
			}
			else if (WasDown(Input->MouseButtons[PlatformMouseButton_Left]))
			{
				WorldUI->UpdateModelInteraction = true;
			}
			else if (WasDown(Input->MouseButtons[PlatformMouseButton_Right]) &&
				IsDown(Input->Shift))
			{
				if (IsITargetEq(WorldUI->ITarget, Model) &&
					(WorldUI->Selected.Count ||
					(WorldUI->IModel.Target == ModelTargetElement_Model)))
				{
					WorldUI->ITarget = UI_InteractionTarget_Tools;
				}
			}
		} break;

		case UI_InteractionType_Select:
		{
			if (WasDown(Input->MouseButtons[PlatformMouseButton_Left]))
			{
				WorldUI->UpdateModelInteraction = true;
				WorldUI->NextToExecute = WorldUI->Interaction;
				ZeroStruct(WorldUI->Interaction);
			}
		} break;

		default:
		{
			if (IsUp(Input->MouseButtons[PlatformMouseButton_Left]))
			{
				ZeroStruct(WorldUI->Interaction);
			}
		}
	}


	WorldUI->ToExecute = WorldUI->NextToExecute;
	ZeroStruct(WorldUI->NextToExecute);
	ZeroStruct(WorldUI->NextHotInteraction);
}
