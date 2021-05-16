
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

inline ui_interaction
SetSplitSelectIntr(u32 ModelID)
{
	ui_interaction Result;
	Result.TypeID = SetIntrTypeID(UI_InteractionTarget_Tools, UI_InteractionType_Select);
	Result.ID.ID[0] = ModelID;
	Result.ID.ID[1] = 0;

	return Result;
}

inline b32
AreEqual(ui_interaction A, ui_interaction B)
{
	b32 Result = AreEqual(A.ID, B.ID) && (A.TypeID == B.TypeID);
	return Result;
}

inline u32
GetIntAxisID(tools_axis_id AxisID)
{
	u32 Result = (u32)AxisID - 1;
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
			// TODO: Add FreeToolInfo func?
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

// TODO: return chek if ptr to _model_ struct is supply?
void
AddToSelectedBuffer(element_id_buffer *Selected, u32 ElementID, u32 ITarget, b32 ShiftDown)
{
	if (ShiftDown && Selected->Count)
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
						Selected->Elements[Index] = Selected->Elements[--ElementCount];
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
			if (RayModelsIntersect(&Editor->MainArena, Editor->WorkModels, Editor->WorkModelsCount,
				WorldUI->MouseRay, &IModel->ID, &IModel->Face))
			{
				Interaction = SetModelSelectInteraction(IModel->ID, 0, 0, IModel->Target);

				if (AreEqual(Interaction, WorldUI->ToExecute))
				{
					if (IsDown(Input->Ctrl))
					{
						work_model *Model = Editor->WorkModels + WorldUI->IModel.ID;
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
			work_model *Model = Editor->WorkModels + WorldUI->IModel.ID;
			IModel->Face = {};

			if (RayAABBIntersect(WorldUI->MouseRay, Model->AABB, Model->Offset))
			{
				if (RayModelFacesIntersect(Model, WorldUI->MouseRay, &IModel->Face))
				{
					Interaction = SetModelSelectInteraction(IModel, IModel->Target);

					if (AreEqual(Interaction, WorldUI->ToExecute))
					{
						AddToSelectedBuffer(&WorldUI->Selected, IModel->Face.ID, IModel->Target, IsDown(Input->Shift));
					}
				}
			}
		} break;

		case ModelTargetElement_Edge:
		{
			work_model *Model = Editor->WorkModels + WorldUI->IModel.ID;
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
						AddToSelectedBuffer(&WorldUI->Selected, IModel->Edge.ID, IModel->Target, IsDown(Input->Shift));
					}
				}

			}
		} break;
	}

	WorldUI->NextHotInteraction = Interaction;
}

// TODO: Delete or change implementation
v3
ComputeToolPos(work_model *Model, element_id_buffer *UniqIndeces,
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
				model_face *Face = Model->Data.Faces + FaceIndex;

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

				v3 V = Model->Data.Vertices[VertexIndex] + ModelOffset;
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
				model_edge *Edge = Model->Data.Edges + EdgeIndex;

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

				v3 V = Model->Data.Vertices[VertexIndex] + ModelOffset;
				Result += V;
			}

			Result /= (f32)UniqIndeces->Count;
		} break;
	}

	return Result;
}


// TODO: Pass struct ptr to _Apply_ functions
// TODO: Optimizey
void
ApplyRotation(work_model *Model, element_id_buffer *UniqIndeces,
	model_target_element ElementTarget, v3 RotationOrigin, m4x4 Rotation)
{
	v3 *SourceVertices = Model->Source->Vertices;
	v3 *DisplayVertices = Model->Data.Vertices;
	vertex_transform_state *TransStates = Model->VertexTrans;
	
	v3 ModelSpaceRotOrigin = RotationOrigin - Model->Offset;

	switch (ElementTarget)
	{
		case ModelTargetElement_Model:
		{
			for (u32 Index = 0;
				Index < Model->Data.VertexCount;
				++Index)
			{
				v3 VSource = SourceVertices[Index];
				vertex_transform_state *Trans = TransStates + Index;

				m4x4 Scale = ToM4x4(Trans->S);
				m4x4 CurrRotation = ConvertQuatToM4x4(Trans->R);
				m4x4 Translate = TranslateMat(Trans->T);

#if 0
				quat ConvQuat = Normalize(ConvertMatToQuat(CurrRotation));
				m4x4 ConvMath = ConvertQuatToMat(ConvQuat);
#endif
				v3 RotOrigin = ModelSpaceRotOrigin - Trans->T;
				m4x4 RelativeRotation = TranslateMat(-RotOrigin) * Rotation * TranslateMat(RotOrigin);

				CurrRotation = CurrRotation * RelativeRotation;
				
				m4x4 Transform = Scale * CurrRotation * Translate;

				Trans->R = Normalize(ConvertMatToQuat(CurrRotation));
				Trans->T = Transform.Row3.xyz;

				DisplayVertices[Index] = VSource * Transform;
			}
		} break;

		case ModelTargetElement_Edge:
		case ModelTargetElement_Face:
		{
			// TODO: add scale process
			for (u32 Index = 0;
				Index < UniqIndeces->Count;
				++Index)
			{
				u32 VertexIndex = UniqIndeces->Elements[Index];

				v3 VSource = SourceVertices[VertexIndex];
				vertex_transform_state *Trans = TransStates + VertexIndex;

				m4x4 Scale = ToM4x4(Trans->S);
				m4x4 Translate = TranslateMat(Trans->T);
				m4x4 CurrRotation = ConvertQuatToM4x4(Trans->R);

				v3 RotOrigin = ModelSpaceRotOrigin - Trans->T;
				m4x4 RelativeRotation = TranslateMat(-RotOrigin) * Rotation * TranslateMat(RotOrigin);

				CurrRotation = CurrRotation * RelativeRotation;

				m4x4 Transform = Scale * CurrRotation * Translate;

				Trans->R = Normalize(ConvertMatToQuat(CurrRotation));
				Trans->T = Transform.Row3.xyz;

				DisplayVertices[VertexIndex] = VSource * Transform;
			}
		} break;
	}

	Model->AABB = ComputeMeshAABB(Model->Data.Vertices, Model->Data.VertexCount);
}

