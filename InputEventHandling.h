
static u32 GetHotUnit(EntityManager *entityManager, AssetHandler *assetHandler, v2 mousePosZeroToOne, Camera camera)
{
	v3 camP = camera.pos; // todo camera or debugCamera? Maybe we should again unify them
	v3 camD = ScreenZeroToOneToDirecion(camera, mousePosZeroToOne);
   
	For(entityManager->unitArray)
	{
		Entity *e = it;
		m4x4 mat = QuaternionToMatrix4(Inverse(e->orientation));
		v3 rayP = mat * (camP - e->visualPos);
		v3 rayD = mat * camD;
      
		MeshInfo *info = GetMeshInfo(assetHandler, e->meshId);
		if (!info)continue;
		AABB aabb = info->aabb;
		aabb.maxDim *= e->scale;
		aabb.minDim *= e->scale;
		f32 curIntersectionMin = MAXF32;
      
		f32 x = rayP.x;
		f32 dx = rayD.x;
		f32 y = rayP.y;
		f32 dy = rayD.y;
		f32 z = rayP.z;
		f32 dz = rayD.z;
      
		f32 aabbMinX = aabb.minDim.x;
		f32 aabbMaxX = aabb.maxDim.x;
		f32 aabbMinY = aabb.minDim.y;
		f32 aabbMaxY = aabb.maxDim.y;
		f32 aabbMinZ = aabb.minDim.z;
		f32 aabbMaxZ = aabb.maxDim.z;
      
		f32 t1x = (aabbMaxX - x) / dx;
		if (dx > 0 && t1x <= curIntersectionMin)
		{
			curIntersectionMin = t1x;
		}
      
		f32 t2x = (aabbMinX - x) / dx;
		if (dx < 0 && t2x <= curIntersectionMin)
		{
			curIntersectionMin = t2x;
		}
      
		f32 t1y = (aabbMaxY - y) / dy;
		if (dy > 0 && t1y <= curIntersectionMin)
		{
			curIntersectionMin = t1y;
		}
      
		f32 t2y = (aabbMinY - y) / dy;
		if (dy < 0 && t2y <= curIntersectionMin)
		{
			curIntersectionMin = t2y;
		}
      
		f32 t1z = (aabbMaxZ - z) / dz;
		if (dz > 0 && t1z <= curIntersectionMin)
		{
			curIntersectionMin = t1z;
		}
      
		f32 t2z = (aabbMinZ - z) / dz;
		if (dz < 0 && t2z <= curIntersectionMin)
		{
			curIntersectionMin = t2z;
		}
		v3 curExit = rayD * curIntersectionMin + rayP;
      
		if (PointInAABB(aabb, curExit)) return (u32)(it - entityManager->unitArray.data);
	}
   
	return 0xFFFFFFFF;
}

