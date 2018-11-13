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

	v4 pickedColor;
	Tweeker tweeker; // for now, rethink this
};
enum EditorUIElementEnum
{
	EditorUI_ColorPicker
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

struct Editor
{
	EditorUIElementDynamicArray elements;
};


// todo: i don't think it matters here, but I really have to get these color things straight.
static ColorPicker CreateColorPicker(v4 initialColor)
{

	// WARINIG : the factors DO NOT commute with the interpolation, do they?

	// calculating bar pos and picker pos : colors in the bar are of the form 0xFF00abFF or 0xFFabFF00, or something like that (2F, 20, 2 something)
	ColorPicker ret;

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
	u32 c = Pack4x8(initialColor);
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
static bool ColorPickerHandleEvent(ColorPicker *picker, KeyStateMessage message, Input input)
{
	if (!message.key == Key_leftMouse)
	{
		return false;
	}

	if ((message.flag & KeyState_PressedThisFrame))
	{
		if (PointInRectangle(picker->sliderPos, picker->width, picker->sliderHeight, input.mouseZeroToOne))
		{
			picker->selecting = (message.flag & KeyState_Down) ? PickerSelecting_Slider : PickerSelecting_Nothing;
			return false;
		}
		if (PointInRectangle(picker->pos, picker->width, picker->height, input.mouseZeroToOne))
		{
			picker->selecting = (message.flag & KeyState_Down) ? PickerSelecting_ColorPicker : PickerSelecting_Nothing;
			return false;
		}
		if (PointInRectangle(picker->killRectPos, picker->headerSize, picker->headerSize, input.mouseZeroToOne))
		{
			WriteSingleTweeker(picker->tweeker);
			return true;
		}
		if (PointInRectangle(picker->headerPos, picker->width, picker->headerSize, input.mouseZeroToOne))
		{
			picker->selecting = (message.flag & KeyState_Down) ? PickerSelecting_Header : PickerSelecting_Nothing;
			return false;
		}

	}

	if (message.flag & KeyState_ReleasedThisFrame)
	{
		picker->selecting = PickerSelecting_Nothing;
	}
	return false;
}

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

static void EditorHandleEvents(Editor *editor, KeyStateMessage message, Input input)
{
	For(editor->elements)
	{
		switch (it->type)
		{
		case EditorUI_ColorPicker:
		{
			bool shouldClose = ColorPickerHandleEvent(&it->picker, message, input);
			if (shouldClose)
			{
				ArrayRemove(&editor->elements, it);
			}
		}break;
		default:
		{
			Die;
		}break;

		}

	}
}

static void ColorPickerUpdate(ColorPicker *picker, Input input)
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

	picker->pickedColor = V4(1.0f, color);

	//todo if we use this in some other context, we either have to make everything tweekers, or have to re-formalize this.
	picker->tweeker.vec4 = picker->pickedColor;
	Tweek(picker->tweeker);
}

static void ColorPickerRender(RenderGroup *rg, ColorPicker *picker)
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
	PushString(rg, V2(picker->pos.x + 0.0f * pickerFontXIncrease * pickerFontSize, fontY), FormatString("a:%f32", picker->pickedColor.a), pickerFontSize, globalFont);
	PushString(rg, V2(picker->pos.x + 1.0f * pickerFontXIncrease * pickerFontSize, fontY), FormatString("r:%f32", picker->pickedColor.r), pickerFontSize, globalFont);
	PushString(rg, V2(picker->pos.x + 2.0f * pickerFontXIncrease * pickerFontSize, fontY), FormatString("g:%f32", picker->pickedColor.g), pickerFontSize, globalFont);
	PushString(rg, V2(picker->pos.x + 3.0f * pickerFontXIncrease * pickerFontSize, fontY), FormatString("b:%f32", picker->pickedColor.b), pickerFontSize, globalFont);

}

static void RenderEditor(RenderGroup *rg, Editor editor)
{
	For(editor.elements)
	{
		switch (it->type)
		{
		case EditorUI_ColorPicker:
		{
			ColorPickerRender(rg, &it->picker);
		}break;
		default:
		{
			Die;
		}break;

		}

	}
}

