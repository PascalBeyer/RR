
static void PlacingUnitsHandleEvent(ExecuteData *exe, World *world, UnitHandler *unitHandler, AssetHandler *assetHandler, KeyStateMessage message, Input input)
{
	if (message.flag & KeyState_ReleasedThisFrame)
	{
		switch (message.key)
		{
		case Key_leftMouse:
		{
			// todo only ones that are in the tree, i.e. are gamePlay related
			Entity *clickedE = GetHotEntity(world, assetHandler, input.mouseZeroToOne); 
			if (clickedE)
			{
				Quaternion q = { 1, 1, 0, 0 };

				if (!TileBlocked(world, clickedE->physicalPos + V3i(0, 0, -1)))
				{
					Entity *e = exe->placingUnits.unitsToPlace[0];
					e->physicalPos = clickedE->physicalPos + V3i(0, 0, -1);
					UnorderedRemove(&exe->placingUnits.unitsToPlace, 0);

					if (!exe->placingUnits.unitsToPlace)
					{
						ChangeExecuteState(world, unitHandler, exe, Execute_PathCreator);
						exe->pathCreator.hotUnit = GetUnitIndex(e);
						exe->pathCreator.state = PathCreator_CreatingPath;
						return;
					}
				}
			}
		}break;
		}
	}
}


static void PathCreatorHandleEvent(World *world, ExecuteData *exe, UnitHandler *unitHandler, AssetHandler *assetHandler, KeyStateMessage message, Input input)
{
	Camera cam = world->camera;
	PathCreator *pathCreator = &exe->pathCreator;
	if (message.flag & KeyState_PressedThisFrame)
	{
		switch (pathCreator->state)
		{
		case PathCreator_None:
		{
			switch (message.key)
			{
			case Key_leftMouse:
			{
				u32 unitIndex = GetHotUnit(world, unitHandler, assetHandler, input.mouseZeroToOne, cam);

				if (unitIndex >= unitHandler->amount)
				{
					break;
				}

				if (message.flag & KeyState_ControlDown)
				{
					u32 serial = unitHandler->entitySerials[unitIndex];
					Entity *e = GetEntity(world, serial);
					ChangeExecuteState(world, unitHandler, exe, Execute_PlacingUnits);
					ArrayAdd(&exe->placingUnits.unitsToPlace, e);

					break;
				}

				pathCreator->hotUnit = unitIndex;
				pathCreator->state = PathCreator_CreatingPath;

				ResetWorld(world);
				
				For(unitHandler->programs[unitIndex])
				{
					AdvanceGameState(world, unitHandler, exe, false);
				}
			}break;
			case Key_F6:
			{
				ChangeExecuteState(world, unitHandler, exe, Execute_Simulation);
			}break;
			}
		}break;

		case PathCreator_CreatingPath:
		{
			Assert(pathCreator->hotUnit < unitHandler->amount);

			switch (message.key)
			{
			case Key_leftMouse:
			{

				if (PointInRectangle(pathCreator->finishButton, input.mouseZeroToOne))
				{
					pathCreator->state = PathCreator_None;
					pathCreator->hotUnit = 0xFFFFFFFF;
					ResetWorld(world);
					break;
				}

				auto program = &unitHandler->programs[pathCreator->hotUnit];
				u32 serial = unitHandler->entitySerials[pathCreator->hotUnit];
				Entity *entity = GetEntity(world, serial);

				if (PointInRectangle(pathCreator->resetUnitButton, input.mouseZeroToOne))
				{
					program->amount = 0;
					break;
				}

				v3 p = ScreenZeroToOneToZ(cam, input.mouseZeroToOne, entity->physicalPos.z);
				v3 d = p - V3(GetPhysicalPositionAfterMove(entity));

				i32 xSign = (d.x > 0) ? 1 : -1;
				i32 ySign = (d.y > 0) ? 1 : -1;

				if (BoxNorm(d) < 0.5f)
				{
					AddInstruction(program, Unit_Wait);
				}
				else if ((f32)ySign * d.y > (f32)xSign * d.x)
				{
					if (ySign > 0)
					{
						AddInstruction(program, Unit_MoveUp);
					}
					else
					{
						AddInstruction(program, Unit_MoveDown);
					}
				}
				else
				{
					if (xSign > 0)
					{
						AddInstruction(program, Unit_MoveRight);
					}
					else
					{
						AddInstruction(program, Unit_MoveLeft);
					}
				}

			}break;
			case Key_rightMouse:
			{
				Assert(pathCreator->hotUnit < unitHandler->amount);
				auto program = &unitHandler->programs[pathCreator->hotUnit];
				if (!program->amount) break;
				--program->amount;
			}break;
			case Key_F6:
			{
				ChangeExecuteState(world, unitHandler, exe, Execute_Simulation);
			}break;

			}
		}break;
		default:
		{
			Die;
		}break;
		}

	}
}

