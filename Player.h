#ifndef RR_PLAYER
#define RR_PLAYER

struct MarkingRect
{
	bool mouseCreatesBox;
	v2 savedMousePos;
	v2 min;
	v2 max;
};

struct Player
{
	MarkingRect markingRect;

	u8 playerIndex;
	u32 minerals;
	u32 gas;

	u32 steeringGroupingIndex;
};

Player InitPlayer(u32 screenPixelWidth, u32 screenPixelHeight, float screenWidth, TileMap *tileMap, float screenScrollSpeed);

#if 0
void UpdateMarkingRect(MarkingRect *rect, EntitySelection *entitySelection, Input *input, Screen * screen, EntitySelection *entitys)
{
	v2 mousePos = input->mousePos;
	if (!rect->mouseCreatesBox)
	{
		if (input->mouse->leftButtonDown)
		{
			rect->mouseCreatesBox = true;
			rect->savedMousePos = mousePos;
		}
		return;
	}

	rect->min = Min(rect->savedMousePos, mousePos);
	rect->max = Max(rect->savedMousePos, mousePos);

	if (!input->mouse->leftButtonDown)
	{
		rect->mouseCreatesBox = false;
		entitySelection->Clear();

		v3 camPos = cam.pos;
		float screenWidth = screen->width;
		float screenHeight = screen->height;

		v2 dim = rect->max - rect->min;
		v2 adjustedPos = rect->min - dim * 0.5f;

		float posXinGame = adjustedPos.x / (float)screen->pixelWidth * screenWidth;
		float posYinGame = adjustedPos.y / (float)screen->pixelHeight * screenHeight;

		float width = dim.x / (float)screen->pixelWidth * screenWidth;
		float height = dim.y / (float)screen->pixelHeight * screenHeight;

		v3 camRectUL = screen->cam.pos + screen->cam.basis.d3 * screen->focalLength * 2.5f - 0.5f * (screenWidth * screen->cam.basis.d1 + screenHeight * screen->cam.basis.d2);

		v3 inGameRectUL = camRectUL + posXinGame * screen->cam.basis.d1 + posYinGame * screen->cam.basis.d2;
		v3 inGameRectUR = inGameRectUL + screen->cam.basis.d1 * width;
		v3 inGameRectBL = inGameRectUL + screen->cam.basis.d2 * height;
		v3 inGameRectBR = inGameRectUL + screen->cam.basis.d1 * width + screen->cam.basis.d2 * height;

		v3 inGamePos = inGameRectUL + 0.5f * (screen->cam.basis.d1 * width + screen->cam.basis.d2 * height);

		float ctUL = -camPos.z / (inGameRectUL.z - camPos.z);
		float ctUR = -camPos.z / (inGameRectUR.z - camPos.z);
		float ctBL = -camPos.z / (inGameRectBL.z - camPos.z);
		float ctBR = -camPos.z / (inGameRectBR.z - camPos.z);
		float ctPos = -camPos.z / (inGamePos.z - camPos.z);

		v2 projRectUL = p12(ctUL * (inGameRectUL - camPos) + camPos);
		v2 projRectUR = p12(ctUR * (inGameRectUR - camPos) + camPos);
		v2 projRectBL = p12(ctBL * (inGameRectBL - camPos) + camPos);
		v2 projRectBR = p12(ctBR * (inGameRectBR - camPos) + camPos);
		v2 projRectPos = p12(ctPos * (inGamePos - camPos) + camPos);

		v2 ULUR = projRectUL - projRectUR;
		v2 PULUR = PerpendicularVector(ULUR);

		v2 ULBL = projRectUL - projRectBL;
		v2 PULBL = PerpendicularVector(ULBL);

		v2 BRUR = projRectBR - projRectUR;
		v2 PBRUR = PerpendicularVector(BRUR);

		v2 BRBL = projRectBR - projRectBL;
		v2 PBRBL = PerpendicularVector(BRBL);

		for (int i = 0; i< entitys->amountSelected; i++)
		{
			Entity *entity = entitys->Get(i);

			v2 unitPos = entity->GetPos();

			float up = Dot(PULUR, unitPos - projRectUR);
			float down = Dot(PBRBL, unitPos - projRectBL);
			float left = Dot(PULBL, unitPos - projRectBL);
			float right = Dot(PBRUR, unitPos - projRectUR);

			if (up < 0.0f && down < 0.0f && left > 0.0f && right > 0.0f)
			{
				entitySelection->PushBack(entity);
			}
		}
		rect->min = V2();
		rect->max = V2();
	}
}
#endif