#define SCALE_SPEED 0.4f
// TODO: Optimize!!!
void
ApplyScale(work_model *Model, scale_tools *Tool, element_id_buffer *UniqIndeces,
	model_target_element TargetElement, b32 IsGlobalSpace)
{
	model_data *SourceModel = Model->Source;
	v3 *SourceVertices = SourceModel->Vertices;
	v3 *DisplayVertices = Model->Data.Vertices;
	vertex_transform_state *TransStates = Model->VertexTrans;

	f32 ScaleFactor = Tool->ScaleParam.w * SCALE_SPEED;
	u32 AxisID = GetIntAxisID(Tool->InteractAxis);

	v3 ScaleV = {};
	ScaleV.E[AxisID] = 1.0f * ScaleFactor;
	ScaleV += V3(1.0f);

	switch (TargetElement)
	{
		case ModelTargetElement_Model:
		{
			for (u32 Index = 0;
				Index < Model->Data.VertexCount;
				++Index)
			{
				v3 VSource = SourceVertices[Index];
				vertex_transform_state *Trans = TransStates + Index;

				m4x4 CurrScale = ToM4x4(Trans->S);
				m4x4 Rotation = ConvertQuatToM4x4(Trans->R);
				m4x4 Translate = TranslateMat(Trans->T);

				m4x4 ApplyScale;
				if (IsGlobalSpace)
				{
					m4x4 MAxis = Rotation;
					m4x4 InvRot = Transpose(MAxis);
					ApplyScale = ScaleMat(ScaleV);
					ApplyScale = MAxis * ApplyScale * InvRot;
				}
				else
				{
					ApplyScale = ScaleMat(ScaleV);
				}

				CurrScale = CurrScale * ApplyScale;

				m4x4 Transform = CurrScale * Rotation * Translate;

				Trans->S = ToM3x3(CurrScale);
				Trans->T = Transform.Row3.xyz;

				DisplayVertices[Index] = VSource * Transform;
			}
		} break;

		case ModelTargetElement_Face:
		case ModelTargetElement_Edge:
		{
			v3 ToolOffset = Tool->P - Model->Offset;

			for (u32 Index = 0;
				Index < UniqIndeces->Count;
				++Index)
			{
				u32 VertexIndex = UniqIndeces->Elements[Index];

				v3 VSource = SourceVertices[VertexIndex];
				vertex_transform_state *Trans = TransStates + VertexIndex;

				m4x4 CurrScale = ToM4x4(Trans->S);
				m4x4 Rotation = ConvertQuatToM4x4(Trans->R);
				m4x4 Translate = TranslateMat(Trans->T);

				m4x4 MAxis = Rotation;
				m4x4 InvRot = Transpose(MAxis);

				m4x4 ApplyScale;
				if (IsGlobalSpace)
				{
					ApplyScale = ScaleMat(ScaleV);
					ApplyScale = MAxis * ApplyScale * InvRot;
				}
				else
				{
					m4x4 TAxis = ToM4x4(Tool->Axis) * InvRot;
					m4x4 InvTAxis = Transpose(TAxis);

					ApplyScale = ScaleMat(ScaleV);
					ApplyScale = InvTAxis * ApplyScale * TAxis;
				}

				v3 ScaleOrigin = (ToolOffset - Trans->T) * InvRot;

				m4x4 NegTrans = TranslateMat(-ScaleOrigin);
				m4x4 PosTrans = TranslateMat(ScaleOrigin);
				m4x4 ScaleTransform = NegTrans * ApplyScale * PosTrans;

				CurrScale = CurrScale * ScaleTransform;

				m4x4 Transform = CurrScale * Rotation * Translate;

				Trans->S = ToM3x3(CurrScale);
				Trans->T = Transform.Row3.xyz;

				v3 TransResult = VSource * Transform;

				DisplayVertices[VertexIndex] = TransResult;
			}
		} break;
	}

	Model->AABB = ComputeMeshAABB(Model->Data.Vertices, Model->Data.VertexCount);
}