static void SimHandleEvents(World *world, ExecuteData *exe, KeyStateMessage message)
{
	SimData *sim = &exe->simData;
	if (message.flag & KeyState_PressedThisFrame)
	{
		switch (message.key)
		{
		case Key_plus:
		{
			sim->timeScale = 2.0f * sim->timeScale;
		}break;
		case Key_minus:
		{
			sim->timeScale = sim->timeScale / 2.0f;
		}break;
		case Key_escape:
		{
			ChangeExecuteState(world, NULL, exe, Execute_PathCreator);
		}break;
		}
	}

}

static void VictoryHandleEvents(ExecuteData *exe, World *world, KeyStateMessage message)
{
	if (message.flag & KeyState_PressedThisFrame)
	{
		ChangeExecuteState(world, NULL, exe, Execute_PathCreator);
	}
}

static void ExecuteHandleEvents(World *world, UnitHandler *unitHandler, AssetHandler *assetHandler, ExecuteData *exe, KeyStateMessage message, Input input)
{
	switch (exe->state)
	{
	case Execute_PlacingUnits:
	{
		PlacingUnitsHandleEvent(exe, world, unitHandler, assetHandler, message, input);
	}break;
	case Execute_Simulation:
	{
		SimHandleEvents(world, exe, message);
	}break;
	case Execute_PathCreator:
	{
		PathCreatorHandleEvent(world, exe, unitHandler, assetHandler, message, input);
	}break;
	case Execute_Victory:
	{
		VictoryHandleEvents(exe, world, message);
	}break;

	InvalidDefaultCase;

	}
	
}

static void ColorPickersHandleEvents(Editor *editor, KeyStateMessage message, Input input)
{

	For(editor->elements)
	{
		switch (it->type)
		{
		case EditorUI_ColorPicker:
		{
			ColorPicker *picker = &it->picker;
			if (!message.key == Key_leftMouse)
			{
				break;
			}

			if ((message.flag & KeyState_PressedThisFrame))
			{
				if (PointInRectangle(picker->sliderPos, picker->width, picker->sliderHeight, input.mouseZeroToOne))
				{
					picker->selecting = (message.flag & KeyState_Down) ? PickerSelecting_Slider : PickerSelecting_Nothing;
					return;
				}
				if (PointInRectangle(picker->pos, picker->width, picker->height, input.mouseZeroToOne))
				{
					picker->selecting = (message.flag & KeyState_Down) ? PickerSelecting_ColorPicker : PickerSelecting_Nothing;
					return;
				}
				if (PointInRectangle(picker->killRectPos, picker->headerSize, picker->headerSize, input.mouseZeroToOne))
				{
					if (picker->isTweeker)
					{
						WriteSingleTweeker(*picker->tweeker);
					}
					ArrayRemove(&editor->elements, it);
					return;
				}
				if (PointInRectangle(picker->headerPos, picker->width, picker->headerSize, input.mouseZeroToOne))
				{
					picker->selecting = (message.flag & KeyState_Down) ? PickerSelecting_Header : PickerSelecting_Nothing;
					return;
				}

			}
			if (message.flag & KeyState_ReleasedThisFrame)
			{
				picker->selecting = PickerSelecting_Nothing;
			}

			return;
		}break;
		default:
		{
			Die;
		}break;

		}
	}
}

