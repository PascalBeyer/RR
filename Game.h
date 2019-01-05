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

#include "Animation.h"
#include "World.h"

#include "AssetHandler.h"

#include "Generation.h"

#include "KdTree.h"

#include "SoundMixer.h"
#include "Renderer.h"

#include "Entity.h"

#include "PathCreator.h"

#include "Simulate.h"

#include "Lighting.h"

//#include "MovementHandler.h"
#include "Editor.h" // todo rank this once we know more.

#include "Level.h"

#include "InputEventHandling.h"

#include "Draw.h"


enum GameMode
{
	Game_Execute,
	Game_Editor,
	Game_TitleScreen,
	//...
	Game_Count,
};

static char* GameModeStrings[Game_Count] =
{
	"Execute",
	"Editor",
	"TitleScreen"
};

struct GameState
{
	GameMode mode;
    
	Arena *constantArena;
	Arena *workingArena;
	Arena *currentStateArena;
    
	// tempoary
	DAEReturn testMesh;
    
	// Engine
	WorkHandler *workHandler;
	AssetHandler assetHandler;
	SoundMixer soundMixer;
	Font font; // todo make this into an asset
    
	// level specific stuff
	ExecuteData executeData;
	World world;
	Editor editor;
};

static GameState gameState;


static void SwitchGameMode(GameState *state, GameMode mode)
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
            auto world = &state->world;
            ResetTree(world, &world->entityTree);
            For(world->entities)
            {
                InsertEntity(world, it);
            }
            ChangeExecuteState(world, &state->executeData, Execute_PathCreator);
            state->mode = Game_Execute;
        }break;
        case Game_Editor:
        {
            state->mode = Game_Editor;
            ResetWorld(&state->world);
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
    
	ret.constantArena = constantArena;
	
	ret.workHandler = workHandler;
	ret.font = LoadFont("consola.ttf", constantArena);
	globalFont = ret.font;
	InitConsole(constantArena);
	ret.workingArena = InitArena(PushData(constantArena, u8, MegaBytes(50)), MegaBytes(50));
	ret.assetHandler = CreateAssetHandler(constantArena, ret.workingArena);
	ret.soundMixer = {};
	ret.editor = InitEditor(constantArena);
	
	u32 constantArenaRestCapacity = constantArena->capacity - (u32)(constantArena->current - constantArena->base) - 1;
    
	ret.currentStateArena = InitArena(PushData(constantArena, u8, constantArenaRestCapacity), constantArenaRestCapacity);
    
	ret.world = InitWorld(ret.currentStateArena, &ret.assetHandler, (u32)screenWidth, (u32)screenHeight);
	ret.executeData = InitExecute();
    
	ChangeExecuteState(&ret.world, &ret.executeData, Execute_PathCreator);
    
    
	f32 pifac = 6.0f * 0.25f * pi32;
	Quaternion standartRotation = QuaternionFromAngleAxis(pifac, V3(1, 0, 0));
	
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
	
	LoadLevel(CreateString("bridge"), &ret.world, ret.currentStateArena, &ret.assetHandler, &ret.editor);
    
	ret.testMesh = ReadDAE(ret.currentStateArena, "obj/dude.dae");
    
	SwitchGameMode(&ret, Game_Editor);
	
	return ret;
}

