#ifndef RR_EDITOR
#define RR_EDITOR

enum PickerSelectionTag
{
	PickerSelecting_Nothing,
	PickerSelecting_Slider,
	PickerSelecting_ColorPicker,
	PickerSelecting_Header,
};

// :ColorPicker

struct ColorPicker
{
	v2 pos = {0.5f, 0.5f};
	f32 width = 0.4f;
	f32 height = 0.4f;
	f32 border = 0.005f;
	
	v2 headerPos = {0.5f, 0.49f}; // = pickerPos - V2(0.0f, pickerBorder + pickerHeaderSize)
	f32 headerSize = 0.005f;
   
	v2 sliderPos = {0.5f, 0.905f}; // = pickerPos + V2(0.0f, pickerBorder + pickerHeight);
	f32 sliderHeight = 0.05f;
	f32 sliderSelectedColorPos = 0.0f;
	u32 sliderColor = 0xFFFF0000;
	b32 selecting = 0;
   
	v2 killRectPos = {0.895f, 0.49f}; // V2(headerPos.x + picker->width - picker->headerSize, headerPos.y);
   
	v2 pickerPos = {0.0f, 0.0f}; // relative to the Color picker. (0, 0) is pure white
   
	v4 *pickedColor;
   
	b32 isTweeker;
	Tweeker *tweeker; // for now, rethink this
};
DefineDynamicArray(ColorPicker);

// todo: i don't think it matters here, but I really have to get these color things straight.
static ColorPicker CreateColorPicker(v4 *initialColor)
{
   // WARINIG : the factors DO NOT commute with the interpolation, do they?
   
   // calculating bar pos and picker pos : colors in the bar are of the form 0xFF00abFF or 0xFFabFF00, or something like that (2F, 20, 2 something)
   ColorPicker ret;
   
   ret.isTweeker = false;
   Tweekable(f32, pickerBorder);
   Tweekable(f32, pickerWidth);
   Tweekable(f32, pickerHeight);
   Tweekable(f32, pickerSliderHeight);
   Tweekable(f32, pickerHeaderSize);
   
   ret.border = pickerBorder;
   ret.width = pickerWidth;
   ret.height = pickerHeight;
   ret.sliderHeight = pickerSliderHeight;
   ret.headerSize = pickerSliderHeight;
   
   ret.headerPos = ret.pos - V2(0.0f, ret.headerSize + ret.border);
   ret.sliderPos = ret.pos + V2(0.0f, ret.border + ret.height);
   ret.killRectPos = V2(ret.headerPos.x + ret.width - ret.headerSize, ret.headerPos.y);
   
   ret.pickedColor = initialColor;
   u32 c = Pack4x8(*initialColor);
   u32 a = (c >> 24) & 0xFF;
   u32 b = (c >> 16) & 0xFF;
   u32 g = (c >> 8) & 0xFF;
   u32 r = (c >> 0) & 0xFF;
   
   u32 max = Max(Max(g, b), r);
   
   if (max == 0)
   {
      ret.pickerPos = V2(ret.width, ret.height);
      ret.sliderColor = (0xFF << 24) | (0xFF << 16) | (0u << 8) | 0u;
      ret.sliderSelectedColorPos = 0.0f;
      return ret;
   }
   
   f32 yfac = (f32)(0xFF - max) / 255.0f;
   b *= 0xFF;
   g *= 0xFF;
   r *= 0xFF;
   b /= max;
   g /= max;
   r /= max;
   
   u32 min = Min(Min(g, b), r);
   u32 addedInX = min;
   f32 xfac = (f32)addedInX / 255.0f;
   
   if (min == 0xFF)
   {
      ret.pickerPos = V2(0, 0);
      ret.sliderColor = (0xFF << 24) | (0xFF << 16) | (0u << 8) | 0u;
      ret.sliderSelectedColorPos = 0.0f;
      return ret;
   }
   
   //work in u32, for percision 
   u32 bs = (b - addedInX) * 0xFF / (0xFF - addedInX); // added in X = 0xFF? 
   u32 rs = (r - addedInX) * 0xFF / (0xFF - addedInX);
   u32 gs = (g - addedInX) * 0xFF / (0xFF - addedInX);
   
   ret.pickerPos = V2( (1.0f - xfac) * ret.width, yfac * ret.height);
   ret.sliderColor = (0xFF << 24) | (bs << 16) | (gs << 8) | rs;
   
   f32 scale = 1.0f / 6.0f;
   f32 scaledWidth = scale * ret.width;
   
   if (bs == 0xFF)
   {
      if (rs > 0) // purple to blue
      {
         f32 factor = (f32)rs / 255.0f;
         ret.sliderSelectedColorPos = (5.0f + factor) * scaledWidth;
      }
      else // blue to cyan
      {
         f32 factor = (f32)gs / 255.0f;
         ret.sliderSelectedColorPos = (0.0f + factor) * scaledWidth;
      }
   }
   else if(gs == 0xFF)
   {
      if (bs > 0) // cyan to green
      {
         f32 factor = (f32)bs / 255.0f;
         ret.sliderSelectedColorPos = (1.0f + factor) * scaledWidth;
      }
      else // green to yellow
      {
         f32 factor = (f32)rs / 255.0f;
         ret.sliderSelectedColorPos = (2.0f + factor) * scaledWidth;
      }
   }
   else if(rs == 0xFF)
   {
      if (gs > 0) // yellow to red
      {
         f32 factor = (f32)gs / 255.0f;
         ret.sliderSelectedColorPos = (3.0f + factor) * scaledWidth;
      }
      else // red to purple
      {
         f32 factor = (f32)bs / 255.0f;
         ret.sliderSelectedColorPos = (4.0f + factor) * scaledWidth;
      }
   }
   else
   {
      Die;
      ret.sliderSelectedColorPos = 0.0f;
   }
   return ret;
}
DefineArray(ColorPicker);