Player InitPlayer(u32 screenPixelWidth, u32 screenPixelHeight, float screenTileWidth, TileMap *tileMap, float screenScrollSpeed)
{
	Player ret;
	ret.steeringGroupingIndex = 0;
	float focalLength = 1.0f;
	float screenScrollAreaWidth = 20.0f;
	ret.markingRect = {};
	return ret;
}


v2 AdjustTargetPos(v2 targetPos, TileMap tileMap, v2 unitPos)
{
	Tile *targetedTile = GetTile(tileMap, targetPos);

	//todo handle positions out of tilemap

	if (targetedTile->type == 0)
	{
		return targetPos;
	}
	else
	{
		Tile *newTarget = GetFirstTileWithType(tileMap, targetPos, unitPos, 0);

		if (newTarget)
		{
			return newTarget->pos + V2(0.5f, 0.5f);
		}
		else
		{
			return unitPos;
		}

	}
}

#if 0
void HandleRightClick(Player *player, EntitySelection *entities, v2 targetPos, TileMap tileMap)
{
	v2 centerOfMass = v2();
	float xMax = NAN, yMax = NAN, xMin = NAN, yMin = NAN;
	for (u32 i = 0; i < player->entitySelection.amountSelected; i++)
	{
		v2 unitPos = player->entitySelection.Get(i)->pos;
		centerOfMass += unitPos;
		xMax = Max(xMax, unitPos.x);
		yMax = Max(yMax, unitPos.y);
		xMin = Min(xMin, unitPos.x);
		yMin = Min(yMin, unitPos.y);
	}
	centerOfMass /= player->entitySelection.amountSelected;
	if (++player->steeringGroupingIndex == 0)
	{
		player->steeringGroupingIndex++;
	}

	for (u32 i = 0; i < player->entitySelection.amountSelected; i++)
	{
		Entity *entity = player->entitySelection.Get(i);

		switch (entity->type)
		{
		case Entity_Unit:
		{
			Unit *unit = (Unit *)entity;

			Clear(&unit->way);
			unit->idle = false;
			unit->steeringGroupId = player->steeringGroupingIndex;

			u32 boxConstant = 3;

			bool unitsInAllignedMoveBox = ((xMax - xMin) <= boxConstant) && ((yMax - yMin) <= boxConstant);
			bool targetPointInBox = ((xMax >= targetPos.x) && (targetPos.x >= xMin) && (yMax >= targetPos.y) && (targetPos.y >= yMin));

			if (unitsInAllignedMoveBox && !targetPointInBox)
			{
				v2 unitTarget = (unit->e.pos - centerOfMass) + targetPos;

				v2 adjustedCenterOfMass = AdjustTargetPos(centerOfMass, tileMap, unit->e.pos);
				ArrayAdd(&unit->way, AdjustTargetPos(unitTarget, tileMap, adjustedCenterOfMass));
			}
			else
			{
				v2 unitTarget = targetPos;

				ArrayAdd(&unit->way, AdjustTargetPos(unitTarget, tileMap, unit->e.pos));
			}

		}break;
		case Entity_Building:
		{
			Building *building = (Building *)entity;
			building->producing = true;
		}break;
		}
	}
}
void Update(Player *player, Input *input, TileMap tileMap, EntitySelection *entities, Camera cam)
{
	Die;
	//UpdateMarkingRect(&player->markingRect, &player->entitySelection, input, entities);
//	if (input->mouse.rightButtonPressedThisFrame)
	{
		//HandleRightClick(player, entities, ScreenToInGame(cam, input->mousePos, ), tileMap);
	}
}

#endif


#endif