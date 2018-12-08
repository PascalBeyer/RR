#ifndef RR_GAME
#define RR_GAME

#include "BasicTypes.h"

#include "buffers.h"

#include "Intrinsics.h"
#include "Math.h"

#include "Arena.h"
#include "String.h"

#include "ConsoleHeader.h"
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

#include "World.h"

#include "AssetHandler.h"

#include "Generation.h"

#include "Animation.h"
#include "KdTree.h"

#include "SoundMixer.h"
#include "Renderer.h"

#include "Entity.h"

#include "Lighting.h"

#include "Level.h"

//#include "MovementHandler.h"
#include "Editor.h" // todo rank this once we know more.


enum GameMode
{
	Game_Execute,
	Game_PathCreator,
	Game_Editor,
	Game_TitleScreen,
	//...
};

struct GameState
{
	GameMode mode;

	Arena *constantArena;
	Arena *workingArena;
	Arena *currentStateArena;

	// Engine
	WorkHandler *workHandler;
	AssetHandler assetHandler;
	SoundMixer soundMixer;
	Font font; // todo make this into an asset

	// level specific stuff
	UnitHandler unitHandler;
	PathCreator pathCreator;
	World world;
	Editor editor;
};

static GameState gameState;


static void GameGoToMode(GameState *state, GameMode mode)
{
#if 0
	switch (state->mode)
	{

	}
#endif

	switch (mode)
	{
	case Game_Execute:
	{
		ResetUnits(&state->unitHandler);
		state->mode = Game_Execute;
	}break;
	case Game_PathCreator:
	{
		ResetUnits(&state->unitHandler);
		state->mode = Game_PathCreator;
	}break;
	case Game_Editor:
	{
		ResetUnits(&state->unitHandler); 
		state->mode = Game_Editor;
		state->editor.focusPoint = i12(p12(state->world.camera.pos));
		state->world.camera.basis = v3StdBasis;
	}break;

	InvalidDefaultCase;

	}

}


//debug stuff, has to be after gamestate for now, as console commands have to alter the gamestate
#include "ConsoleCommands.h"
#include "Console.h"
#include "DebugBody.h"

static bool viewLighting = false;