static void UpdateColorPicker(ColorPicker *picker, Input input)
{
   Tweekable(f32, pickerBorder);
   Tweekable(f32, pickerWidth);
   Tweekable(f32, pickerHeight);
   Tweekable(f32, pickerSliderHeight);
   Tweekable(f32, pickerHeaderSize);
   
   // todo remove this, once we can drag the window
   picker->border = pickerBorder;
   picker->width = pickerWidth;
   picker->height = pickerHeight;
   picker->sliderHeight = pickerSliderHeight;
   picker->headerSize = pickerHeaderSize;
   
   if (picker->selecting == PickerSelecting_Header)
   {
      picker->pos += input.mouseZeroToOneDelta;
      v2 sliderPos = picker->pos + V2(0.0f, picker->border + picker->height);
      picker->sliderPos = sliderPos;
      
   }
   
   if (picker->selecting == PickerSelecting_ColorPicker)
   {
      picker->pickerPos.x = Clamp(input.mouseZeroToOne.x, picker->pos.x, picker->pos.x + picker->width) - picker->pos.x;
      picker->pickerPos.y = Clamp(input.mouseZeroToOne.y, picker->pos.y, picker->pos.y + picker->height) - picker->pos.y;
   }
   
   u32 colors[7] =
   {
      0xFFFF0000, // blue
      0xFFFFFF00, // cyan
      0xFF00FF00, // green
      0xFF00FFFF, // yellow
      0xFF0000FF, // red
      0xFFFF00FF, // purple
      0xFFFF0000, // blue
   };
   
   f32 scale = 1.0f / 6.0f;
   f32 scaledWidth = scale * picker->width;
   if (picker->selecting == PickerSelecting_Slider)
   {
      MapRangeToRangeCapped(input.mouseZeroToOne.x, picker->sliderPos.x, picker->sliderPos.x + picker->width, 0.0f, 1.0f);
      picker->sliderSelectedColorPos = Clamp(input.mouseZeroToOne.x - picker->sliderPos.x, 0.0f, picker->width);
      u32 index = (u32)(picker->sliderSelectedColorPos / (scaledWidth));
      Assert(index < ArrayCount(colors));
      u32 secondIndex = index + 1;
      if (index == 6) secondIndex = index;
      
      picker->sliderColor = Pack3x8(LerpVector3(Unpack3x8(colors[index]), index * scaledWidth, Unpack3x8(colors[secondIndex]), secondIndex * scaledWidth, picker->sliderSelectedColorPos));
   }
   
   v3 ul = V3(1.0f, 1.0f, 1.0f);
   v3 ur = Unpack3x8(picker->sliderColor);
   v3 bottom = V3(0.0f, 0.0f, 0.0f);
   
   f32 xfac = MapRangeToRangeCapped(picker->pickerPos.x, 0.0f, picker->width, 0.0f, 1.0f);
   f32 yfac = MapRangeToRangeCapped(picker->pickerPos.y, 0.0f, picker->height, 0.0f, 1.0f);
   
   v3 interpWithWhite = LerpVector3(ul, ur, xfac);
   v3 color = LerpVector3(interpWithWhite, bottom, yfac);
   
   *picker->pickedColor = V4(1.0f, color);
   
   //todo if we use this in some other context, we either have to make everything tweekers, or have to re-formalize this.
   //picker->tweeker.vec4 = picker->pickedColor;
   //Tweek(picker->tweeker);
}


// :EditorPanel

struct TweekerPointer
{
	TweekerType type;
	String name;
   
