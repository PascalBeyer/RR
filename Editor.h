#ifndef RR_EDITOR
#define RR_EDITOR



enum PickerSelectionTag
{
	PickerSelecting_Nothing,
	PickerSelecting_Slider,
	PickerSelecting_ColorPicker,
	PickerSelecting_Header,
};

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
enum EditorUIElementEnum
{
	EditorUI_ColorPicker,

};

struct EditorUIElement
{
	EditorUIElementEnum type;
	union
	{
		ColorPicker picker;
	};
};

DefineDynamicArray(EditorUIElement);


enum EditorState
{
	EditorState_Default					= 0x0,
	EditorState_Scaling					= 0x1,
	EditorState_Rotating				= 0x2,
	EditorState_Moving					= 0x3,
	EditorState_DragingPanel			= 0x4,
	EditorState_PickingColor			= 0x5,
	EditorState_AlteringValue			= 0x6,
	EditorState_PlacingNewMesh			= 0x7,
	EditorState_DeleteSelection			= 0x8,

	EditorState_OrbitingCamera			= 0x100,

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

// todo we do not have to initialize this everywhere we do, but what evs.
struct EditorSelect
{
	u32 placedSerial;
	Quaternion initialOrientation;
	f32 initialScale;
	v3i initialPhysicalPos;
	v3 initialOffset;
	v4 initialColor;
};

struct EditorEntityUndoRedoData
{
	u32 placedSerial;
	u32 meshId;
	Quaternion orientation;
	f32 scale;
	v3i physicalPos;
	v3 offset;
	v4 color;

	EntityType type;
	u64 flags;
};


DefineDynamicArray(EntityCopyData);

typedef EntityCopyDataDynamicArray EditorClipBoard;

DefineDynamicArray(EditorSelect);

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

	union
	{
		EditorEntityUndoRedoData preModifyMesh;
	};
	union
	{
		EditorEntityUndoRedoData postModifyMesh;
	};
	
};


DefineDynamicArray(EditorAction);

struct Editor
{
	EditorUIElementDynamicArray elements;
	EditorSelectDynamicArray hotEntityInfos;
	EditorState state = EditorState_Default;

	EditorClipBoard clipBoard;
	EditorActionDynamicArray undoRedoBuffer;
	u32 undoRedoAt;

	EditorPanel panel;

	b32 snapToTileMap;

	v3 focusPoint;
};

struct EulerAngle
{
	f32 XRotation;
	f32 YRotation;
	f32 ZRotation;
};

static EulerAngle QuaternionToEulerAngle(Quaternion q)
{
	EulerAngle ret;

	// roll (x-axis rotation)
	f32 sinr_cosp = 2.0f * (q.w * q.x + q.y * q.z);
	f32 cosr_cosp = 1.0f - 2.0f * (q.x * q.x + q.y * q.y);
	ret.XRotation = atan2f(sinr_cosp, cosr_cosp);

	// pitch (y-axis rotation)
	f32 sinp = 2.0f * (q.w * q.y - q.z * q.x);
	if (fabs(sinp) >= 1)
		ret.YRotation = copysignf(PI / 2, sinp); // use 90 degrees if out of range
	else
		ret.YRotation = asinf(sinp);

	// yaw (z-axis rotation)
	f32 siny_cosp = 2.0f * (q.w * q.z + q.x * q.y);
	f32 cosy_cosp = 1.0f - 2.0f * (q.y * q.y + q.z * q.z);
	ret.ZRotation = atan2f(siny_cosp, cosy_cosp);

	return ret;
}

static bool LoadLevel(String fileName, UnitHandler *unitHandler, World *world, Arena *currentStateArena, AssetHandler *assetHandler, Editor *editor);

static Quaternion EulerAngleToQuaternion(EulerAngle angle)
{

	f32 cr = cosf(angle.XRotation * 0.5f);
	f32 sr = sinf(angle.XRotation * 0.5f);
	f32 cp = cosf(angle.YRotation * 0.5f);
	f32 sp = sinf(angle.YRotation * 0.5f);
	f32 cy = cosf(angle.ZRotation * 0.5f);
	f32 sy = sinf(angle.ZRotation * 0.5f);

	Quaternion q;
	q.w = cy * cp * cr + sy * sp * sr;
	q.x = cy * cp * sr - sy * sp * cr;
	q.y = sy * cp * sr + cy * sp * cr;
	q.z = sy * cp * cr - cy * sp * sr;
	return q;
}