static String GetFilePathFromName(String fileName)
{
	u32 positionOfLastSlash = 0;
	for (u32 i = fileName.length - 1; i < MAXU32; i--)
	{
		if (fileName[i] == '/')
		{
			positionOfLastSlash = i;
			break;
		}
	}
	if (!positionOfLastSlash)
	{
		return {};
	}

	String ret; 
	ret.data = fileName.data;
	ret.length = positionOfLastSlash + 1;
	return ret;
}


//look at 1:50 in vertex vbo
// todo right now this allocates into "arena", exept name and bitmap are hardcoded to go into constant arena and virtual alloc
static MaterialDynamicArray LoadMTL(AssetHandler *assetHandler, String path, String fileName, Arena *arena)
{
	MaterialDynamicArray ret = MaterialCreateDynamicArray();

	u8 *frameArenaReset = frameArena->current;
	defer(frameArena->current = frameArenaReset);
	File file = LoadFile((char *)FormatCString("%s%s\0", path, fileName).data, frameArena);
	Assert(file.fileSize);
	String string = CreateString((Char *)file.memory, file.fileSize); 
	
	b32 success = true;

	while (string.length)
	{
		Material cur = {};
		String line = ConsumeNextLineSanitize(&string);
		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }

		// newmtl
		{
			String ident = EatToNextSpaceReturnHead(&line);
			Assert(ident == "newmtl");
			EatSpaces(&line);
			
			cur.name = CopyString(line, constantArena);
		}

		line = ConsumeNextLineSanitize(&string);
		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }

		// Ns
		{
			String ident = EatToNextSpaceReturnHead(&line);
			Assert(ident == "Ns");
			EatSpaces(&line);
			cur.spectularExponent = StoF(line, &success);
		}

		line = ConsumeNextLineSanitize(&string);
		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }

		// Ka
		{
			String ident = EatToNextSpaceReturnHead(&line);
			Assert(ident == "Ka");
			EatSpaces(&line);
			String head = EatToNextSpaceReturnHead(&line);
			f32 a1 = StoF(head, &success);

			EatSpaces(&line);
			head = EatToNextSpaceReturnHead(&line);
			f32 a2 = StoF(head, &success);

			EatSpaces(&line);
			head = EatToNextSpaceReturnHead(&line);
			f32 a3 = StoF(head, &success);
			
			cur.ambientColor = V3(a1, a2, a3);
		}

		line = ConsumeNextLineSanitize(&string);
		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }

		// Kd
		{
			String ident = EatToNextSpaceReturnHead(&line);
			Assert(ident == "Kd");
			EatSpaces(&line);
			String head = EatToNextSpaceReturnHead(&line);
			f32 a1 = StoF(head, &success);

			EatSpaces(&line);
			head = EatToNextSpaceReturnHead(&line);
			f32 a2 = StoF(head, &success);

			EatSpaces(&line);
			head = EatToNextSpaceReturnHead(&line);
			f32 a3 = StoF(head, &success);

			cur.diffuseColor = V3(a1, a2, a3);
		}

		line = ConsumeNextLineSanitize(&string);
		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }

		// Ks
		{
			String ident = EatToNextSpaceReturnHead(&line);
			Assert(ident == "Ks");
			EatSpaces(&line);
			String head = EatToNextSpaceReturnHead(&line);
			f32 a1 = StoF(head, &success);

			EatSpaces(&line);
			head = EatToNextSpaceReturnHead(&line);
			f32 a2 = StoF(head, &success);

			EatSpaces(&line);
			head = EatToNextSpaceReturnHead(&line);
			f32 a3 = StoF(head, &success);

			cur.specularColor = V3(a1, a2, a3);
		}

		line = ConsumeNextLineSanitize(&string);
		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }

		// Ke
		{
			String ident = EatToNextSpaceReturnHead(&line);
			Assert(ident == "Ke");
			EatSpaces(&line);
			String head = EatToNextSpaceReturnHead(&line);
			f32 a1 = StoF(head, &success);

			EatSpaces(&line);
			head = EatToNextSpaceReturnHead(&line);
			f32 a2 = StoF(head, &success);

			EatSpaces(&line);
			head = EatToNextSpaceReturnHead(&line);
			f32 a3 = StoF(head, &success);

			cur.ke = V3(a1, a2, a3);
		}

		line = ConsumeNextLineSanitize(&string);
		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }

		// Ni
		{
			String ident = EatToNextSpaceReturnHead(&line);
			Assert(ident == "Ni");
			EatSpaces(&line);
			String head = EatToNextSpaceReturnHead(&line);
			f32 a1 = StoF(head, &success);

			cur.indexOfReflection = a1;
		}

		line = ConsumeNextLineSanitize(&string);
		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }

		// d
		{
			String ident = EatToNextSpaceReturnHead(&line);
			Assert(ident == "d");
			EatSpaces(&line);
			String head = EatToNextSpaceReturnHead(&line);
			f32 a1 = StoF(head, &success);

			cur.dissolved = a1;
		}

		line = ConsumeNextLineSanitize(&string);
		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }

		// illum
		{
			String ident = EatToNextSpaceReturnHead(&line);
			Assert(ident == "illum");
			EatSpaces(&line);
			String head = EatToNextSpaceReturnHead(&line);
			u32 a1 = StoU(head, &success);

			cur.illuminationModel = a1;
		}

		line = ConsumeNextLineSanitize(&string);
		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }

		// map_Kd
		{
			String ident = EatToNextSpaceReturnHead(&line);
			Assert(ident == "map_Kd");
			EatSpaces(&line);
			if(assetHandler) cur.bitmapID = RegisterAsset(assetHandler, Asset_Texture, (char *)FormatCString("%s%s%c1", path, line, '\0').data);
		}

		ArrayAdd(&ret, cur);
	}

	Assert(success);

	return ret;
}