	union 
	{
		u32 *u;
		f32 *f;
		v2 *vec2;
		v3 *vec3;
		v3i *vec3i;
		v4 *vec4;
		b32 *b;
		Quaternion *q;
	};
   
};

static TweekerPointer CreateTweekerPointer(TweekerType type, char *name, void *address)
{
	TweekerPointer ret;
	ret.type = type;
	ret.name = CreateString(name);
	ret.u = (u32 *)address;
	return ret;
}

DefineDynamicArray(TweekerPointer);

struct TextInput
{
	String string;
	u32 maxLength;
};

struct EditorPanel
{
	b32 visible;
	v2 pos;
	u32 hotValue;
	Char hotValueXYZ;
	TextInput textInput;
   
	TweekerPointerDynamicArray values;
};


static f32 HeightForTweekerPanel(TweekerType type, f32 editorBorderWidth, f32 editorPanelFontSize)
{
	switch (type)
	{
      case Tweeker_b32:
      {
         return (editorPanelFontSize + editorBorderWidth);
      }break;
      case Tweeker_u32:
      {
         return (editorPanelFontSize + editorBorderWidth);
      }break;
      case Tweeker_EntityType:
      {
         return (2.0f * editorPanelFontSize + editorBorderWidth);
      }break;
      case Tweeker_f32:
      {
         return (editorPanelFontSize + editorBorderWidth);
      }break;
      case Tweeker_v2:
      {
         return (3.0f * editorPanelFontSize + editorBorderWidth);
         
      }break;
      case Tweeker_v3:
      {
         return (4.0f * editorPanelFontSize + editorBorderWidth);
      }break;
      case Tweeker_v3i:
      {
         return (4.0f * editorPanelFontSize + editorBorderWidth);
      }break;
      case Tweeker_EulerAngle:
      {
         return (4.0f * editorPanelFontSize + editorBorderWidth);
      }break;
      case Tweeker_v4:
      {
         f32 colorRectSize = 3.0f * editorPanelFontSize;
         f32 height = 2.0f * editorBorderWidth + colorRectSize;
         
         return height;
      }break;
      default:
      {
         Die;
         return 0.0f;
      }break;
	}
   
}


enum EditorState
{
	EditorState_Default					= 0x0,
	EditorState_Scaling					= 0x1,
	EditorState_Rotating		   		= 0x2,
	EditorState_Moving		 			= 0x3,
	EditorState_DragingPanel   			= 0x4,
	EditorState_PickingColor	   		= 0x5,
	EditorState_AlteringValue		  	= 0x6,
	EditorState_PlacingNewMesh		 	= 0x7,
	EditorState_DeleteSelection			= 0x8,
   
	EditorState_OrbitingCamera		 	= 0x100,
   
};

static String EditorStateToString(EditorState state)
{
	switch (state)
	{
      case EditorState_Default:					return CreateString("Default");
      case EditorState_Scaling:					return CreateString("Scaling");
      case EditorState_Rotating:					return CreateString("Rotating");
      case EditorState_Moving:					return CreateString("Moving");
      case EditorState_DragingPanel:				return CreateString("DraggingPanel");
      case EditorState_PickingColor:				return CreateString("PickingColor");
      case EditorState_AlteringValue:				return CreateString("AlteringValue"); 
      case EditorState_PlacingNewMesh:			return CreateString("PlacingNewValue");
      case EditorState_DeleteSelection:			return CreateString("DeleteSelection");
      case EditorState_OrbitingCamera:			return CreateString("OrbitingCamera");
      
	}
	return CreateString("EditorState_Unknown");
}

enum EditorActionType
{
	EditorAction_None,
	EditorAction_AlterMesh,
	EditorAction_DeleteMesh,
	EditorAction_PlaceMesh,
   
	EditorAction_AlterTile,
   
	EditorAction_BeginBundle,
	EditorAction_EndBundle,
   
	EditorAction_Count,
   
};

struct EditorAction
{
	EditorActionType type;
   
   u32 serial;
   
	// we could store only one, by altering it if we redo/undo
   EntityCopyData preModifyMesh;
   EntityCopyData postModifyMesh;
	
	
};
DefineDynamicArray(EditorAction);

struct EditorEntities
{
   // todo make some sort of bucketed array.
	
	EntityDynamicArray entities;
   u32 entitySerializer;
	u32DynamicArray entitySerialMap;
};

static Entity *GetEntity(EditorEntities *editorEntities, u32 serialNumber)
{
	u32 index = editorEntities->entitySerialMap[serialNumber];
	Entity *ret = editorEntities->entities + index;
	return ret;
}

