
inline ui_id
IDFromModel(u32 ModelIndex, u32 FaceIndex = 0, u32 EdgeIndex = 0)
{
	ui_id Result;
	Result.ID[0] = ModelIndex;
	Result.ID[1] = (u64)(FaceIndex << 32) | EdgeIndex;

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
IsActiveModel(editor_world_ui *UI, u32 ModelID)
{
	b32 Result = UI->IModel.Target && (UI->IModel.ID == ModelID);

	return Result;
}

inline ui_interaction
SetSelectInteraction(interact_model *IModel, u16 Target)
{
	ui_interaction Result;

	Result.TypeID = SetIntrTypeID(Target, UI_InteractionType_Select);
	Result.ID = IDFromModel(IModel->ID, IModel->Face.ID, IModel->Edge.ID);

	return Result;
}

// TODO: Make general
inline ui_interaction
SetSelectInteraction(u32 ModelID, u32 FaceID = 0, u32 EdgeID = 0, u16 Target = 0)
{
	ui_interaction Result;

	Result.TypeID = SetIntrTypeID(Target, UI_InteractionType_Select);
	Result.ID = IDFromModel(ModelID, FaceID, EdgeID);

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
		}
	}

	if (IsKepDown(Input->Alt) && WasUp(Input->Shift))
	{
		if (IsITargetEq(WorldUI->ITarget, Tools))
		{
			WorldUI->ITarget = UI_InteractionTarget_Model;
			WorldUI->Tools.IsInit = false;
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

// TODO: Make possible buffer clearing

// TODO: Collate Add..ToSelectedBuffer?
inline void
AddFaceToSelectedBuffer(selected_elements_buffer *Buffer, model *Model, u32 ElementID)
{
	model_face *AddFace = Model->Faces + ElementID;
	for (u32 Index = 0;
		Index < Buffer->Count;
		++Index)
	{
		u32 BufferElementID = Buffer->Elements[Index];
		if (BufferElementID != ElementID)
		{
			model_face *CompFace = Model->Faces + BufferElementID;
			face_edge_match MatchResult = MatchFaceEdge(AddFace, CompFace);
			
			if (MatchResult.Succes)
			{
				// TODO: Change to memory arena for resize opportunity
				if (Buffer->Count < Buffer->MaxCount)
				{
					Buffer->Elements[Buffer->Count++] = ElementID;
				}
				else
				{
					Assert(0);
				}
			}
		}
	}
}

inline void
AddEdgeToSelectedBuffer(selected_elements_buffer *Buffer, model *Model, u32 ElementID)
{
	model_edge *AddEdge = Model->Edges + ElementID;
	for (u32 Index = 0;
		Index < Buffer->Count;
		++Index)
	{
		u32 BufferElementID = Buffer->Elements[Index];
		if (BufferElementID != ElementID)
		{
			model_edge *CompEdge = Model->Edges + BufferElementID;
			edge_vertex_match MatchResult = MatchEdgeVertex(AddEdge, CompEdge);

			if (MatchResult.Succes)
			{
				// TODO: Change to memory arena for resize opportunity
				if (Buffer->Count < Buffer->MaxCount)
				{
					Buffer->Elements[Buffer->Count++] = ElementID;
				}
				else
				{
					Assert(0);
				}
			}
		}
	}
}

void
AddToSelectedBuffer(selected_elements_buffer *Buffer,
	model *Model, u32 ElementID, u32 ITarget, b32 ShiftDown)
{
	if (Model && ShiftDown && Buffer->Count)
	{
		switch (ITarget)
		{
			case ModelTargetElement_Face:
			{
				AddFaceToSelectedBuffer(Buffer, Model, ElementID);
			} break;

			case ModelTargetElement_Edge:
			{
				AddEdgeToSelectedBuffer(Buffer, Model, ElementID);
			} break;
		}
	}
	else
	{
		Buffer->Count = 1;
		Assert(Buffer->Count < Buffer->MaxCount);
		*Buffer->Elements = ElementID;
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
				Interaction = SetSelectInteraction(IModel->ID, 0, 0, WorldUI->ITarget);

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
				if (RayModelFaceIntersect(Model, WorldUI->MouseRay, &IModel->Face))
				{
					Interaction = SetSelectInteraction(IModel, WorldUI->ITarget);

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
				if (RayModelEdgeInterset(Model, WorldUI->MouseRay, &IModel->Edge, EdgeIntersetRadius))
				{
					Interaction = SetSelectInteraction(IModel, WorldUI->ITarget);

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
ComputeAveragePos(model *Model, selected_elements_buffer *SelectBuffer, u32 ElementTarget)
{
	v3 Result;

	v3 ModelOffset = Model->Offset;
	switch (ElementTarget)
	{
		case ModelTargetElement_Model:
		{
			Result = ((Model->AABB.Min + ModelOffset) + (Model->AABB.Max + ModelOffset)) / 2.0f;
		} break;

		case ModelTargetElement_Face:
		{
			
		} break;

		case ModelTargetElement_Edge:
		{

		} break;
	}

	return Result;
}

internal void
ApplyTransformForAllFaces(model *Model, selected_elements_buffer *SelectBuffer,
	model_target_element ElementTarget, m4x4 Transform)
{
}

void
ApplyTransformForAllElement(model *Model, selected_elements_buffer *SelectBuffer,
	model_target_element ElementTarget, m4x4 Transform)
{	
	switch (ElementTarget)
	{
		case ModelTargetElement_Model:
		{
			
		} break;

		case ModelTargetElement_Face:
		{

		} break;

		case ModelTargetElement_Edge:
		{

		} break;
	}
}

internal inline m4x4
GetRotateMatrixFormAxisID(f32 Angle, tools_axis_id ID)
{
	m4x4 Result;

	switch (ID)
	{
		case ToolsAxisID_XAxis:
		{
			Result = XRotation(Angle);
		} break;
		case ToolsAxisID_YAxis:
		{
			Result = YRotation(Angle);
		} break;
		case ToolsAxisID_ZAxis:
		{
			Result = ZRotation(Angle);
		} break;
	}

	return Result;
}

internal inline void
UpdateModelAxis(model *Model, tools_axis_id ID, m4x4 Rotate)
{
	switch (ID)
	{
		case ToolsAxisID_XAxis:
		{
			Model->YAxis = Model->YAxis * Rotate;
			Model->ZAxis = Model->ZAxis * Rotate;
		} break;
		case ToolsAxisID_YAxis:
		{
			Model->XAxis = Model->XAxis * Rotate;
			Model->ZAxis = Model->ZAxis * Rotate;
		} break;
		case ToolsAxisID_ZAxis:
		{
			Model->XAxis = Model->XAxis * Rotate;
			Model->YAxis = Model->YAxis * Rotate;
		} break;
	}
}

// TODO: Implement drawing progres angle
internal void
ProcessRotateTool(rotate_tools *Tool, model *Model, selected_elements_buffer *SelectBuffer,
	ray_params Ray, model_target_element ElementTarget)
{
	f32 DotRayPlane = Dot(Ray.Dir, Tool->InteractPlane.N);
	f32 tRay = RayPlaneIntersect(Ray, Tool->InteractPlane, DotRayPlane);
	//if (DotRayPlane == 0) // TODO: Handle this case
	if (tRay >= 0)
	{
		v3 CurrentVector = Ray.Pos + (Ray.Dir + tRay);
		CurrentVector = Normalize(CurrentVector - Tool->CenterPos);

		// TODO: Move to set _move_ interaction?
		if (!Tool->EnterActiveState)
		{
			Tool->EnterActiveState = true;
			Tool->BeginVector = CurrentVector;
			Tool->PrevVector = CurrentVector;
		}


		if (Tool->PrevVector != CurrentVector)
		{
			f32 RotateDir = Dot(Cross(Tool->PrevVector, CurrentVector), Tool->InteractPlane.N);
			RotateDir = RotateDir / Abs(RotateDir);

			f32 RotateSpeed = 4.0f;
			f32 DotAngle = Dot(CurrentVector, Tool->PrevVector);
			DotAngle = Clamp(-1.0f, DotAngle, 1.0f);
			f32 AngleBetween = (ACos(DotAngle) * RotateDir) * RotateSpeed;

			if (AngleBetween != 0)
			{
				m4x4 Rotate = GetRotateMatrixFormAxisID(AngleBetween, Tool->InteractAxis);
		
				if (ElementTarget == ModelTargetElement_Model)
				{
					UpdateModelAxis(Model, Tool->InteractAxis, Rotate);
				}
				// TODO: Complete
				//ApplyTransformForAllElement()

			}
		}

		Tool->PrevVector = CurrentVector;
	}
}

// TODO: Add more tools!!!
void
InitTools(editor_world_ui *WorldUI, tools *Tools, model *ModelsArr)
{
	selected_elements_buffer *SelectBuffer = &WorldUI->Selected;
	interact_model *IModel = &WorldUI->IModel;
	model *Model = ModelsArr + IModel->ID;

	// TODO: Define pos. as common
	tool_type ToolType = (tool_type)Tools->Type;
	if (ToolType == ToolType_Rotate ||
		ToolType == ToolType_Translate ||
		ToolType == ToolType_Scale)
	{
		Tools->Rotate = {};

		Tools->Rotate.CenterPos = ComputeAveragePos(Model, SelectBuffer, IModel->Target);
		Tools->Rotate.Radius = ROTATE_TOOLS_DIAMETER / 2.0f;
	}

	Tools->IsInit = true;
}

internal void inline
UpdateModelInteractionTools(game_editor_state *Editor, game_input *Input, render_group *RenderGroup)
{
	ui_interaction Interaction = {};
	editor_world_ui *WorldUI = &Editor->WorldUI;
	tools *Tools = &WorldUI->Tools;

	// TODO: Extend
	if (!Tools->IsInit)
	{
		InitTools(WorldUI, Tools, Editor->Models);
	}

	ray_params Ray = WorldUI->MouseRay;
	switch (Tools->Type)
	{
		// TODO: Make sphere sizeble or on same distance to the camera
		case ToolType_Rotate:
		{
			rotate_tools *RotateTool = &Tools->Rotate;
			model *Model = Editor->Models + WorldUI->IModel.ID;

			if (RotateTool->InteractAxis == ToolsAxisID_None)
			{
				RotateTool->AxisMask = {};

				v3 PointOnSphere;
				if (RaySphereIntersect(Ray, RotateTool->CenterPos, RotateTool->Radius, &PointOnSphere))
				{
					v3 DirFromCenter = Normalize(PointOnSphere - RotateTool->CenterPos);
					f32 XDotP = Abs(Dot(Model->XAxis, DirFromCenter));
					f32 YDotP = Abs(Dot(Model->YAxis, DirFromCenter));
					f32 ZDotP = Abs(Dot(Model->ZAxis, DirFromCenter));
					
					tools_axis_id InteractAxis = ToolsAxisID_None;
					if (ZDotP <= RTOOLS_AXIS_INTERACT_THRESHOLD)
					{
						InteractAxis = ToolsAxisID_ZAxis;
						RotateTool->AxisMask.z = 1.0f;
						RotateTool->InteractPlane.N = Model->ZAxis;
					}
					else if (YDotP <= RTOOLS_AXIS_INTERACT_THRESHOLD)
					{
						InteractAxis = ToolsAxisID_YAxis;
						RotateTool->AxisMask.y = 1.0f;
						RotateTool->InteractPlane.N = Model->YAxis;
					}
					else if (XDotP <= RTOOLS_AXIS_INTERACT_THRESHOLD)
					{
						InteractAxis = ToolsAxisID_XAxis;
						RotateTool->AxisMask.x = 1.0f;
						RotateTool->InteractPlane.N = Model->XAxis;
					}

					Interaction.TypeID = SetIntrTypeID(UI_InteractionTarget_Tools, UI_InteractionType_Select);
					Interaction.ID.ID[0] = ToolType_Rotate;
					Interaction.ID.ID[1] = InteractAxis;

					if ((InteractAxis != ToolsAxisID_None) && AreEqual(WorldUI->Interaction, Interaction))
					{
						RotateTool->InteractAxis = InteractAxis;
						RotateTool->InteractPlane.D = Dot(RotateTool->InteractPlane.N, RotateTool->CenterPos);
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
					ProcessRotateTool(RotateTool, Model, &WorldUI->Selected, Ray,
						(model_target_element)WorldUI->IModel.Target);
				}
				else
				{
					ZeroStruct(RotateTool->InteractPlane);
					ZeroStruct(RotateTool->BeginVector);
					ZeroStruct(RotateTool->AxisMask);
					RotateTool->InteractAxis = ToolsAxisID_None;
					RotateTool->EnterActiveState = false;
				}
			}

			PushRotateSphere(RenderGroup, Editor->StaticMesh[0].Mesh, RotateTool->CenterPos,
				Model->XAxis, Model->YAxis, Model->ZAxis, RotateTool->AxisMask);
		} break;
		case ToolType_Scale:
		{
			PushSphere(RenderGroup, Editor->StaticMesh[0].Mesh, V3(0, 1, 0));
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
