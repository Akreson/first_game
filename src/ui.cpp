
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
	b32 Result = (UI->ITarget == UI_InteractionTarget_Model) &&
		(UI->IModel.Target) && (UI->IModel.ID == ModelID);

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

inline ui_interaction
SetSelectInteraction(u32 ModelID, u32 FaceID = 0, u32 EdgeID = 0, u16 Target = 0)
{
	ui_interaction Result;

	Result.TypeID = SetIntrTypeID(Target, UI_InteractionType_Select);;
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
	if (IsDown(Input->Alt))
	{
		WorldUI->UpdateITarget = false;
	}
	else if (WasDown(Input->Alt))
	{
		WorldUI->UpdateITarget = true;
	}

	if (WorldUI->UpdateITarget && !WorldUI->Selected.Count)
	{
		if (WasDown(Input->MouseButtons[PlatformMouseButton_Extended0]))
		{
			if ((WorldUI->ITarget == UI_InteractionTarget_Model) &&
				WorldUI->IModel.Target)
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
	}

	if (IsKepDown(Input->Alt) && IsGoDown(Input->Shift))
	{
		if (WorldUI->Selected.Count)
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
	if (ShiftDown && Buffer->Count)
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

internal void inline
UpdateModelInteractionTools(game_editor_state *Editor, game_input *Input, render_group *RenderGroup)
{
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
	
	if (WorldUI->ITarget == UI_InteractionTarget_Model &&
		WorldUI->UpdateITarget)
	{
		UpdateModelInteractionElement(Editor, Input, RenderGroup);
	}
	else if (WorldUI->ITarget == UI_InteractionTarget_Tools)
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
				WorldUI->UpdateITarget = false;
				WorldUI->Interaction = WorldUI->HotInteraction;
			}
			else if (WasDown(Input->MouseButtons[PlatformMouseButton_Left]))
			{
				WorldUI->UpdateITarget = true;
			}
			/*else if (WasDown(Input->MouseButtons[PlatformMouseButton_Right]))
			{
				if (IsITargetOnModel(WorldUI->ITarget))
				{
					WorldUI->ITarget = UI_InteractionTarget_Tools;
				}
			}*/
		} break;

		case UI_InteractionType_Select:
		{
			if (WasDown(Input->MouseButtons[PlatformMouseButton_Left]))
			{
				WorldUI->UpdateITarget = true;
				WorldUI->NextToExecute = WorldUI->Interaction;
				ZeroStruct(WorldUI->Interaction);
			}
		} break;
	}

	WorldUI->ToExecute = WorldUI->NextToExecute;
	ZeroStruct(WorldUI->NextToExecute);
	ZeroStruct(WorldUI->NextHotInteraction);
}
