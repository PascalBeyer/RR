#include "Game.h"	

#define pi32 3.14159265359f
#include "Random.h"
#include "Debug.h"
#include "World.h"

GameState InitGame(int screenWidth, int screenHeight, WorkHandler *workHandler)
{
	GameState ret = {};

	ret.workHandler = workHandler;	

	u16 newTileSize = 50;
	int tileMapHeight = 200;
	int tileMapWidth = 250;

	//tileMap = TileMap("map.tm");
	
	ret.tileMap = PushStruct(constantArena, TileMap);
	*ret.tileMap = InitTileMap(tileMapWidth, tileMapHeight, newTileSize, constantArena);
	ret.time = 0;
	ret.player = PushStruct(constantArena, Player);
	*ret.player = InitPlayer((u32)screenWidth, (u32)screenHeight, 5.0f, ret.tileMap, 0.1f);
	ret.assetHandler = new AssetHandler();
	ret.assetHandler->LoadAsset({ Asset_Building });
	ret.assetHandler->LoadAsset({ Asset_Font });
	ret.assetHandler->LoadAsset({ Asset_Unit });
	ret.assetHandler->LoadAsset({ Asset_TileMapTiles });

	ret.soundMixer = PushStruct(constantArena, SoundMixer); 
	*ret.soundMixer = SoundMixer(ret.assetHandler);
	//soundMixer.PlaySound({ Asset_Map }, 0);
	
	
	ret.particals = new Partical[100];

	for (u32 partIt = 0; partIt < 100; partIt++)
	{
		Partical *partical = ret.particals + partIt;
		partical->Color = V4(1, 0, 1, 0);
		partical->dColor = V4(-0.5f, 0, 0, 0);
		partical->vel = V2(1, 1);
		partical->pos = V2(15, 10);
	}


	ret.entities = new EntitySelection();
	ret.units = new UnitSelection();
	
	float expectedSecondsPerFrame = (1.0f / 60.0f);

	Unit *unit1 = new Unit(V2(25, 25), 4.0f, 4.0f, expectedSecondsPerFrame, 0.5f);
	Unit *unit2 = new Unit(V2(4, 8), 4.0f, 4.0f, expectedSecondsPerFrame, 0.5f);
	Unit *unit3 = new Unit(V2(7, 8), 4.0f, 4.0f, expectedSecondsPerFrame, 0.5f);
	Unit *unit4 = new Unit(V2(4, 6), 4.0f, 4.0f, expectedSecondsPerFrame, 0.5f);

	//unit1->way.push_back(v3(6.5f, 6.5f, 0.0f));
	//unit1->way.push_back(v3(4.5f, 6.5f, 0.0f));
	ret.entities->PushBack(unit1);
	ret.entities->PushBack(unit2);
	ret.entities->PushBack(unit3);
	ret.entities->PushBack(unit4);
	ret.units->PushBack(unit1);
	ret.units->PushBack(unit2);
	ret.units->PushBack(unit3);
	ret.units->PushBack(unit4);
	
	ret.player->entitySelection.PushBack(unit2);
	HandleRightClick(*ret.player, ret.entities, V2(45, 30), *ret.tileMap);

	ret.player->entitySelection.PushBack(unit3);	
	HandleRightClick(*ret.player, ret.entities, V2(25, 25), *ret.tileMap);

#if WriteMode
	
#else
	
	
#endif // WriteMode


	ret.debugUI = new UI();

	return ret;
}


