#ifndef RR_GAME
#define RR_GAME

#include "BasicTypes.h"

#include "buffers.h"
#include "Arena.h"
#include "WorkHandler.h"
#include "Math.h"
#include "Color.h"
#include "File.h"
#include "Matrix.h"

#include "LinearAlgebra.h"

#include "SIMD.h"
#include "Random.h"
#include "Input.h"
#include "Intrinsics.h"
#include "TileMap.h"
#include "Bitmap.h"
#include "String.h"
#include "Sound.h"
#include "Fonts.h"
#include "World.h"

#include "Screen.h"
#include "Animation.h"
#include "Button.h"
#include "KdTree.h"
#include "Entity.h"

#include "AssetHandler.h"

#include "UI.h"
#include "Debug.h"
#include "SoundMixer.h"
#include "Renderer.h"
#include "Unit.h"

#include "Lighting.h"
#include "EntitySelection.h"

#include "Building.h"

#include "Player.h"

#include "MovementHandler.h"

#include "Console.h"

struct Partical
{
	v2 pos;
	v2 vel;
	v4 Color;
	v4 dColor;
};

struct GameState
{
	EntitySelection *entities;
	UnitSelection *units;
	TileMap *tileMap;
	World *world;

	Screen *screen;
	Player *player;	
	WorkHandler *workHandler;
	AssetHandler *assetHandler;
	SoundMixer *soundMixer;
	
	Font font;

	Console console;

	UI *debugUI;
};

#define pi32 3.14159265359f

static GameState InitGame(int screenWidth, int screenHeight, WorkHandler *workHandler)
{
	GameState ret = {};

	ret.workHandler = workHandler;

	u16 newTileSize = 50;
	int tileMapHeight = 200;
	int tileMapWidth = 250;

	//tileMap = TileMap("map.tm");

	ret.console.history = PushArray(constantArena, String, 300);
	ret.console.historyMaxLength = 300;

	ret.font = LoadFont("consola.ttf");

	ret.tileMap = PushStruct(constantArena, TileMap);
	*ret.tileMap = InitTileMap(tileMapWidth, tileMapHeight, newTileSize, constantArena);
	ret.player = PushStruct(constantArena, Player);
	*ret.player = InitPlayer((u32)screenWidth, (u32)screenHeight, 5.0f, ret.tileMap, 0.1f);
	ret.assetHandler = new AssetHandler();
	ret.assetHandler->LoadAsset({ Asset_Building });
	ret.assetHandler->LoadAsset({ Asset_Unit });
	ret.assetHandler->LoadAsset({ Asset_TileMapTiles });

	ret.soundMixer = PushStruct(constantArena, SoundMixer);
	*ret.soundMixer = SoundMixer(ret.assetHandler);
	//soundMixer.PlaySound({ Asset_Map }, 0);

	AABB stoneaabb = { V3(0, 0, -4), V3(1, 1, -2) };
	Triangle *t = PushArray(constantArena, Triangle, 0);
	u32 amountOfTriangles = 0;

	s32 skyRadius = 40;
	amountOfTriangles += CreateSkyBoxAndPush({ V3(-skyRadius, -skyRadius, -skyRadius), V3(skyRadius, skyRadius, skyRadius) }, RGBAfromHEX(0x87CEEB), constantArena).amount;
	amountOfTriangles += CreateStoneAndPush(stoneaabb, 5.0f, constantArena, 0).amount;

	AABB groundaabb = { V3(-20, -20, -0.25f), V3(20, 20, 0) };
	amountOfTriangles += GenerateAndPushTriangleFloor(groundaabb, constantArena).amount;

	//PushTriangleToArenaIntendedNormal(constantArena, V3(0, -10, 0), V3(10, 10, 0), V3(-10, 10, 0), 0xFFFFFFFF, V3(0, 0, -1));
	//amountOfTriangles += 1;

	World *world = PushZeroStruct(constantArena, World);
	ret.world = world;
	world->lightSource = V3(20, 0, -20);
	world->triangles.arr = t;
	world->triangles.amount = amountOfTriangles;
	world->camera.pos = V3(0, 0, -2.5f);
	world->camera.basis = v3StdBasis;

	world->debugCamera.pos = V3(0, 0, -10);
	world->debugCamera.basis = v3StdBasis;

	//InitLighting(world);

	ret.screen = PushStruct(constantArena, Screen);
	*ret.screen = CreateScreen((f32)screenWidth, (f32)screenHeight, (float)ret.tileMap->width, (float)ret.tileMap->height, 10, 1);

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
	HandleRightClick(ret.player, ret.entities, V2(45, 30), *ret.tileMap);

	ret.player->entitySelection.PushBack(unit3);
	HandleRightClick(ret.player, ret.entities, V2(25, 25), *ret.tileMap);

	ret.debugUI = new UI();

	return ret;
}