static Entity *CreateEntity(EditorEntities *editorEntities, EntityType type, u32 meshID, v3i pos, f32 scale, Quaternion orientation, v3 offset, v4 color, u64 flags)
{
	Entity ret;
	ret.meshId = meshID;
	ret.scale = scale;
	ret.orientation = orientation;
	ret.physicalPos = pos;
	ret.initialPos = pos;
	ret.offset = offset;
	ret.color = color;
	ret.frameColor = V4(1, 1, 1, 1);
	ret.serialNumber = editorEntities->entitySerializer++;
	ret.type = type;
	ret.flags = flags;
	ret.interpolation = {};
	
	u32 arrayIndex = ArrayAdd(&editorEntities->entities, ret);
   
	Assert(ret.serialNumber == editorEntities->entitySerialMap.amount);
	ArrayAdd(&editorEntities->entitySerialMap, arrayIndex);
	
	return editorEntities->entities + arrayIndex;
};

static void RestoreEntity(EditorEntities *editorEntities, u32 serial, u32 meshID, f32 scale, Quaternion orientation, v3i pos, v3 offset, v4 color, EntityType type, u64 flags)
{
	Entity ret;
	ret.meshId = meshID;
	ret.scale = scale;
	ret.orientation = orientation;
	ret.physicalPos = pos;
	ret.offset = offset;
	ret.color = color;
	ret.frameColor = V4(1, 1, 1, 1);
	ret.serialNumber = serial;
	ret.type = type;
	ret.flags = flags;
	ret.interpolation = {};
   
	u32 arrayIndex = ArrayAdd(&editorEntities->entities, ret);
   
	Assert(editorEntities->entitySerialMap[serial] == 0xFFFFFFFF);
	editorEntities->entitySerialMap[serial] = arrayIndex;
}

static void RemoveEntity(EditorEntities *editorEntities, u32 serial)
{
	u32 index = editorEntities->entitySerialMap[serial];
	Entity *toRemove = editorEntities->entities + index;
   
	editorEntities->entitySerialMap[serial] = 0xFFFFFFFF;
   
	u32 lastIndex = editorEntities->entities.amount - 1;
	if (index != lastIndex)
	{
		u32 serialNumberToChange = editorEntities->entities[lastIndex].serialNumber;
		editorEntities->entitySerialMap[serialNumberToChange] = index;
	}
	UnorderedRemove(&editorEntities->entities, index);
}

static EditorEntities InitEditorEntities(Arena *currentStateArena, Level *level)
{
	EditorEntities ret;
   
   ret.entitySerializer = 0;
	ret.entities = EntityCreateDynamicArray(level->entities.amount);
   ret.entitySerialMap = u32CreateDynamicArray(level->entities.amount);
   
   For(level->entities)
   {
      CreateEntity(&ret, it->type, it->meshId, it->physicalPos, it->scale, it->orientation, it->offset, it->color, it->flags);
   }
   
	return ret;
}

typedef EntityCopyDataDynamicArray EditorClipBoard;

struct EditorLevelInfo
{
   String name;
   Camera camera;
   LightSource lightSource;
   u32 blocksNeeded;
};

struct Editor
{
   EditorState state = EditorState_Default;
   
   Camera camera;
	v3 focusPoint;
   
   EditorLevelInfo levelInfo;
   
   EditorEntities editorEntities;
   
   u32DynamicArray hotEntitySerials;
	EntityCopyDataDynamicArray hotEntityInitialStates;
   
	EditorClipBoard clipBoard;
   
	EditorActionDynamicArray undoRedoBuffer;
	u32 undoRedoAt;
   
	b32 snapToTileMap; // todo if there are more flags, we could bundle em here
   
   EditorPanel panel;
   ColorPickerDynamicArray colorPickers;
};

static void EditorGoToNone(Editor *editor)
{
   editor->state = EditorState_Default;
}

static bool EditorHasSelection(Editor *editor)
{
   return (editor->hotEntitySerials.amount > 0);
}

static void ResetEditorPanel(Editor *editor)
{
   editor->panel.values.amount = 0;
   editor->panel.visible = false;
}

static void EditorSelectNothing(Editor *editor) 
{
   editor->hotEntitySerials.amount = 0;
   editor->hotEntityInitialStates.amount = 0;
   ResetEditorPanel(editor);
}

static void NewLevel(Editor *editor)
{
   EditorSelectNothing(editor);
   EditorGoToNone(editor);
   ResetEditorPanel(editor);
   
   editor->undoRedoBuffer.amount = 0;
   editor->undoRedoAt = 0xFFFFFFFF;
   
   editor->focusPoint = V3();
   
   EditorEntities *editorEntities = &editor->editorEntities;
	editorEntities->entities.amount = 0; // this before ResetTree, makes the reset faster
	editorEntities->entitySerialMap.amount = 0;
	editorEntities->entitySerializer = 0;
}


