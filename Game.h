#ifndef RR_GAME
#define RR_GAME

#include "BasicTypes.h"

#include "buffers.h"

#include "Intrinsics.h"
#include "Math.h"

#include "Arena.h"
#include "String.h"

#include "Array.h"
#include "Debug.h"

#include "WorkHandler.h"
#include "Color.h"
#include "File.h"

#include "LinearAlgebra.h"

#include "SIMD.h"
#include "Random.h"
#include "Input.h"

#include "Bitmap.h"
#include "Fonts.h"
#include "AssetHandler.h"

#include "World.h"

#include "TileMap.h"

#include "ConsoleHeader.h"
#include "Screen.h"
#include "Animation.h"
#include "Button.h"
#include "KdTree.h"

#include "SoundMixer.h"
#include "Renderer.h"

#include "Entity.h"

#include "Lighting.h"

#include "Player.h"

//#include "MovementHandler.h"
#include "Editor.h" // todo rank this once we know more.


struct Partical
{
	v2 pos;
	v2 vel;
	v4 Color;
	v4 dColor;
};

struct GameState
{
	TileMap *tileMap;
	World *world;

	Screen *screen;
	Player *player;	
	WorkHandler *workHandler;
	AssetHandler *assetHandler;
	UnitHandler unitHandler;
	SoundMixer *soundMixer;
	Editor editor;

	TriangleMesh testMesh;

	Font font;

	DEBUGKeyTracker keyTracker;
};

//debug stuff, has to be after gamestate for now, as console commands have to alter the gamestate
#include "ConsoleCommands.h"
#include "Console.h"
#include "DebugBody.h"

static bool viewLighting = false;

static void UpdateDEBUGKeyTracker(DEBUGKeyTracker *tracker, KeyStateMessage message)
{
	switch (message.key)
	{
	case Key_a:
	{
		tracker->aDown = message.flag & KeyState_Down;
	}break;
	case Key_s:
	{
		tracker->sDown = message.flag & KeyState_Down;
	}break;
	case Key_d:
	{
		tracker->dDown = message.flag & KeyState_Down;
	}break;
	case Key_w:
	{
		tracker->wDown = message.flag & KeyState_Down;
	}break;
	case Key_space:
	{
		tracker->spaceDown = message.flag & KeyState_Down;
	}break;
	}
}