static void PathCreatorHandleEvent(EntityManager *entityManager, ExecuteData *exe, AssetHandler *assetHandler, KeyStateMessage message, Input input)
{
	Camera cam = exe->camera;
	PathCreator *pathCreator = &exe->pathCreator;
	if (message.flag & KeyState_PressedThisFrame)
	{
      if(message.key == Key_tab) exe->debug = !exe->debug;
      if(message.key == Key_middleMouse) exe->middleMouseDown = true;
      
		switch (pathCreator->state)
		{
         case PathCreator_None:
         {
            switch (message.key)
            {
               case Key_leftMouse:
               {
                  u32 unitIndex = GetHotUnit(entityManager, assetHandler, input.mouseZeroToOne, cam);
                  
                  if (unitIndex == 0xFFFFFFFF)
                  {
                     break;
                  }
                  
                  pathCreator->hotUnit = unitIndex;
                  pathCreator->state = PathCreator_CreatingPath;
                  
                  ResetEntityManager(entityManager);
                  f32 dt = 1.0f;
                  
                  For(entityManager->unitData[unitIndex].instructions)
                  {
                     GameExecuteUpdate(entityManager, exe, assetHandler, dt, input);
                  }
               }break;
               case Key_F6:
               {
                  ChangeExecuteState(entityManager, exe, Execute_Simulation);
               }break;
            }
         }break;
         
         case PathCreator_CreatingPath:
         {
            Assert(pathCreator->hotUnit != 0xFFFFFFFF);
            
            switch (message.key)
            {
               case Key_leftMouse:
               {
                  
                  if (PointInRectangle(pathCreator->finishButton, input.mouseZeroToOne))
                  {
                     pathCreator->state = PathCreator_None;
                     pathCreator->hotUnit = 0xFFFFFFFF;
                     ResetEntityManager(entityManager);
                     break;
                  }
                  
                  UnitData *data = entityManager->unitData + pathCreator->hotUnit;
                  Entity *entity = GetEntity(entityManager, data->serial);
                  auto program = &data->instructions;
                  if (PointInRectangle(pathCreator->resetUnitButton, input.mouseZeroToOne))
                  {
                     program->amount = 0;
                     break;
                  }
                  
                  v3 p = ScreenZeroToOneToZ(cam, input.mouseZeroToOne, entity->physicalPos.z);
                  v3 d = p - entity->visualPos;
                  
                  i32 xSign = (d.x > 0) ? 1 : -1;
                  i32 ySign = (d.y > 0) ? 1 : -1;
                  
                  // todo maybe combine these with render? so we are sure that these are always doing the same thing
                  if (BoxNorm(d) < 0.5f)
                  {
                     ArrayAdd(program, Unit_Wait);
                  }
                  else if ((f32)ySign * d.y > (f32)xSign * d.x)
                  {
                     if (ySign > 0)
                     {
                        ArrayAdd(program, Unit_MoveUp);
                     }
                     else
                     {
                        ArrayAdd(program, Unit_MoveDown);
                     }
                  }
                  else
                  {
                     if (xSign > 0)
                     {
                        ArrayAdd(program, Unit_MoveRight);
                     }
                     else
                     {
                        ArrayAdd(program, Unit_MoveLeft);
                     }
                  }
                  
               }break;
               case Key_rightMouse:
               {
                  UnitData *data = entityManager->unitData + pathCreator->hotUnit;
                  auto program = &data->instructions;
                  if (!program->amount) break;
                  --program->amount;
               }break;
               case Key_F6:
               {
                  ChangeExecuteState(entityManager, exe, Execute_Simulation);
               }break;
               
            }
         }break;
         default:
         {
            Die;
         }break;
		}
	}
   
   if(message.flag & KeyState_ReleasedThisFrame)
   {
      if(message.key == Key_middleMouse) exe->middleMouseDown = false;
   }
   
}

static void SimHandleEvents(EntityManager *entityManager, ExecuteData *exe, KeyStateMessage message)
{
	SimData *sim = &exe->simData;
	if (message.flag & KeyState_PressedThisFrame)
	{
      if(message.key == Key_middleMouse) exe->middleMouseDown = true;
      if(message.key == Key_tab) exe->debug = !exe->debug;
      
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
         case Key_F6:
         {
            ChangeExecuteState(entityManager, exe, Execute_PathCreator);
         }break;
		}
	}
   
   
   if(message.flag & KeyState_ReleasedThisFrame)
   {
      if(message.key == Key_middleMouse) exe->middleMouseDown = false;
   }
}

static void VictoryHandleEvents(ExecuteData *exe, EntityManager *entityManager, KeyStateMessage message)
{
	if (message.flag & KeyState_PressedThisFrame)
	{
		ChangeExecuteState(entityManager, exe, Execute_PathCreator);
	}
}

static void ExecuteHandleEvents(EntityManager *entityManager, AssetHandler *assetHandler, ExecuteData *exe, KeyStateMessage message, Input input)
{
	switch (exe->state)
	{
      case Execute_Simulation:
      {
         SimHandleEvents(entityManager, exe, message);
      }break;
      case Execute_PathCreator:
      {
         PathCreatorHandleEvent(entityManager, exe, assetHandler, message, input);
      }break;
      case Execute_Victory:
      {
         VictoryHandleEvents(exe, entityManager, message);
      }break;
      
      InvalidDefaultCase;
      
	}
	
}

