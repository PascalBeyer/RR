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
	EditorState_Default,
	EditorState_Scaling,
	EditorState_Rotating,
	EditorState_Moving,
	EditorState_DragingPanel,
	EditorState_PickingColor,
	EditorState_AlteringValue,
	EditorState_TileMapEditor,
	EditorState_TileMapPlacingGoal,
	EditorState_TileMapPlacingSpawner,
	EditorState_OrbitingCamera,
};

static String EditorStateToString(EditorState state)
{
	switch (state)
	{
	case EditorState_Default:
	{
		return CreateString("EditorState_None");
	}break;
	case EditorState_Scaling:
	{
		return CreateString("EditorState_Scaling");
	}break;
	case EditorState_Rotating:
	{
		return CreateString("EditorState_Rotating");
	}break;
	case EditorState_Moving:
	{
		return CreateString("EditorState_Moving");
	}break;
	case EditorState_DragingPanel:
	{
		return CreateString("EditorState_DraggingPanel");
	}break;
	case EditorState_PickingColor:
	{
		return CreateString("EditorState_PickingColor");
	}break;
	case EditorState_AlteringValue: 
	{
		return CreateString("EditorState_AlteringValue"); 
	}break;
	}
	return CreateString("Unknown EditorState.");
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
		v4 *vec4;
		b32 *b;
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

struct Editor
{
	EditorUIElementDynamicArray elements;
	PlacedMesh *hotMesh;
	u32 unitIndex;
	EditorState state = EditorState_Default;

	EditorPanel panel;

	Quaternion rotater = { 1, 0, 0, 0 };
	f32 scaler = 1.0f;
	v3 mover = V3();

	v3 focusPoint;
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


static void RenderEditorPanel(RenderGroup *rg, Editor editor, Font font)
{
	if (editor.state == EditorState_Rotating || editor.state == EditorState_Scaling || editor.state == EditorState_Moving)
	{
		return;
	}
	
	EditorPanel *p = &editor.panel;
	if (!p->visible) return;

	Tweekable(v4, editorPanelColor, V4(1, 1, 1, 1));
	Tweekable(v4, editorPanelValueColor, V4(1, 0.5f, 0.7f, 1.0f)); 
	Tweekable(v4, editorPanelHeaderColor, V4(1, 0.4f, 0.6f, 0.8f));
	Tweekable(f32, editorBorderWidth, 0.01f);
	Tweekable(f32, editorHeaderWidth, 0.08f);
	Tweekable(f32, editorPanelFontSize, 0.01f);
	Tweekable(f32, editorPanelWidth, 0.1f);
	Tweekable(f32, editorPanelHeight, 0.6f);

	f32 panelWidth = editorPanelWidth;
	f32 panelHeight = editorPanelHeight;

	//whole thang
	PushRectangle(rg, p->pos, panelWidth, panelHeight, editorPanelColor);

	//header
	f32 widthWithoutBoarder = panelWidth - 2 * editorBorderWidth;
	PushRectangle(rg, p->pos + V2(editorBorderWidth, editorBorderWidth), widthWithoutBoarder, editorHeaderWidth, editorPanelHeaderColor);
	PushString(rg, p->pos + V2(editorBorderWidth, editorBorderWidth), EditorStateToString(editor.state), 0.5f * editorHeaderWidth, font);

	u32 xyz = editor.panel.hotValueXYZ;
	v2 valuePos = p->pos + V2(editorBorderWidth, editorBorderWidth + editorHeaderWidth);
	For(p->values)
	{
		
		String name = FormatString("%s:", it->name);
		f32 offset = PushString(rg, valuePos, name, editorPanelFontSize, font);

		v2 writePos = valuePos + V2(0, editorPanelFontSize);
		v2 continuePos = valuePos + V2(offset + 3 * editorPanelFontSize, 0);

		switch (it->type)
		{
		case Tweeker_b32:
		{
			f32 height = editorPanelFontSize + editorBorderWidth;
			PushRectangle(rg, valuePos, widthWithoutBoarder, height, editorPanelValueColor);

			if (*it->b)
			{
				PushString(rg, continuePos, "True", editorPanelFontSize, font);
			}
			else
			{
				PushString(rg, continuePos, "False", editorPanelFontSize, font);
			}

			valuePos.y += height;
		}break;
		case Tweeker_u32:
		{
			f32 height = editorPanelFontSize + editorBorderWidth;
			PushRectangle(rg, valuePos, widthWithoutBoarder, height, editorPanelValueColor);

			if (p->values.data + p->hotValue == it)
			{
				PushString(rg, continuePos, p->textInput.string, editorPanelFontSize, font);
				valuePos.y += height;
				break;
			}

			String str = UtoS(*it->u);
			PushString(rg, continuePos, str, editorPanelFontSize, font);
			valuePos.y += height;
		}break;
		case Tweeker_f32:
		{
			f32 height = editorPanelFontSize + editorBorderWidth;
			PushRectangle(rg, valuePos, widthWithoutBoarder, height, editorPanelValueColor);
			if (p->values.data + p->hotValue == it)
			{
				PushString(rg, continuePos, p->textInput.string, editorPanelFontSize, font);
				valuePos.y += height;
				break;
			}

			String str = FtoS(*it->f);
			PushString(rg, continuePos, str, editorPanelFontSize, font);

			valuePos.y += height;
		}break;
		case Tweeker_v2:
		{
			f32 height = 3.0f * editorPanelFontSize + editorBorderWidth;
			PushRectangle(rg, valuePos, widthWithoutBoarder, height, editorPanelValueColor);

			if (xyz == 'x' && (p->values.data + p->hotValue == it))
			{
				String str = FormatString("x: %s", p->textInput.string);
				PushString(rg, writePos, str, editorPanelFontSize, font);
			}
			else
			{
				String strx = FormatString("x: %s", FtoS(it->vec2->x));
				PushString(rg, writePos, strx, editorPanelFontSize, font);
			}

			writePos.y += editorPanelFontSize;

			if (xyz == 'y' && (p->values.data + p->hotValue == it))
			{
				String str = FormatString("y: %s", p->textInput.string);
				PushString(rg, writePos, str, editorPanelFontSize, font);
			}
			else
			{
			
				String stry = FormatString("y: %s", FtoS(it->vec2->y));
				PushString(rg, writePos, stry, editorPanelFontSize, font);
			}
			valuePos.y += height;

		}break;
		case Tweeker_v3:
		{
			f32 height = 4.0f * editorPanelFontSize + editorBorderWidth;
			PushRectangle(rg, valuePos, widthWithoutBoarder, height, editorPanelValueColor);

			if (xyz == 'x' && (p->values.data + p->hotValue == it))
			{
				String str = FormatString("x: %s", p->textInput.string);
				PushString(rg, writePos, str, editorPanelFontSize, font);
			}
			else
			{
				String strx = FormatString("x: %s", FtoS(it->vec3->x));
				PushString(rg, writePos, strx, editorPanelFontSize, font);
			}
			writePos.y += editorPanelFontSize;
			if (xyz == 'y' && (p->values.data + p->hotValue == it))
			{
				String str = FormatString("y: %s", p->textInput.string);
				PushString(rg, writePos, str, editorPanelFontSize, font);
			}
			else
			{
				String stry = FormatString("y: %s", FtoS(it->vec3->y));
				PushString(rg, writePos, stry, editorPanelFontSize, font);
			}
			writePos.y += editorPanelFontSize;

			if (xyz == 'z' && (p->values.data + p->hotValue == it))
			{
				String str = FormatString("z: %s", p->textInput.string);
				PushString(rg, writePos, str, editorPanelFontSize, font);
			}
			else
			{
				String strz = FormatString("z: %s", FtoS(it->vec3->z));
				PushString(rg, writePos, strz, editorPanelFontSize, font);
			}
			valuePos.y += height;
		}break;
		case Tweeker_v4:
		{
			f32 colorRectSize = 3.0f * editorPanelFontSize;
			f32 height = 2.0f * editorBorderWidth + colorRectSize;
			PushRectangle(rg, valuePos, widthWithoutBoarder, height, editorPanelValueColor);
			
			v4 color = *it->vec4;
			PushRectangle(rg, V2(valuePos.x + widthWithoutBoarder - colorRectSize - editorBorderWidth, valuePos.y + editorBorderWidth), colorRectSize, colorRectSize, color);
			valuePos.y += height;
		}break;
		default:
		{
			Die;
		}break;
		}
		
		valuePos.y += editorBorderWidth;
	}

}


static Editor InitEditor(Camera camera, Arena *constantArena)
{
	Editor ret;
	ret.elements = EditorUIElementCreateDynamicArray();

	ret.hotMesh = NULL;
	ret.unitIndex = 0xFFFFFFFF;
	ret.state = EditorState_Default;

	Tweekable(v2, initialEditorPanelPos, V2(0.75f, 0.2f));
	
	ret.panel.pos = initialEditorPanelPos;
	ret.panel.visible = false;
	ret.panel.values = TweekerPointerCreateDynamicArray();
	ret.panel.textInput.string = PushArray(constantArena, Char, 50);
	ret.panel.textInput.string.length = 0;
	ret.panel.textInput.maxLength = 50;
	ret.panel.hotValue = 0xFFFFFFFF;

	ret.focusPoint = i12(ScreenZeroToOneToInGame(camera, V2(0.5f, 0.5f)));

	ret.rotater = { 1, 0, 0, 0 };
	ret.scaler = 1.0f;
	ret.mover = V3();
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
//returns whether or not it should be closed

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

static void RenderColorPicker(RenderGroup *rg, ColorPicker *picker)
{
	Tweekable(v4, borderColor);

	//BackGround
	v2 windowUL = picker->pos - V2(picker->border, 2.0f * picker->border + picker->headerSize);
	f32 windowWidth = picker->width + 2.0f * picker->border;
	f32 windowHeight = picker->height + 2.0f * picker->border + picker->sliderHeight + 2.0f * picker->border + picker->headerSize;

	PushRectangle(rg, windowUL, windowWidth, windowHeight, borderColor);
	
	//Header
	v2 headerPos = picker->pos - V2(0.0f, picker->headerSize + picker->border);
	picker->headerPos = headerPos;
	//PushRectangle(rg, headerPos, picker->width, picker->pickerHeaderSize, borderColor);
	Tweekable(v4, headerKillColor); 
	v2 killRectPos = V2(headerPos.x + picker->width - picker->headerSize, headerPos.y);
	picker->killRectPos = killRectPos;
	PushRectangle(rg, killRectPos, picker->headerSize, picker->headerSize, headerKillColor);
	
	f32 scale = 1.0f / 6.0f;
	f32 scaledWidth = scale * picker->width;

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

	for (u32 i = 0; i < 6; i++)
	{
		PushRectangle(rg, V2(picker->sliderPos.x + i * scaledWidth, picker->sliderPos.y), scaledWidth, picker->sliderHeight, colors[i], colors[(i + 1)], colors[i], colors[(i + 1)]);
	}

	//Slider
	Tweekable(f32, sliderSlider);

	f32 ppx = picker->pos.x + picker->pickerPos.x;
	f32 ppy = picker->pos.y + picker->pickerPos.y;

	PushRectangle(rg, picker->pos, picker->width, picker->height, 0xFFFFFFFF, picker->sliderColor, 0xFF000000, 0xFF000000);

	//picker rect
	PushRectangle(rg, V2(ppx - 2.0f * picker->border, ppy - 2.0f * picker->border), 4.0f * picker->border, 1.0f * picker->border, borderColor);
	PushRectangle(rg, V2(ppx - 2.0f * picker->border, ppy + 2.0f * picker->border), 4.0f * picker->border, 1.0f * picker->border, borderColor);
	PushRectangle(rg, V2(ppx - 2.0f * picker->border, ppy - 2.0f * picker->border), 1.0f * picker->border, 4.0f * picker->border, borderColor);
	PushRectangle(rg, V2(ppx + 1.0f * picker->border, ppy - 2.0f * picker->border), 1.0f * picker->border, 4.0f * picker->border, borderColor);

	//slider
	v2 pickerSliderBoxPos = V2(picker->sliderPos.x + picker->sliderSelectedColorPos - 0.5f * (sliderSlider + picker->border), picker->sliderPos.y - 1.5f * picker->border);
	v2 pickerSliderColorPos = V2(picker->sliderPos.x + picker->sliderSelectedColorPos - 0.5f * sliderSlider, picker->sliderPos.y - 0.5f * picker->border);
	PushRectangle(rg, pickerSliderBoxPos, sliderSlider + picker->border, picker->sliderHeight + 3.0f * picker->border, borderColor);
	PushRectangle(rg, pickerSliderColorPos, sliderSlider, picker->sliderHeight + picker->border, picker->sliderColor);

	//a r g b
	Tweekable(f32, pickerFontSize);
	Tweekable(f32, pickerFontXIncrease);
	f32 fontY = picker->pos.y + picker->width - pickerFontSize;
	PushString(rg, V2(picker->pos.x + 0.0f * pickerFontXIncrease * pickerFontSize, fontY), FormatString("a:%f32", picker->pickedColor->a), pickerFontSize, globalFont);
	PushString(rg, V2(picker->pos.x + 1.0f * pickerFontXIncrease * pickerFontSize, fontY), FormatString("r:%f32", picker->pickedColor->r), pickerFontSize, globalFont);
	PushString(rg, V2(picker->pos.x + 2.0f * pickerFontXIncrease * pickerFontSize, fontY), FormatString("g:%f32", picker->pickedColor->g), pickerFontSize, globalFont);
	PushString(rg, V2(picker->pos.x + 3.0f * pickerFontXIncrease * pickerFontSize, fontY), FormatString("b:%f32", picker->pickedColor->b), pickerFontSize, globalFont);

}


static void RenderEditor(RenderGroup *rg, Editor editor)
{
	PlacedMesh *mesh = editor.hotMesh;
	if (!mesh) return;
	AABB transformedAABB = mesh->untransformedAABB;

	transformedAABB.minDim *= mesh->scale;
	transformedAABB.maxDim *= mesh->scale;

	m4x4 mat = Translate(QuaternionToMatrix(mesh->orientation), mesh->pos);

	v3 d1 = V3(transformedAABB.maxDim.x - transformedAABB.minDim.x, 0, 0);
	v3 d2 = V3(0, transformedAABB.maxDim.y - transformedAABB.minDim.y, 0);
	v3 d3 = V3(0, 0, transformedAABB.maxDim.z - transformedAABB.minDim.z);

	v3 p[8] =
	{
		mat * transformedAABB.minDim,			//0

		mat * (transformedAABB.minDim + d1),		//1
		mat * (transformedAABB.minDim + d2),		//2
		mat * (transformedAABB.minDim + d3),		//3

		mat * (transformedAABB.minDim + d1 + d2),	//4
		mat * (transformedAABB.minDim + d2 + d3),	//5
		mat * (transformedAABB.minDim + d3 + d1),	//6

		mat * transformedAABB.maxDim,			//7

	};

	//_upper_ square
	PushLine(rg, p[0], p[1]);
	PushLine(rg, p[1], p[4]);
	PushLine(rg, p[4], p[2]);
	PushLine(rg, p[2], p[0]);

	//_lower_ square
	PushLine(rg, p[7], p[5]);
	PushLine(rg, p[5], p[3]);
	PushLine(rg, p[3], p[6]);
	PushLine(rg, p[6], p[7]);

	//_connecting_ lines
	PushLine(rg, p[0], p[3]);
	PushLine(rg, p[2], p[5]);
	PushLine(rg, p[1], p[6]);
	PushLine(rg, p[4], p[7]);

}

static void RenderEditorUI(RenderGroup *rg, Editor editor, Font font)
{

	RenderEditorPanel(rg, editor, font);

	For(editor.elements)
	{
		switch (it->type)
		{
		case EditorUI_ColorPicker:
		{
			RenderColorPicker(rg, &it->picker);
		}break;
		default:
		{
			Die;
		}break;

		}

	}

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


static void EditorGoToNone(Editor *editor)
{
	editor->state = EditorState_Default;
	editor->rotater = { 1.0f, 0, 0, 0 };
	editor->scaler = 1.0f;
	editor->mover = V3();
}

static PlacedMesh *GetHotMesh(World *world, v2 mousePosZeroToOne)
{
	v3 camP = world->camera.pos; // todo camera or debugCamera? Maybe we should again unify them
	v3 camD = ScreenZeroToOneToDirecion(world->camera, mousePosZeroToOne);

	For(world->placedMeshes)
	{
		m4x4 mat = QuaternionToMatrix(Inverse(it->orientation));
		v3 rayP = mat * (camP - it->pos);
		v3 rayD = mat * camD;
		AABB aabb = it->untransformedAABB;
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

		if (PointInAABB(aabb, curExit)) return it;

	}

	return NULL;
}

// maybe this is a bad function, as it does more then it says
static void EditorSetHotMeshToNULL(Editor *editor) 
{
	editor->panel.values.amount = 0;
	editor->panel.visible = false;
	editor->hotMesh = NULL;
}

static void EditorHandleEvents(Editor *editor, UnitHandler *unitHandler, World *world, KeyStateMessage message, Input input, f32 focalLength, f32 aspectRatio)
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
			
			break;
		}break;
		default:
		{
			Die;
		}break;

		}

	}

	if(message.flag & KeyState_Down)
	{
		switch (editor->state)
		{
		case EditorState_PickingColor:
		{
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
						switch(it->type)
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
						case Tweeker_b32:
						{
							*it->b = !*it->b;
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
						case Tweeker_v3:
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

						pos.y += height;
					}

					return;
				}
				// todo here we have to have some logic of UI vs Screen maybe just put flags on editor and to this in update?

				if (editor->hotMesh)
				{
					editor->panel.values.amount = 0;
					editor->hotMesh = NULL;
					editor->panel.visible = false;
				}

				PlacedMesh *hotMesh = GetHotMesh(world, input.mouseZeroToOne);
				
				if (hotMesh)
				{
					editor->hotMesh = hotMesh;
					editor->panel.visible = true;

					ArrayAdd(&editor->panel.values, CreateTweekerPointer(Tweeker_v3, "Pos", &hotMesh->pos));
					ArrayAdd(&editor->panel.values, CreateTweekerPointer(Tweeker_f32, "Scale", &hotMesh->scale));
					ArrayAdd(&editor->panel.values, CreateTweekerPointer(Tweeker_v4, "Color", &hotMesh->color));
					//ArrayAdd(&editor->panel.values, CreateTweeker()); orientation
				}
			}break;
			case Key_g:
			{
				editor->state = EditorState_Moving;
			}break;
			case Key_r:
			{
				editor->state = EditorState_Rotating;
			}break;
			case Key_s:
			{
				editor->state = EditorState_Scaling;
			}break;
			case Key_tab:
			{
				editor->state = EditorState_TileMapEditor;
				EditorSetHotMeshToNULL(editor);

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
				//// todo
				// Right/left, Top/bottom, front/back toggle
				
				cam->b3 = -cam->b3;
				cam->b1 = -cam->b1;

				cam->pos = editor->focusPoint - (cam->pos - editor->focusPoint);
				
			}break;
			case Key_numDot:
			{
				if (editor->hotMesh)
				{
					cam->pos = editor->hotMesh->pos + cam->pos - editor->focusPoint;
					editor->focusPoint = editor->hotMesh->pos;
				}
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
				EditorGoToNone(editor);
			}break;
			case Key_rightMouse:
			{
				editor->hotMesh->orientation = Inverse(editor->rotater) * editor->hotMesh->orientation;
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
				EditorGoToNone(editor);
			}break;
			case Key_rightMouse:
			{
				editor->hotMesh->pos -= editor->mover;
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
				EditorGoToNone(editor);
			}break;
			case Key_rightMouse:
			{
				editor->hotMesh->scale /= editor->scaler;
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

			if (message.key == Key_backSpace)
			{
				if (t->string.length)
				{
					t->string.length--;
				}
				return;
			}

			if (message.key == Key_enter)
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
				}
				
				t->string.length = 0;
				editor->panel.hotValue = 0xFFFFFFFF;
				EditorGoToNone(editor);
				return;
			}

		}break;
		case EditorState_TileMapEditor:
		{
			switch (message.key)
			{
			case Key_tab:
			{
				editor->state = EditorState_Default;
			}break;
			case Key_leftMouse:
			{
				v2 clickedP = ScreenZeroToOneToInGame(world->camera, input.mouseZeroToOne);
				Tile *tile = GetTile(world->tileMap, clickedP);
				if (tile)
				{
					switch(tile->type)
					{
					case Tile_Blocked:
					{
						tile->type = Tile_Empty;
					}break;
					case Tile_Empty:
					{
						tile->type = Tile_Blocked;
					}break;
					}
				}
			}break;
			case Key_g:
			{
				editor->state = EditorState_TileMapPlacingGoal;
			}break;
			case Key_s:
			{
				editor->state = EditorState_TileMapPlacingSpawner;
			}break;

			}

		}break;
		case EditorState_TileMapPlacingGoal:
		{
			switch (message.key)
			{
			case Key_leftMouse:
			{
				v2 clickedP = ScreenZeroToOneToInGame(world->camera, input.mouseZeroToOne);
				Tile *tile = GetTile(world->tileMap, clickedP);
				if (tile)
				{
					tile->type = (tile->type == Tile_Goal) ? Tile_Empty : Tile_Goal;
				}
				editor->state = EditorState_TileMapEditor;
			}break;
			case Key_rightMouse:
			{
				editor->state = EditorState_TileMapEditor;
			}break;
				
			}
			

		}break;
		case EditorState_TileMapPlacingSpawner:
		{
			switch (message.key)
			{
			case Key_leftMouse:
			{
				v2 clickedP = ScreenZeroToOneToInGame(world->camera, input.mouseZeroToOne);
				Tile *tile = GetTile(world->tileMap, clickedP);
				if (tile)
				{
					tile->type = (tile->type == Tile_Spawner) ? Tile_Empty : Tile_Spawner;
				}
				editor->state = EditorState_TileMapEditor;
			}break;
			case Key_rightMouse:
			{
				editor->state = EditorState_TileMapEditor;
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
static v3 GetMidPoint(PlacedMesh mesh)
{
	AABB transformedAABB = mesh.untransformedAABB;

	transformedAABB.minDim *= mesh.scale;
	transformedAABB.maxDim *= mesh.scale;

	m4x4 mat = Translate(QuaternionToMatrix(mesh.orientation), mesh.pos);

	v3 mid = 0.5f * (transformedAABB.maxDim + transformedAABB.minDim);


	return mat * mid;
}

// todo maybe make all this matrix stuff more consitent
static void UpdateEditor(Editor *editor, UnitHandler *unitHandler, World *world, Input input)
{
	Camera *cam = &world->camera;
	PlacedMesh* mesh = editor->hotMesh;

	UpdateColorPickers(editor, input);

	switch (editor->state)
	{
	case EditorState_TileMapEditor:
	{
		ColorForTileMap(world);
		return;
	}break;
	case EditorState_TileMapPlacingGoal:
	{
		ColorForTileMap(world);
		return;
	}break;
	case EditorState_TileMapPlacingSpawner:
	{
		ColorForTileMap(world);
		return;
	}break;
	case EditorState_OrbitingCamera:
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
		return;
	}break;
	

	}


	if (!editor->hotMesh)
	{
		return;
	}

	Tweekable(v4, editorMeshSelectColor, V4(1.0f, 0.4f, 0.5f, 0.1f));

	switch (editor->state)
	{
	case EditorState_Default:
	{
		mesh->frameColor *= editorMeshSelectColor;
	}break;
	case EditorState_Rotating:
	{
		AABB transformedAABB = mesh->untransformedAABB;

		transformedAABB.minDim *= mesh->scale;
		transformedAABB.maxDim *= mesh->scale;

		m4x4 mat = Translate(QuaternionToMatrix(mesh->orientation), mesh->pos);

		v3 mid = 0.5f * (transformedAABB.maxDim + transformedAABB.minDim);
		v3 front = V3(transformedAABB.minDim.x, 0.5f * (transformedAABB.maxDim.y + transformedAABB.minDim.y), 0.0f);

		// todo maybe  this should rotate in the plane depending on the pos
		// right now get projected onto z = 0, probably should be projected onto z = pos.z
		v2 midPoint = p12(mat * mid); // mid == pos?
		v2 frontPoint = p12(mat * front);
		v2 mousePos = ScreenZeroToOneToInGame(world->camera, input.mouseZeroToOne);
			
		
		f32 angle = AngleBetween(frontPoint - midPoint, mousePos - midPoint);

		Quaternion q = QuaternionFromAngleAxis(angle, V3(0, 0, 1));
			
		editor->hotMesh->orientation = q * editor->hotMesh->orientation;
		editor->rotater = q * editor->rotater;

	}break;
	case EditorState_Scaling:
	{
		AABB transformedAABB = mesh->untransformedAABB;

		transformedAABB.minDim *= mesh->scale;
		transformedAABB.maxDim *= mesh->scale;
		m4x4 mat = Translate(QuaternionToMatrix(mesh->orientation), mesh->pos);

		v3 mid = i12(0.5f * (transformedAABB.maxDim + transformedAABB.minDim).xy);
		v3 midPoint = (mat * mid); // pos?

		m4x4 proj = Projection(world->camera.aspectRatio, world->camera.focalLength) * CameraTransform(cam->basis.d1, cam->basis.d2, cam->basis.d3, cam->pos);
		v4 projectiveMidPoint = V4(midPoint, 1.0f);
			 
		v4 projectedMidPoint = (proj * projectiveMidPoint);
		Assert(projectedMidPoint.w);
		v2 screenPoint = (projectedMidPoint / projectedMidPoint.w).xy; // in -1 to 1 range
		screenPoint = 0.5f * screenPoint + V2(0.5f, 0.5f);
		screenPoint = V2(screenPoint.x, 1.0f - screenPoint.y);
		v2 mouseD =  3.0f * input.mouseZeroToOneDelta;
		if (mouseD == V2()) break;

		v2 p = input.mouseZeroToOne;
		v2 d = Normalize(p - screenPoint);
		f32 dot = Dot(mouseD, d);

		f32 exp = expf(dot);

		editor->scaler *= exp;
		editor->hotMesh->scale *= exp;
		editor->hotMesh->pos = V3(p12(midPoint) + exp * p12(editor->hotMesh->pos - midPoint), editor->hotMesh->pos.z);

	}break;
	case EditorState_Moving:
	{	
		v3 p = editor->hotMesh->pos;
		v3 cp = cam->pos;
		v3 diff = cp - p;

		v3 oldP = ScreenZeroToOneToScreenInGame(*cam, input.mouseZeroToOne - input.mouseZeroToOneDelta);
		v3 newP = ScreenZeroToOneToScreenInGame(*cam, input.mouseZeroToOne);

		v3 oP = SolveLinearSystem((cp - oldP), cam->b1, cam->b2, diff);
		v3 nP = SolveLinearSystem((cp - newP), cam->b1, cam->b2, diff);
			
		m3x3 mat = Columns3x3((cp - oldP), cam->b1, cam->b2);
		v3 asd = mat * oP; // == cp - p == 

		v3 oI = cp + oP.x * (oldP - cp); // = p + op.y * cam->b1 + op.z * cam->b2
		v3 nI = cp + nP.x * (newP - cp);
			
		v3 realDelta = nI - oI;

		editor->hotMesh->pos += realDelta;
		editor->mover += realDelta;
	}break;
	case EditorState_DragingPanel:
	{
		mesh->frameColor *= editorMeshSelectColor;
		editor->panel.pos += input.mouseZeroToOneDelta;
	}break;
	case EditorState_PickingColor:
	{
	
	}break;
	case EditorState_AlteringValue:
	{
		mesh->frameColor *= editorMeshSelectColor;
	}break;

	default:
	{
		Die;
	}break;
	}
}

// todo  make the controles as they are in blender
// num pad keys
// middle mouse button for roato potato // done?
// make aabb recalc ? right now it is behind by a frame.

#endif // !RR_EDITOR