static TriangleMesh ReadObj(AssetHandler *assetHandler, char *fileName)
{
	String filename = CreateString(fileName);
	String path = GetFilePathFromName(filename);
	Arena *arena = constantArena;
	TriangleMesh ret = {};

	u8 *frameArenaReset = frameArena->current;
	defer(frameArena->current = frameArenaReset);

	File file = LoadFile(fileName, frameArena);
	if (!file.fileSize) return {};
	String string = CreateString((Char *)file.memory, file.fileSize);

	// here mtllib "bla.mtl"
	String mtllib = ConsumeNextLineSanitize(&string);
	while (string.length && mtllib.length == 0 || mtllib[0] == '#') { mtllib = ConsumeNextLineSanitize(&string); }
	Assert(string.length);
	String ident = EatToNextSpaceReturnHead(&mtllib);
	Assert(ident == "mtllib");
	EatSpaces(&mtllib);

	MaterialDynamicArray materials = LoadMTL(assetHandler, path, mtllib, frameArena); 
	// todo make sure that multiples get handled, i.e make this assets.

	b32 success = true;
	u32 amountOfVertsBefore = 1;
	u32 amountOfNormalsBefore = 1;
	u32 amountOfUVsBefore = 1;
	u32 amountOfIndeciesBefore = 0;
	String line = ConsumeNextLineSanitize(&string);

	u16PtrDynamicArray indexPointerArray = u16PtrCreateDynamicArray();
	IndexSetDynamicArray indexSets = IndexSetCreateDynamicArray();

	// begin on vertex daisy chain
	ret.vertices.data = (VertexFormat *)arena->current;

	while (string.length)
	{
		IndexSet cur;

		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }
		if (!string.length) break;

		// o
		{
			String head = EatToNextSpaceReturnHead(&line);
			Assert(head == "o");
			EatSpaces(&line);
			String o = line; // ignored for now
		}

		line = ConsumeNextLineSanitize(&string);
		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }
		if (!string.length) break;

		// todo  speed, here we first load and then copy later when we read in the triangles, not sure if we could make this better...

		BeginArray(frameArena, v3, tempVertexArray); // could silent fill and the push, which is not as "save".
		while (line[0] == 'v' && line[1] == ' ')
		{
			Eat1(&line);
			EatSpaces(&line);
			String f1 = EatToNextSpaceReturnHead(&line);
			EatSpaces(&line);
			String f2 = EatToNextSpaceReturnHead(&line);
			EatSpaces(&line);
			String f3 = EatToNextSpaceReturnHead(&line);
			
			f32 s1 = StoF(f1, &success);
			f32 s2 = StoF(f2, &success);
			f32 s3 = StoF(f3, &success);
			v3 *val = PushStruct(frameArena, v3);
			*val = V3(s1, s2, s3);
			
			line = ConsumeNextLineSanitize(&string);
		}
		EndArray(frameArena, v3, tempVertexArray);
		
		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }
		if (!string.length) break;

		BeginArray(frameArena, v2, textrueCoordinates);
		while (line[0] == 'v' && line[1] == 't')
		{
			Eat1(&line);
			Eat1(&line);
			EatSpaces(&line);
			String f1 = EatToNextSpaceReturnHead(&line);
			EatSpaces(&line);
			String f2 = EatToNextSpaceReturnHead(&line);

			f32 s1 = StoF(f1, &success);
			f32 s2 = StoF(f2, &success);
			v2 *val = PushStruct(frameArena, v2);
			*val = V2(s1, s2);

			line = ConsumeNextLineSanitize(&string);
		}
		EndArray(frameArena, v2, textrueCoordinates);

		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }
		if (!string.length) break;
		BeginArray(frameArena, v3, normals);
		while (line[0] == 'v' && line[1] == 'n')
		{
			Eat1(&line);
			Eat1(&line);
			EatSpaces(&line);
			String f1 = EatToNextSpaceReturnHead(&line);
			EatSpaces(&line);
			String f2 = EatToNextSpaceReturnHead(&line);
			EatSpaces(&line);
			String f3 = EatToNextSpaceReturnHead(&line);

			f32 s1 = StoF(f1, &success);
			f32 s2 = StoF(f2, &success);
			f32 s3 = StoF(f3, &success);
			v3 *val = PushStruct(frameArena, v3);
			*val = V3(s1, s2, s3);

			line = ConsumeNextLineSanitize(&string);
		}
		EndArray(frameArena, v3, normals);

		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }
		if (!string.length) break;

		// usemtl
		{
			String head = EatToNextSpaceReturnHead(&line);
			Assert(head == "usemtl");
			EatSpaces(&line);
			String name = line;
			For(materials)
			{
				if (it->name == name)
				{
					cur.mat = *it;
					break;
				}
			}
		}

		line = ConsumeNextLineSanitize(&string);
		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }
		if (!string.length) break;

		// s -smoothening, what ever that means
		{
			String head = EatToNextSpaceReturnHead(&line);
			Assert(head == "s");
			EatSpaces(&line);
			u32 s = StoU(line, &success); // ignored for now
		}

		line = ConsumeNextLineSanitize(&string);
		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }
		if (!string.length) break;

		struct UVList
		{
			u16 uvIndex;
			u16 flattendIndex;
			u16 normalIndex;
			UVList *next;
		};
		typedef UVList * UVListPtr;
		DefineArray(UVListPtr);

		Clear(workingArena);
		UVListPtrArray flatten = PushZeroArray(workingArena, UVListPtr, tempVertexArray.amount);
		u32 vertexArrayLength = 0;

		cur.offset = amountOfIndeciesBefore;
		
		BeginArray(frameArena, u16, indecies);
		ArrayAdd(&indexPointerArray, indecies.data);
		while (line[0] == 'f')
		{
			Eat1(&line);

			// we assume they are all here

			for (u32 parseIndex = 0; parseIndex < 3; parseIndex++)
			{
				EatSpaces(&line);
				String s1 = EatToCharReturnHead(&line, '/', ' ');
				Eat1(&line);
				String s2 = EatToCharReturnHead(&line, '/', ' ');
				Eat1(&line);
				String s3 = EatToCharReturnHead(&line, ' ');


				u32 u1 = StoU(s1, &success) - amountOfVertsBefore;		// this has to be relative to be an index, but also saved absolute wrt the flattend array
				u32 u2 = StoU(s2, &success) - amountOfUVsBefore;		// this is relative as we just need it to build our array
				u32 u3 = StoU(s3, &success) - amountOfNormalsBefore;	// this is also relative 

				//todo no handling for meshes, that have more then 0xFFFE verticies
				
				// we are flattening trough an array of vertex positions
				u16 flattenedIndex = 0xFFFF;
				for (UVListPtr it = flatten[u1]; it; it = it->next)
				{
					if (it->uvIndex == u2 && it->normalIndex == u3)
					{
						flattenedIndex = it->flattendIndex;
						break;
					}
				}

				if (flattenedIndex == 0xFFFF)
				{
					UVList *append = PushStruct(workingArena, UVList);
					append->flattendIndex = vertexArrayLength++;
					append->next = flatten[u1];
					append->uvIndex = (u16)u2;
					append->normalIndex = (u16)u3;
					flatten[u1] = append;
					
					flattenedIndex = append->flattendIndex;
				}

				*PushStruct(frameArena, u16) = flattenedIndex + ret.vertices.amount;
			}

			line = ConsumeNextLineSanitize(&string);
		}
		EndArray(frameArena, u16, indecies);
		cur.amount = indecies.amount;

		ArrayAdd(&indexSets, cur);

		amountOfIndeciesBefore += indecies.amount;
		amountOfVertsBefore += tempVertexArray.amount;
		amountOfNormalsBefore += normals.amount;
		amountOfUVsBefore += textrueCoordinates.amount;
		ret.vertices.amount += vertexArrayLength;

		// vertexArrayLength allready incrented by flattening
		VertexFormatArray save = PushArray(arena, VertexFormat, vertexArrayLength);
		//
		// WARNING, we are daisy chaining here into constantArena and into frameArena
		//
		For (flatten)
		{
			u32 it_index = (u32)(it - &flatten[0]);
			for (UVListPtr i = *it; i; i = i->next)
			{
				save[i->flattendIndex].p = tempVertexArray[it_index];
				save[i->flattendIndex].uv = textrueCoordinates[i->uvIndex];
				save[i->flattendIndex].n = normals[i->normalIndex];
				save[i->flattendIndex].c = 0xFFFFFFFF;
			}
		}
	}
	

	ret.indices.data = (u16 *)arena->current;
	Assert(indexPointerArray.amount == indexSets.amount);
	for(u32 i = 0; i < indexPointerArray.amount; i++)
	{
		IndexSet set = indexSets[i];
		u16 *source = indexPointerArray[i];
		u16 *dest = PushData(arena, u16, set.amount);
		memcpy(dest, source, set.amount * sizeof(u16));
	}
	ret.indices.amount = (u32)((u16 *)arena->current - ret.indices.data);

	ret.indexSets = PushArray(arena, IndexSet, indexSets.amount);
	memcpy(ret.indexSets.data, indexSets.data, indexSets.amount * sizeof(IndexSet));

	ret.type = TriangleMeshType_List;

	RegisterTriangleMesh(&ret);

	Assert(success);
	
	return ret;
}

static void UpdateEditor(Editor *editor, Input input)
{
	For(editor->elements)
	{
		switch (it->type)
		{
		case EditorUI_ColorPicker:
		{
			ColorPickerUpdate(&it->picker, input);
		}break;
		default:
		{
			Die;
		}break;

		}

	}
}

static void WriteTexture(char *fileName, Bitmap bitmap)
{
	u8 *arenaCur = frameArena->current;
	defer(frameArena->current = arenaCur);

	Assert(bitmap.height == AssetBitmapSize);
	Assert(bitmap.width  == AssetBitmapSize);
	Assert(bitmap.pixels);

	u32 fileSize = AssetBitmapSize * AssetBitmapSize * sizeof(u32);
	File file = CreateFile(PushData(frameArena, u8, fileSize), fileSize);
	memcpy(file.memory, bitmap.pixels, fileSize);
	WriteEntireFile(fileName, file);
}



#endif // !RR_EDITOR