static void ColorPickersHandleEvents(Editor *editor, KeyStateMessage message, Input input)
{
   
   For(picker, editor->colorPickers)
   {
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
            
            UnorderedRemove(&editor->colorPickers, (u32)(picker-- - editor->colorPickers.data));
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
   }
}

static v3i AdjustForCamera(Camera cam, KeyEnum key)
{
	v3 camV = B3(cam.orientation);
   //v3 _camV = QuaternionToMatrix3(cam.orientation) * V3(0, 0, 1); these are indeed equal
   
   f32 x = camV.x;
   f32 y = camV.y;
   
	if (Abs(x) > Abs(y)) // which one matters more
	{
		if(x > 0)
      {
         // we are looking in V3(1, 0, 0) direction.
         switch(key)
         {
            case Key_up:
            {
               return V3i(1, 0, 0);
            }break;
            case Key_down:
            {
               return V3i(-1, 0, 0);
            }break;
            case Key_left:
            {
               return V3i(0, -1, 0);
            }break;
            case Key_right:
            {
               return V3i(0, 1, 0);
            }break;
         }
         
      }
      else
      {
         // we are looking in V3(-1, 0, 0) direction.
         switch(key)
         {
            case Key_up:
            {
               return V3i(-1, 0, 0);
            }break;
            case Key_down:
            {
               return V3i(1, 0, 0);
            }break;
            
            case Key_left:
            {
               return V3i(0, 1, 0);
            }break;
            case Key_right:
            {
               return V3i(0, -1, 0);
            }break;
         }
      }
	}
   else // y matters more
   {
      if(y > 0)
      {
         switch(key)
         {
            case Key_up:
            {
               return V3i(0, 1, 0);
            }break;
            case Key_down:
            {
               return V3i(0, -1, 0);
            }break;
            
            case Key_left:
            {
               return V3i(1, 0, 0);
            }break;
            case Key_right:
            {
               return V3i(-1, 0, 0);
            }break;
         }
      }
      else
      {
         switch(key)
         {
            case Key_up:
            {
               return V3i(0, -1, 0);
            }break;
            case Key_down:
            {
               return V3i(0, 1, 0);
            }break;
            case Key_left:
            {
               return V3i(-1, 0, 0);
            }break;
            case Key_right:
            {
               return V3i(1, 0, 0);
            }break;
         }
      }
   }
   
	return V3i();
}