static void GameUpdateAndRender(GameState *gameState, RenderCommands *renderCommands, Input *input, SoundBuffer *soundBuffer)
{
	TileMap *tileMap = gameState->tileMap;
	Player *player = gameState->player;
	EntitySelection *entities = gameState->entities;
	UnitSelection *units = gameState->units;
	Screen *screen = gameState->screen;

	BEGIN_TIMED_BLOCK(GameUpdateAndRender);
	u16 tileSize = tileMap->tileSize;

	//Update(*gameState->player, input, *tileMap, gameState->entities);

	RenderGroup renderGroup = InitRenderGroup(gameState->assetHandler, renderCommands);
	RenderGroup *rg = &renderGroup;
	ClearPushBuffer(rg);

	f32 aspectRatio = (f32)renderCommands->width / (f32)renderCommands->height;
	f32 width = (f32)renderCommands->width;
	f32 height = (f32)renderCommands->height;
	f32 focalLength = renderCommands->focalLength;

	World *world = gameState->world;

	static bool debugCam = false;


	if (input->keybord.z.pressedThisFrame)
	{

		if (input->keybord.shift.isDown)
		{
			world->debugCamera = world->camera;
		}
		else
		{
			debugCam = !debugCam;
		}
	}

	Camera *cam = debugCam ? &world->debugCamera : &world->camera;

	UpdateCamGodMode(input, cam);
	PushRenderSetUp(rg, *cam, world->lightSource, (Setup_Projective | Setup_ShadowMapping));

	PushClear(rg, V4(1.0f, 0.1f, 0.1f, 0.1f));

	//MoveUnits(units, *tileMap);
	float screenWidth = (f32)renderCommands->width;
	float screenHeight = (f32)renderCommands->height;

	v3 screenHeightVector = { 0, screenHeight, 0 };
	v3 screenWidthVector = { screenWidth, 0, 0 };

	v3 screenPos = cam->pos;

	v2 screenUL = ScreenZeroToOneToInGame(*cam, V2(), aspectRatio, focalLength);
	v2 screenUR = ScreenZeroToOneToInGame(*cam, V2(width, 0.0f), aspectRatio, focalLength);
	v2 screenBL = ScreenZeroToOneToInGame(*cam, V2(0.0f, height), aspectRatio, focalLength);
	v2 screenBR = ScreenZeroToOneToInGame(*cam, V2(width, height), aspectRatio, focalLength);

	float minScreenX = Min(Min(screenUL.x, screenUR.x), Min(screenBL.x, screenBR.x));
	float minScreenY = Min(Min(screenUL.y, screenUR.y), Min(screenBL.y, screenBR.y));

	float maxScreenX = Max(Max(screenUL.x, screenUR.x), Max(screenBL.x, screenBR.x));
	float maxScreenY = Max(Max(screenUL.y, screenUR.y), Max(screenBL.y, screenBR.y));

	int minScreenXi = (int)Max((int)floorf(minScreenX) - 1.0f, 0.0f);
	int minScreenYi = (int)Max((int)floorf(minScreenY) - 1.0f, 0.0f);

	int maxScreenXi = (int)Min((int)ceilf(maxScreenX) + 1.0f, (float)tileMap->width);
	int maxScreenYi = (int)Min((int)ceilf(maxScreenY) + 1.0f, (float)tileMap->height);

	//DEBUG_ON_OFF(DrawWholeMap, false)
	//{
	//	minScreenXi = 0;
	//	minScreenYi = 0;
	//	maxScreenXi = tileMap->width;
	//	maxScreenYi = tileMap->height;
	//}

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

	v4 allColorsOfTheRainbow[] =
	{
		V4(1, 1, 1, 1), //white = 0
		V4(1, 1, 1, 0), //yellow = 1
		V4(1, 1, 0, 0), //red = 2
		V4(1, 1, 0, 1), //purple = 3
		V4(1, 0, 1, 1), //cyan = 4
		V4(1, 0, 0, 1), //blue = 5
		V4(1, 0, 1, 0), //green = 6
		V4(1, 0, 0, 0), //black  = 7

		V4(1, 1, 1, 0.5f),
		V4(1, 1, 0, 0.5f),
		V4(1, 1, 0.5f, 1),
		V4(1, 0.5f, 1, 1),
		V4(1, 0.5f, 0.5f, 1),
		V4(1, 0.5f, 1, 0.5f),
		V4(1, 0.5f, 0.5f, 0.5f)

	};


	static KdNode *selectedLeaf;
	static v2 selectedPoint;
#if 0
	if (input->mouse.leftButtonPressedThisFrame)
	{
		v2 clickedPoint = ScreenZeroToOneToInGame(*cam, input->mouseZeroToOne, aspectRatio, focalLength);
		KdNode* clickedLeaf = GetLeaf(world->kdTree, i12(clickedPoint));
		if (selectedLeaf && selectedLeaf != clickedLeaf)
		{
			LightingTriangle **selectedTs = selectedLeaf->triangles;
			for (u32 i = 0; i < selectedLeaf->amountOfTriangles; i++)
			{
				selectedTs[i]->color = 0.5f * (selectedTs[i]->normal + V3(1, 1, 1));
			}

			LightingTriangle **clickedTs = clickedLeaf->triangles;
			for (u32 i = 0; i < clickedLeaf->amountOfTriangles; i++)
			{
				clickedTs[i]->color = V3(0, 0, 0);
			}
		}
		selectedLeaf = clickedLeaf;
		selectedPoint = clickedPoint;

	}
	if (selectedLeaf)
	{
		PushAABBOutLine(rg, selectedLeaf->aabb.minDim, selectedLeaf->aabb.maxDim);
		PushDebugPointCuboid(rg, i12(selectedPoint));
	}
#endif

	static bool viewLighting = false;
	if (input->keybord.l.pressedThisFrame || input->keybord.o.pressedThisFrame)
	{
		viewLighting = !viewLighting;
	}
	if (viewLighting)
	{
		if (input->keybord.o.pressedThisFrame)
		{
			PushLightingSolution(rg, gameState->world);
		}
		else
		{
			PushLightingImage(rg);
		}

	}
	else
	{
		Triangle *t = world->triangles.arr;
		for (u32 triI = 0; triI < world->triangles.amount; triI++)
		{
			PushTriangle(rg, t[triI].cv1, t[triI].cv2, t[triI].cv3);
		}
#if 0
		for (u32 triI = 0; triI < 40; triI++)
		{
			Triangle sd = t[triI];
			v3 pos = 0.3333f * (sd.p1 + sd.p2 + sd.p3);
			//PushCuboid(rg, pos, 0.3f * v3StdBasis, V4(1, 1, 0, 0));
			PushLine(rg, pos, pos + sd.normal);
		}
#endif
	}

	//PushUnit(rg, V3(0, 0, 0), 0, Asset_Unit, 1.0f);
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

	cam = &world->camera;

	v3 camV1 = cam->basis.d1;
	v3 camV2 = cam->basis.d2;
	v3 camV3 = cam->basis.d3;

#if 0
	DEBUG_ON_OFF(DrawCamera, false)
	{
		float camBoxSize = 0.4f;
		PushCuboid(rg, cam->pos - camBoxSize * V3(0.5f, 0.5f, 0.0f), Scale(cam->basis, camBoxSize), V4(1, 0, 1, 0));

		v3 camPos = cam->pos;

		m4x4 proj = Projection(aspectRatio, focalLength) * CameraTransform(camV1, camV2, camV3, camPos);
		m4x4 inv = InvOrId(proj);
		m4x4 id = proj * inv;
		v3 p1 = inv * V3(-1, -1, -1);
		v3 p2 = inv * V3(1, -1, -1);
		v3 p3 = inv * V3(-1, 1, -1);
		v3 p4 = inv * V3(1, 1, -1);
		PushQuadrilateral(rg, p1, p2, p3, p4, V4(0.5f, 1.0f, 0.0f, 0.0f));

		v3 pp1 = i12(ScreenZeroToOneToInGame(*cam, V2(), aspectRatio, focalLength));
		v3 pp2 = i12(ScreenZeroToOneToInGame(*cam, V2(1.0f, 0.0f), aspectRatio, focalLength));
		v3 pp3 = i12(ScreenZeroToOneToInGame(*cam, V2(0.0f, 1.0f), aspectRatio, focalLength));
		v3 pp4 = i12(ScreenZeroToOneToInGame(*cam, V2(1.0f, 1.0f), aspectRatio, focalLength));
		PushQuadrilateral(rg, pp1, pp2, pp3, pp4, V4(0.5f, 0.0f, 1.0f, 0.0f));

		v2 markingRectDim = player->markingRect.max - player->markingRect.min;
		v2 adjustedMarkingPos = player->markingRect.min - 0.5f * markingRectDim;
		v2 markingRectV1 = { markingRectDim.x, 0 };
		v2 markingRectV2 = { 0, markingRectDim.y };
		//Die; // project marking pos here
	}
#endif

	PushRenderSetUp(rg, *cam, world->lightSource, Setup_ZeroToOne);
	//PushRectangle(rg, player->markingRect, 0.0f); todo : redo this.

	UpdateConsole(&gameState->console, input);
	DrawConsole(rg, gameState->console);

	gameState->soundMixer->ToOutput(soundBuffer);

	END_TIMED_BLOCK(GameUpdateAndRender)
}
#endif