// TODO: Optimize
void
ApplyTranslate(work_model *Model, element_id_buffer *UniqIndeces,
	translate_tools *TransTool, model_target_element ElementTarget, b32 IsGlobalSpace)
{
	v3 TransDir = TransTool->TransParam.xyz;
	v3 *DisplayVertices = Model->Data.Vertices;

	switch (ElementTarget)
	{
		case ModelTargetElement_Model:
		{
			v3 Translate = TransDir * TransTool->TransParam.w;
			Model->Offset += Translate;
		} break;

		case ModelTargetElement_Edge:
		case ModelTargetElement_Face:
		{
			v3 *Source = Model->Source->Vertices;
			vertex_transform_state *TransState = Model->VertexTrans;
			
			v3 ResultTranslate = TransDir * TransTool->TransParam.w;
			
			for (u32 Index = 0;
				Index < UniqIndeces->Count;
				++Index)
			{
				u32 VertexIndex = UniqIndeces->Elements[Index];

				v3 VSource = Source[VertexIndex];
				vertex_transform_state *Trans = TransState + VertexIndex;

				Trans->T += ResultTranslate;

				DisplayVertices[VertexIndex] += ResultTranslate;
			}

			Model->AABB = ComputeMeshAABB(Model->Data.Vertices, Model->Data.VertexCount);
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

	f32 tRay = RayPlaneIntersect(Ray, Tool->InteractPlane);

	v3 CurrentVector;
	if (Abs(RPosDotPlaneN) <= RTOOLS_AXIS_INTERACT_THRESHOLD)
	{
		v3 PointOnSphere;
		if (RaySphereIntersect(Ray, Tool->P, Tool->Radius, &PointOnSphere))
		{
			v3 DirFromCenter = Normalize(PointOnSphere - Tool->P);
			f32 PDotD = Dot(Tool->InteractPlane.N, DirFromCenter);

			CurrentVector = Normalize(DirFromCenter - (Tool->InteractPlane.N * PDotD));
#if DEBUG_MODE
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
		// TODO: make cmp more correct
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
				m4x4 CurrentRotateAxis = ToM4x4(Axis);
				m4x4 InvCurrentAxis = Transpose(CurrentRotateAxis);

				m4x4 ResultAxis = Rotate * CurrentRotateAxis;
				m4x4 ResultTransform = InvCurrentAxis * ResultAxis;

				m4x4 ToolCurrentAxis;
				if (Tool->DefaultAxisSet)
				{
					ToolCurrentAxis = ToM4x4(Tool->Axis);
					ToolCurrentAxis = ToolCurrentAxis * Rotate;
				}
				else
				{
					ToolCurrentAxis = ResultAxis;
				}

				Tool->Transform = ResultTransform;
				Tool->PrevVector = CurrentVector;
				Tool->Axis.X = GetRow(ToolCurrentAxis, 0);
				Tool->Axis.Y = GetRow(ToolCurrentAxis, 1);
				Tool->Axis.Z = GetRow(ToolCurrentAxis, 2);

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
	if (ADotC >= PerpThreshold)
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
		Result.IsSet = true;

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
	plane_params Plane = CreatePlane(Axis, Dot(Axis, ToolCenterP));

	f32 tRay = RayPlaneIntersect(Ray, Plane);
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

	f32 ModEdgeXYHalfSize = AxisParams.EdgeXYHalfSize + TOOL_EDGE_INTR_SCALE_FALCTOR;
	v3 ArrowDim = V3(AxisParams.ArrowHalfSize + TOOL_ARROW_INTR_SCALE_FALCTOR);

	v3 XEdgeHalfDim = V3(AxisParams.Axis.EdgeLenHalfSize, ModEdgeXYHalfSize, ModEdgeXYHalfSize);
	v3 YEdgeHalfDim = V3(ModEdgeXYHalfSize, AxisParams.Axis.EdgeLenHalfSize, ModEdgeXYHalfSize);
	v3 ZEdgeHalfDim = V3(ModEdgeXYHalfSize, ModEdgeXYHalfSize, AxisParams.Axis.EdgeLenHalfSize);

	rect3 XArrowAABB = CreateRect(ArrowDim, DefaultAxis.X*AxisParams.Axis.Len);
	rect3 YArrowAABB = CreateRect(ArrowDim, DefaultAxis.Y*AxisParams.Axis.Len);
	rect3 ZArrowAABB = CreateRect(ArrowDim, DefaultAxis.Z*AxisParams.Axis.Len);

	rect3 XEdgeAABB = CreateRect(XEdgeHalfDim, DefaultAxis.X*AxisParams.Axis.EdgeCenter);
	rect3 YEdgeAABB = CreateRect(YEdgeHalfDim, DefaultAxis.Y*AxisParams.Axis.EdgeCenter);
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

internal b32
ProcessScaleToolTransform(scale_tools *Tool, ray_params Ray)
{
	b32 Result = false;

	u32 IntrAxisID = GetIntAxisID(Tool->InteractAxis);
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

		// TODO: Redundant?
		ActiveAxis->EdgeCenter = CurrentP * 0.5f;
		ActiveAxis->EdgeLenHalfSize = ActiveAxis->EdgeCenter;

		if (Tool->PrevP != CurrentP)
		{
			f32 ScaleFactor = (CurrentP - Tool->PrevP);

			// TODO: Remove later
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

	u32 IntrAxisID = GetIntAxisID(Tool->InteractAxis);
	v3 IntrAxis = Tool->Axis.Row[IntrAxisID];

	if (!Tool->IsPlaneIntr)
	{
		ray_params AxisRay = CreateRay(Tool->P, IntrAxis);

		f32 CurrentP = 0;
		if (ClosestPBeetwenRay(Ray, AxisRay, 0, &CurrentP))
		{
			// TODO: Move to set _move_ interaction?
			if (!Tool->EnterActiveState)
			{
				Tool->Intr.P.Begin = CurrentP;
				Tool->Intr.P.Prev = CurrentP;
				Tool->EnterActiveState = true;
			}

			if (Tool->Intr.P.Prev != CurrentP)
			{
				f32 TransFactor = (CurrentP - Tool->Intr.P.Prev);
				Tool->TransParam.xyz = IntrAxis;
				Tool->TransParam.w = TransFactor;
				
				Tool->P += Tool->TransParam.xyz * Tool->TransParam.w;
				Tool->Intr.P.Prev = CurrentP - TransFactor;
				Result = true;
			}
		}
	}
	else
	{
		plane_params Plane = CreatePlane(IntrAxis, Dot(IntrAxis, Tool->P));
		f32 tPlane = RayPlaneIntersect(Ray, Plane);

		if (tPlane >= 0)
		{
			v3 CurrentV = PointOnRay(Ray, tPlane);

			if (!Tool->EnterActiveState)
			{
				Tool->Intr.V.Begin = CurrentV;
				Tool->Intr.V.Prev = CurrentV;
				Tool->EnterActiveState = true;
			}

			if (Tool->Intr.V.Prev != CurrentV)
			{
				v3 TransV = CurrentV - Tool->Intr.V.Prev;
				f32 MoveLen = Length(TransV);
				v3 MoveDir = Normalize(TransV, MoveLen);

				Tool->TransParam.xyz = MoveDir;
				Tool->TransParam.w = MoveLen;

				Tool->P += TransV;
				Tool->Intr.V.Prev = CurrentV;
				Result = true;
			}
		}
	}

	return Result;
}

internal inline trans_tool_axis_params
ModTransToolDefaultParams(trans_tool_axis_params Params, f32 Scale)
{
	trans_tool_axis_params Result;

	Result.Axis.Len = Params.Axis.Len * Scale;
	Result.Axis.EdgeCenter = Params.Axis.EdgeCenter * Scale;
	Result.Axis.EdgeLenHalfSize = Params.Axis.EdgeLenHalfSize * Scale;
	Result.EdgeXYHalfSize = Params.EdgeXYHalfSize * Scale;
	Result.ArrowRadius = Params.ArrowRadius * Scale;
	Result.PlaneRelDim = Params.PlaneRelDim * Scale;

	return Result;
}

internal inline tools_axis_id
RayTransToolAxisTest(ray_params Ray, trans_tool_axis_params AxisParams,
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

// NOTE: Axis return in global space
// TODO: Keep axis in axis aligned view?
m3x3
SetAxisForTool(work_model *Model, element_id_buffer *Selected, u32 ElementTarget)
{
	m3x3 Result = {};

	if (ElementTarget == ModelTargetElement_Model)
	{
		Result = Model->Axis;
	}
	else if (Selected->Count > 1)
	{
		// TODO: Extend for local and normal axis display mod
		Result = Identity3x3();
	}
	else if (Selected->Count == 1)
	{
		u32 ElementID = Selected->Elements[0];
		switch (ElementTarget)
		{
			case ModelTargetElement_Edge:
			{
				model_edge *Edge = Model->Data.Edges + ElementID;
				edge_faces_norm RelatedNorm = GetEdgeFacesRelatedNormals(Model, Edge);
				v3 V0 = Model->Data.Vertices[Edge->V0];
				v3 V1 = Model->Data.Vertices[Edge->V1];

				Result.Z = Normalize(V1 - V0);
				Result.Y = NLerp(RelatedNorm.N0, 0.5f, RelatedNorm.N1);
				Result.X = Cross(Result.Y, Result.Z);
			} break;

			case ModelTargetElement_Face:
			{
				model_face *Face = Model->Data.Faces + ElementID;
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

// TODO: Add sign mod to Z axis
internal tools_axis_id
RayTransToolAxisPlaneTest(ray_params Ray, trans_tool_axis_params ScaleAxisParams,
	m3x3 Axis, v3 Pos, f32 ZSignMod)
{
	tools_axis_id Result = ToolsAxisID_None;

	//m3x3 DefaultAxis = Identity3x3();
	//DefaultAxis.Z *= ZSignMod;

	plane_params XPlane = CreatePlane(Axis.X, Dot(Axis.X, Pos));
	plane_params YPlane = CreatePlane(Axis.Y, Dot(Axis.Y, Pos));
	plane_params ZPlane = CreatePlane(Axis.Z, Dot(Axis.Z, Pos));

	v3 tP;
	tP.x = RayPlaneIntersect(Ray, XPlane);
	tP.y = RayPlaneIntersect(Ray, YPlane);
	tP.z = RayPlaneIntersect(Ray, ZPlane);

	// NOTE: Unrolling bubble sort
	f32 tMin[3];
	tMin[0] = Min(tP.x, Min(tP.y, tP.z));
	tMin[1] = Min(Min(Max(tP.x, tP.y), Max(tP.x, tP.z)), Max(tP.y, tP.z));
	tMin[2] = Max(tP.x, Max(tP.y, tP.z));

	f32 PlaneDim = ScaleAxisParams.PlaneRelDim;
	for (u32 MinIndex = 0; MinIndex < ArrayCount(tP.E); ++MinIndex)
	{
		f32 CurrMin = tMin[MinIndex];

		if (CurrMin >= 0)
		{
			tools_axis_id ToTest;
			for (u32 Index = 0; Index < ArrayCount(tP.E); ++Index)
			{
				if (tP.E[Index] == CurrMin)
					ToTest = (tools_axis_id)(Index + 1);
			}

			switch (ToTest)
			{
				case ToolsAxisID_X:
				{
					v3 XPoint = PointOnRay(Ray, tP.x) - Pos;
					f32 YLen = Dot(XPoint, Axis.Y);
					f32 ZLen = Dot(XPoint, Axis.Z);

					if (((YLen > 0) && (ZLen > 0)) &&
						((YLen <= PlaneDim) && (ZLen <= PlaneDim)))
					{
						Result = ToolsAxisID_X;
					}
				} break;

				case ToolsAxisID_Y:
				{
					v3 YPoint = PointOnRay(Ray, tP.y) - Pos;
					f32 XLen = Dot(YPoint, Axis.X);
					f32 ZLen = Dot(YPoint, Axis.Z);

					if (((XLen > 0) && (ZLen > 0)) &&
						((XLen <= PlaneDim) && (ZLen <= PlaneDim)))
					{
						Result = ToolsAxisID_Y;
					}
				} break;

				case ToolsAxisID_Z:
				{
					v3 ZPoint = PointOnRay(Ray, tP.z) - Pos;
					f32 XLen = Dot(ZPoint, Axis.X);
					f32 YLen = Dot(ZPoint, Axis.Y);

					if (((XLen > 0) && (YLen > 0)) &&
						((XLen <= PlaneDim) && (YLen <= PlaneDim)))
					{
						Result = ToolsAxisID_Z;
					}
				} break;
			}

			if (Result) break;
		}
	}

	return Result;
}

// TODO: Add more tools!!!
void
InitTools(editor_world_ui *WorldUI, tools *Tools, work_model *Model, page_memory_arena *PageArena)
{
	element_id_buffer *Selected = &WorldUI->Selected;
	interact_model *IModel = &WorldUI->IModel;

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
			InitAxisParams->PlaneRelDim = TRANSLATE_TOOL_SIZE * 0.18f;
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

		case ToolType_Split:
		{
			Tools->Split = {};

			if (!Tools->SplitBuffer.Elem)
			{
				Tools->SplitBuffer.MaxCount = 100;
				PagePushArray(PageArena, split_buffer_element, Tools->SplitBuffer.MaxCount, Tools->SplitBuffer.Elem, 0);
			}
		} break;

		InvalidDefaultCase;
	}

	Tools->IsInit = true;
}

inline point_to_edge_proj
GetStartEdgeForSplit(ray_params MouseRay, work_model *Model, element_ray_result *Face)
{
	point_to_edge_proj Result = {};

	if (RayAABBIntersect(MouseRay, Model->AABB, Model->Offset))
	{
		if (RayModelFacesIntersect(Model, MouseRay, Face))
		{
			Result = GetClosestEdgeToPointOnFace(Model, Face->ID, Face->P);
		}
	}

	return Result;
}

inline void
PushElementToSplitBuff(page_memory_arena *PageArena, split_buffer *Buffer, split_buffer_element Elem)
{
	if (Buffer->Count == Buffer->MaxCount)
	{
		PagePushArray(PageArena, split_buffer_element, Buffer->MaxCount, Buffer->Elem, 0);
		Buffer->MaxCount *= 2;
	}
	
	Buffer->Elem[Buffer->Count++] = Elem;
}

inline void
SetSplitToolData(split_tool *Split, split_buffer *SplitBuffer, model_data *Data,
	u32 StartFaceID, page_memory_arena *PageArena)
{
	f32 tSplit = Split->StartEdge.t;

	v3 *Vertices = Data->Vertices;
	model_edge *ModelEdges = Data->Edges;

	u32 StartEdgeID = Split->StartEdge.ID;

	u32 CurrentFaceID = StartFaceID;
	u32 CurrentEdgeID = StartEdgeID;

	__m128i Zero_4x = _mm_setzero_si128();
	__m128i One_4x = _mm_set1_epi32(1);

	model_edge *Edge = Split->StartEdge.Edge;
	u32 FromVertexEdgeIndex = 0;

	b32 LoopNotFull = true;
	while (LoopNotFull)
	{
		split_buffer_element Elem;
		Elem.EdgeID = CurrentEdgeID;
		Elem.Faces = Edge->Faces;

		u32 ToVertexEdgeIndex = !FromVertexEdgeIndex;

		u32 FromVertexID = Edge->VertexID[FromVertexEdgeIndex];
		u32 ToVertexID = Edge->VertexID[ToVertexEdgeIndex];

		v3 V0 = Vertices[FromVertexID];
		v3 V1 = Vertices[ToVertexID];

		Elem.V = Lerp(V0, tSplit, V1);

		PushElementToSplitBuff(PageArena, SplitBuffer, Elem);

		__m128i TestEdge = _mm_load_si128((__m128i *)Edge);

		model_face *Face = Data->Faces + CurrentFaceID;
		__m128i Edge0 = _mm_load_si128((__m128i *)(ModelEdges + Face->EdgesID[0]));
		__m128i Edge1 = _mm_load_si128((__m128i *)(ModelEdges + Face->EdgesID[1]));
		__m128i Edge2 = _mm_load_si128((__m128i *)(ModelEdges + Face->EdgesID[2]));
		__m128i Edge3 = _mm_load_si128((__m128i *)(ModelEdges + Face->EdgesID[3]));

		TestEdge = ShuffleU32(TestEdge, 0, 1, 1, 0);

		__m128i FromVertexID_4x = _mm_set1_epi32(FromVertexID);
		__m128i TestEdgeVertexIDIndex = _mm_cmpeq_epi32(FromVertexID_4x, TestEdge);

		u32 TestVertexID = iToMaskU32(TestEdgeVertexIDIndex);
		u32 TestVertexIDMask0 = TestVertexID & 0x3;
		u32 TestVertexIDMask1 = TestVertexID & 0xC;

		__m128i TestVertexIDMask0_4x = _mm_set1_epi32(TestVertexIDMask0);
		__m128i TestVertexIDMask1_4x = _mm_set1_epi32(TestVertexIDMask1);

		Edge0 = ShuffleU32(Edge0, 0, 1, 0, 1);
		Edge1 = ShuffleU32(Edge1, 0, 1, 0, 1);
		Edge2 = ShuffleU32(Edge2, 0, 1, 0, 1);
		Edge3 = ShuffleU32(Edge3, 0, 1, 0, 1);

		u32 OppositeEdgeIndex = UINT_MAX;

		u32 MaskArr[4];
		u32 SetBitArr[4];

		__m128i CmpMask0 = _mm_cmpeq_epi32(Edge0, TestEdge);
		__m128i CmpMask1 = _mm_cmpeq_epi32(Edge1, TestEdge);
		__m128i CmpMask2 = _mm_cmpeq_epi32(Edge2, TestEdge);
		__m128i CmpMask3 = _mm_cmpeq_epi32(Edge3, TestEdge);

		u32 Mask0 = iToMaskU32(CmpMask0);
		u32 Mask1 = iToMaskU32(CmpMask1);
		u32 Mask2 = iToMaskU32(CmpMask2);
		u32 Mask3 = iToMaskU32(CmpMask3);

		MaskArr[0] = Mask0;
		MaskArr[1] = Mask1;
		MaskArr[2] = Mask2;
		MaskArr[3] = Mask3;

		SetBitArr[0] = CountOfSetBits(Mask0);
		SetBitArr[1] = CountOfSetBits(Mask1);
		SetBitArr[2] = CountOfSetBits(Mask2);
		SetBitArr[3] = CountOfSetBits(Mask3);

		__m128i Mask_4x = _mm_load_si128((__m128i *)MaskArr);
		__m128i SetBit_4x = _mm_load_si128((__m128i *)SetBitArr);

		__m128i OppositeMask = _mm_cmpeq_epi32(SetBit_4x, Zero_4x);
		u32 OppositeMaskU32 = iToMaskU32(OppositeMask);

		Assert(CountOfSetBits(OppositeMaskU32) == 1);

		bit_scan_result OppositeIndex = FindLeastSignificantSetBit(OppositeMaskU32);
		Assert(OppositeIndex.Succes);

		__m128i OneMatchMask = _mm_cmpeq_epi32(SetBit_4x, One_4x);
		__m128i OneMatchEdge = _mm_and_si128(Mask_4x, OneMatchMask);

		__m128i OneMatchTest0 = _mm_cmpeq_epi32(OneMatchEdge, TestVertexIDMask0_4x);
		__m128i OneMatchTest1 = _mm_cmpeq_epi32(OneMatchEdge, TestVertexIDMask1_4x);

		u32 OneMatchTestMask = iToMaskU32(_mm_or_si128(OneMatchTest0, OneMatchTest1));
		bit_scan_result OneMatchTestIndex = FindLeastSignificantSetBit(OneMatchTestMask);
		Assert(OneMatchTestIndex.Succes);

		//TODO: Change?
		u32 OppositeEdgeID = Face->EdgesID[OppositeIndex.Index];
		model_edge *Opposite = ModelEdges + OppositeEdgeID;
		model_edge *Common = ModelEdges + Face->EdgesID[OneMatchTestIndex.Index];

		edge_vertex_match CommonVertex = MatchEdgeVertex(Opposite, Common);
		Assert(CommonVertex.Succes);

		Edge = Opposite;
		FromVertexEdgeIndex = CommonVertex.Index;
		CurrentEdgeID = OppositeEdgeID;
		CurrentFaceID = (Opposite->Face0 == CurrentFaceID) ? Opposite->Face1 : Opposite->Face0;

		LoopNotFull = (StartEdgeID != OppositeEdgeID);
	}
}

void
ApplySplit(work_model *Model, split_buffer *SplitBuffer)
{

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
	work_model *Model = Editor->WorkModels + WorldUI->IModel.ID;

	if (!Tools->IsInit)
	{
		InitTools(WorldUI, Tools, Model, &Editor->PageArena);
	}

	ray_params Ray = WorldUI->MouseRay;
	switch (Tools->Type)
	{
		// TODO: Implement drawing progres angle
		case ToolType_Rotate:
		{
			rotate_tools *RotateTool = &Tools->Rotate;
			Tools->IsGlobalSpace = IsDown(Input->Ctrl);

			ray_to_point_params PosRelParams =
				GetRayToPointRelParam(Ray.P, RotateTool->P, Tools->AdjustScaleDist);

			RotateTool->Radius = RotateTool->InitRadius * PosRelParams.ScaleFactor;
			RotateTool->FromPosToRayP = Normalize(PosRelParams.ToolToRayV, PosRelParams.LenV);

			m3x3 Axis;
			if (Tools->IsGlobalSpace)
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
				Tools->IsGlobalSpace = IsDown(Input->Ctrl);

				ScaleTool->AxisMask = {};

				if (Tools->IsGlobalSpace)
				{
					Axis = Identity3x3();
				}
				else
				{
					Axis = SetAxisForTool(Model, &WorldUI->Selected, WorldUI->IModel.Target);
				}

				//// TODO: Temp solution, handle different space mod properly
				//if (WorldUI->Selected.Count > 1)
				//{
				//	IsGlobalSpace = true;
				//}

				f32 ZSignMod = 1.0f; //Sign(Dot(Axis.Z, RayPCenterP));
				//Axis.Z *= ZSignMod;

				tools_axis_id InteractAxis = RayScaleToolAxisTest(Ray, ScaleAxisParams, Axis, ScaleTool->P, ZSignMod);
				if (InteractAxis)
				{
					u32 AxisID = GetIntAxisID(InteractAxis);
					ScaleTool->AxisMask.E[AxisID] = 1.0f;
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
						ApplyScale(Model, ScaleTool, &Tools->UniqIndeces, TargetElement, Tools->IsGlobalSpace);
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
				ModTransToolDefaultParams(TransTool->InitAxisParams, PosRelParams.ScaleFactor);

			m3x3 Axis;
			if (TransTool->InteractAxis == ToolsAxisID_None)
			{
				Tools->IsGlobalSpace = IsDown(Input->Ctrl);

				TransTool->AxisMask = {};
				TransTool->PlaneMask = {};
				TransTool->IsPlaneIntr = false;

				if (Tools->IsGlobalSpace)
				{
					Axis = Identity3x3();
				}
				else
				{
					Axis = SetAxisForTool(Model, &WorldUI->Selected, WorldUI->IModel.Target);
				}

				f32 ZSignMod = 1.0f; //Sign(Dot(Axis.Z, RayPCenterP));
				//Axis.Z *= ZSignMod;

				tools_axis_id InteractAxis = RayTransToolAxisPlaneTest(Ray, ScaleAxisParams, Axis, TransTool->P, ZSignMod);
				if (InteractAxis)
				{
					TransTool->IsPlaneIntr = true;
					u32 AxisID = GetIntAxisID(InteractAxis);
					TransTool->PlaneMask.E[AxisID] = 1.0f;
				}
				else
				{
					InteractAxis = RayTransToolAxisTest(Ray, ScaleAxisParams, Axis, TransTool->P, ZSignMod);
					if (InteractAxis)
					{
						u32 AxisID = GetIntAxisID(InteractAxis);
						TransTool->AxisMask.E[AxisID] = 1.0f;
					}
				}

				Interaction = SetToolAxisIntr(ToolType_Translate, UI_InteractionType_Select, InteractAxis);
				if ((InteractAxis != ToolsAxisID_None) && AreEqual(WorldUI->Interaction, Interaction))
				{
					TransTool->Axis = Axis;
					TransTool->AxisMask.w = 1.0f;
					TransTool->PlaneMask.w = 1.0f;
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
						ApplyTranslate(Model, &Tools->UniqIndeces, TransTool, TargetElement, Tools->IsGlobalSpace);
					}
				}
				else
				{
					TransTool->Intr = {};
					TransTool->InteractAxis = ToolsAxisID_None;
					TransTool->EnterActiveState = false;
				}
			}

			PushTranslateTool(RenderGroup, ScaleAxisParams, Axis, TransTool->AxisMask, TransTool->PlaneMask,
				TransTool->P, PosRelParams.ScaleFactor, Editor->StaticMesh[BuiltInMesh_TranslateArrow].Mesh);
		} break;

		case ToolType_Split:
		{
			split_tool *Split = &Tools->Split;

			interact_model *IModel = &WorldUI->IModel;
			split_buffer *SplitBuffer = &Tools->SplitBuffer;

			IModel->Face = {};
			SplitBuffer->Count = 0;

			Split->StartEdge = GetStartEdgeForSplit(WorldUI->MouseRay, Model, &IModel->Face);
			if (Split->StartEdge.Edge)
			{
				Split->StartEdge.t = IsDown(Input->Shift) ? Split->StartEdge.t : 0.5f;
				SetSplitToolData(Split, SplitBuffer, &Model->Data, IModel->Face.ID, &Editor->PageArena);

				Interaction = SetSplitSelectIntr(IModel->ID);
				if (AreEqual(WorldUI->ToExecute, Interaction))
				{
					WorldUI->Selected.Count = 0;
				}
				
				PushSplitToolSegment(RenderGroup, &Tools->SplitBuffer, Model->Offset, V3(1));
			}
		} break;
	}

	WorldUI->NextHotInteraction = Interaction;
}

void
EditorUIInteraction(game_editor_state *Editor, game_input *Input, render_group *RenderGroup)
{
	editor_world_ui *WorldUI = &Editor->WorldUI;

	ProcessWorldUIInput(WorldUI, Input);

	if (IsITargetEq(WorldUI->ITarget, Model) && WorldUI->UpdateModelInteraction)
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
			else if (WasDown(Input->MouseButtons[PlatformMouseButton_Right]) && IsDown(Input->Shift))
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