static v3 GetAveragePosForSelection(Editor *editor)
{
	v3 averagePos = V3();
	For(editor->hotEntitySerials)
	{
      Entity *e = GetEntity(&editor->editorEntities, *it);
		averagePos += GetRenderPos(*e);
	}
	averagePos /= (f32)editor->hotEntitySerials.amount;
	return averagePos;
}

static void EditorLoadLevel(Editor *editor, Arena *currentStateArena, Level *level)
{
   editor->editorEntities = InitEditorEntities(currentStateArena, level);
   
   EditorSelectNothing(editor);
   EditorGoToNone(editor);
   editor->undoRedoBuffer.amount = 0;
   editor->camera = level->camera;
   editor->levelInfo.camera = level->camera;
   editor->levelInfo.name = CopyString(level->name, currentStateArena);
   editor->levelInfo.lightSource = level->lightSource;
   editor->levelInfo.blocksNeeded = level->blocksNeeded;
   editor->focusPoint = V3(ScreenZeroToOneToInGame(editor->camera, V2(0.5f, 0.5f)), 0.0f);
   
   ResetEditorPanel(editor);
}

// should the editor be a thing you load when you open it or instanciate?
static Editor InitEditor(Arena *constantArena)
{
   Editor ret;
   
   ret.state = EditorState_Default;
   
   ret.colorPickers   = ColorPickerCreateDynamicArray();
   ret.clipBoard      = EntityCopyDataCreateDynamicArray();
   
   ret.undoRedoBuffer = EditorActionCreateDynamicArray();
   ret.undoRedoAt     = 0xFFFFFFFF;
   
   ret.hotEntitySerials = u32CreateDynamicArray();
   ret.hotEntityInitialStates = EntityCopyDataCreateDynamicArray();
   
   Tweekable(v2, initialEditorPanelPos, V2(0.75f, 0.2f));
   
   ret.panel.pos = initialEditorPanelPos;
   ret.panel.visible = false;
   ret.panel.values = TweekerPointerCreateDynamicArray();
   ret.panel.textInput.string = PushArray(constantArena, Char, 50);
   ret.panel.textInput.string.length = 0;
   ret.panel.textInput.maxLength = 50;
   ret.panel.hotValue = 0xFFFFFFFF;
   
   ret.focusPoint = V3();
   
   return ret;
}

static void WriteSingleTweeker(Tweeker tweeker);

static void UpdateColorPickers(Editor *editor, Input input)
{
   For(editor->colorPickers)
   {
      UpdateColorPicker(it, input);
   }
}

static v3i SnapToTileMap(v3 pos)
{
   return V3i((i32)pos.x, (i32)pos.y, (i32)pos.z);
}

static v3i RoundToTileMap(v3 pos)
{
   return V3i((i32)floorf(pos.x + 0.5f), (i32)floorf(pos.y + 0.5f), (i32)floorf(pos.z + 0.5f));
}

static Entity *GetHotEntity(Camera cam, EditorEntities *editorEntities, AssetHandler *handler, v2 mousePosZeroToOne)
{
   
   
   v3 camP = cam.pos; // todo camera or debugCamera? Maybe we should again unify them
   v3 camD = ScreenZeroToOneToDirecion(cam, mousePosZeroToOne);
   
   f32 minDist = F32MAX;
   
   Entity *ret = NULL;
   
   For(editorEntities->entities)
   {
      
      MeshInfo *info = GetMeshInfo(handler, it->meshId);
      
      if (!info) continue; // probably not on screen, if never rendered
      
      m4x4 mat = QuaternionToMatrix4(Inverse(it->orientation)); // todo save these?
      v3 rayP = mat * (camP - GetRenderPos(*it));
      v3 rayD = mat * camD; 
      // better rayCast system, right now this loads every mesh, to find out the aabb....
      
      AABB aabb = info->aabb;
      
      aabb.maxDim *= it->scale;
      aabb.minDim *= it->scale;
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
      
      
      if (PointInAABB(aabb, curExit))
      {
         f32 dist = Dist(curExit, rayP);
         if (dist < minDist)
         {
            minDist = dist;
            ret = it;
         }
      }
   }
   
   return ret;
}

// todo when we make an afford to go data oriented, stuff like this will havet to go through an api.
static void ApplyEntityDataToEntity(Entity *e, EntityCopyData *data)
{
   e->orientation = data->orientation;
   e->physicalPos = data->physicalPos;
   e->offset	  = data->offset;
   e->color       = data->color;
   e->scale	   = data->scale;
   
   e->type		= data->type;
   e->flags	   = data->flags;
   
   e->meshId      = data->meshId;
}

