
static void DrawNumberOnTile(RenderGroup *rg, u32 number, v3i pos, v4 color = V4(1, 0.2f, 0.3f, 0.6f))
{
	f32 border = 0.05f;
	f32 dim = 1.0f;
	f32 dimWoBorder = dim - 2.0f * border;

	f32 curZ = (f32)number * 0.0004f;

	PushRectangle(rg, V3(pos) - 0.5f * V3(dim, dim, curZ + 0.0001f), V3(dim, 0, 0), V3(0, dim, 0), V4(1, 1, 1, 1));
	PushRectangle(rg, V3(pos) - 0.5f * V3(dimWoBorder, dimWoBorder, curZ + 0.0002f), V3(dimWoBorder, 0, 0), V3(0, dimWoBorder, 0), color);

	f32 halfDim = dimWoBorder / 2.0f;
	if (number < 10)
	{
		v3 p1 = V3(pos) - 0.5f * V3(halfDim, halfDim, curZ + 0.0003f);
		v3 p2 = p1 + V3(halfDim, 0, 0);
		v3 p3 = p1 + V3(0, halfDim, 0);
		v3 p4 = p1 + V3(halfDim, halfDim, 0);

		CharData data = globalFont.charData['0' + number];

		PushTexturedQuad(rg, p1, p2, p3, p4, globalFont.bitmap, V4(1, 1, 1, 1), true, data.minUV, data.maxUV);
	}
	else if (number < 100)
	{
		u32 first = number / 10;
		u32 second = number - first * 10;



		{
			v3 p1 = V3(pos) - 0.5f * V3(dimWoBorder, halfDim, curZ + 0.0003f);
			v3 p2 = p1 + V3(halfDim, 0, 0);
			v3 p3 = p1 + V3(0, halfDim, 0);
			v3 p4 = p1 + V3(halfDim, halfDim, 0);

			CharData data = globalFont.charData['0' + first];

			PushTexturedQuad(rg, p1, p2, p3, p4, globalFont.bitmap, V4(1, 1, 1, 1), true, data.minUV, data.maxUV);
		}

		{
			v3 p1 = V3(pos) - 0.5f * V3(0, halfDim, curZ + 0.0003f);
			v3 p2 = p1 + V3(halfDim, 0, 0);
			v3 p3 = p1 + V3(0, halfDim, 0);
			v3 p4 = p1 + V3(halfDim, halfDim, 0);

			CharData data = globalFont.charData['0' + second];

			PushTexturedQuad(rg, p1, p2, p3, p4, globalFont.bitmap, V4(1, 1, 1, 1), true, data.minUV, data.maxUV);
		}
	}
}