void GameUpdateAndRender(GameState *gameState, RenderCommands *renderCommands, Input *input, SoundBuffer *soundBuffer)
{
	TileMap *tileMap = gameState->tileMap;
	Player *player = gameState->player;
	EntitySelection *entities = gameState->entities;
	UnitSelection *units = gameState->units;
	Screen *screen = player->screen;

	BEGIN_TIMED_BLOCK(GameUpdateAndRender);
	u16 tileSize = tileMap->tileSize;
	
	//Update(*gameState->player, input, *tileMap, gameState->entities);
	gameState->time += input->mouse->expectedTimePerFrame;
	
	RenderGroup renderGroup = InitRenderGroup(gameState->assetHandler, renderCommands);
	RenderGroup *rg = &renderGroup;

	f32 aspectRatio = (f32)screen->pixelWidth / (f32)screen->pixelHeight;

	Camera *cam = &screen->cam;

	ClearPushBuffer(rg);

	UpdateCamGodMode(input, cam);
	PushProjectivTransform(rg, cam->pos, cam->basis, aspectRatio, screen->focalLength);


#if 0

	DEBUG_ON_OFF(debugCamera, true)
	{
	}
	else
	{
		PushProjectivTransform(rg, screen->cam.pos, screen->cam.basis, aspectRatio, screen->focalLength);
	}

#endif // 0

	PushClear(rg, V4(1.0f, 0.1f, 0.1f, 0.1f));

	MoveUnits(units, *tileMap);

	v3 screenHeightVector = { 0, screen->height, 0};
	v3 screenWidthVector = {screen->width, 0, 0};

	v3 screenPos = screen->cam.pos;
	float screenWidth = screen->width;
	float screenHeight = screen->height;

	v2 screenUL = screen->ScreenToInGame(V2());
	v2 screenUR = screen->ScreenToInGame(V2(screen->pixelWidth, 0.0f));
	v2 screenBL = screen->ScreenToInGame(V2(0.0f, screen->pixelHeight));
	v2 screenBR = screen->ScreenToInGame(V2(screen->pixelWidth, screen->pixelHeight));

	float minScreenX = Min(Min(screenUL.x, screenUR.x), Min(screenBL.x, screenBR.x));
	float minScreenY = Min(Min(screenUL.y, screenUR.y), Min(screenBL.y, screenBR.y));

	float maxScreenX = Max(Max(screenUL.x, screenUR.x), Max(screenBL.x, screenBR.x));
	float maxScreenY = Max(Max(screenUL.y, screenUR.y), Max(screenBL.y, screenBR.y));

	int minScreenXi = (int)Max((int)floorf(minScreenX) - 1.0f, 0.0f);
	int minScreenYi = (int)Max((int)floorf(minScreenY) - 1.0f, 0.0f);

	int maxScreenXi = (int)Min((int)ceilf(maxScreenX) + 1.0f, (float)tileMap->width);
	int maxScreenYi = (int)Min((int)ceilf(maxScreenY) + 1.0f, (float)tileMap->height);

	DEBUG_ON_OFF(DrawWholeMap, false)
	{
		minScreenXi = 0;
		minScreenYi = 0;
		maxScreenXi = tileMap->width;
		maxScreenYi = tileMap->height;
	}

	/*
	for (u32 x = minScreenXi; x < maxScreenXi; x++)
	{
		for (u32 y = minScreenYi; y < maxScreenYi; y++)
		{
			Tile tile = *GetTile(*tileMap, x, y);
			//renderGroup.PushAsset(v3::Inclusion12(tile.pos), { Asset_TileMapTiles }, tile.type, v3StdBasis);
			//renderGroup.PushAsset(v3(x, y, randomSamples[(x*y + x + y) % 1000] - 0.01f), { Asset_Building }, 0, Scale(v3StdBasis, 1));
			renderGroup.PushCuboid(V3(x, y, -tile.height + 1), { {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, tile.height} }, V4(1.0f, 0.9f, 0.9f, 0.9f));
			
		}
	}
	*/
	
	v4 allColorsOfTheRainbow [] =
	{
		V4(1, 1, 1, 1), //white = 0
		V4(1, 1, 1, 0), //yellow = 1
		V4(1, 1, 0, 0), //red = 2
		V4(1, 1, 0, 1), //purple = 3
		V4(1, 0, 1, 1), //cyan = 4
		V4(1, 0, 0, 1), //blue = 5
		V4(1, 0, 1, 0), //green = 6
		V4(1, 0, 0, 0), // black  = 7


		V4(1, 1, 1, 0.5f),
		V4(1, 1, 0, 0.5f),
		V4(1, 1, 0.5f, 1),
		V4(1, 0.5f, 1, 1),
		V4(1, 0.5f, 0.5f, 1),
		V4(1, 0.5f, 1, 0.5f),
		V4(1, 0.5f, 0.5f, 0.5f)

	};
	for (u32 iter = 0; iter < 1; iter++)
	{
		AABB aabb = { V3((f32)(iter * 20) - 4.0f, -4, 0), V3(iter * 20 + 4, 4, 8) };

		TriangleArray tArray = CreateStoneAndPush(aabb, 5.0f, frameArena, iter);
		Triangle *t = tArray.arr;
		u32 amountOfTrianglesInStrone = tArray.amount;
		for (u32 triI = 0; triI < amountOfTrianglesInStrone; triI++)
		{
			f32 shadeOfGray = (aabb.maxDim.x - t[triI].p2.x) / (aabb.maxDim.x - aabb.minDim.x);
			PushTriangle(rg, t[triI].p1, t[triI].p2, t[triI].p3, shadeOfGray * V3(1, 1, 1));
		}

		for (u32 i = 0; i < 8; i++)
		{
			PushCuboid(rg, AABBCorner(aabb, i), 0.1f * v3StdBasis, allColorsOfTheRainbow[i]);
		}
	}


#if 0
	for (u32 i = 0; i < entities->amountSelected; i++)
	{
		Entity *entityPtr = entities->Get(i);

		Unit *unitPtr;
		unitPtr = dynamic_cast<Unit*> (entityPtr);
		if (unitPtr)
		{
			//TODO: Get Entities in reachable region
			//unitPtr->Update(&entities, &tileMap);

			renderGroup.PushUnit(v3(unitPtr->GetPos(), -tileMap.GetHeight(unitPtr->GetPos())) + 1, unitPtr->facingDirection, Asset_Unit, unitPtr->GetSize());
			renderGroup.PushDebugString(i12(unitPtr->GetPos() + v2(-0.1f, -0.1f)), String(unitPtr->facingDirection), 1);
			if (!unitPtr->way.empty())
			{
				v2 wayVector = unitPtr->pos;
				for (int i = unitPtr->way.size() - 1; i >= 0; i--)
				{
					v2 toVector = unitPtr->way.at(i);
					//renderGroup.PushLine(Shape::Line(wayVector, toVector)); 
					wayVector = toVector;
				}
			}
			if (unitPtr->idle)
			{
				//renderGroup.PushRectangle(Shape::Rectangle(unitPtr->pos, 0.1f, 0.1f, v4(1.0f, 0.5f, 0, 0.5f)), 0.0f);
			}
				
		}

		Building *buildingPtr;
		buildingPtr = dynamic_cast<Building*> (entityPtr);
		if (buildingPtr)
		{
			buildingPtr->Update(tileSize, entities);
			renderGroup.PushBuilding(i12(buildingPtr->GetPos()), Asset_Building, buildingPtr->GetSize());
			
		}
	}
#endif
	//renderGroup.PushUnit(v3(unit1->pos, -tileMap.GetHeight(unit1->GetPos()) + 1), unit1->facingDirection, Asset_Unit, unit1->radius);

	float unitBoxSize = 0.2f;

	v3 camV1 = screen->cam.basis.d1;
	v3 camV2 = screen->cam.basis.d2; 
	v3 camV3 = screen->cam.basis.d3;

	DEBUG_ON_OFF(DrawCamera, false)
	{

		float camBoxSize = 0.4f;
		PushCuboid(rg, screen->cam.pos - camBoxSize * V3(0.5f, 0.5f, 0.0f), Scale(screen->cam.basis, camBoxSize), V4(1, 0, 1, 0));

		v3 camPos = screen->cam.pos;

		v3 camRectUL = screen->cam.pos + screen->cam.basis.d3 * screen->focalLength * 2.5f -0.5f * (screenWidth * screen->cam.basis.d1 + screenHeight * screen->cam.basis.d2);
		PushRectangle(rg, camRectUL, screen->cam.basis.d1 * screenWidth, screen->cam.basis.d2 * screenHeight, V4(0.5f, 1.0f, 0.0f, 0.0f));
	
		PushQuadrilateral(rg, V3(screen->ScreenToInGame(V2()), 0), V3(screen->ScreenToInGame(V2(screen->pixelWidth, 0.0f)), 0), V3(screen->ScreenToInGame(V2(0.0f, screen->pixelHeight)), 0), V3(screen->ScreenToInGame(V2(screen->pixelWidth, screen->pixelHeight)), 0), V4(0.5f, 0.0f, 1.0f, 0.0f));

		v2 adjustedMarkingPos = player->markingRect.pos - 0.5f *player->markingRect.dim;
		v2 markingRectV1 = { player->markingRect.dim.x, 0 };
		v2 markingRectV2 = { 0, player->markingRect.dim.y };
		PushQuadrilateral(rg, V3(screen->ScreenToInGame(adjustedMarkingPos), 0), V3(screen->ScreenToInGame(adjustedMarkingPos + markingRectV1), 0), V3(screen->ScreenToInGame(adjustedMarkingPos + markingRectV2), 0), V3(screen->ScreenToInGame(adjustedMarkingPos + markingRectV1 +markingRectV2), 0), V4(0.8f, 0.0f, 1.0f, 0.0f));

	}

	PushOrthogonalTransform(rg, (float)screen->pixelWidth, (float)screen->pixelHeight);
	//PushRectangle(rg, player->markingRect, 0.0f); todo : redo this.


	gameState->soundMixer->ToOutput(soundBuffer);

	END_TIMED_BLOCK(GameUpdateAndRender)
}