static void ResetHotMeshes(Editor *editor)
{
   for(u32 i = 0; i < editor->hotEntitySerials.amount; i++)
   {
      Entity *e = GetEntity(&editor->editorEntities, editor->hotEntitySerials[i]);
      EntityCopyData *data = editor->hotEntityInitialStates + i;
      ApplyEntityDataToEntity(e, data);
   }
}

// kills all redos
static void AddActionToUndoRedoBuffer(Editor *editor, EditorAction toAdd)
{
   editor->undoRedoBuffer.amount = ++editor->undoRedoAt;
   
   ArrayAdd(&editor->undoRedoBuffer, toAdd);
   
   Assert((editor->undoRedoAt + 1) == editor->undoRedoBuffer.amount);
}

static void EditorPerformUndo(Editor *editor)
{
   if (editor->undoRedoAt == 0xFFFFFFFF) return;
   EditorEntities *editorEntities = &editor->editorEntities;
   
   EditorAction toReverse = editor->undoRedoBuffer[editor->undoRedoAt--];
   
   switch (toReverse.type)
   {
      case EditorAction_AlterMesh:
      {
         EntityCopyData *data = &toReverse.preModifyMesh;
         Entity *e = GetEntity(editorEntities, toReverse.serial);
         
         ApplyEntityDataToEntity(e, data);
      }break;
      case EditorAction_DeleteMesh: 
      {
         EntityCopyData data = toReverse.preModifyMesh;
         u32 serial = toReverse.serial;
         RestoreEntity(editorEntities, serial, data.meshId, data.scale, data.orientation, data.physicalPos, data.offset, data.color, data.type, data.flags);
      }break;
      case EditorAction_PlaceMesh:
      {
         RemoveEntity(editorEntities, toReverse.serial);
      }break;
      case EditorAction_BeginBundle:
      {
         
      }break;
      case EditorAction_EndBundle:
      {
         while (editor->undoRedoBuffer[editor->undoRedoAt].type != EditorAction_BeginBundle)
         {
            EditorPerformUndo(editor);
         }
         editor->undoRedoAt--; // for the BeginBundle
      }break;
      
      InvalidDefaultCase;
   }
}


static void EditorPerformRedo(Editor *editor)
{
   if (editor->undoRedoAt == (editor->undoRedoBuffer.amount - 1)) return;
   EditorAction toReverse = editor->undoRedoBuffer[++editor->undoRedoAt];
   
   EditorEntities *editorEntities = &editor->editorEntities;
   
   switch (toReverse.type)
   {
      case EditorAction_AlterMesh:
      {
         EntityCopyData *data = &toReverse.postModifyMesh;
         Entity *e = GetEntity(editorEntities, toReverse.serial);
         ApplyEntityDataToEntity(e, data);
      }break;
      case EditorAction_DeleteMesh:
      {
         RemoveEntity(editorEntities, toReverse.serial);
      }break;
      case EditorAction_PlaceMesh:
      {
         EntityCopyData data = toReverse.postModifyMesh;
         RestoreEntity(editorEntities, toReverse.serial, data.meshId, data.scale, data.orientation, data.physicalPos, data.offset, data.color, data.type, data.flags);
      }break;
      case EditorAction_BeginBundle:
      {
         while (editor->undoRedoBuffer[editor->undoRedoAt].type != EditorAction_EndBundle)
         {
            EditorPerformRedo(editor);
         }
      }break;
      case EditorAction_EndBundle:
      {
         return;
      }break;
      
      InvalidDefaultCase;
   }
}

static void PushAlterMeshModifies(Editor *editor)
{
   EditorEntities *editorEntities = &editor->editorEntities;
   for(u32 i = 0; i < editor->hotEntitySerials.amount; i++)
   {
      Entity *e = GetEntity(editorEntities, editor->hotEntitySerials[i]);
      EntityCopyData *data = editor->hotEntityInitialStates + i;
      
      EditorAction toAdd;
      toAdd.type = EditorAction_AlterMesh;
      toAdd.serial = e->serialNumber;
      toAdd.preModifyMesh  = *data;
      toAdd.postModifyMesh = EntityToData(*e);
      AddActionToUndoRedoBuffer(editor, toAdd);
   }
}

