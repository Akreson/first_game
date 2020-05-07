
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
IsActiveModel(editor_world_ui *UI, u32 ModelID)
{
	b32 Result = (UI->IModel.ID == ModelID) &&
		((UI->ITarget >= UI_InteractionTarget_Model) &&
		(UI->ITarget < UI_InteractionTarget_ModelCount));

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
	if (WasDown(Input->MouseButtons[PlatformMouseButton_Extended0]))
	{
		if (WorldUI->ITarget)
		{
			++WorldUI->ITarget;
			if (WorldUI->ITarget == UI_InteractionTarget_ModelCount)
			{
				WorldUI->ITarget = UI_InteractionTarget_Model;
			}
		}
	}

	if (IsKepDown(Input->Alt) && IsGoDown(Input->Shift))
	{
		WorldUI->ITarget = UI_InteractionTarget_None;
		ZeroStruct(WorldUI->Interaction);
	}
}

internal void inline
UpdateModelInteractionElement(game_editor_state *Editor, game_input *Input, render_group *RenderGroup)
{
	ui_interaction Interaction = {};

	editor_world_ui *WorldUI = &Editor->WorldUI;
	interact_model *IModel = &WorldUI->IModel;

	switch (WorldUI->ITarget)
	{
		case UI_InteractionTarget_None:
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
						WorldUI->ITarget = UI_InteractionTarget_Model;
					}
				}
			}
		} break;

		case UI_InteractionTarget_Model:
		{

		} break;

		case UI_InteractionTarget_ModelFace:
		{
			model *Model = Editor->Models + WorldUI->IModel.ID;
			IModel->Face = {};

			if (RayAABBIntersect(WorldUI->MouseRay, Model->AABB, Model->Offset))
			{
				if (RayModelFaceIntersect(Model, WorldUI->MouseRay, &IModel->Face))
				{
					Interaction = SetSelectInteraction(IModel, WorldUI->ITarget);
				}
			}
		} break;

		case UI_InteractionTarget_ModelEdge:
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
				}
			}
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

	if (WorldUI->UpdateITarget)
	{
		UpdateModelInteractionElement(Editor, Input, RenderGroup);
	}

	// TODO: Split to anouther function
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
