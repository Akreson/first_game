
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

inline ui_interaction
SetModelSelectInteraction(u32 ModelID, u32 FaceID = 0, u32 EdgeID = 0, u16 Target = 0)
{
	ui_interaction Result;

	Result.TypeID = SetIntrTypeID(UI_InteractionTarget_Model, UI_InteractionType_Select);
	Result.ID = IDFromModel(Target, ModelID, FaceID, EdgeID);

	return Result;
}

inline ui_interaction
SetToolAxisIntr(tool_type Tool, ui_interaction_type Type, tools_axis_id Axis)
{
	ui_interaction Result;
	Result.TypeID = SetIntrTypeID(UI_InteractionTarget_Tools, Type);
	Result.ID.ID[0] = Tool;
	Result.ID.ID[1] = Axis;

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

internal inline ray_to_point_params
GetRayToPointRelParam(v3 RayP, v3 ToolP, f32 AdjustScaleDist)
{
	ray_to_point_params Result;
	Result.ToolToRayV = RayP - ToolP;
	Result.LenV = Length(Result.ToolToRayV);
	Result.ScaleFactor = Result.LenV / AdjustScaleDist;

	return Result;
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
			
			ray_to_point_params PosRelParams =
				GetRayToPointRelParam(WorldUI->MouseRay.P, Model->Offset, WorldUI->Tools.AdjustScaleDist);

			f32 EdgeIntersetRadius = 0.017f * PosRelParams.ScaleFactor;
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

	Model->AABB = ComputeMeshAABB(Model->Vertex, Model->VertexCount);
}

// TODO: Optimize!!!
void
ApplyScale(model *Model, element_id_buffer *UniqIndeces,
	model_target_element ElementTarget, v3 Origin, v4 ScaleParam)
{
	v3 ScaleV = ScaleParam.xyz * ScaleParam.w;
	v3 ScaleOrigin = Origin - Model->Offset;

	switch (ElementTarget)
	{
		case ModelTargetElement_Model:
		{
			for (u32 Index = 0;
				Index < Model->VertexCount;
				++Index)
			{
				v3 V = Model->Vertex[Index];

				v3 NormV = Normalize(V);
				f32 VDotS = Dot(NormV, ScaleParam.xyz);
				//VDotS = Abs(VDotS) <= 0.001f ? 1.0f : VDotS;
				v3 ScaleFactor = ScaleV * Sign(VDotS);

				V = V + ScaleFactor;
				Model->Vertex[Index] = V;
			}
		} break;

		// NOTE: Must not be used for move plane int Z direction
		// Or one edge in X direction
		// TODO: Implement forbidding this behavior?
		// TODO: Another method of linear scaling?
		case ModelTargetElement_Edge:
		case ModelTargetElement_Face:
		{
			for (u32 Index = 0;
				Index < UniqIndeces->Count;
				++Index)
			{
				u32 VertexIndex = UniqIndeces->Elements[Index];
				v3 V = Model->Vertex[VertexIndex];

				V -= ScaleOrigin;

				v3 NormV = Normalize(V);
				f32 VDotS = Dot(NormV, ScaleParam.xyz);
				VDotS = Abs(VDotS) <= 0.001f ? 1.0f : VDotS;
				v3 ScaleFactor = ScaleV * Sign(VDotS);

				V = V + ScaleFactor;
				V += ScaleOrigin;
				Model->Vertex[VertexIndex] = V;
			}
		} break;
	}

	Model->AABB = ComputeMeshAABB(Model->Vertex, Model->VertexCount);
}

// TODO: Optimize
void
ApplyTranslate(model *Model, element_id_buffer *UniqIndeces,
	model_target_element ElementTarget, v3 TransParam)
{
	switch (ElementTarget)
	{
		case ModelTargetElement_Model:
		{
			Model->Offset += TransParam;
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

				V += TransParam;
				Model->Vertex[VertexIndex] = V;
			}

			Model->AABB = ComputeMeshAABB(Model->Vertex, Model->VertexCount);
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
		if (RaySphereIntersect(Ray, Tool->P, Tool->Radius, &PointOnSphere))
		{
			v3 DirFromCenter = Normalize(PointOnSphere - Tool->P);
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
		CurrentVector = PointOnRay(Ray, tRay);
		CurrentVector = Normalize(CurrentVector - Tool->P);

		*ResultVector = CurrentVector;
		Result = true;
	}

	return Result;
}

internal inline b32
InitRotToolProcessing(rotate_tools *Tool, ray_params Ray, tools_axis_id InteractAxis)
{
	b32 Result = false;

	// NOTE: InteractPlane must be full init before call SetCurrentDirVector
	Tool->InteractPlane.D = Dot(Tool->InteractPlane.N, Tool->P);

	v3 CurrentVector;
	if (SetCurrentDirVector(Tool, Ray, &CurrentVector))
	{
		Tool->InteractAxis = InteractAxis;
		Tool->AxisMask.w = 1.0f;
		Tool->BeginVector = CurrentVector;
		Tool->PrevVector = CurrentVector;
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
		if (Tool->PrevVector != CurrentVector)
		{
			v3 PerpVector = Cross(Tool->PrevVector, CurrentVector);
			f32 RotateDir = Dot(PerpVector, Tool->InteractPlane.N);
			RotateDir = Sign(RotateDir);

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
IsRotateToolAxisPerp(f32 PerpThreshold, v3 Axis, v3 CameraZ)
{
	b32 Result = false;

	f32 ADotC = Abs(Dot(Axis, CameraZ));
	if (Abs(ADotC) >= PerpThreshold)
	{
		Result = true;
	}

	return Result;
}

internal inline rot_tool_perp_axis
SetRotToolPerpInfo(m3x3 Axis, f32 PerpThreshold, v3 FromPosToRayP)
{
	rot_tool_perp_axis Result = {};

	b32 IsXPerp = IsRotateToolAxisPerp(PerpThreshold, Axis.X, FromPosToRayP);
	b32 IsYPerp = IsRotateToolAxisPerp(PerpThreshold, Axis.Y, FromPosToRayP);
	b32 IsZPerp = IsRotateToolAxisPerp(PerpThreshold, Axis.Z, FromPosToRayP);
	b32 IsHavePerpAxis = IsXPerp | IsYPerp | IsZPerp;

	u32 PerpAxisIndex;
	if (IsHavePerpAxis)
	{
		Result.IsSet = 1;

		if (IsXPerp)
			Result.Index = 0;
		else if (IsYPerp)
			Result.Index = 1;
		else if (IsZPerp)
			Result.Index = 2;
	}

	return Result;
}

internal inline b32
IsRotateToolPerpAxisIntreract(v3 ToolCenterP, f32 Radius, ray_params Ray, v3 Axis)
{
	b32 Result = false;

	plane_params Plane = {};
	Plane.N = Axis;
	Plane.D = Dot(Axis, ToolCenterP);

	f32 ADotR = Dot(Axis, Ray.Dir);
	f32 tRay = RayPlaneIntersect(Ray, Plane, ADotR);
	v3 P = PointOnRay(Ray, tRay);

	f32 RadiusSq = Square(Radius);
	f32 FromCenterToP = LengthSq(P - ToolCenterP);
	f32 NormLength = FromCenterToP / RadiusSq;

	if (NormLength <= RTOOLS_PERP_AXIS_INTERACT_MAX_THRESHOLD &&
		NormLength >= RTOOLS_PERP_AXIS_INTERACT_MIN_THRESHOLD)
	{
		Result = true;
	}

	return Result;
}

internal inline v3i
GetIntrPerpAxisInfo(rotate_tools *Tool, ray_params Ray, m3x3 Axis)
{
	v3i Result = {};

	if (Tool->PerpInfo.IsSet)
	{
		u32 PerpAxisIndex = Tool->PerpInfo.Index;
		Result.E[PerpAxisIndex] =
			IsRotateToolPerpAxisIntreract(Tool->P, Tool->Radius, Ray, Axis.Row[PerpAxisIndex]);
	}

	return Result;
}

internal inline scl_tool_display_params
SetDefaultDisplayParams(scl_tool_default_params Params)
{
	scl_tool_display_params Result;

	Result.X = Params.Axis;
	Result.Y = Params.Axis;
	Result.Z = Params.Axis;
	Result.ArrowHalfSize = Params.ArrowHalfSize;
	Result.EdgeXYHalfSize = Params.EdgeXYHalfSize;

	return Result;
}

internal inline void
SetDefaultDisplayParams(scl_tool_display_params *DispParams, scl_tool_default_params DefParams, tools_axis_id ActiveAxis)
{
	DispParams->ArrowHalfSize = DefParams.ArrowHalfSize;
	DispParams->EdgeXYHalfSize = DefParams.EdgeXYHalfSize;

	u32 ActiveIntrAxisID = (u32)ActiveAxis - 1;
	for (u32 Index = 0;
		Index < (ToolsAxisID_Count - 1);
		++Index)
	{
		if (Index != ActiveIntrAxisID)
		{
			DispParams->Axis[Index] = DefParams.Axis;
		}
	}
}

internal inline v3i
GetIntrOnSphereAxisInfo(rotate_tools *Tool, ray_params Ray, m3x3 Axis)
{
	v3i Result = {};

	v3 PointOnSphere;
	if (RaySphereIntersect(Ray, Tool->P, Tool->Radius, &PointOnSphere))
	{
		v3 DirFromCenter = Normalize(PointOnSphere - Tool->P);
		f32 XDotP = Abs(Dot(Axis.X, DirFromCenter));
		f32 YDotP = Abs(Dot(Axis.Y, DirFromCenter));
		f32 ZDotP = Abs(Dot(Axis.Z, DirFromCenter));

		Result.z = (ZDotP <= RTOOLS_AXIS_INTERACT_THRESHOLD);
		Result.y = (YDotP <= RTOOLS_AXIS_INTERACT_THRESHOLD);
		Result.x = (XDotP <= RTOOLS_AXIS_INTERACT_THRESHOLD);
	}

	return Result;
}

internal inline tools_axis_id
SetRotIntrAxisInfo(rotate_tools *Tool, m3x3 Axis, v3i IntrOnSphereAxisBool, v3i IntrPerpAxisBool)
{
	tools_axis_id Result = ToolsAxisID_None;

	if (IntrOnSphereAxisBool.z || IntrPerpAxisBool.z)
	{
		Result = ToolsAxisID_Z;
		Tool->AxisMask.z = 1.0f;
		Tool->InteractPlane.N = Axis.Z;
	}
	else if (IntrOnSphereAxisBool.y || IntrPerpAxisBool.y)
	{
		Result = ToolsAxisID_Y;
		Tool->AxisMask.y = 1.0f;
		Tool->InteractPlane.N = Axis.Y;
	}
	else if (IntrOnSphereAxisBool.x || IntrPerpAxisBool.x)
	{
		Result = ToolsAxisID_X;
		Tool->AxisMask.x = 1.0f;
		Tool->InteractPlane.N = Axis.X;
	}

	return Result;
}

internal inline scl_tool_default_params
ModScaleToolDefauldParams(scl_tool_default_params Params, f32 Scale)
{
	scl_tool_default_params Result;

	Result.Axis.Len = Params.Axis.Len * Scale;
	Result.Axis.EdgeCenter = Params.Axis.EdgeCenter * Scale;
	Result.Axis.EdgeLenHalfSize = Params.Axis.EdgeLenHalfSize * Scale;
	Result.EdgeXYHalfSize = Params.EdgeXYHalfSize * Scale;
	Result.ArrowHalfSize = Params.ArrowHalfSize * Scale;

	return Result;
}

// TODO: Cleaning up?
internal inline tools_axis_id
RayScaleToolAxisTest(ray_params Ray, scl_tool_default_params AxisParams,
	m3x3 Axis, v3 AxisOffset, f32 ZSignMod)
{
	tools_axis_id Result = ToolsAxisID_None;

	m4x4 InvAxis = Transpose(ToM4x4(Axis));
	v3 InvPos = -(AxisOffset * InvAxis);
	SetTranslation(&InvAxis, InvPos);

	v4 InvRayD = V4(Ray.Dir, 0) * InvAxis;
	v4 InvRayP = V4(Ray.P, 1.0f) * InvAxis;
	ray_params InvRay = CreateRay(InvRayP.xyz, InvRayD.xyz);
	
	m3x3 DefaultAxis = Identity3x3();
	DefaultAxis.Z *= ZSignMod;

	f32 ArrowIntrScaleFactor = 0.02f;
	f32 EdgeIntrScaleFactor = 0.02f;
	f32 ModEdgeXYHalfSize = AxisParams.EdgeXYHalfSize + TOOL_EDGE_INTR_SCALE_FALCTOR;

	v3 ArrowDim = V3(AxisParams.ArrowHalfSize + TOOL_ARROW_INTR_SCALE_FALCTOR);
	v3 XEdgeHalfDim = V3(AxisParams.Axis.EdgeLenHalfSize, ModEdgeXYHalfSize, ModEdgeXYHalfSize);
	rect3 XArrowAABB = CreateRect(ArrowDim, DefaultAxis.X*AxisParams.Axis.Len);
	rect3 XEdgeAABB = CreateRect(XEdgeHalfDim, DefaultAxis.X*AxisParams.Axis.EdgeCenter);

	v3 YEdgeHalfDim = V3(ModEdgeXYHalfSize, AxisParams.Axis.EdgeLenHalfSize, ModEdgeXYHalfSize);
	rect3 YArrowAABB = CreateRect(ArrowDim, DefaultAxis.Y*AxisParams.Axis.Len);
	rect3 YEdgeAABB = CreateRect(YEdgeHalfDim, DefaultAxis.Y*AxisParams.Axis.EdgeCenter);

	v3 ZEdgeHalfDim = V3(ModEdgeXYHalfSize, ModEdgeXYHalfSize, AxisParams.Axis.EdgeLenHalfSize);
	rect3 ZArrowAABB = CreateRect(ArrowDim, DefaultAxis.Z*AxisParams.Axis.Len);
	rect3 ZEdgeAABB = CreateRect(ZEdgeHalfDim, DefaultAxis.Z*AxisParams.Axis.EdgeCenter);

	b32 IsHitXArrow = RayAABBIntersect(InvRay, XArrowAABB);
	b32 IsHitXEdge = RayAABBIntersect(InvRay, XEdgeAABB);

	b32 IsHitYArrow = RayAABBIntersect(InvRay, YArrowAABB);
	b32 IsHitYEdge = RayAABBIntersect(InvRay, YEdgeAABB);

	b32 IsHitZArrow = RayAABBIntersect(InvRay, ZArrowAABB);
	b32 IsHitZEdge = RayAABBIntersect(InvRay, ZEdgeAABB);

	if (IsHitXArrow || IsHitXEdge) Result = ToolsAxisID_X;
	if (IsHitYArrow || IsHitYEdge) Result = ToolsAxisID_Y;
	if (IsHitZArrow || IsHitZEdge) Result = ToolsAxisID_Z;

	return Result;
}

// TODO: Collate Scale and Trans tool processing

internal b32
ProcessScaleToolTransform(scale_tools *Tool, ray_params Ray)
{
	b32 Result = false;

	u32 IntrAxisID = (u32)Tool->InteractAxis - 1;
	v3 ScaleAxis = Tool->Axis.Row[IntrAxisID];
	ray_params AxisRay = CreateRay(Tool->P, ScaleAxis);

	f32 CurrentP = 0;
	if (ClosestPBeetwenRay(Ray, AxisRay, 0, &CurrentP))
	{
		// TODO: Move to set _move_ interaction?
		if (!Tool->EnterActiveState)
		{
			Tool->BeginP = CurrentP;
			Tool->PrevP = CurrentP;
			Tool->EnterActiveState = true;
		}

		tools_axis_params *ActiveAxis = Tool->DisplayState.Axis + IntrAxisID;
		ActiveAxis->Len = CurrentP;
		ActiveAxis->EdgeCenter = CurrentP * 0.5f;
		ActiveAxis->EdgeLenHalfSize = ActiveAxis->EdgeCenter;

		if (Tool->PrevP != CurrentP)
		{
			f32 ScaleFactor = (CurrentP - Tool->PrevP);

			Tool->ScaleParam.xyz = ScaleAxis;
			Tool->ScaleParam.w = ScaleFactor;
			Tool->PrevP = CurrentP;
			Result = true;
		}
	}

	return Result;
}

internal b32
ProcessTransToolTransform(translate_tools *Tool, ray_params Ray)
{
	b32 Result = false;

	u32 IntrAxisID = (u32)Tool->InteractAxis - 1;
	v3 TransAxis = Tool->Axis.Row[IntrAxisID];
	ray_params AxisRay = CreateRay(Tool->P, TransAxis);

	f32 CurrentP = 0;
	if (ClosestPBeetwenRay(Ray, AxisRay, 0, &CurrentP))
	{
		// TODO: Move to set _move_ interaction?
		if (!Tool->EnterActiveState)
		{
			Tool->BeginP = CurrentP;
			Tool->PrevP = CurrentP;
			Tool->EnterActiveState = true;
		}

		if (Tool->PrevP != CurrentP)
		{
			f32 TransFactor = (CurrentP - Tool->PrevP);

			Tool->TransParam = TransAxis * TransFactor;
			Tool->P += Tool->TransParam;
			Tool->PrevP = CurrentP - TransFactor;
			Result = true;
		}
	}

	return Result;
}

internal inline trans_tool_axis_params
ModTransToolDefauldParams(trans_tool_axis_params Params, f32 Scale)
{
	trans_tool_axis_params Result;

	Result.Axis.Len = Params.Axis.Len * Scale;
	Result.Axis.EdgeCenter = Params.Axis.EdgeCenter * Scale;
	Result.Axis.EdgeLenHalfSize = Params.Axis.EdgeLenHalfSize * Scale;
	Result.EdgeXYHalfSize = Params.EdgeXYHalfSize * Scale;
	Result.ArrowRadius = Params.ArrowRadius * Scale;

	return Result;
}

internal inline tools_axis_id
RayTranslateToolAxisTest(ray_params Ray, trans_tool_axis_params AxisParams,
	m3x3 Axis, v3 AxisOffset, f32 ZSignMod)
{
	tools_axis_id Result = ToolsAxisID_None;

	m4x4 InvAxis = Transpose(ToM4x4(Axis));
	v3 InvPos = -(AxisOffset * InvAxis);
	SetTranslation(&InvAxis, InvPos);

	v4 InvRayD = V4(Ray.Dir, 0) * InvAxis;
	v4 InvRayP = V4(Ray.P, 1.0f) * InvAxis;
	ray_params InvRay = CreateRay(InvRayP.xyz, InvRayD.xyz);

	m3x3 DefaultAxis = Identity3x3();
	DefaultAxis.Z *= ZSignMod;

	f32 ModArrowRadius = AxisParams.ArrowRadius + TOOL_ARROW_INTR_SCALE_FALCTOR;
	f32 ModEdgeXYHalfSize = AxisParams.EdgeXYHalfSize + TOOL_EDGE_INTR_SCALE_FALCTOR;

	v3 XEdgeHalfDim = V3(AxisParams.Axis.EdgeLenHalfSize, ModEdgeXYHalfSize, ModEdgeXYHalfSize);
	v3 YEdgeHalfDim = V3(ModEdgeXYHalfSize, AxisParams.Axis.EdgeLenHalfSize, ModEdgeXYHalfSize);
	v3 ZEdgeHalfDim = V3(ModEdgeXYHalfSize, ModEdgeXYHalfSize, AxisParams.Axis.EdgeLenHalfSize);
	
	rect3 XEdgeAABB = CreateRect(XEdgeHalfDim, DefaultAxis.X*AxisParams.Axis.EdgeCenter);
	rect3 YEdgeAABB = CreateRect(YEdgeHalfDim, DefaultAxis.Y*AxisParams.Axis.EdgeCenter);
	rect3 ZEdgeAABB = CreateRect(ZEdgeHalfDim, DefaultAxis.Z*AxisParams.Axis.EdgeCenter);

	b32 IsHitXEdge = RayAABBIntersect(InvRay, XEdgeAABB);
	b32 IsHitXArrow = RaySphereIntersect(InvRay, DefaultAxis.X*AxisParams.Axis.Len, ModArrowRadius);

	b32 IsHitYEdge = RayAABBIntersect(InvRay, YEdgeAABB);
	b32 IsHitYArrow = RaySphereIntersect(InvRay, DefaultAxis.Y*AxisParams.Axis.Len, ModArrowRadius);

	b32 IsHitZEdge = RayAABBIntersect(InvRay, ZEdgeAABB);
	b32 IsHitZArrow = RaySphereIntersect(InvRay, DefaultAxis.Z*AxisParams.Axis.Len, ModArrowRadius);

	if (IsHitXArrow || IsHitXEdge) Result = ToolsAxisID_X;
	if (IsHitYArrow || IsHitYEdge) Result = ToolsAxisID_Y;
	if (IsHitZArrow || IsHitZEdge) Result = ToolsAxisID_Z;

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

				Assert((Length(Result.Z) >= 0.98f) && (Length(Result.Z) <= 1.001f));
				Assert((Length(Result.Y) >= 0.98f) && (Length(Result.Y) <= 1.001f));
				Assert((Length(Result.X) >= 0.98f) && (Length(Result.X) <= 1.001f));
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
		case ToolType_Rotate:
		{
			rotate_tools *Rotate = &Tools->Rotate;
			*Rotate = {};

			Rotate->P = ComputeToolPos(Model, &Tools->UniqIndeces, Selected, IModel->Target);
			Rotate->InitRadius = ROTATE_TOOL_DIAMETER * 0.5f;
			Rotate->PerpThreshold = 0.95f;
		} break;

		case ToolType_Translate:
		{
			translate_tools *Trans = &Tools->Translate;
			*Trans = {};

			Trans->P = ComputeToolPos(Model, &Tools->UniqIndeces, Selected, IModel->Target);
			
			trans_tool_axis_params *InitAxisParams = &Trans->InitAxisParams;
			InitAxisParams->Axis.Len = TRANSLATE_TOOL_SIZE;
			InitAxisParams->Axis.EdgeLenHalfSize = (TRANSLATE_TOOL_SIZE * 0.85f) * 0.5f;
			InitAxisParams->Axis.EdgeCenter = InitAxisParams->Axis.Len - InitAxisParams->Axis.EdgeLenHalfSize;
			InitAxisParams->EdgeXYHalfSize = 0.004f;
			InitAxisParams->ArrowRadius = TRANSLATE_TOOL_ARROW_R;
		} break;

		case ToolType_Scale:
		{
			scale_tools *Scale = &Tools->Scale;
			*Scale = {};
			
			Scale->P = ComputeToolPos(Model, &Tools->UniqIndeces, Selected, IModel->Target);
			
			scl_tool_default_params *InitAxisParams = &Scale->InitAxisParams;
			InitAxisParams->Axis.Len = SCALE_TOOL_SIZE;
			InitAxisParams->Axis.EdgeLenHalfSize = (SCALE_TOOL_SIZE * 0.85f) * 0.5f;
			InitAxisParams->Axis.EdgeCenter = InitAxisParams->Axis.Len - InitAxisParams->Axis.EdgeLenHalfSize;
			InitAxisParams->EdgeXYHalfSize = 0.004f;
			InitAxisParams->ArrowHalfSize = SCALE_TOOL_SIZE * 0.045f;
		} break;


		InvalidDefaultCase;
	}

	Tools->IsInit = true;
}

// TODO: Apply transform only when exit move interaction
// for rotate and translate?
// TODO: Add interact quad for interact with 2 axis at the same time
// for translate (and scale?)
internal void inline
UpdateModelInteractionTools(game_editor_state *Editor, game_input *Input, render_group *RenderGroup)
{
	ui_interaction Interaction = {};
	editor_world_ui *WorldUI = &Editor->WorldUI;
	tools *Tools = &WorldUI->Tools;
	model *Model = Editor->Models + WorldUI->IModel.ID;

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
			ray_to_point_params PosRelParams =
				GetRayToPointRelParam(Ray.P, RotateTool->P, Tools->AdjustScaleDist);

			RotateTool->Radius = RotateTool->InitRadius * PosRelParams.ScaleFactor;
			RotateTool->FromPosToRayP = Normalize(PosRelParams.ToolToRayV, PosRelParams.LenV);

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
			
			RotateTool->PerpInfo = SetRotToolPerpInfo(Axis, RotateTool->PerpThreshold, RotateTool->FromPosToRayP);

			if (RotateTool->InteractAxis == ToolsAxisID_None)
			{
				RotateTool->AxisMask = {};
				v3i IntrPerpAxisBool = GetIntrPerpAxisInfo(RotateTool, Ray, Axis);
				v3i IntrOnSphereAxisBool = GetIntrOnSphereAxisInfo(RotateTool, Ray, Axis);

				tools_axis_id InteractAxis = SetRotIntrAxisInfo(RotateTool, Axis, IntrOnSphereAxisBool, IntrPerpAxisBool);

				Interaction = SetToolAxisIntr(ToolType_Rotate, UI_InteractionType_Select, InteractAxis);
				if ((InteractAxis != ToolsAxisID_None) && AreEqual(WorldUI->Interaction, Interaction))
				{
					if (InitRotToolProcessing(RotateTool, Ray, InteractAxis))
					{
						Interaction.TypeID = SetIntrTypeID(UI_InteractionTarget_Tools, UI_InteractionType_Move);
						WorldUI->Interaction = Interaction;
					}
				}
			}
			else
			{
				Interaction = SetToolAxisIntr(ToolType_Rotate, UI_InteractionType_Move, RotateTool->InteractAxis);

				if (AreEqual(WorldUI->Interaction, Interaction))
				{
					if (ProcessRotateToolTransform(RotateTool, Ray, Axis))
					{
						model_target_element TargetElement = (model_target_element)WorldUI->IModel.Target;
						if (TargetElement == ModelTargetElement_Model)
						{
							Model->Axis = RotateTool->Axis;
							Assert((Length(Model->Axis.Z) >= 0.98f) && (Length(Model->Axis.Z) <= 1.001f));
							Assert((Length(Model->Axis.Y) >= 0.98f) && (Length(Model->Axis.Y) <= 1.001f));
							Assert((Length(Model->Axis.X) >= 0.98f) && (Length(Model->Axis.X) <= 1.001f));
						}

						if (!RotateTool->DefaultAxisSet)
						{
							Axis = RotateTool->Axis;
						}

						ApplyRotation(Model, &Tools->UniqIndeces, TargetElement,
							RotateTool->P, RotateTool->Transform);
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

			PushRotateTool(RenderGroup, Editor->StaticMesh[BuiltInMesh_RotateToolSphere].Mesh, RotateTool->P,
				PosRelParams.ScaleFactor, Axis, RotateTool->AxisMask, RotateTool->PerpInfo.V,
				RotateTool->FromPosToRayP);
		} break;

		case ToolType_Scale:
		{
			scale_tools *ScaleTool = &Tools->Scale;

			ray_to_point_params PosRelParams =
				GetRayToPointRelParam(Ray.P, ScaleTool->P, Tools->AdjustScaleDist);
			
			scl_tool_default_params ScaleAxisParams =
				ModScaleToolDefauldParams(ScaleTool->InitAxisParams, PosRelParams.ScaleFactor);
			
			m3x3 Axis;
			if (ScaleTool->InteractAxis == ToolsAxisID_None)
			{
				ScaleTool->AxisMask = {};

				if (IsDown(Input->Ctrl))
				{
					Axis = Identity3x3();
				}
				else
				{
					Axis = SetAxisForTool(Model, &WorldUI->Selected, WorldUI->IModel.Target);
				}

				f32 ZSignMod = 1.0f; //Sign(Dot(Axis.Z, RayPCenterP));
				//Axis.Z *= ZSignMod;

				tools_axis_id InteractAxis = RayScaleToolAxisTest(Ray, ScaleAxisParams, Axis, ScaleTool->P, ZSignMod);
				if (InteractAxis)
				{
					ScaleTool->AxisMask.E[(u32)InteractAxis - 1] = 1.0f;
				}

				Interaction = SetToolAxisIntr(ToolType_Scale, UI_InteractionType_Select, InteractAxis);
				if ((InteractAxis != ToolsAxisID_None) && AreEqual(WorldUI->Interaction, Interaction))
				{
					ScaleTool->Axis = Axis;
					ScaleTool->AxisMask.w = 1.0f;
					ScaleTool->InteractAxis = InteractAxis;

					Interaction.TypeID = SetIntrTypeID(UI_InteractionTarget_Tools, UI_InteractionType_Move);
					WorldUI->Interaction = Interaction;
				}

				ScaleTool->DisplayState = SetDefaultDisplayParams(ScaleAxisParams);
			}
			else
			{
				Axis = ScaleTool->Axis;
				Interaction = SetToolAxisIntr(ToolType_Scale, UI_InteractionType_Move, ScaleTool->InteractAxis);

				if (AreEqual(WorldUI->Interaction, Interaction))
				{
					if (ProcessScaleToolTransform(ScaleTool, Ray))
					{
						model_target_element TargetElement = (model_target_element)WorldUI->IModel.Target;
						ApplyScale(Model, &Tools->UniqIndeces, TargetElement, ScaleTool->P,
							ScaleTool->ScaleParam);
					}
				
					SetDefaultDisplayParams(&ScaleTool->DisplayState, ScaleAxisParams, ScaleTool->InteractAxis);
				}
				else
				{
					ScaleTool->BeginP = 0;
					ScaleTool->PrevP = 0;
					ScaleTool->InteractAxis = ToolsAxisID_None;
					ScaleTool->EnterActiveState = false;
					ScaleTool->DisplayState = SetDefaultDisplayParams(ScaleAxisParams);
				}
			}

			PushScaleTool(RenderGroup, ScaleTool->P, Axis, ScaleTool->DisplayState, ScaleTool->AxisMask);
		} break;
		case ToolType_Translate:
		{
			translate_tools *TransTool = &Tools->Translate;

			ray_to_point_params PosRelParams =
				GetRayToPointRelParam(Ray.P, TransTool->P, Tools->AdjustScaleDist);

			trans_tool_axis_params ScaleAxisParams =
				ModTransToolDefauldParams(TransTool->InitAxisParams, PosRelParams.ScaleFactor);

			m3x3 Axis;
			if (TransTool->InteractAxis == ToolsAxisID_None)
			{
				TransTool->AxisMask = {};

				if (IsDown(Input->Ctrl))
				{
					Axis = Identity3x3();
				}
				else
				{
					Axis = SetAxisForTool(Model, &WorldUI->Selected, WorldUI->IModel.Target);
				}

				f32 ZSignMod = 1.0f; //Sign(Dot(Axis.Z, RayPCenterP));
				//Axis.Z *= ZSignMod;

				tools_axis_id InteractAxis = RayTranslateToolAxisTest(Ray, ScaleAxisParams, Axis, TransTool->P, ZSignMod);
				if (InteractAxis)
				{
					TransTool->AxisMask.E[(u32)InteractAxis - 1] = 1.0f;
				}

				Interaction = SetToolAxisIntr(ToolType_Translate, UI_InteractionType_Select, InteractAxis);
				if ((InteractAxis != ToolsAxisID_None) && AreEqual(WorldUI->Interaction, Interaction))
				{
					TransTool->Axis = Axis;
					TransTool->AxisMask.w = 1.0f;
					TransTool->InteractAxis = InteractAxis;

					Interaction.TypeID = SetIntrTypeID(UI_InteractionTarget_Tools, UI_InteractionType_Move);
					WorldUI->Interaction = Interaction;
				}
			}
			else
			{
				Axis = TransTool->Axis;
				Interaction = SetToolAxisIntr(ToolType_Translate, UI_InteractionType_Move, TransTool->InteractAxis);

				if (AreEqual(WorldUI->Interaction, Interaction))
				{
					if (ProcessTransToolTransform(TransTool, Ray))
					{
						model_target_element TargetElement = (model_target_element)WorldUI->IModel.Target;
						ApplyTranslate(Model, &Tools->UniqIndeces, TargetElement, TransTool->TransParam);
					}
				}
				else
				{
					TransTool->BeginP = 0;
					TransTool->PrevP = 0;
					TransTool->InteractAxis = ToolsAxisID_None;
					TransTool->EnterActiveState = false;
				}
			}

			PushTranslateTool(RenderGroup, ScaleAxisParams, Axis, TransTool->AxisMask,
				TransTool->P, PosRelParams.ScaleFactor, Editor->StaticMesh[BuiltInMesh_TranslateArrow].Mesh);
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