static void EditorPushUndo(Editor *editor)
{
   Assert(editor->hotEntitySerials.amount);
   
   if (editor->hotEntitySerials.amount > 1)
   {
      EditorAction toAdd;
      toAdd.type = EditorAction_BeginBundle;
      AddActionToUndoRedoBuffer(editor, toAdd);
   }
   
   
   switch (editor->state)
   {
      case EditorState_Scaling:
      case EditorState_Rotating:
      case EditorState_Moving:
      case EditorState_PickingColor:
      case EditorState_AlteringValue:
      {
         PushAlterMeshModifies(editor);
      }break;
      case EditorState_PlacingNewMesh:
      {
         // we assume that this is called right after we place the entities.
         // and that we selected them
         for(u32 i = 0; i < editor->hotEntitySerials.amount; i++)
         {
            EditorAction toAdd;
            toAdd.type = EditorAction_PlaceMesh;
            toAdd.serial = editor->hotEntitySerials[i];
            toAdd.postModifyMesh = editor->hotEntityInitialStates[i];
            
            AddActionToUndoRedoBuffer(editor, toAdd);
         }
      }break;
      case EditorState_DeleteSelection: 
      {
         // to be called when the selection did not get cleared yet.
         for(u32 i = 0; i < editor->hotEntitySerials.amount; i++)
         {
            EditorAction toAdd;
            toAdd.type = EditorAction_DeleteMesh;
            toAdd.serial = editor->hotEntitySerials[i];
            EditorEntities *manager = &editor->editorEntities;
            Entity *e = GetEntity(manager, toAdd.serial);
            
            toAdd.preModifyMesh = EntityToData(*e);
            
            AddActionToUndoRedoBuffer(editor, toAdd);
         }
      }break;
      
      
      default:
      {
         Die;
      }break;
      
   }
   
   if (editor->hotEntitySerials.amount > 1)
   {
      EditorAction toAdd;
      toAdd.type = EditorAction_EndBundle;
      AddActionToUndoRedoBuffer(editor, toAdd);
   }
}

static void ResetSelectionInitials(Editor *editor)
{
   for(u32 i = 0; i < editor->hotEntitySerials.amount; i++)
   {
      u32 serial = editor->hotEntitySerials[i];
      Entity *e = GetEntity(&editor->editorEntities, serial);
      editor->hotEntityInitialStates[i] = EntityToData(*e);
   }
}

static void EditorUpdateCamFocus(Editor *editor, Camera *cam, Input input)
{
   //
   // The general map should be Quaternion -> Basis, mapping q to the -> ROW <- vectors
   // of the associated Matrix
   //
   
   Quaternion c = cam->orientation;
   
   v2 mouseDelta = input.mouseDelta;
   
   f32 rotSpeed = 0.001f * 3.141592f;
   
   f32 mouseCXRot =  mouseDelta.y * rotSpeed;
   f32 mouseZRot  = -mouseDelta.x * rotSpeed;
   
   Quaternion rotX = AxisAngleToQuaternion(mouseCXRot, V3(1, 0, 0));
   Quaternion rotZ = AxisAngleToQuaternion(mouseZRot,  V3(0, 0, 1));
   Quaternion rot = rotX * c * rotZ;
   
   v3 delta = cam->pos - editor->focusPoint;
   
   // todo really understand how this is the right math
   Quaternion conj = Inverse(c) * Inverse(rotX) * c * Inverse(rotZ);
   cam->pos = editor->focusPoint +  QuaternionToMatrix3(conj) * delta;
   
   cam->orientation = rot;
}

static void FrameColorSelection(Editor *editor, v4 editorMeshSelectColor)
{
   EditorEntities *editorEntities = &editor->editorEntities;
   For(editor->hotEntitySerials)
   {
      GetEntity(editorEntities, *it)->frameColor *= editorMeshSelectColor;
   }
}