static void RenderPathCreator(RenderGroup *rg, World *world, ExecuteData *exe, PathCreator *pathCreator, UnitHandler *unitHandler, Input input)
{
	if (pathCreator->hotUnit < unitHandler->amount)
	{
		auto path = unitHandler->programs[pathCreator->hotUnit];
		
		u32 serial = unitHandler->entitySerials[pathCreator->hotUnit];
		Entity *e = GetEntity(world, serial);
		u32 pathCounter = 0;

		ResetWorld(world);

		For(path)
		{
			DrawNumberOnTile(rg, pathCounter, e->physicalPos);
			AdvanceGameState(world, unitHandler, exe, false);
			pathCounter++;
		}

		DrawNumberOnTile(rg, pathCounter, e->physicalPos);

		v3i pos = e->physicalPos;
		v3 mouseP = ScreenZeroToOneToZ(world->camera, input.mouseZeroToOne, e->physicalPos.z);
		v3 mouseToPath = mouseP - GetRenderPos(*e);

		pathCounter++;

		v4 newTileColor = V4(1, 0.4f, 0.1f, 0.3f);

#if 0
		Entity *suppotingEntity = GetEntityInTile(world, GetPhysicalPositionAfterMove(e) + V3i(0, 0, 1));
		if (!suppotingEntity || !(suppotingEntity->flags & EntityFlag_SupportsUnit))
		{
			DrawNumberOnTile(rg, pathCounter, pos + V3i(0, 0, 1), newTileColor);
		}
		else
#endif // !0

		if (BoxNorm(mouseToPath) > 0.5f)
		{
			i32 xSign = (mouseToPath.x > 0) ? 1 : -1;
			i32 ySign = (mouseToPath.y > 0) ? 1 : -1;

			if ((f32)ySign * mouseToPath.y > (f32)xSign * mouseToPath.x)
			{
				if (ySign > 0)
				{
					DrawNumberOnTile(rg, pathCounter, pos + V3i(0, 1, 0), newTileColor);
				}
				else
				{
					DrawNumberOnTile(rg, pathCounter, pos + V3i(0, -1, 0), newTileColor);
				}
			}
			else
			{
				if (xSign > 0)
				{
					DrawNumberOnTile(rg, pathCounter, pos + V3i(1, 0, 0), newTileColor);
				}
				else
				{
					DrawNumberOnTile(rg, pathCounter, pos + V3i(-1, 0, 0), newTileColor);
				}
			}
		}
		else
		{
			DrawNumberOnTile(rg, pathCounter, pos, newTileColor);
		}

	}

	For(world->entities)
	{
		switch (it->type)
		{
		case Entity_Dude:
		{
			PushTriangleMesh(rg, it->meshId, it->orientation, V3((it->physicalPos)), it->scale, V4(0.75f, 0.0f, 0.0f, 0.0f));
			PushTriangleMesh(rg, it->meshId, it->orientation, V3((it->initialPos)), it->scale, it->color * it->frameColor);
		}break;
		case Entity_Block:
		{
			PushTriangleMesh(rg, it->meshId, it->orientation, V3((it->physicalPos)), it->scale, V4(0.75f, 0.0f, 0.0f, 0.0f));
			PushTriangleMesh(rg, it->meshId, it->orientation, V3((it->initialPos)), it->scale, it->color * it->frameColor);
		}break;
		default:
		{
			PushTriangleMesh(rg, it->meshId, it->orientation, GetRenderPos(*it), it->scale, it->color * it->frameColor);
		}break;
		}
	}
}

static void RenderPathCreatorUI(RenderGroup *rg, PathCreator *pathCreator)
{
	switch (pathCreator->state)
	{
	case PathCreator_CreatingPath:
	{
		PushRectangle(rg, pathCreator->finishButton.pos, pathCreator->finishButton.width, pathCreator->finishButton.height, V4(1.0f, 0.5, 0.7f, 0.5f));
		PushString(rg, pathCreator->finishButton.pos, "Finish", 0.05f, globalFont);
		PushRectangle(rg, pathCreator->resetUnitButton.pos, pathCreator->resetUnitButton.width, pathCreator->resetUnitButton.height, V4(1.0f, 0.7f, 0.3f, 0.5f));
		PushString(rg, pathCreator->resetUnitButton.pos, "Reset", 0.05f, globalFont);
	}break;
	}
}

static void RenderSimulateUI(RenderGroup *rg, SimData *sim)
{
	v2 pos = V2(0.05f, 0.05f);
	f32 width = 0.3f;
	f32 height = 0.05f;
	f32 border = 0.01f;

	PushRectangle(rg, pos, width, height, V4(1.0f, 0.2f, 0.3f, 0.6f));
	f32 percentage = (f32)sim->blocksCollected / (f32)sim->blocksNeeded;
	PushRectangle(rg, pos + V2(border, border), percentage * (width - 2.0f * border), (height - 2.0f * border), V4(1.0f, 0.4f, 0.6f, 0.9f));
	String progressString = FormatString("%u32/%u32", sim->blocksCollected, sim->blocksNeeded);
	PushString(rg, pos + V2(border, border), progressString, (height - 2.0f * border), globalFont);

}

static void RenderSimulate(RenderGroup *rg, World *world, UnitHandler *unitHandler)
{
#if 1
	For(world->entities)
	{
		PushTriangleMesh(rg, it->meshId, it->orientation, GetRenderPos(*it, unitHandler->t), it->scale, it->color * it->frameColor);
	}
#else
	For(world->entities)
	{
		PushTriangleMesh(rg, it->meshId, it->orientation, GetRenderPos(*it), it->scale, it->color * it->frameColor);
	}
#endif
}