static v3i AdjustForCamera(Camera cam, UnitInstruction intendedDir)
{
	v3 camV = cam.b3;
	if (Abs(camV.x) > Abs(camV.y))
	{
		if (camV.x > 0.0f)
		{
			switch (intendedDir)
			{
			case Unit_MoveUp:
			{
				return V3i(1, 0, 0);
			}break;
			case Unit_MoveDown:
			{
				return V3i(-1, 0, 0);
			}break;
			case Unit_MoveLeft:
			{
				return V3i(0, -1, 0);
			}break;
			case Unit_MoveRight:
			{
				return V3i(0, 1, 0);
			}break;
			}
		}
		else
		{
			switch (intendedDir)
			{
			case Unit_MoveUp:
			{
				return V3i(-1, 0, 0);
			}break;
			case Unit_MoveDown:
			{
				return V3i(1, 0, 0);
			}break;
			case Unit_MoveLeft:
			{
				return V3i(0, 1, 0);
			}break;
			case Unit_MoveRight:
			{
				return V3i(0, -1, 0);
			}break;
			}
		}
	}
	else
	{
		if (camV.y > 0.0f)
		{

			switch (intendedDir)
			{
			case Unit_MoveUp:
			{
				return V3i(0, 1, 0);
			}break;
			case Unit_MoveDown:
			{
				return V3i(0, -1, 0);
			}break;
			case Unit_MoveLeft:
			{
				return V3i(1, 0,  0);
			}break;
			case Unit_MoveRight:
			{
				return V3i(-1, 0, 0);
			}break;
			}
		}
		else
		{
			switch (intendedDir)
			{
			case Unit_MoveUp:
			{
				return V3i(0, -1, 0);
			}break;
			case Unit_MoveDown:
			{
				return V3i(0, 1, 0);
			}break;
			case Unit_MoveLeft:
			{
				return V3i(-1, 0, 0);
			}break;
			case Unit_MoveRight:
			{
				return V3i(1, 0, 0);
			}break;
			}

		}
	}

	return V3i();
}