static GameState InitGame(int screenWidth, int screenHeight, WorkHandler *workHandler, Arena *constantArena)
{
	GameState ret = {};

	ret.mode = Game_Editor;

	ret.constantArena = constantArena;
	
	ret.workHandler = workHandler;
	ret.font = LoadFont("consola.ttf", constantArena);
	globalFont = ret.font;
	InitConsole(constantArena);
	ret.assetHandler = CreateAssetHandler(constantArena);
	ret.soundMixer = {};
	ret.unitHandler = CreateUnitHandler(constantArena, &ret.assetHandler);

	ret.workingArena = InitArena(PushData(constantArena, u8, MegaBytes(50)), MegaBytes(50));
	
	u32 constantArenaRestCapacity = constantArena->capacity - (u32)(constantArena->current - constantArena->base) - 1;

	ret.currentStateArena = InitArena(PushData(constantArena, u8, constantArenaRestCapacity), constantArenaRestCapacity);

	u32 tileMapHeight = 10;
	u32 tileMapWidth = 10;
	f32 pifac = 6.0f * 0.25f * pi32;
	Quaternion standartRotation = QuaternionFromAngleAxis(pifac, V3(1, 0, 0));
	AABB groundaabb = { V3(-20, -20, -0.25f), V3(20, 20, 0) };

	Tweekable(v3, cameraStartingPos);
	ret.world = {};
	ret.world.tileMap = InitTileMap(&ret.assetHandler, tileMapWidth, tileMapHeight, ret.currentStateArena);
	ret.world.lightSource = V3(20, 0, -20);
	ret.world.triangles = {};
	ret.world.camera.pos = cameraStartingPos;
	ret.world.camera.basis = v3StdBasis;
	ret.world.camera.aspectRatio = (f32)screenWidth / (f32)screenHeight;
	ret.world.camera.focalLength = 1.0f;
	ret.world.debugCamera.pos = cameraStartingPos;
	ret.world.debugCamera.basis = v3StdBasis;
	ret.world.debugCamera.aspectRatio = (f32)screenWidth / (f32)screenHeight;
	ret.world.debugCamera.focalLength = 1.0f;
	ret.world.placedMeshes = PlacedMeshCreateDynamicArray();

	u32 treeMeshId = RegisterAsset(&ret.assetHandler, Asset_Mesh, "tree.mesh");
	u32 unitMeshId = RegisterAsset(&ret.assetHandler, Asset_Mesh, "dude.mesh");

	AABB unitMeshAABB = GetMesh(&ret.assetHandler, unitMeshId)->aabb;
	ArrayAdd(&ret.world.placedMeshes, CreatePlacedMesh(unitMeshId, 10.6f, standartRotation, V3(3, 3, 0), V4(1, 1, 1, 1), unitMeshAABB));

	AABB treeMeshAABB = GetMesh(&ret.assetHandler, treeMeshId)->aabb;
	ArrayAdd(&ret.world.placedMeshes, CreatePlacedMesh(treeMeshId, 0.1f, standartRotation, V3(5, 1, 0), V4(1, 1, 1, 1), treeMeshAABB));
	ArrayAdd(&ret.world.placedMeshes, CreatePlacedMesh(treeMeshId, 0.1f, standartRotation, V3(5, 6, 0), V4(1, 1, 1, 1), treeMeshAABB));
	ArrayAdd(&ret.world.placedMeshes, CreatePlacedMesh(treeMeshId, 0.1f, standartRotation, V3(3, 1, 0), V4(1, 1, 1, 1), treeMeshAABB));
	ArrayAdd(&ret.world.placedMeshes, CreatePlacedMesh(treeMeshId, 0.1f, standartRotation, V3(5, 2, 0), V4(1, 1, 1, 1), treeMeshAABB));
	ArrayAdd(&ret.world.placedMeshes, CreatePlacedMesh(treeMeshId, 0.1f, standartRotation, V3(2, 1, 0), V4(1, 1, 1, 1), treeMeshAABB));
	ArrayAdd(&ret.world.placedMeshes, CreatePlacedMesh(treeMeshId, 0.1f, standartRotation, V3(5, 4, 0), V4(1, 1, 1, 1), treeMeshAABB));
	ArrayAdd(&ret.world.placedMeshes, CreatePlacedMesh(treeMeshId, 0.1f, standartRotation, V3(0, 9, 0), V4(1, 1, 1, 1), treeMeshAABB));
	
	

	Quaternion id = {1, 0, 0, 0};
	
	u32 cubeMeshId = RegisterAsset(&ret.assetHandler, Asset_Mesh, "cube.mesh");
	
#if 1
	AABB cubeMeshAABB = GetMesh(&ret.assetHandler, cubeMeshId)->aabb;
	for (u32 x = 0; x < tileMapWidth; x++)
	{
		for (u32 y = 0; y < tileMapHeight; y++)
		{
			u32 meshIndex = ArrayAdd(&ret.world.placedMeshes, CreatePlacedMesh(cubeMeshId, 0.5f, id, V3(x, y, 0.5f), V4(1, 1, 1, 1), cubeMeshAABB));
			GetTile(ret.world.tileMap, V2(x, y))->meshIndex = meshIndex;
		}
	}
#endif

	ret.editor = InitEditor(ret.world.camera, ret.currentStateArena);

	ret.pathCreator.hotUnit = 0xFFFFFFFF;
	ret.pathCreator.state = PathCreator_None;

	CreateUnit(&ret.unitHandler, V2(4, 2), standartRotation);
	CreateUnit(&ret.unitHandler, V2(5, 3), QuaternionFromAngleAxis(pifac, V3(1, 0, 0)));
	CreateUnit(&ret.unitHandler, V2(6, 2), QuaternionFromAngleAxis(pifac, V3(1, 0, 0)));

#if 0
	For(ret.assetHandler.textureCatalog)
	{
		u8 *restore = frameArena->current;

		Bitmap *bit = GetTexture(&ret.assetHandler, it->id);
		AssetInfo info = GetAssetInfo(&ret.assetHandler, it->id);
		String name = FormatCString("textures/%s", info.name);
		Bitmap down = DownSampleTexture(*bit);
		WriteTexture(name.cstr, down);

		frameArena->current = restore;
	}
#endif


	ConvertNewAssets(&ret.assetHandler, ret.currentStateArena, ret.workingArena);

	return ret;
}