static void RenderPlacingUnits(RenderGroup *rg, ExecuteData *exe, World *world, AssetHandler *assetHandler, Input input)
{
	Entity *clickedE = GetHotEntity(world, assetHandler, input.mouseZeroToOne);
	if (clickedE)
	{
		v3i pos = clickedE->physicalPos + V3i(0, 0, -1);

		Entity *e = exe->placingUnits.unitsToPlace[0];
		if (!TileBlocked(world, pos))
		{
			PushTriangleMesh(rg, world->dudeMeshId, e->orientation, V3(pos), e->scale, V4(1.0f, 0.2f, 1.0f, 0.3f));
		}
		else
		{
			PushTriangleMesh(rg, world->dudeMeshId, e->orientation, V3(pos), e->scale, V4(1.0f, 1.0f, 0.2f, 0.3f));
		}
	}

	For(exe->placingUnits.unitsToPlace)
	{
		(*it)->frameColor.a = 0.0f;
	}

	For(world->entities)
	{
		PushTriangleMesh(rg, it->meshId, it->orientation, GetRenderPos(*it), it->scale, it->color * it->frameColor);
	}
}

static void RenderExecute(RenderGroup *rg, World *world, ExecuteData *exe, UnitHandler *unitHandler, AssetHandler *assetHandler, Input input)
{
	switch (exe->state)
	{
	case Execute_PlacingUnits:
	{
		RenderPlacingUnits(rg, exe, world, assetHandler, input);
	}break;
	case Execute_PathCreator:
	{
		RenderPathCreator(rg, world, exe, &exe->pathCreator, unitHandler, input);
	}break;
	case Execute_Simulation:
	{
		RenderSimulate(rg, world, unitHandler);
	}break;
	case Execute_Victory:
	{
		RenderSimulate(rg, world, unitHandler);
	}break;
	InvalidDefaultCase;

	}

}

static void RenderVictoryUI(RenderGroup *rg)
{
	PushString(rg, V2(0.1f, 0.1f), "Victory", 0.2f, globalFont);
}