static void EditorHandleEvents(Editor *editor, AssetHandler *assetHandler, KeyStateMessage message, Input input, Arena *currentStateArena)
{
   
   EditorEntities *editorEntities = &editor->editorEntities;
   Camera *cam = &editor->camera;
   
	For(picker, editor->colorPickers)
	{
		
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
            UnorderedRemove(&editor->colorPickers, (u32)(picker-- - editor->colorPickers.data));
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
   }
   
   if (message.flag & KeyState_PressedThisFrame)
   {
      switch (editor->state)
      {
         case EditorState_PickingColor:
         {
            EditorPushUndo(editor);
            ResetSelectionInitials(editor);
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
                  if (editor->panel.visible && !editor->panelIsHidden &&  PointInRectangle(editor->panel.pos, editorPanelWidth, editorPanelHeight, input.mouseZeroToOne))
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
                              if (editor->hotEntitySerials.amount != 1) break;
                              Entity *e = GetEntity(editorEntities, editor->hotEntitySerials[0]);
                              
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
                                 ArrayAdd(&editor->colorPickers,  picker);
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
                  
                  Entity *hotEntity = GetHotEntity(editor->camera, editorEntities, assetHandler, input.mouseZeroToOne);
                  
                  if (message.flag & KeyState_ShiftDown)
                  {
                     if (!hotEntity) break;
                     
                     b32 allreadyIn = false;
                     For(editor->hotEntitySerials)
                     {
                        if (*it == hotEntity->serial)
                        {
                           allreadyIn = true;
                           u32 it_index = (u32)(it - editor->hotEntitySerials.data);
                           UnorderedRemove(&editor->hotEntitySerials, it_index);
                           UnorderedRemove(&editor->hotEntityInitialStates, it_index);
                        }
                     }
                     
                     if (allreadyIn) break;
                     
                     EntityData toAdd = EntityToData(*hotEntity);
                     
                     ArrayAdd(&editor->hotEntityInitialStates, toAdd);
                     ArrayAdd(&editor->hotEntitySerials, hotEntity->serial);
                     
                     editor->panel.values.amount = 0;
                     editor->panel.visible = true;
                     ArrayAdd(&editor->panel.values, CreateTweekerPointer(Tweeker_b32, "Edit Physical", &editor->editingPhysical));
                     break;
                  }
                  
                  // todo should this go inside if(hotEntity)?
                  EditorSelectNothing(editor);
                  
                  if (hotEntity)
                  {
                     EntityData toAdd = EntityToData(*hotEntity);
                     
                     ArrayAdd(&editor->hotEntityInitialStates, toAdd);
                     ArrayAdd(&editor->hotEntitySerials, hotEntity->serial);
                     editor->panel.visible = true;
                     ArrayAdd(&editor->panel.values, CreateTweekerPointer(Tweeker_b32, "Edit Physical", &editor->editingPhysical));
                     ArrayAdd(&editor->panel.values, CreateTweekerPointer(Tweeker_EntityType, "EntityType", &hotEntity->type));
                     ArrayAdd(&editor->panel.values, CreateTweekerPointer(Tweeker_v3i, "PhysicalPos", &hotEntity->physicalPos));
                     
                     // todo can we make this offset by using the to Add?
                     ArrayAdd(&editor->panel.values, CreateTweekerPointer(Tweeker_v3, "VisualPos", &hotEntity->visualPos));
                     ArrayAdd(&editor->panel.values, CreateTweekerPointer(Tweeker_f32, "Scale", &hotEntity->scale));
                     // todo make euler angle v3i
                     ArrayAdd(&editor->panel.values, CreateTweekerPointer(Tweeker_EulerAngle, "Orientation", &hotEntity->orientation));
                     ArrayAdd(&editor->panel.values, CreateTweekerPointer(Tweeker_v4, "Color", &hotEntity->color));
                     ArrayAdd(&editor->panel.values, CreateTweekerPointer(Tweeker_u32, "Serial", &hotEntity->serial));
                  }
               }break;
               case Key_tab:
               {
                  if (EditorHasSelection(editor))
                  {
                     editor->panelIsHidden = !editor->panelIsHidden;
                  }
               }break;
               case Key_left:
               {
                  if(!editor->hotEntitySerials) break;
                  v3i inc = AdjustForCamera(*cam, Key_left);
                  For(editor->hotEntitySerials)
                  {
                     Entity *mesh = GetEntity(editorEntities, *it);
                     mesh->physicalPos += inc;
                  }
                  EditorPushUndo(editor);
                  ResetSelectionInitials(editor);
               }break;
               case Key_right:
               {
                  if(!editor->hotEntitySerials) break;
                  v3i inc = AdjustForCamera(*cam, Key_right);
                  For(editor->hotEntitySerials)
                  {
                     Entity *mesh = GetEntity(editorEntities, *it);
                     mesh->physicalPos += inc;
                  }
                  EditorPushUndo(editor);
                  ResetSelectionInitials(editor);
                  
               }break;
               case Key_up:
               {
                  if(!editor->hotEntitySerials) break;
                  if (message.flag & KeyState_ControlDown)
                  {
                     For(editor->hotEntitySerials)
                     {
                        Entity *mesh = GetEntity(editorEntities, *it);
                        mesh->physicalPos += V3i(0, 0, -1);
                     }
                     break;
                  }
                  
                  v3i inc = AdjustForCamera(*cam, Key_up);
                  For(editor->hotEntitySerials)
                  {
                     Entity *mesh = GetEntity(editorEntities, *it);
                     mesh->physicalPos += inc;
                  }
                  EditorPushUndo(editor);
                  ResetSelectionInitials(editor);
               }break;
               case Key_down:
               {
                  if(!editor->hotEntitySerials) break;
                  if (message.flag & KeyState_ControlDown)
                  {
                     For(editor->hotEntitySerials)
                     {
                        Entity *mesh = GetEntity(editorEntities, *it);
                        mesh->physicalPos += V3i(0, 0, 1);
                     }
                     break;
                  }
                  
                  v3i inc = AdjustForCamera(*cam, Key_down);
                  For(editor->hotEntitySerials)
                  {
                     Entity *mesh = GetEntity(editorEntities, *it);
                     mesh->physicalPos += inc;
                  }
                  EditorPushUndo(editor);
                  ResetSelectionInitials(editor);
               }break;
               
               case Key_g:
               {
                  // we reset once we did an action
                  if (!editor->hotEntitySerials) break;
                  editor->state = EditorState_Moving;
               }break;
               case Key_r:
               {
                  if (!editor->hotEntitySerials) break;
                  editor->state = EditorState_Rotating;
               }break;
               case Key_s:
               {
                  if ((message.flag & KeyState_ControlDown))
                  {
                     if (!editor->levelInfo.name.amount) break;
                     
                     // todo maybe not needed...
                     For(editorEntities->entities)
                     {
                        it->initialPos = it->physicalPos;
                     }
                     
                     char *fileName = FormatCString("level/%s.level", editor->levelInfo.name);
                     Level level = EditorStateToLevel(editor);
                     if (WriteLevel(fileName, level, assetHandler))
                     {
                        ConsoleOutput("Saved!");
                     }
                     else
                     {
                        ConsoleOutputError("Could not save file %c*.", fileName);
                     }
                     
                     break;
                  }
                  
                  if (!editor->hotEntitySerials) break;
                  editor->state = EditorState_Scaling;
               }break;
               case Key_c:
               {
                  if (!editor->hotEntitySerials) break;
                  
                  if (message.flag & KeyState_ControlDown)
                  {
                     editor->clipBoard.amount = 0;
                     v3 averagePos = GetAveragePosForSelection(editor);
                     v3i averageTile = SnapToTileMap(averagePos);
                     averageTile.z = 0;
                     // we are in default state, so initalstates are just the actual states
                     Reserve(&editor->clipBoard, editor->hotEntityInitialStates.amount);
                     memcpy(editor->clipBoard.data, editor->hotEntityInitialStates.data, editor->hotEntityInitialStates.amount * sizeof(EntityData));
                     editor->clipBoard.amount = editor->hotEntityInitialStates.amount;
                     
                     For(editor->clipBoard) // slow?
                     {
                        it->physicalPos -= averageTile;
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
                     ArrayAdd(&editor->panel.values, CreateTweekerPointer(Tweeker_b32, "Edit physical position", &editor->editingPhysical));
                     editor->panel.visible = true;
                     
                     EditorSelectNothing(editor);
                  }
               }break;
               case Key_z:
               {
                  if (message.flag & KeyState_ControlDown)
                  {
                     EditorSelectNothing(editor);
                     EditorPerformUndo(editor);
                  }
               }break;
               case Key_y:
               {
                  if (message.flag & KeyState_ControlDown)
                  {
                     EditorSelectNothing(editor);
                     EditorPerformRedo(editor);
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
                  cam->orientation = AxisAngleToQuaternion(-PI / 2.0f, V3(1, 0, 0));
                  cam->pos = editor->focusPoint + V3(0, Norm(editor->focusPoint - cam->pos), 0);
                  
               }break;
               case Key_num3:
               {
                  // Right looking left
                  cam->orientation = AxisAngleToQuaternion(PI/2.0f, V3(0, 1, 0)) * AxisAngleToQuaternion(-PI/2.0f, V3(1, 0, 0));
                  cam->pos = editor->focusPoint + V3(Norm(editor->focusPoint - cam->pos), 0, 0);
                  
               }break;
               case Key_num7:
               {
                  // Top
                  cam->orientation = QuaternionId();
                  cam->pos = editor->focusPoint + V3(0, 0, -Norm(editor->focusPoint - cam->pos));
                  
               }break;
               case Key_num9:
               {
                  // toggle
                  cam->orientation = AxisAngleToQuaternion(PI, V3(0, 1, 0)) * cam->orientation;
                  cam->pos = editor->focusPoint - (cam->pos - editor->focusPoint);
                  
               }break;
               case Key_numDot:
               {
                  if (!EditorHasSelection(editor))
                  {
                     break;
                  }
                  
                  v3 averagePos = GetAveragePosForSelection(editor);
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
                  EditorPushUndo(editor);
                  ResetSelectionInitials(editor);
                  EditorGoToNone(editor);
               }break;
               case Key_rightMouse:
               {
                  ResetHotMeshes(editor);
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
                  if (editor->editingPhysical)
                  {
                     for(u32 i = 0; i < editor->hotEntitySerials.amount; i++)
                     {
                        Entity *e = GetEntity(editorEntities, editor->hotEntitySerials[i]);
                        e->visualPos = V3(e->physicalPos) + editor->hotEntityInitialStates[i].offset;
                     }
                  }
                  
                  EditorPushUndo(editor);
                  ResetSelectionInitials(editor);
                  EditorGoToNone(editor);
               }break;
               case Key_rightMouse:
               {
                  ResetHotMeshes(editor);
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
                  EditorPushUndo(editor);
                  ResetSelectionInitials(editor);
                  EditorGoToNone(editor);
               }break;
               case Key_rightMouse:
               {
                  ResetHotMeshes(editor);
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
            Char c = KeyToChar(message.key, false);
            if (c == '-' || c == '.' || '0' <= c && c <= '9')
            {
               if (t->string.length + 1 <= t->maxLength)
               {
                  t->string[t->string.length++] = c;
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
                              angle.XRotation = (f32)((newValue - 180.0f) / 360.0f * PI * 2.0f);
                           }break;
                           case 'y':
                           {
                              angle.YRotation = (f32)((newValue -180.0f) / 360.0f * PI * 2.0f);
                           }break;
                           case 'z':
                           {
                              angle.ZRotation = (f32)((newValue - 180.0f) / 360.0f * PI * 2.0f);
                           }break;
                           
                           default:
                           {
                              Die;
                           }break;
                        }
                        
                        *val.q = EulerAngleToQuaternion(angle);
                        EulerAngle agains = QuaternionToEulerAngle(*val.q);
                        i32 sd = 32;
                     }break;
                     
                     
                     InvalidDefaultCase;
                  }
                  
                  t->string.length = 0;
                  editor->panel.hotValue = 0xFFFFFFFF;
                  
                  EditorPushUndo(editor);
                  ResetSelectionInitials(editor);
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
                  v2 clickedP = ScreenZeroToOneToInGame(editor->camera, input.mouseZeroToOne);
                  
                  v3i clickedTile = SnapToTileMap(i12(clickedP));
                  v3 clickedOffset = V3(clickedTile) - V3(clickedP, 0.0f);
                  
                  EditorSelectNothing(editor);
                  
                  For(editor->clipBoard)
                  {
                     v3i pos = it->physicalPos + clickedTile;
                     v3 offset = (it->offset);
                     
                     Entity *e = CreateEntity(editorEntities, it->type,it->meshId, pos, it->scale, it->orientation, offset, it->color, it->flags);
                     
                     ArrayAdd(&editor->hotEntityInitialStates, EntityToData(*e));
                     ArrayAdd(&editor->hotEntitySerials, e->serial);
                  }
                  
                  EditorPushUndo(editor); // needs them to be selected
                  ResetSelectionInitials(editor);
                  EditorGoToNone(editor);
               }break;
               case Key_rightMouse:
               {
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
                  EditorPushUndo(editor);
                  For(editor->hotEntitySerials)
                  {
                     RemoveEntity(editorEntities, *it);
                  }
                  
                  EditorSelectNothing(editor);
                  EditorGoToNone(editor);
               }break;
               case Key_rightMouse:
               {
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