static void GameUpdateAndRender(GameState *gameState, RenderCommands *renderCommands, Input input, SoundBuffer *soundBuffer)
{
	TimedBlock;

	TileMap tileMap = gameState->world.tileMap;
	UnitHandler *unitHandler = &gameState->unitHandler;
	AssetHandler *assetHandler = &gameState->assetHandler;
	Editor *editor = &gameState->editor;
	World *world = &gameState->world;
	SoundMixer *soundMixer = &gameState->soundMixer;

	f32 width = (f32)renderCommands->width;
	f32 height = (f32)renderCommands->height;

	f32 aspectRatio = (f32)renderCommands->width / (f32)renderCommands->height;
	Tweekable(b32, debugCam);

	Camera *cam = debugCam ? &world->debugCamera : &world->camera;

	Tweekable(v3, lightPos);
	f32 focalLength = world->camera.focalLength;

	//reset
	For(world->placedMeshes)
	{
		it->frameColor = V4(1, 1, 1, 1);
	}


	f32 dt = input.secondsPerFrame;
	// HandleInput
	{
		KeyMessageBuffer buffer = input.buffer;
		for (u32 i = 0; i < buffer.amountOfMessages; i++)
		{
			KeyStateMessage message = buffer.messages[i];

			if(DebugHandleEvents(message, input)) continue;

			ConsoleHandleKeyMessage(message, &input);

			if (ConsoleActive())
			{
				continue;
			}

			switch (gameState->mode)
			{
			case Game_PathCreator:
			{
				
				PathCreatorHandleEvent(&gameState->pathCreator, unitHandler, message, input, focalLength, aspectRatio, world->camera);
				ColorPickersHandleEvents(&gameState->editor, message, input);

			}break;
			case Game_Execute:
			{

				ColorPickersHandleEvents(&gameState->editor, message, input);

			}break;
			case Game_Editor:
			{
				EditorHandleEvents(editor, unitHandler, world, message, input, focalLength, aspectRatio);
			}break;
			InvalidDefaultCase;
			}

			if (message.key == Key_F6 && (message.flag & KeyState_PressedThisFrame))
			{
				if (gameState->mode == Game_Execute)
				{
					GameGoToMode(gameState, Game_PathCreator);
				}
				else if(gameState->mode == Game_PathCreator)
				{
					GameGoToMode(gameState, Game_Execute);
				}
			}

			if (message.key == Key_l && (message.flag & KeyState_PressedThisFrame))
			{
				viewLighting = !viewLighting;
			}
			if (message.key == Key_o && (message.flag & KeyState_PressedThisFrame))
			{
				viewLighting = !viewLighting;
				CalculateLightingSolution(world, gameState->constantArena);
			}
		}

	}

	
	 //update
	switch (gameState->mode)
	{
	case Game_PathCreator:
	{
		ColorForPathEditor(unitHandler, &gameState->pathCreator, world, input);
		ColorForTileMap(world);
		UpdateColorPickers(editor, input);
		UpdateCamGame(&input, cam);

	}break;
	case Game_Execute:
	{
		UpdateUnits(unitHandler, tileMap, dt);
		UpdateColorPickers(editor, input);
		UpdateCamGame(&input, cam);

	}break;
	case Game_Editor:
	{
		UpdateEditor(editor, unitHandler, world, input);
		//UpdateCamFocus(&input, cam, &gameState->keyTracker);

	}break;
	}

	RenderGroup renderGroup = InitRenderGroup(assetHandler, renderCommands);
	RenderGroup *rg = &renderGroup;
	ClearPushBuffer(rg);


	PushRenderSetup(rg, *cam, lightPos/*world->lightSource*/, (Setup_Projective | Setup_ShadowMapping));
	PushClear(rg, V4(1.0f, 0.1f, 0.1f, 0.1f));

	RenderUnits(rg, unitHandler);

	Tweekable(b32, DrawMeshOutlines);

	For(world->placedMeshes)
	{
		PushTriangleMesh(rg, it->meshId, it->orientation, it->pos, it->scale, it->color * it->frameColor);
		
		if (DrawMeshOutlines)
		{

			AABB transformedAABB = it->untransformedAABB;

			transformedAABB.minDim *= it->scale;
			transformedAABB.maxDim *= it->scale;

			m4x4 mat = Translate(QuaternionToMatrix(it->orientation), it->pos);


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



	PushDebugPointCuboid(rg, lightPos);

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

	cam = &world->camera;

	v3 camV1 = cam->basis.d1;
	v3 camV2 = cam->basis.d2;
	v3 camV3 = cam->basis.d3;

#if 1
	Tweekable(b32, drawCamera);

	if(drawCamera)
	{
		float camBoxSize = 0.4f;
		
		v3 camPos = cam->pos;

		PushDebugPointCuboid(rg, camPos);

		PushLine(rg, cam->pos, cam->pos + camV1, 0xFFFF0000);
		PushLine(rg, cam->pos, cam->pos + camV2, 0xFF00FF00);
		PushLine(rg, cam->pos, cam->pos + camV3, 0xFF0000FF);

		m4x4 proj = Projection(aspectRatio, focalLength) * CameraTransform(camV1, camV2, camV3, camPos);
		m4x4 inv = InvOrId(proj);
		m4x4 id = proj * inv;
		v3 p1 = inv * V3(-1, -1, -1);
		v3 p2 = inv * V3(1, -1, -1);
		v3 p3 = inv * V3(-1, 1, -1);
		v3 p4 = inv * V3(1, 1, -1);
		PushQuadrilateral(rg, p1, p2, p3, p4, V4(0.5f, 1.0f, 0.0f, 0.0f));

		v3 pp1 = i12(ScreenZeroToOneToInGame(*cam, V2(0.0f, 0.0f)));
		v3 pp2 = i12(ScreenZeroToOneToInGame(*cam, V2(1.0f, 0.0f)));
		v3 pp3 = i12(ScreenZeroToOneToInGame(*cam, V2(0.0f, 1.0f)));
		v3 pp4 = i12(ScreenZeroToOneToInGame(*cam, V2(1.0f, 1.0f)));
		PushQuadrilateral(rg, pp1, pp2, pp3, pp4, V4(0.5f, 0.0f, 1.0f, 0.0f));
	}
#endif
	RenderEditor(rg, gameState->editor);

	PushRenderSetup(rg, *cam, world->lightSource, Setup_ZeroToOne); //todo  make PushRenderSetup have optional lightsource.

	//if(asd) PushCenteredRectangle(rg, *asd, 0.01f, 0.01f, V4(1, 1, 1, 1));

	//PushTexturedRect(rg, V2(0.1f, 0.1f), 0.5f, 0.5f, gameState->font.bitmap);

#if 1
	f32 drawAlltexturesAdvance = 1.0f / (f32)Asset_Texture_Amount;
	for(u32 i = 0; i < Asset_Texture_Amount; i++)
	{
		auto it = assetHandler->textureList.base + i;
		PushTexturedRect(rg, V2(drawAlltexturesAdvance * i, 0.1f), drawAlltexturesAdvance, drawAlltexturesAdvance, it->bitmap);
	}
#endif
	
	
	//u32 id = RegisterAsset(assetHandler, Asset_Texture, "obj/stone.texture");
	//PushTexturedRect(rg, V2(0.1f + 0.2f, 0.1f), 0.2f, 0.2f, *GetTexture(assetHandler, id));
	//PushString(rg, V2(0.1f, 0.5f), CreateString("asd"), 0.1f, gameState->font);

	//UpdateColorPickers(&gameState->editor, input);
	RenderEditorUI(rg, gameState->editor, gameState->font);

	UpdateConsole(input);
	DrawConsole(rg);

	ToOutput(soundMixer, soundBuffer, gameState->workingArena);

}
#endif