// todo maybe make all this matrix stuff more consitent
static void UpdateEditor(Editor *editor, Input input)
{
   EditorEntities *editorEntities = &editor->editorEntities;
   Camera *cam = &editor->camera;
   
   UpdateColorPickers(editor, input);
   
   switch (editor->state)
   {
      case EditorState_DeleteSelection:
      {
         return;
      }break;
      case EditorState_OrbitingCamera:
      {
         EditorUpdateCamFocus(editor, cam, input);
         return;
      }break;
      
   }
   
   if (!EditorHasSelection(editor))
   {
      return;
   }
   
   Tweekable(v4, editorMeshSelectColor, V4(1.0f, 0.4f, 0.5f, 0.1f));
   
   switch (editor->state)
   {
      case EditorState_Default:
      {
         FrameColorSelection(editor, editorMeshSelectColor);
      }break;
      case EditorState_Rotating:
      {
         v3 averagePos = GetAveragePosForSelection(editor);
         
         //todo not sure about all this being in the z= 0 plane
         v2 oldP = ScreenZeroToOneToInGame(*cam, input.mouseZeroToOne - input.mouseZeroToOneDelta);
         v2 newP = ScreenZeroToOneToInGame(*cam, input.mouseZeroToOne);
         
         v2 relP1 = newP - p12(averagePos);
         v2 relP2 = oldP - p12(averagePos);
         f32 angle = AngleBetween(relP2, relP1);
         Quaternion q = AxisAngleToQuaternion(angle, V3(0, 0, 1));
         m4x4 mat = QuaternionToMatrix4(q);
         
         For(editor->hotEntitySerials)
         {
            Entity *e = GetEntity(editorEntities, *it);
            
            v4 relPos = V4(GetRenderPos(*e) - averagePos, 1.0f);
            e->orientation = q * e->orientation;
            v3 pos = averagePos + (mat * relPos).xyz;
            e->physicalPos = RoundToTileMap(pos);
            e->offset = pos - V3(e->physicalPos);
         }
      }break;
      case EditorState_Scaling:
      {
         v2 mouseD = input.mouseZeroToOneDelta;
         if (mouseD == V2()) break;
         
         v3 averagePos = GetAveragePosForSelection(editor);
         
         m4x4 proj = Projection(cam->aspectRatio, cam->focalLength) * CameraTransform(cam->orientation, cam->pos);
         v4 projectiveMidPoint = V4(averagePos, 1.0f);
         v4 projectedMidPoint = (proj * projectiveMidPoint);
         Assert(projectedMidPoint.w);
         v2 screenPoint = projectedMidPoint.xy / projectedMidPoint.w; // in -1 to 1 range
         screenPoint = 0.5f * screenPoint + V2(0.5f, 0.5f);
         screenPoint = V2(screenPoint.x, 1.0f - screenPoint.y);
         v2 p = input.mouseZeroToOne;
         v2 d = Normalize(p - screenPoint);
         f32 dot = 3.0f * Dot(mouseD, d); // scale speed
         f32 exp = expf(dot);
         
         For(editor->hotEntitySerials)
         {
            Entity *mesh = GetEntity(editorEntities, *it);
            v3 delta = GetRenderPos(*mesh) - averagePos;
            v3 newPos = exp * delta + averagePos;
            
            mesh->physicalPos = RoundToTileMap(newPos);
            mesh->offset = newPos - V3(mesh->physicalPos);
            mesh->scale *= exp;
         }
      }break;
      case EditorState_Moving:
      {	
         // todo speed, but whatever
         v3 averagePos = GetAveragePosForSelection(editor);
         v3 oldP = ScreenZeroToOneToScreenInGame(*cam, input.mouseZeroToOne - input.mouseZeroToOneDelta);
         v3 newP = ScreenZeroToOneToScreenInGame(*cam, input.mouseZeroToOne);
         v3 camPos = cam->pos;
         m3x3 camM = QuaternionToMatrix3(cam->orientation);
         v3 camb1 = GetRow(camM, 0);
         v3 camb2 = GetRow(camM, 1);
         v3 diff = camPos - averagePos;
         v3 oP = SolveLinearSystem((camPos - oldP), camb1, camb2, diff);
         v3 nP = SolveLinearSystem((camPos - newP), camb1, camb2, diff);
         
         m3x3 mat = Columns3x3((camPos - oldP), camb1, camb2);
         
         v3 oI = camPos + oP.x * (oldP - camPos); // = p + op.y * cam->b1 + op.z * cam->b2
         v3 nI = camPos + nP.x * (newP - camPos);
         v3 realDelta = nI - oI;
         
         For(editor->hotEntitySerials)
         {
            Entity *e = GetEntity(editorEntities, *it);
            
            v3 pos = GetRenderPos(*e) + realDelta;
            
            e->physicalPos = RoundToTileMap(pos);
            e->offset = pos - V3(e->physicalPos);
         }
         
      }break;
      case EditorState_DragingPanel:
      {
         FrameColorSelection(editor, editorMeshSelectColor);
         editor->panel.pos += input.mouseZeroToOneDelta;
      }break;
      case EditorState_PickingColor:
      {
         
      }break;
      case EditorState_AlteringValue:
      {
         FrameColorSelection(editor, editorMeshSelectColor);
      }break;
      
      default:
      {
         Die;
      }break;
   }
}


static Level EditorStateToLevel(Editor *editor)
{
   EditorEntities *editorEntities = &editor->editorEntities;
   EditorLevelInfo *info = &editor->levelInfo;
   Level level;
   
   level.camera        = info->camera;
   level.lightSource   = info->lightSource;
   level.blocksNeeded  = info->blocksNeeded;
   level.entities      = PushArray(frameArena, EntityCopyData, editorEntities->entities.amount);
   
   for(u32 i = 0; i < editorEntities->entities.amount; i++)
   {
      level.entities[i] = EntityToData(editorEntities->entities[i]);
   }
   
   return level;
}



#endif // !RR_EDITOR