static GameState InitGame(int screenWidth, int screenHeight, WorkHandler *workHandler)
{
	GameState ret = {};
	ret.workHandler = workHandler;

	int tileMapHeight = 10;
	int tileMapWidth = 10;

	ret.font = LoadFont("consola.ttf");

	globalFont = ret.font;

	InitConsole(ret.font);
	
	ret.assetHandler = PushStruct(constantArena, AssetHandler);
	ret.assetHandler = CreateAssetHandler(constantArena);

	ret.tileMap = PushStruct(constantArena, TileMap);
	*ret.tileMap = InitTileMap(ret.assetHandler, tileMapWidth, tileMapHeight, constantArena);

	ret.player = PushStruct(constantArena, Player);
	*ret.player = InitPlayer((u32)screenWidth, (u32)screenHeight, 5.0f, ret.tileMap, 0.1f);

	ret.soundMixer = PushStruct(constantArena, SoundMixer);
	*ret.soundMixer = {};

	i32 skyRadius = 40;
	TriangleArray t = CreateSkyBoxAndPush({ V3(-skyRadius, -skyRadius, -skyRadius), V3(skyRadius, skyRadius, skyRadius) }, RGBAfromHEX(0x87CEEB), constantArena);

	f32 pifac = 6.0f * 0.25f * pi32;
	Quaternion standartRotation = QuaternionFromAngleAxis(pifac, V3(1, 0, 0));
	AABB groundaabb = { V3(-20, -20, -0.25f), V3(20, 20, 0) };

	World *world = PushZeroStruct(constantArena, World);
	ret.world = world;
	world->lightSource = V3(20, 0, -20);
	world->triangles = t;
	world->camera.pos = V3(0, -1.3f, -2.5f);
	Quaternion r = QuaternionFromAngleAxis(-0.1f, V3(0, 1, 0));
	world->camera.basis = TransformBasis(v3StdBasis, QuaternionToMatrix(r));

	world->debugCamera.pos = V3(0, 0, -10);
	world->debugCamera.basis = v3StdBasis;
	world->placedMeshes = PlacedMeshCreateDynamicArray();

	TriangleMesh treeMesh = LoadMesh(ret.assetHandler, "obj/tree.mesh");

	ArrayAdd(&world->placedMeshes, CreatePlacedMesh(treeMesh, 0.1f, standartRotation, V3(5, 1, 0)));
	//ArrayAdd(&world->placedMeshes, CreatePlacedMesh(treeMesh, 0.1f, standartRotation, V3(5, 6, 0)));
	//ArrayAdd(&world->placedMeshes, CreatePlacedMesh(treeMesh, 0.1f, standartRotation, V3(3, 1, 0)));
	//ArrayAdd(&world->placedMeshes, CreatePlacedMesh(treeMesh, 0.1f, standartRotation, V3(5, 2, 0)));
	//ArrayAdd(&world->placedMeshes, CreatePlacedMesh(treeMesh, 0.1f, standartRotation, V3(2, 1, 0)));
	//ArrayAdd(&world->placedMeshes, CreatePlacedMesh(treeMesh, 0.1f, standartRotation, V3(5, 4, 0)));
	//ArrayAdd(&world->placedMeshes, CreatePlacedMesh(treeMesh, 0.1f, standartRotation, V3(0, 10, 0)));


	ret.screen = PushStruct(constantArena, Screen);
	*ret.screen = CreateScreen((f32)screenWidth, (f32)screenHeight, (float)ret.tileMap->width, (float)ret.tileMap->height, 10, 1);

	ret.editor.elements = EditorUIElementCreateDynamicArray();


	ret.unitHandler = CreateUnitHandler(constantArena, ret.assetHandler);

	CreateUnit(&ret.unitHandler, V2(0, 0), standartRotation);
	CreateUnit(&ret.unitHandler, V2(1, 0), QuaternionFromAngleAxis(pifac, V3(1, 0, 0)));
	CreateUnit(&ret.unitHandler, V2(0, 1), QuaternionFromAngleAxis(pifac, V3(1, 0, 0)));
	CreateUnit(&ret.unitHandler, V2(0, 1), QuaternionFromAngleAxis(pifac, V3(1, 0, 0)));

	return ret;
}