static void EditorHandleEvents(Editor *editor, UnitHandler *unitHandler, World *world, AssetHandler *assetHandler, KeyStateMessage message, Input input, Arena *currentStateArena)
{
	Camera *cam = &world->camera;
	For(editor->elements)
	{
		switch (it->type)
		{
		case EditorUI_ColorPicker:
		{
			ColorPicker *picker = &it->picker;
			if (!message.key == Key_leftMouse)
			{
				break;
			}

			if ((message.flag & KeyState_PressedThisFrame))
			{
				if (PointInRectangle(picker->sliderPos, picker->width, picker->sliderHeight, input.mouseZeroToOne))
				{
					picker->selecting = PickerSelecting_Slider;
					return;
				}
				if (PointInRectangle(picker->pos, picker->width, picker->height, input.mouseZeroToOne))
				{
					picker->selecting = PickerSelecting_ColorPicker;
					return;
				}
				if (PointInRectangle(picker->killRectPos, picker->headerSize, picker->headerSize, input.mouseZeroToOne))
				{
					if (picker->isTweeker)
					{
						WriteSingleTweeker(*picker->tweeker);
					}
					ArrayRemove(&editor->elements, it);
					return;
				}
				if (PointInRectangle(picker->headerPos, picker->width, picker->headerSize, input.mouseZeroToOne))
				{
					picker->selecting = PickerSelecting_Header;
					return;
				}

			}
			if (message.flag & KeyState_ReleasedThisFrame)
			{
				picker->selecting = PickerSelecting_Nothing;
			}

			break;
		}break;
		default:
		{
			Die;
		}break;

		}

	}

	if (message.flag & KeyState_Down)
	{
		switch (editor->state)
		{
		case EditorState_PickingColor:
		{
			EditorPushUndo(editor, world);

			ResetSelectionInitials(editor, world);
			EditorGoToNone(editor);
		}break;
		case EditorState_Default:
		{
			switch (message.key)
			{
			case Key_leftMouse:
			{
				Tweekable(f32, editorPanelWidth);
				Tweekable(f32, editorPanelHeight);
				if (editor->panel.visible && PointInRectangle(editor->panel.pos, editorPanelWidth, editorPanelHeight, input.mouseZeroToOne))
				{
					Tweekable(f32, editorBorderWidth);
					Tweekable(f32, editorHeaderWidth);
					Tweekable(f32, editorPanelFontSize);
					EditorPanel *p = &editor->panel;

					v2 headerPos = editor->panel.pos + V2(editorBorderWidth, editorBorderWidth);
					f32 widthWithoutBoarder = editorPanelWidth - 2.0f * editorBorderWidth;
					if (PointInRectangle(headerPos, widthWithoutBoarder, editorHeaderWidth, input.mouseZeroToOne))
					{
						editor->state = EditorState_DragingPanel;
						return;
					}

					v2 pos = V2(headerPos.x, headerPos.y + editorHeaderWidth + editorBorderWidth);
					For(p->values)
					{
						f32 height = HeightForTweekerPanel(it->type, editorBorderWidth, editorPanelFontSize);
						switch (it->type)
						{
						case Tweeker_u32:
						case Tweeker_f32:
						{
							if (PointInRectangle(pos, widthWithoutBoarder, height, input.mouseZeroToOne))
							{
								editor->state = EditorState_AlteringValue;
								editor->panel.hotValue = (u32)(it - p->values.data);
								return;
							}
						}break;
						case Tweeker_EntityType:
						{
							if (editor->hotEntityInfos.amount != 1) break;
							Entity *e = GetEntity(world, editor->hotEntityInfos[0].placedSerial);

							if (PointInRectangle(pos, 0.5f * widthWithoutBoarder, height, input.mouseZeroToOne))
							{
								if (e->type != Entity_None)
								{
									e->type = (EntityType)(e->type - 1);
								}
								else
								{
									e->type = (EntityType)(Entity_Count - 1);
								}
								e->flags = GetStandardFlags(e->type);
								return;
							}
							else if (PointInRectangle(pos + V2(0.5f * widthWithoutBoarder, 0.0f), 0.5f * widthWithoutBoarder, height, input.mouseZeroToOne))
							{
								e->type = (EntityType)(e->type + 1);
								if (e->type == Entity_Count)
								{
									e->type = Entity_None;
								}

								e->flags = GetStandardFlags(e->type);
								return;
							}
						}break;
						case Tweeker_b32:
						{
							if (PointInRectangle(pos, widthWithoutBoarder, height, input.mouseZeroToOne))
							{
								*it->b = !*it->b;
								return;
							}
						}break;
						case Tweeker_v2:
						{
							if (PointInRectangle(V2(pos.x, pos.y + editorPanelFontSize), widthWithoutBoarder, editorPanelFontSize, input.mouseZeroToOne))
							{
								editor->state = EditorState_AlteringValue;
								editor->panel.hotValue = (u32)(it - p->values.data);
								editor->panel.hotValueXYZ = 'x';
								return;
							}
							else if (PointInRectangle(V2(pos.x, pos.y + 2.0f * editorPanelFontSize), widthWithoutBoarder, editorPanelFontSize, input.mouseZeroToOne))
							{
								editor->state = EditorState_AlteringValue;
								editor->panel.hotValue = (u32)(it - p->values.data);
								editor->panel.hotValueXYZ = 'y';
								return;
							}
						}break;
						case Tweeker_EulerAngle:
						case Tweeker_v3:
						case Tweeker_v3i:
						{
							if (PointInRectangle(V2(pos.x, pos.y + editorPanelFontSize), widthWithoutBoarder, editorPanelFontSize, input.mouseZeroToOne))
							{
								editor->state = EditorState_AlteringValue;
								editor->panel.hotValue = (u32)(it - p->values.data);
								editor->panel.hotValueXYZ = 'x';
								return;
							}
							else if (PointInRectangle(V2(pos.x, pos.y + 2.0f * editorPanelFontSize), widthWithoutBoarder, editorPanelFontSize, input.mouseZeroToOne))
							{
								editor->state = EditorState_AlteringValue;
								editor->panel.hotValue = (u32)(it - p->values.data);
								editor->panel.hotValueXYZ = 'y';
								return;
							}
							else if (PointInRectangle(V2(pos.x, pos.y + 3.0f * editorPanelFontSize), widthWithoutBoarder, editorPanelFontSize, input.mouseZeroToOne))
							{
								editor->state = EditorState_AlteringValue;
								editor->panel.hotValue = (u32)(it - p->values.data);
								editor->panel.hotValueXYZ = 'z';
								return;
							}

						}break;
						case Tweeker_v4:
						{
							if (PointInRectangle(pos, widthWithoutBoarder, height, input.mouseZeroToOne))
							{
								editor->state = EditorState_PickingColor;
								editor->panel.hotValue = (u32)(it - p->values.data);

								ColorPicker picker = CreateColorPicker(it->vec4);
								ArrayAdd(&editor->elements, { EditorUI_ColorPicker, picker });
								return;
							}
						}break;
						default:
						{
							Die;
						}break;
						}

						pos.y += height + editorBorderWidth;
					}

					return;
				}

				Entity *hotEntity = GetHotEntity(world, assetHandler, input.mouseZeroToOne);

				if (message.flag & KeyState_ShiftDown)
				{
					if (!hotEntity) break;

					b32 allreadyIn = false;
					For(editor->hotEntityInfos)
					{
						if (it->placedSerial == hotEntity->serialNumber)
						{
							allreadyIn = true;
							u32 it_index = (u32)(it - editor->hotEntityInfos.data);
							UnorderedRemove(&editor->hotEntityInfos, it_index);
						}
					}

					if (allreadyIn) break;

					EditorSelect toAdd = EntityToEditorSelect(hotEntity);

					ArrayAdd(&editor->hotEntityInfos, toAdd);

					editor->panel.values.amount = 0;
					editor->panel.visible = true;
					ArrayAdd(&editor->panel.values, CreateTweekerPointer(Tweeker_b32, "SnapToTileMap", &editor->snapToTileMap));
					break;
				}

				if (EditorHasSelection(editor))
				{
					EditorSelectNothing(editor);
				}

				if (hotEntity)
				{
					EditorSelect toAdd = EntityToEditorSelect(hotEntity);

					ArrayAdd(&editor->hotEntityInfos, toAdd);
					editor->panel.visible = true;

					ArrayAdd(&editor->panel.values, CreateTweekerPointer(Tweeker_b32, "SnapToTileMap", &editor->snapToTileMap));
					ArrayAdd(&editor->panel.values, CreateTweekerPointer(Tweeker_EntityType, "EntityType", &hotEntity->type));
					ArrayAdd(&editor->panel.values, CreateTweekerPointer(Tweeker_v3i, "PhysicalPos", &hotEntity->physicalPos));
					ArrayAdd(&editor->panel.values, CreateTweekerPointer(Tweeker_v3, "Offset", &hotEntity->offset));
					ArrayAdd(&editor->panel.values, CreateTweekerPointer(Tweeker_f32, "Scale", &hotEntity->scale));
					// todo make euler angle v3i
					ArrayAdd(&editor->panel.values, CreateTweekerPointer(Tweeker_EulerAngle, "Orientation", &hotEntity->orientation));
					ArrayAdd(&editor->panel.values, CreateTweekerPointer(Tweeker_v4, "Color", &hotEntity->color));
					ArrayAdd(&editor->panel.values, CreateTweekerPointer(Tweeker_u32, "Serial", &hotEntity->serialNumber));
				}
			}break;
			case Key_tab:
			{
				if (EditorHasSelection(editor))
				{
					editor->panel.visible = !editor->panel.visible;
				}
			}break;
			case Key_left:
			{
				v3i inc = AdjustForCamera(*cam, Unit_MoveLeft);
				For(editor->hotEntityInfos)
				{
					Entity *mesh = GetEntity(world, it->placedSerial);
					mesh->physicalPos += inc;
				}
			}break;
			case Key_right:
			{
				v3i inc = AdjustForCamera(*cam, Unit_MoveRight);
				For(editor->hotEntityInfos)
				{
					Entity *mesh = GetEntity(world, it->placedSerial);
					mesh->physicalPos += inc;
				}

			}break;
			case Key_up:
			{
				if (message.flag & KeyState_ControlDown)
				{
					For(editor->hotEntityInfos)
					{
						Entity *mesh = GetEntity(world, it->placedSerial);
						mesh->physicalPos.z--;
					}
					break;
				}

				v3i inc = AdjustForCamera(*cam, Unit_MoveUp);
				For(editor->hotEntityInfos)
				{
					Entity *mesh = GetEntity(world, it->placedSerial);
					mesh->physicalPos += inc;
				}

			}break;
			case Key_down:
			{
				if (message.flag & KeyState_ControlDown)
				{
					For(editor->hotEntityInfos)
					{
						Entity *mesh = GetEntity(world, it->placedSerial);
						mesh->physicalPos.z++;
					}
					break;
				}

				v3i inc = AdjustForCamera(*cam, Unit_MoveDown);
				For(editor->hotEntityInfos)
				{
					Entity *mesh = GetEntity(world, it->placedSerial);
					mesh->physicalPos += inc;
				}

			}break;

			case Key_g:
			{
				ResetSelectionInitials(editor, world);
				editor->state = EditorState_Moving;
			}break;
			case Key_r:
			{
				ResetSelectionInitials(editor, world);
				editor->state = EditorState_Rotating;
			}break;
			case Key_s:
			{
				ResetSelectionInitials(editor, world);

				if ((message.flag & KeyState_ControlDown))
				{
					if (!world->loadedLevel.name.amount) break;

					For(world->entities)
					{
						it->initialPos = it->physicalPos;
					}

					char *fileName = FormatCString("level/%s.level", world->loadedLevel.name);
					if (WriteLevel(fileName, *world, *unitHandler, assetHandler))
					{
						ConsoleOutput("Saved!");
					}
					else
					{
						ConsoleOutputError("Could not save file %c*.", fileName);
					}

					break;
				}
				editor->state = EditorState_Scaling;
			}break;
			case Key_c:
			{
				if (!editor->hotEntityInfos) break;

				if (message.flag & KeyState_ControlDown)
				{
					editor->clipBoard.amount = 0;
					v3 averagePos = GetAveragePosForSelection(editor, world);
					v3i averageTile = SnapToTileMap(averagePos);
					averageTile.z = 0;
					For(editor->hotEntityInfos)
					{
						Entity *mesh = GetEntity(world, it->placedSerial);
						EntityCopyData data;
						data.color = mesh->color;
						data.meshId = mesh->meshId;
						data.orientation = mesh->orientation;
						data.physicalPos = mesh->physicalPos - averageTile;
						data.offset = mesh->offset;
						data.scale = mesh->scale;
						data.flags = mesh->flags;
						data.type = mesh->type;
						ArrayAdd(&editor->clipBoard, data);
					}
				}

			}break;
			case Key_v:
			{
				if (!editor->clipBoard) break;
				if (message.flag & KeyState_ControlDown)
				{
					editor->state = EditorState_PlacingNewMesh;

					ResetEditorPanel(editor);
					ArrayAdd(&editor->panel.values, CreateTweekerPointer(Tweeker_b32, "SnapToTileMap", &editor->snapToTileMap));
					editor->panel.visible = true;

					EditorSelectNothing(editor);
				}
			}break;
			case Key_z:
			{
				if (message.flag & KeyState_ControlDown)
				{
					EditorSelectNothing(editor);
					EditorPerformUndo(editor, world);
				}
			}break;
			case Key_y:
			{
				if (message.flag & KeyState_ControlDown)
				{
					EditorSelectNothing(editor);
					EditorPerformRedo(editor, world);
				}
			}break;
			case Key_x:
			{
				if (EditorHasSelection(editor))
				{
					editor->state = EditorState_DeleteSelection;
				}
			}break;
			case Key_middleMouse:
			{
				editor->state = EditorState_OrbitingCamera;
			}break;

			case Key_num1:
			{
				// front
				cam->b3 = V3(0, 1, 0);
				cam->b1 = V3(1, 0, 0);
				cam->b2 = V3(0, 0, 1);
				cam->pos = editor->focusPoint + V3(0, -Norm(editor->focusPoint - cam->pos), 0);

			}break;
			case Key_num3:
			{
				// Right
				cam->b3 = V3(-1, 0, 0);
				cam->b1 = V3(0, -1, 0);
				cam->b2 = V3(0, 0, 1);
				cam->pos = editor->focusPoint + V3(Norm(editor->focusPoint - cam->pos), 0, 0);

			}break;
			case Key_num7:
			{
				// Top
				cam->b1 = V3(1, 0, 0);
				cam->b2 = V3(0, 1, 0);
				cam->b3 = V3(0, 0, 1);
				cam->pos = editor->focusPoint + V3(0, 0, -Norm(editor->focusPoint - cam->pos));

			}break;
			case Key_num9:
			{
				// toggle
				cam->b3 = -cam->b3;
				cam->b1 = -cam->b1;

				cam->pos = editor->focusPoint - (cam->pos - editor->focusPoint);

			}break;
			case Key_numDot:
			{
				if (!EditorHasSelection(editor))
				{
					break;
				}

				v3 averagePos = GetAveragePosForSelection(editor, world);
				cam->pos = averagePos + cam->pos - editor->focusPoint;
				editor->focusPoint = averagePos;

			}break;
			case Key_mouseWheelBack:
			{
				cam->pos = 1.1f * (cam->pos - editor->focusPoint) + editor->focusPoint;

			}break;
			case Key_mouseWheelForward:
			{
				cam->pos = 0.9f * (cam->pos - editor->focusPoint) + editor->focusPoint;
			}break;


			}
		}break;
		case EditorState_Rotating:
		{
			switch (message.key)
			{
			case Key_leftMouse:
			{
				EditorPushUndo(editor, world);
				ResetSelectionInitials(editor, world);
				EditorGoToNone(editor);
			}break;
			case Key_rightMouse:
			{
				ResetHotMeshes(editor, world);
				ResetSelectionInitials(editor, world);
				EditorGoToNone(editor);
			}break;

			}
		}break;
		case EditorState_Moving:
		{
			switch (message.key)
			{
			case Key_leftMouse:
			{
				if (editor->snapToTileMap)
				{
					For(editor->hotEntityInfos)
					{
						Entity *mesh = GetEntity(world, it->placedSerial);
						mesh->offset = V3();
					}
				}

				EditorPushUndo(editor, world);
				ResetSelectionInitials(editor, world);
				EditorGoToNone(editor);
			}break;
			case Key_rightMouse:
			{
				ResetHotMeshes(editor, world);
				ResetSelectionInitials(editor, world);
				EditorGoToNone(editor);
			}break;

			}
		}break;
		case EditorState_Scaling:
		{
			switch (message.key)
			{
			case Key_leftMouse:
			{
				EditorPushUndo(editor, world);
				ResetSelectionInitials(editor, world);
				EditorGoToNone(editor);
			}break;
			case Key_rightMouse:
			{
				ResetHotMeshes(editor, world);
				ResetSelectionInitials(editor, world);
				EditorGoToNone(editor);
			}break;

			}
		}break;
		case EditorState_DragingPanel:
		{
		}break;
		case EditorState_AlteringValue:
		{
			TextInput *t = &editor->panel.textInput;
			Char input = KeyToChar(message.key, false);
			if (input == '-' || input == '.' || '0' <= input && input <= '9')
			{
				if (t->string.length + 1 <= t->maxLength)
				{
					t->string[t->string.length++] = input;
					return;
				}
				ConsoleOutputError("To many symbols, exceeds max length of 50");

				return;
			}


			switch (message.key)
			{
			case Key_backSpace:
			{
				if (t->string.length)
				{
					t->string.length--;
				}
				return;
			}break;
			case Key_enter:
			{
				b32 success = true;

				u32 i = editor->panel.hotValue;
				auto val = editor->panel.values[i];
				switch (val.type)
				{
				case Tweeker_f32:
				{
					f32 newValue = StoF(t->string, &success);
					if (success)
					{
						*val.f = newValue;
						break;
					}
					ConsoleOutputError("Couldn't parse Editor input as f32.");
				}break;
				case Tweeker_u32:
				{
					u32 newValue = StoU(t->string, &success);
					if (success)
					{
						*val.u = newValue;
						break;
					}
					ConsoleOutputError("Couldn't parse Editor input as u32.");
				}break;

				case Tweeker_v2:
				{
					f32 newValue = StoF(t->string, &success);
					if (success)
					{
						switch (editor->panel.hotValueXYZ)
						{
						case 'x':
						{
							val.vec2->x = newValue;
						}break;
						case 'y':
						{
							val.vec2->y = newValue;
						}break;
						default:
						{
							Die;
						}break;
						}

						break;
					}
					ConsoleOutputError("Couldn't parse Editor input as f32.");
				}break;
				case Tweeker_v3:
				{
					f32 newValue = StoF(t->string, &success);
					if (success)
					{
						switch (editor->panel.hotValueXYZ)
						{
						case 'x':
						{
							val.vec3->x = newValue;
						}break;
						case 'y':
						{
							val.vec3->y = newValue;
						}break;
						case 'z':
						{
							val.vec3->z = newValue;
						}break;

						default:
						{
							Die;
						}break;
						}
						break;
					}
					ConsoleOutputError("Couldn't parse Editor input as f32.");
				}break;
				case Tweeker_v3i:
				{
					i32 newValue = StoI(t->string, &success);
					if (success)
					{
						switch (editor->panel.hotValueXYZ)
						{
						case 'x':
						{
							val.vec3i->x = newValue;
						}break;
						case 'y':
						{
							val.vec3i->y = newValue;
						}break;
						case 'z':
						{
							val.vec3i->z = newValue;
						}break;

						default:
						{
							Die;
						}break;
						}
						break;
					}
					ConsoleOutputError("Couldn't parse Editor input as i32.");
				}break;
				case Tweeker_EulerAngle:
				{
					f32 newValue = StoF(t->string, &success);

					if (!success)
					{
						ConsoleOutputError("Couldn't parse Editor input as f32.");
						break;
					}
					Quaternion q = *val.q;
					EulerAngle angle = QuaternionToEulerAngle(q);
					switch (editor->panel.hotValueXYZ)
					{
					case 'x':
					{
						angle.XRotation = newValue / 360.0f * PI;
					}break;
					case 'y':
					{
						angle.YRotation = newValue / 360.0f * PI;
					}break;
					case 'z':
					{
						angle.ZRotation = newValue / 360.0f * PI;
					}break;

					default:
					{
						Die;
					}break;
					}

					*val.q = EulerAngleToQuaternion(angle);

				}break;


				InvalidDefaultCase;
				}

				t->string.length = 0;
				editor->panel.hotValue = 0xFFFFFFFF;

				EditorPushUndo(editor, world);
				ResetSelectionInitials(editor, world);
				EditorGoToNone(editor);
				break;
			}break;
			case Key_leftMouse:
			case Key_rightMouse:
			{
				t->string.length = 0;
				editor->panel.hotValue = 0xFFFFFFFF;
				EditorGoToNone(editor);
			}break;


			}

		}break;
		case EditorState_PlacingNewMesh:
		{
			switch (message.key)
			{
			case Key_leftMouse:
			{
				v2 clickedP = ScreenZeroToOneToInGame(world->camera, input.mouseZeroToOne);

				v3i clickedTile = SnapToTileMap(i12(clickedP));
				v3 clickedOffset = V3(clickedTile) - V3(clickedP, 0.0f);

				EditorSelectNothing(editor);

				For(editor->clipBoard)
				{
					v3i pos = it->physicalPos + clickedTile;
					v3 offset = editor->snapToTileMap ? V3() : (it->offset + clickedOffset);
					Entity mesh = CreateEntity(world, it->meshId, it->scale, it->orientation, pos, offset, it->color, it->type, it->flags);
					EditorSelect add;
					add.initialOrientation = mesh.orientation;
					add.initialPhysicalPos = mesh.physicalPos;
					add.initialOffset = mesh.offset;
					add.initialScale = mesh.scale;
					add.initialColor = mesh.color;
					add.placedSerial = mesh.serialNumber;
					ArrayAdd(&editor->hotEntityInfos, add);
				}

				EditorPushUndo(editor, world); // needs them to be selected
				ResetSelectionInitials(editor, world);
				EditorGoToNone(editor);
			}break;
			case Key_rightMouse:
			{
				ResetSelectionInitials(editor, world);
				EditorGoToNone(editor);
			}break;
			}
		}break;
		case EditorState_DeleteSelection:
		{
			switch (message.key)
			{
			case Key_leftMouse:
			{
				// needs the meshes to be selected, so not yet dead
				EditorPushUndo(editor, world);
				For(editor->hotEntityInfos)
				{
					RemoveEntity(world, it->placedSerial);
				}
				EditorSelectNothing(editor);
				ResetSelectionInitials(editor, world);
				EditorGoToNone(editor);
			}break;
			case Key_rightMouse:
			{
				ResetSelectionInitials(editor, world);
				EditorGoToNone(editor);
			}break;
			}
		}break;
		case EditorState_OrbitingCamera:
		{

		}break;
		InvalidDefaultCase;
		}
	}

	if (message.flag & KeyState_Up)
	{
		switch (message.key)
		{

		case Key_leftMouse:
		{
			switch (editor->state)
			{
			case EditorState_DragingPanel:
			{
				EditorGoToNone(editor);
			}break;
			}

		}break;

		case Key_middleMouse:
		{
			if (editor->state == EditorState_OrbitingCamera)
			{
				editor->state = EditorState_Default;
			}

		}break;
		}
	}

}