static v3 GetAveragePosForSelection(Editor *editor, World *world)
{
	v3 averagePos = V3();
	For(editor->hotEntityInfos)
	{
		averagePos += GetRenderPos(*GetEntity(world, it->placedSerial));
	}
	averagePos /= (f32)editor->hotEntityInfos.amount;
	return averagePos;
}

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


static Editor InitEditor(Arena *constantArena)
{
	Editor ret;
	ret.elements = EditorUIElementCreateDynamicArray();
	ret.clipBoard = EntityCopyDataCreateDynamicArray();

	ret.undoRedoBuffer = EditorActionCreateDynamicArray();
	ret.undoRedoAt = 0xFFFFFFFF;
	
	ret.hotEntityInfos = EditorSelectCreateDynamicArray();
	ret.state = EditorState_Default;

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
		ret.sliderColor = (0xFF << 24) | (0xFF << 16) | (0 << 8) | 0;
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
		ret.sliderColor = (0xFF << 24) | (0xFF << 16) | (0 << 8) | 0;
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


static void WriteSingleTweeker(Tweeker tweeker);

// todo: could save these up and then on access do the mem copy. Not sure if worth the complexity
// todo unordered remove (just move the last guy)
static void ArrayRemove(EditorUIElementDynamicArray *arr, EditorUIElement *elem)
{
	u32 index = (u32)(elem - arr->data);
	
	if (index < arr->amount)
	{
		void *to = elem;
		void *from = (elem + 1);

		u32 amount = arr->amount - index;
		memcpy(to, from, amount * sizeof(EditorUIElement));
		
		arr->amount--;
	}
	else
	{
		Die;
	}
}

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

static v3i SnapToTileMap(v3 pos)
{
	return V3i((i32)pos.x, (i32)pos.y, (i32)pos.z);
}

static v3i RoundToTileMap(v3 pos)
{
	return V3i((i32)floorf(pos.x + 0.5f), (i32)floorf(pos.y + 0.5f), (i32)floorf(pos.z + 0.5f));
}

static void UpdateColorPickers(Editor *editor, Input input)
{
	For(editor->elements)
	{
		switch (it->type)
		{
		case EditorUI_ColorPicker:
		{
			UpdateColorPicker(&it->picker, input);
		}break;
		default:
		{
			Die;
		}break;

		}

	}
}

static void EditorGoToNone(Editor *editor)
{
	editor->state = EditorState_Default;
#if 0
	For(editor->hotEntityInfos)
	{
		Entity *e = GetEntity(level, it->placedSerial);
		it->initialPos			= e->pos;
		it->initialScale		= e->scale;
		it->initialOrientation	= e->orientation;
		it->initialColor		= e->color;
	}
#endif
}

static Entity *GetHotMesh(World *world, AssetHandler *handler, v2 mousePosZeroToOne)
{
	v3 camP = world->camera.pos; // todo camera or debugCamera? Maybe we should again unify them
	v3 camD = ScreenZeroToOneToDirecion(world->camera, mousePosZeroToOne);

	f32 minDist = F32MAX;

	Entity *ret = NULL;

	For(world->entities)
	{
		MeshInfo *info = GetMeshInfo(handler, it->meshId);

		if (!info) continue; // probably not on screen, if never rendered

		m4x4 mat = QuaternionToMatrix(Inverse(it->orientation)); // todo save these?
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

static void EditorSelectNothing(Editor *editor) 
{
	editor->panel.values.amount = 0;
	editor->panel.visible = false;
	Clear(&editor->hotEntityInfos);
}

static bool WriteLevel(char *fileName, World world, UnitHandler unitHandler, AssetHandler *assetHandler);

static EditorSelect EntityToEditorSelect(Entity *e)
{
	EditorSelect toAdd;
	toAdd.initialColor = e->color;
	toAdd.initialPhysicalPos = e->physicalPos;
	toAdd.initialOffset = e->offset;
	toAdd.initialOrientation = e->orientation;
	toAdd.initialScale = e->scale;
	toAdd.placedSerial = e->serialNumber;
	return toAdd;
};

static bool EditorHasSelection(Editor *editor)
{
	return (editor->hotEntityInfos.amount > 0);
}

static void ResetHotMeshes(Editor *editor, World *world)
{
	For(editor->hotEntityInfos)
	{
		Entity *e = GetEntity(world, it->placedSerial);
		e->physicalPos = it->initialPhysicalPos;
		e->offset = it->initialOffset;
		e->scale = it->initialScale;
		e->orientation = it->initialOrientation;
		e->color = it->initialColor;
	}
}

// kills all redos
static void AddActionToUndoRedoBuffer(Editor *editor, EditorAction toAdd)
{
	editor->undoRedoBuffer.amount = ++editor->undoRedoAt;
	
	ArrayAdd(&editor->undoRedoBuffer, toAdd);

	Assert((editor->undoRedoAt + 1) == editor->undoRedoBuffer.amount);
}

static void EditorPerformUndo(Editor *editor, World *world)
{
	if (editor->undoRedoAt == 0xFFFFFFFF) return;
	EditorAction toReverse = editor->undoRedoBuffer[editor->undoRedoAt--];
	switch (toReverse.type)
	{
	case EditorAction_AlterMesh:
	{
		EditorEntityUndoRedoData data = toReverse.preModifyMesh;
		u32 serial = toReverse.preModifyMesh.placedSerial;
		Assert(toReverse.preModifyMesh.placedSerial == toReverse.postModifyMesh.placedSerial);
		Entity *mesh = GetEntity(world, toReverse.preModifyMesh.placedSerial);
		mesh->color			= data.color;
		mesh->physicalPos	= data.physicalPos;
		mesh->offset		= data.offset;
		mesh->orientation	= data.orientation;
		mesh->scale			= data.scale;
		mesh->type			= data.type;
		mesh->flags			= data.flags;
	}break;
	case EditorAction_DeleteMesh: 
	{
		EditorEntityUndoRedoData data = toReverse.preModifyMesh;
		u32 serial = toReverse.preModifyMesh.placedSerial;
		RestoreEntity(world, serial, data.meshId, data.scale, data.orientation, data.physicalPos, data.offset, data.color, data.type, data.flags);
	}break;
	case EditorAction_PlaceMesh:
	{
		EditorEntityUndoRedoData data = toReverse.postModifyMesh;
		RemoveEntity(world, data.placedSerial);
	}break;
	case EditorAction_BeginBundle:
	{
		
	}break;
	case EditorAction_EndBundle:
	{
		while (editor->undoRedoBuffer[editor->undoRedoAt].type != EditorAction_BeginBundle)
		{
			EditorPerformUndo(editor, world);
		}
		editor->undoRedoAt--;
	}break;

	InvalidDefaultCase;
	}
}


static void EditorPerformRedo(Editor *editor, World *world)
{
	if (editor->undoRedoAt == (editor->undoRedoBuffer.amount - 1)) return;
	EditorAction toReverse = editor->undoRedoBuffer[++editor->undoRedoAt];
	switch (toReverse.type)
	{
	case EditorAction_AlterMesh:
	{
		EditorEntityUndoRedoData data = toReverse.postModifyMesh;
		u32 serial = toReverse.postModifyMesh.placedSerial;
		Assert(toReverse.postModifyMesh.placedSerial == toReverse.postModifyMesh.placedSerial);
		Entity *e = GetEntity(world, toReverse.postModifyMesh.placedSerial);
		e->color = data.color;
		e->physicalPos = data.physicalPos;
		e->offset = data.offset;
		e->orientation = data.orientation;
		e->scale = data.scale;
		e->type = data.type;
		e->flags = data.flags;
	}break;
	case EditorAction_DeleteMesh:
	{
		EditorEntityUndoRedoData data = toReverse.preModifyMesh;
		u32 serial = toReverse.preModifyMesh.placedSerial;
		RemoveEntity(world, serial);
	}break;
	case EditorAction_PlaceMesh:
	{
		EditorEntityUndoRedoData data = toReverse.postModifyMesh;
		RestoreEntity(world, data.placedSerial, data.meshId, data.scale, data.orientation, data.physicalPos, data.offset, data.color, data.type, data.flags);
	}break;
	case EditorAction_BeginBundle:
	{
		while (editor->undoRedoBuffer[editor->undoRedoAt].type != EditorAction_EndBundle)
		{
			EditorPerformRedo(editor, world);
		}
	}break;
	case EditorAction_EndBundle:
	{
		return;
	}break;

	InvalidDefaultCase;
	}
}


static EditorEntityUndoRedoData PreModifyToEditorSelect(EditorSelect select, Editor *editor, World *world)
{
	Entity *e = GetEntity(world, select.placedSerial);
	EditorEntityUndoRedoData ret;
	ret.meshId			= e->meshId;
	ret.placedSerial	= e->serialNumber;
	ret.flags			= e->flags;
	ret.type			= e->type;
	ret.orientation		= select.initialOrientation;
	ret.physicalPos		= select.initialPhysicalPos;
	ret.offset			= select.initialOffset;
	ret.scale			= select.initialScale;
	ret.color			= select.initialColor;
	
	return ret;
}

static EditorEntityUndoRedoData PostModifyToEditorSelect(EditorSelect select, Editor *editor, World *world)
{
	Entity *e = GetEntity(world, select.placedSerial);
	EditorEntityUndoRedoData ret;
	ret.color			= e->color;
	ret.placedSerial	= e->serialNumber;
	ret.orientation		= e->orientation;
	ret.physicalPos		= e->physicalPos;
	ret.offset			= e->offset;
	ret.scale			= e->scale;
	ret.meshId			= e->meshId;
	ret.flags			= e->flags;
	ret.type			= e->type;
	return ret;
}

static void PushAlterMeshModifies(Editor *editor, World *world)
{
	For(editor->hotEntityInfos)
	{
		EditorAction toAdd;
		toAdd.type = EditorAction_AlterMesh;
		toAdd.preModifyMesh = PreModifyToEditorSelect(*it, editor, world);
		toAdd.postModifyMesh = PostModifyToEditorSelect(*it, editor, world);
		AddActionToUndoRedoBuffer(editor, toAdd);
	}
}

static void EditorPushUndo(Editor *editor, World *world)
{
	Assert(editor->hotEntityInfos.amount);

	if (editor->hotEntityInfos.amount > 1)
	{
		EditorAction toAdd;
		toAdd.type = EditorAction_BeginBundle;
		AddActionToUndoRedoBuffer(editor, toAdd);
	}

	
	switch (editor->state)
	{
	case EditorState_Scaling:
	{
		PushAlterMeshModifies(editor, world);
	}break;
	case EditorState_Rotating:
	{
		PushAlterMeshModifies(editor, world);
	}break;
	case EditorState_Moving:
	{
		PushAlterMeshModifies(editor, world);
	}break;
	case EditorState_PickingColor:
	{
		PushAlterMeshModifies(editor, world);
	}break;
	case EditorState_AlteringValue:
	{
		PushAlterMeshModifies(editor, world);
	}break;
	case EditorState_PlacingNewMesh:
	{
		For(editor->hotEntityInfos)
		{
			EditorAction toAdd;
			toAdd.type = EditorAction_PlaceMesh;
			Entity *mesh = GetEntity(world, it->placedSerial);

			toAdd.postModifyMesh = PostModifyToEditorSelect(*it, editor, world);
			
			AddActionToUndoRedoBuffer(editor, toAdd);
		}
	}break;
	case EditorState_DeleteSelection: 
	{
		// to be called when the selection did not get cleared yet.
		For(editor->hotEntityInfos)
		{
			EditorAction toAdd;
			toAdd.type = EditorAction_DeleteMesh;
			Entity *mesh = GetEntity(world, it->placedSerial);

			toAdd.preModifyMesh = PreModifyToEditorSelect(*it, editor, world);

			AddActionToUndoRedoBuffer(editor, toAdd);
		}
	}break;


	default:
	{
		Die;
	}break;

	}

	if (editor->hotEntityInfos.amount > 1)
	{
		EditorAction toAdd;
		toAdd.type = EditorAction_EndBundle;
		AddActionToUndoRedoBuffer(editor, toAdd);
	}
}

static void ResetEditorPanel(Editor *editor)
{
	editor->panel.values.amount = 0;
	editor->panel.visible = false;
}

static void ResetSelectionInitials(Editor *editor, World *world)
{
	For(editor->hotEntityInfos)
	{
		Entity *e = GetEntity(world, it->placedSerial);
		it->initialPhysicalPos = e->physicalPos;
		it->initialOffset = e->offset;
		it->initialOrientation = e->orientation;
		it->initialScale = e->scale;
		it->initialColor = e->color;
	}
}


static void EditorUpdateCamFocus(Editor *editor, Camera *cam, Input input)
{
	v2 mouseDelta = input.mouseDelta;

	f32 rotSpeed = 0.001f * 3.141592f;

	f32 mouseZRot = -mouseDelta.y * rotSpeed; // this should rotate around the z axis
	f32 mouseCXRot = mouseDelta.x * rotSpeed; // this should rotate around the camera x axis

	m3x3 cameraT = Rows3x3(cam->b1, cam->b2, cam->b3);
	m3x3 cameraTInv = Invert(cameraT);

	m3x3 id = cameraT * cameraTInv;
	m3x3 rotX = XRotation3x3(mouseZRot);
	m3x3 rotZ = ZRotation3x3(mouseCXRot);
	m3x3 rot = cameraTInv * rotX * cameraT * rotZ;

	v3 delta = cam->pos - editor->focusPoint;

	cam->pos = editor->focusPoint + rot * delta;

	cam->basis = TransformBasis(cam->basis, rot);

}

static void FrameColorSelection(Editor *editor, World *world, v4 editorMeshSelectColor)
{
	For(editor->hotEntityInfos)
	{
		GetEntity(world, it->placedSerial)->frameColor *= editorMeshSelectColor;
	}
}

// todo maybe make all this matrix stuff more consitent
static void UpdateEditor(Editor *editor, UnitHandler *unitHandler, World *world, Input input)
{
	Camera *cam = &world->camera;
	
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
		FrameColorSelection(editor, world, editorMeshSelectColor);
	}break;
	case EditorState_Rotating:
	{
		v3 averagePos = GetAveragePosForSelection(editor, world);

		//todo not sure about all this being in the z= 0 plane
		v2 oldP = ScreenZeroToOneToInGame(*cam, input.mouseZeroToOne - input.mouseZeroToOneDelta);
		v2 newP = ScreenZeroToOneToInGame(*cam, input.mouseZeroToOne);

		v2 relP1 = newP - p12(averagePos);
		v2 relP2 = oldP - p12(averagePos);
		f32 angle = AngleBetween(relP2, relP1);
		Quaternion q = QuaternionFromAngleAxis(angle, V3(0, 0, 1));
		m4x4 mat = QuaternionToMatrix(q);

		For(editor->hotEntityInfos)
		{
			Entity *e = GetEntity(world, it->placedSerial);
			
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

		v3 averagePos = GetAveragePosForSelection(editor, world);

		m4x4 proj = Projection(cam->aspectRatio, cam->focalLength) * CameraTransform(cam->basis.d1, cam->basis.d2, cam->basis.d3, cam->pos);
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

		For(editor->hotEntityInfos)
		{
			Entity *mesh = GetEntity(world, it->placedSerial);
			v3 delta = GetRenderPos(*mesh) - averagePos;
			v3 newPos = exp * delta + averagePos;

			mesh->physicalPos = RoundToTileMap(newPos);
			mesh->offset = newPos - V3(mesh->physicalPos);
			mesh->scale *= exp;
		}
	}break;
	case EditorState_Moving:
	{	
		v3 averagePos = GetAveragePosForSelection(editor, world);
		v3 oldP = ScreenZeroToOneToScreenInGame(*cam, input.mouseZeroToOne - input.mouseZeroToOneDelta);
		v3 newP = ScreenZeroToOneToScreenInGame(*cam, input.mouseZeroToOne);
		v3 camPos = cam->pos;
		v3 diff = camPos - averagePos;
		v3 oP = SolveLinearSystem((camPos - oldP), cam->b1, cam->b2, diff);
		v3 nP = SolveLinearSystem((camPos - newP), cam->b1, cam->b2, diff);

		m3x3 mat = Columns3x3((camPos - oldP), cam->b1, cam->b2);

		v3 oI = camPos + oP.x * (oldP - camPos); // = p + op.y * cam->b1 + op.z * cam->b2
		v3 nI = camPos + nP.x * (newP - camPos);
		v3 realDelta = nI - oI;

		For(editor->hotEntityInfos)
		{
			Entity *mesh = GetEntity(world, it->placedSerial);

			v3 pos = GetRenderPos(*mesh) + realDelta;

			mesh->physicalPos = RoundToTileMap(pos);
			mesh->offset = pos - V3(mesh->physicalPos);
		}

	}break;
	case EditorState_DragingPanel:
	{
		FrameColorSelection(editor, world, editorMeshSelectColor);
		editor->panel.pos += input.mouseZeroToOneDelta;
	}break;
	case EditorState_PickingColor:
	{
	
	}break;
	case EditorState_AlteringValue:
	{
		FrameColorSelection(editor, world, editorMeshSelectColor);
	}break;

	default:
	{
		Die;
	}break;
	}
}


static void ResetEditor(Editor *editor)
{
	EditorSelectNothing(editor);
	EditorGoToNone(editor);

	editor->undoRedoBuffer.amount = 0;
	editor->undoRedoAt = 0xFFFFFFFF;

	editor->focusPoint = V3();
	editor->elements.amount = 0;
	editor->panel.visible = false;

}

static void NewLevel(UnitHandler *unitHandler, World *world, Editor *editor, Arena *currentStateArena)
{
	ResetLevel(world);
	ResetEditor(editor);
}



#endif // !RR_EDITOR