static void RenderExecuteUI(RenderGroup *rg, ExecuteData *exe)
{
	switch (exe->state)
	{
	case Execute_PlacingUnits:
	{

	}break;
	case Execute_PathCreator:
	{
		RenderPathCreatorUI(rg, &exe->pathCreator);
	}break;
	case Execute_Simulation:
	{
		RenderSimulateUI(rg, &exe->simData);
	}break;
	case Execute_Victory:
	{
		RenderVictoryUI(rg);
	}break;

	InvalidDefaultCase;

	}
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

		f32 height = HeightForTweekerPanel(it->type, editorBorderWidth, editorPanelFontSize);

		switch (it->type)
		{
		case Tweeker_b32:
		{
			PushRectangle(rg, valuePos, widthWithoutBoarder, height, editorPanelValueColor);

			f32 indecatorSquareSize = height - editorBorderWidth;

			v2 squarePos = valuePos + V2(widthWithoutBoarder - height, editorBorderWidth);
			PushRectangle(rg, squarePos, indecatorSquareSize, indecatorSquareSize, V4(1, 1, 1, 1));

			f32 innerSquareSize = indecatorSquareSize - editorBorderWidth;

			if (*it->b)
			{
				PushRectangle(rg, squarePos + 0.5f * V2(editorBorderWidth, editorBorderWidth), innerSquareSize, innerSquareSize, V4(1, 0.3f, 0.8f, 0.3f));
			}
			else
			{
				PushRectangle(rg, squarePos + 0.5f * V2(editorBorderWidth, editorBorderWidth), innerSquareSize, innerSquareSize, V4(1, 0.8f, 0.3f, 0.3f));
			}
		}break;
		case Tweeker_u32:
		{
			PushRectangle(rg, valuePos, widthWithoutBoarder, height, editorPanelValueColor);

			if (p->values.data + p->hotValue == it)
			{
				PushString(rg, continuePos, p->textInput.string, editorPanelFontSize, font);
				break;
			}

			String str = UtoS(*it->u);
			PushString(rg, continuePos, str, editorPanelFontSize, font);
		}break;
		case Tweeker_EntityType:
		{
			PushRectangle(rg, valuePos, widthWithoutBoarder, height, editorPanelValueColor);

			writePos.x += 0.3f * widthWithoutBoarder;

			switch (*it->u)
			{
			case Entity_None:
			{
				PushString(rg, writePos, "None", editorPanelFontSize, font);
			}break;
			case Entity_Block:
			{
				PushString(rg, writePos, "Block", editorPanelFontSize, font);
			}break;
			case Entity_Dude:
			{
				PushString(rg, writePos, "Dude", editorPanelFontSize, font);
			}break;
			case Entity_Wall:
			{
				PushString(rg, writePos, "Wall", editorPanelFontSize, font);
			}break;
			case Entity_Goal:
			{
				PushString(rg, writePos, "Goal", editorPanelFontSize, font);
			}break;
			case Entity_Spawner:
			{
				PushString(rg, writePos, "Spawner", editorPanelFontSize, font);
			}break;

			}

			f32 rightArrowPos = widthWithoutBoarder - editorPanelFontSize;
			//writePos.y += 0.5f * editorPanelFontSize;
			writePos.x -= 0.3f * widthWithoutBoarder;

			PushTriangle(rg, writePos + V2(rightArrowPos, 0), writePos + V2(rightArrowPos, editorPanelFontSize), writePos + V2(widthWithoutBoarder, 0.5f * editorPanelFontSize), V4(1.0f, 1.0f, 0.8f, 0.2f));
			PushTriangle(rg, writePos + V2(editorPanelFontSize, 0), writePos + V2(editorPanelFontSize, editorPanelFontSize), writePos + V2(0, 0.5f * editorPanelFontSize), V4(1.0f, 1.0f, 0.8f, 0.2f));
		}break;
		case Tweeker_f32:
		{
			PushRectangle(rg, valuePos, widthWithoutBoarder, height, editorPanelValueColor);
			if (p->values.data + p->hotValue == it)
			{
				PushString(rg, continuePos, p->textInput.string, editorPanelFontSize, font);
				break;
			}

			String str = FtoS(*it->f);
			PushString(rg, continuePos, str, editorPanelFontSize, font);

		}break;
		case Tweeker_v2:
		{
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

		}break;
		case Tweeker_v3:
		{
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
		}break;
		case Tweeker_v3i:
		{
			PushRectangle(rg, valuePos, widthWithoutBoarder, height, editorPanelValueColor);

			if (xyz == 'x' && (p->values.data + p->hotValue == it))
			{
				String str = FormatString("x: %s", p->textInput.string);
				PushString(rg, writePos, str, editorPanelFontSize, font);
			}
			else
			{
				String strx = FormatString("x: %s", ItoS(it->vec3i->x));
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
				String stry = FormatString("y: %s", ItoS(it->vec3i->y));
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
				String strz = FormatString("z: %s", ItoS(it->vec3i->z));
				PushString(rg, writePos, strz, editorPanelFontSize, font);
			}
		}break;
		case Tweeker_EulerAngle:
		{
			PushRectangle(rg, valuePos, widthWithoutBoarder, height, editorPanelValueColor);

			Quaternion q = *it->q;
			EulerAngle angle = QuaternionToEulerAngle(q);
			angle.XRotation = angle.XRotation / PI * 360.0f;
			angle.YRotation = angle.YRotation / PI * 360.0f;
			angle.ZRotation = angle.ZRotation / PI * 360.0f;
			if (xyz == 'x' && (p->values.data + p->hotValue == it))
			{
				String str = FormatString("xRot: %s", p->textInput.string);
				PushString(rg, writePos, str, editorPanelFontSize, font);
			}
			else
			{
				String strx = FormatString("xRot: %s", FtoS(angle.XRotation));
				PushString(rg, writePos, strx, editorPanelFontSize, font);
			}
			writePos.y += editorPanelFontSize;
			if (xyz == 'y' && (p->values.data + p->hotValue == it))
			{
				String str = FormatString("yRot: %s", p->textInput.string);
				PushString(rg, writePos, str, editorPanelFontSize, font);
			}
			else
			{
				String stry = FormatString("yRot: %s", FtoS(angle.YRotation));
				PushString(rg, writePos, stry, editorPanelFontSize, font);
			}
			writePos.y += editorPanelFontSize;

			if (xyz == 'z' && (p->values.data + p->hotValue == it))
			{
				String str = FormatString("zRot: %s", p->textInput.string);
				PushString(rg, writePos, str, editorPanelFontSize, font);
			}
			else
			{
				String strz = FormatString("zRot: %s", FtoS(angle.ZRotation));
				PushString(rg, writePos, strz, editorPanelFontSize, font);
			}
		}break;
		case Tweeker_v4:
		{
			f32 colorRectSize = 3.0f * editorPanelFontSize;
			PushRectangle(rg, valuePos, widthWithoutBoarder, height, editorPanelValueColor);

			v4 color = *it->vec4;
			PushRectangle(rg, V2(valuePos.x + widthWithoutBoarder - colorRectSize - editorBorderWidth, valuePos.y + editorBorderWidth), colorRectSize, colorRectSize, color);
		}break;
		default:
		{
			Die;
		}break;
		}

		valuePos.y += height + editorBorderWidth;
	}

}