static void GameUpdateAndRender(GameState *gameState, RenderCommands *renderCommands, Input input, SoundBuffer *soundBuffer)
{
	TimedBlock;

	TileMap *tileMap = gameState->tileMap;
	Player *player = gameState->player;
	Screen *screen = gameState->screen;
	auto unitHandler = &gameState->unitHandler;
	auto assetHandler = gameState->assetHandler;
	//Update(*gameState->player, input, *tileMap, gameState->entities);
	f32 dt = input.secondsPerFrame;
	// HandleInput
	{
		KeyMessageBuffer buffer = input.buffer;
		for (u32 i = 0; i < buffer.amountOfMessages; i++)
		{
			KeyStateMessage message = buffer.messages[i];

			ConsoleHandleKeyMessage(message, &input);

			if (ConsoleActive())
			{
				continue;
			}

			EditorHandleEvents(&gameState->editor, message, input);

			UpdateDEBUGKeyTracker(&gameState->keyTracker, message);

			if (message.key == Key_l && message.flag == KeyState_PressedThisFrame)
			{
				viewLighting = !viewLighting;
			}
			if (message.key == Key_o && message.flag == KeyState_PressedThisFrame)
			{
				viewLighting = !viewLighting;
				CalculateLightingSolution(gameState->world);
			}
		}

	}

	RenderGroup renderGroup = InitRenderGroup(gameState->assetHandler, renderCommands);
	RenderGroup *rg = &renderGroup;
	ClearPushBuffer(rg);

	f32 aspectRatio = (f32)renderCommands->width / (f32)renderCommands->height;
	f32 width = (f32)renderCommands->width;
	f32 height = (f32)renderCommands->height;
	f32 focalLength = renderCommands->focalLength;

	World *world = gameState->world;

	Tweekable(b32, debugCam);

	Camera *cam = debugCam ? &world->debugCamera : &world->camera;

	Tweekable(v3, lightPos);

	world->debugCamera.pos = lightPos;

	UpdateCamGodMode(&input, &gameState->world->camera, gameState->keyTracker); // cam?
	PushRenderSetup(rg, *cam, lightPos/*world->lightSource*/, (Setup_Projective | Setup_ShadowMapping));
	PushClear(rg, V4(1.0f, 0.1f, 0.1f, 0.1f));

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

	u32 minScreenXi = (u32)Max((int)floorf(minScreenX) - 1.0f, 0.0f);
	u32 minScreenYi = (u32)Max((int)floorf(minScreenY) - 1.0f, 0.0f);

	u32 maxScreenXi = (u32)Min((int)ceilf(maxScreenX) + 1.0f, (float)tileMap->width);
	u32 maxScreenYi = (u32)Min((int)ceilf(maxScreenY) + 1.0f, (float)tileMap->height);

	Tweekable(b32, DrawWholeMap);
	if (DrawWholeMap)
	{
		minScreenXi = 0;
		minScreenYi = 0;
		maxScreenXi = tileMap->width;
		maxScreenYi = tileMap->height;
	}

	UpdateUnits(unitHandler, dt);
	Tweekable(v2, girlPos);
	unitHandler->infos[3].pos = girlPos;
	RenderUnits(rg, unitHandler);
	
	for (u32 x = minScreenXi; x < maxScreenXi; x++)
	{
		for (u32 y = minScreenYi; y < maxScreenYi; y++)
		{
			Tile *tile = GetTile(*tileMap, x, y);
			Quaternion q = { 1, 0, 0, 0 };
			PushTriangleMesh(rg, tile->mesh, q, V3(0, 0, 0), 1.0f);
		}
	}

	For(world->placedMeshes)
	{
		PushTriangleMesh(rg, it->mesh, it->orientation, it->pos, it->scale);
		PushDebugPointCuboid(rg, it->pos);
	}




#if 0
	Tile *tile = GetTile(*tileMap, 0u, 0u);
	for (u32 i = 0; i < tile->mesh.amountOfVerticies; i++)
	{
		PushLine(rg, tile->mesh.normals[i] + tile->mesh.vertices[i], tile->mesh.vertices[i]);
	}
#endif

	PushDebugPointCuboid(rg, lightPos);

	v4 allColorsOfTheRainbow[] =
	{
		V4(1, 1, 1, 1), //white = 0
		V4(1, 1, 1, 0), //yellow = 1
		V4(1, 1, 0, 0), //red = 2o
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
	//PushRectangle(rg, player->markingRect, 0.0f); todo : redo this.

	PushRenderSetup(rg, *cam, world->lightSource, Setup_ZeroToOne); //todo  make PushRenderSetup have optional lightsource.

	//PushTexturedRect(rg, V2(0.1f, 0.1f), 0.5f, 0.5f, gameState->font.bitmap);
#if 0
	u32 it_index = 0;
	For(gameState->assetHandler->textures)
	{
		//if (it_index == 3) break;
		PushTexturedRect(rg, V2(0.1f + 0.2f * it_index++, 0.1f), 0.2f, 0.2f, it->bitmap);
	}
#endif
	
	u32 id = RegisterAsset(assetHandler, Asset_Texture, "obj/stone.texture");
	//PushTexturedRect(rg, V2(0.1f + 0.2f, 0.1f), 0.2f, 0.2f, *GetTexture(assetHandler, id));
	//PushString(rg, V2(0.1f, 0.5f), CreateString("asd"), 0.1f, gameState->font);

	UpdateEditor(&gameState->editor, input);
	RenderEditor(rg, gameState->editor);
	
	UpdateConsole(input);
	DrawConsole(rg);

	ToOutput(gameState->soundMixer, soundBuffer);

}
#endif