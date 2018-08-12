#include "Player.h"

#include "Unit.h"
#include "Building.h"

Player InitPlayer(u32 screenPixelWidth, u32 screenPixelHeight, float screenTileWidth, TileMap *tileMap, float screenScrollSpeed)
{
	Player ret;
	ret.entitySelection = EntitySelection();
	ret.steeringGroupingIndex = 0;
	float focalLength = 1.0f;
	float screenScrollAreaWidth = 20.0f;
	ret.screen = new Screen(0, 0, -7, screenPixelWidth, screenPixelHeight, screenTileWidth, (float)tileMap->width, (float)tileMap->height, screenScrollSpeed, (float)tileMap->tileSize, screenScrollAreaWidth, focalLength);
	ret.markingRect = MarkingRect();
	return ret;
}


v2 AdjustTargetPos(v2 targetPos, TileMap tileMap, v2 unitPos)
{
	Tile *targetedTile = GetTile(tileMap, targetPos);

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

void HandleRightClick(Player player, EntitySelection *entities, v2 targetPos, TileMap tileMap)
{
	v2 centerOfMass = v2();
	float xMax = NAN, yMax = NAN, xMin = NAN, yMin = NAN;
	for (u32 i = 0; i < player.entitySelection.amountSelected; i++)
	{
		v2 unitPos = player.entitySelection.Get(i)->pos;
		centerOfMass += unitPos;
		xMax = fmax(xMax, unitPos.x);
		yMax = fmax(yMax, unitPos.y);
		xMin = fmin(xMin, unitPos.x);
		yMin = fmin(yMin, unitPos.y);
	}
	centerOfMass /= player.entitySelection.amountSelected;
	if (++player.steeringGroupingIndex == 0)
	{
		player.steeringGroupingIndex++;
	}

	for (u32 i = 0; i < player.entitySelection.amountSelected; i++)
	{
		Entity *entity = player.entitySelection.Get(i);

		Unit *unit;
		Building *building;
		if (unit = dynamic_cast<Unit*> (entity))
		{
			
			unit->way.clear();
			unit->idle = false;
			unit->steeringGroupId = player.steeringGroupingIndex;
			
			u32 boxConstant = 3;

			bool unitsInAllignedMoveBox = ((xMax - xMin) <= boxConstant) && ((yMax - yMin) <= boxConstant);
			bool targetPointInBox = ((xMax >= targetPos.x ) && (targetPos.x >= xMin) && (yMax >= targetPos.y) && (targetPos.y >= yMin));

			if (unitsInAllignedMoveBox && !targetPointInBox) 
			{
				v2 unitTarget = (unit->pos - centerOfMass) + targetPos;

				v2 adjustedCenterOfMass = AdjustTargetPos(centerOfMass, tileMap, unit->pos);
				unit->way.push_back(AdjustTargetPos(unitTarget, tileMap, adjustedCenterOfMass));
			}
			else
			{
				v2 unitTarget = targetPos;

				unit->way.push_back(AdjustTargetPos(unitTarget, tileMap, unit->pos));				
			}
			
			
		}
		else if (building = dynamic_cast<Building*> (entity))
		{
			building->producing = true;
		}
	}
}
void Update(Player player, Input *input, TileMap tileMap, EntitySelection *entities)
{
	player.screen->Update(input->mousePos);
	player.markingRect.Update(&player.entitySelection, input, player.screen, entities);
	if (input->mouse->rightButtonPressedThisFrame)
	{		
		HandleRightClick(player, entities, player.screen->ScreenToInGame(input->mousePos), tileMap);
	}
}