static void RenderEditor(RenderGroup *rg, AssetHandler *assetHandler, Editor editor, World *world, Input input)
{
	For(world->entities)
	{
		PushTriangleMesh(rg, it->meshId, it->orientation, GetRenderPos(*it), it->scale, it->color * it->frameColor);
	}

	switch (editor.state)
	{
	case EditorState_Moving:
	{
		if (!editor.snapToTileMap) break;

		For(editor.hotEntityInfos)
		{
			Entity *e = GetEntity(world, it->placedSerial);

			PushTriangleMesh(rg, e->meshId, e->orientation, V3(SnapToTileMap(GetRenderPos(*e))), e->scale, e->color * V4(0.5f, 1.0f, 1.0f, 1.0f));
		}

	}break;

	case EditorState_PlacingNewMesh:
	{
		v2 mouseP = ScreenZeroToOneToInGame(world->camera, input.mouseZeroToOne);
		v3i clickedTile = SnapToTileMap(i12(mouseP));
		v3 clickedOffset = V3(clickedTile) - V3(mouseP, 0.0f);

		For(editor.clipBoard)
		{
			v3i pos = it->physicalPos + clickedTile;
			v3 offset = editor.snapToTileMap ? V3() : (it->offset + clickedOffset);

			v3 rpos = V3(pos) + offset;
			PushTriangleMesh(rg, it->meshId, it->orientation, rpos, it->scale, it->color * V4(0.5f, 1.0f, 1.0f, 1.0f));
		}

	}break;
	}

	if (!editor.hotEntityInfos.amount) return;

	v3 averagePos = GetAveragePosForSelection(&editor, world);

	PushDebugPointCuboid(rg, V3(averagePos.xy, 0.0f));

	For(editor.hotEntityInfos)
	{
		Entity *e = GetEntity(world, it->placedSerial);
		PushDebugPointCuboid(rg, V3(e->physicalPos), V4(1, 0.3f, 0.6f, 0.1f));
		AABB transformedAABB = GetMesh(assetHandler, e->meshId)->aabb;

		transformedAABB.minDim *= e->scale;
		transformedAABB.maxDim *= e->scale;

		m4x4 mat = Translate(QuaternionToMatrix(e->orientation), GetRenderPos(*e));

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

}

static void RenderEditorUI(RenderGroup *rg, Editor editor, Font font)
{
	if (editor.state == EditorState_DeleteSelection)
	{
		PushString(rg, V2(0.4f, 0.1f), "Delete?", 0.05f, font);
	}

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

static void RenderEntityQuadTree(RenderGroup *rg, TileOctTreeNode *node)
{
	if (!node) return;

	AABB aabb = { V3(node->bound.minDim) - V3(0.5f, 0.5f, 0.5f), V3(node->bound.maxDim) - V3(0.5f, 0.5f, 0.5f) };

	v3 d1 = V3(aabb.maxDim.x - aabb.minDim.x, 0, 0);
	v3 d2 = V3(0, aabb.maxDim.y - aabb.minDim.y, 0);
	v3 d3 = V3(0, 0, aabb.maxDim.z - aabb.minDim.z);

	v3 p[8] =
	{
		aabb.minDim,			//0

		(aabb.minDim + d1),		//1
		(aabb.minDim + d2),		//2
		(aabb.minDim + d3),		//3

		(aabb.minDim + d1 + d2),	//4
		(aabb.minDim + d2 + d3),	//5
		(aabb.minDim + d3 + d1),	//6

		aabb.maxDim,			//7

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

	if (!node->isLeaf)
	{
		for (u32 i = 0; i < 8; i++)
		{
			RenderEntityQuadTree(rg, node->children[i]);
		}
	}
}