static void GameUpdateAndRender(GameState *state, RenderCommands *renderCommands, Input input, SoundBuffer *soundBuffer)
{	
	TimedBlock;
	AssetHandler *assetHandler = &state->assetHandler;
	Editor *editor = &state->editor;
	World *world = &state->world;
	SoundMixer *soundMixer = &state->soundMixer;
	Arena *currentStateArena = state->currentStateArena;
	Font font = state->font;
	ExecuteData *exe = &state->executeData;
    
	Tweekable(b32, debugCam);
    
	Camera *cam = debugCam ? &world->debugCamera : &world->camera;
    
	Tweekable(v3, lightPos);
	f32 aspectRatio = world->camera.aspectRatio;
	f32 focalLength = world->camera.focalLength;
    
	//reset
	For(world->entities)
	{
		it->frameColor = V4(1, 1, 1, 1);
	}
    
	static f32 timeScale = 1.0f;
	// HandleInput
	{
		KeyMessageBuffer buffer = input.buffer;
		for (u32 i = 0; i < buffer.amountOfMessages; i++)
		{
			KeyStateMessage message = buffer.messages[i];
            
			if(DebugHandleEvents(message, input)) continue;
            
			ConsoleHandleEvent(message, &input);
            
			if (ConsoleActive())
			{
				continue;
			}
            
			switch (state->mode)
			{
                case Game_Execute:
                {
                    ColorPickersHandleEvents(&state->editor, message, input);
                    
                    ExecuteHandleEvents(world, assetHandler, exe, message, input);
                    
                }break;
                case Game_Editor:
                {
                    EditorHandleEvents(editor, world, assetHandler, message, input, currentStateArena);
                }break;
                InvalidDefaultCase;
                
			}
            
			if (message.key == Key_F5 && (message.flag & KeyState_PressedThisFrame))
			{
				if (state->mode == Game_Editor)
				{
					SwitchGameMode(state, Game_Execute);
				}
				else
				{
					SwitchGameMode(state, Game_Editor);
				}
			}
            
			if (message.key == Key_l && (message.flag & KeyState_PressedThisFrame))
			{
				viewLighting = !viewLighting;
			}
			if (message.key == Key_o && (message.flag & KeyState_PressedThisFrame))
			{
				viewLighting = !viewLighting;
				//CalculateLightingSolution(level, state->constantArena);
			}
		}
	}
    
	f32 dt = timeScale * input.dt;
	
    //update
	switch (state->mode)
	{
        case Game_Execute:
        {
            UpdateColorPickers(editor, input);
            
            GameExecuteUpdate(world, exe, dt);
            
        }break;
        case Game_Editor:
        {
            UpdateEditor(editor, world, input);
            
        }break;
	}
    
	RenderGroup renderGroup = InitRenderGroup(assetHandler, renderCommands);
	RenderGroup *rg = &renderGroup;
	ClearPushBuffer(rg);
    
	PushRenderSetup(rg, *cam, lightPos, (Setup_Projective | Setup_ShadowMapping));
	PushClear(rg, V4(1.0f, 0.1f, 0.1f, 0.1f));
    
	Tweekable(b32, DrawMeshOutlines);
	if (DrawMeshOutlines)
	{
		For(world->entities)
		{
			MeshInfo *info = GetMeshInfo(assetHandler, it->meshId);
			if (!info) continue;
			AABB transformedAABB = info->aabb;
            
			transformedAABB.minDim *= it->scale;
			transformedAABB.maxDim *= it->scale;
            
			m4x4 mat = Translate(QuaternionToMatrix(it->orientation), GetRenderPos(*it, world->t));
            
            
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
    
	Tweekable(b32, drawCamera);
    
	if (drawCamera)
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
    
	Tweekable(b32, drawEntityTree, 1);
    
	if (drawEntityTree)
	{
		RenderEntityQuadTree(rg, &world->entityTree.root);
	}
    
	switch (state->mode)
	{
        case Game_Editor:
        {
            RenderEditor(rg, assetHandler, state->editor, world, input);
        }break;
        case Game_Execute:
        {
            RenderExecute(rg, world, exe, assetHandler, input);
        }break;
        
	}
    
    
	u32 animationid = RegisterAsset(assetHandler, Asset_Animation, "dude.animation");
	KeyFramedAnimation *animation = GetAnimation(assetHandler, animationid);
	u32 dude = RegisterAsset(assetHandler, Asset_Mesh, "dude.mesh");
	TriangleMesh *mesh = GetMesh(assetHandler, dude);
	DAEReturn ret;
	ret.animation = *animation;
	ret.mesh = *mesh;
    
	AnimationTestStuff(rg, &ret, dt);
	
	PushRenderSetup(rg, *cam, world->lightSource, Setup_ZeroToOne); //todo  make PushRenderSetup have optional lightsource.
    
	switch (state->mode)
	{
        case Game_Editor:
        {
            RenderEditorUI(rg, state->editor, state->font);
        }break;
        case Game_Execute:
        {
            RenderExecuteUI(rg, exe);
        }break;
        
	}
    
    
    
    //PushTexturedRect(rg, V2(0.1f, 0.1f), 0.5f, 0.5f, state->font.bitmap);
    
#if 0
	f32 drawAlltexturesAdvance = 1.0f / (f32)Asset_Texture_Amount;
	for (u32 i = 0; i < Asset_Texture_Amount; i++)
	{
		auto it = assetHandler->textureList.base + i;
		PushTexturedRect(rg, V2(drawAlltexturesAdvance * i, 0.1f), drawAlltexturesAdvance, drawAlltexturesAdvance, it->bitmap);
	}
#endif
    
	UpdateConsole(input);
	DrawConsole(rg);
    
	PushString(rg, V2(0.75f, 0.01f), GameModeStrings[state->mode], 0.03f, font);
    
	ToOutput(soundMixer, soundBuffer, state->workingArena);
    
}
#endif