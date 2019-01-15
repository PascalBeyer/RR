#ifndef RR_GAME
#define RR_GAME

#include "BasicTypes.h"
#include "SIMD.h"

#include "Math.h"
#include "Debug.h"

#include "Arena.h"
#include "DynamicAlloc.h"
#include "String.h"
#include "LinearAlgebra.h"

#include "ConsoleHeader.h"
#include "Tweeker.h"

#include "WorkHandler.h"
#include "Color.h"
#include "File.h"


#include "Random.h"
#include "Input.h"
#include "Camera.h"
#include "Entity.h"

#include "AssetTypes.h"

#include "Animation.h"

#include "AssetHandler.h"

#include "Generation.h"

#include "KdTree.h"

#include "SoundMixer.h"
#include "Renderer.h"

#include "PathCreator.h"

#include "Simulate.h"

#include "Lighting.h"

#include "Editor.h"

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
   
	// Engine
   Arena *constantArena; // this really does not have to be here.... its empty anyway.
	WorkHandler *workHandler;
	AssetHandler assetHandler;
	SoundMixer soundMixer;
	Font font; // todo make this into an asset
   
	// Mode specific stuff
   Arena *currentStateArena;
	ExecuteData executeData;
   EntityManager entityManager;
	Editor editor;
};

static GameState gameState;


static void SwitchGameMode(GameState *state, GameMode mode)
{
   AssetHandler *assetHandler = &state->assetHandler;
	Editor *editor = &state->editor;
   EntityManager *entityManager= &state->entityManager;
	SoundMixer *soundMixer = &state->soundMixer;
	Arena *currentStateArena = state->currentStateArena;
	Font font = state->font;
	ExecuteData *exe = &state->executeData;
   
   // this for now will just relay on the fact that there only these two game modes
	switch (mode)
	{
      case Game_Execute:
      {
         Assert(state->mode == Game_Editor);
         state->mode = Game_Execute;
         
         Level level = LoadLevel(editor->levelInfo.name, frameArena, assetHandler);
         Clear(state->currentStateArena);
         
         state->entityManager = InitEntityManager(state->currentStateArena, &level);
         state->executeData.camera = level.camera;
         state->executeData.debugCamera = level.camera;
         state->executeData.t = 0.0f;
         state->executeData.at = 0;
         state->executeData.lightSource = level.lightSource;
         ChangeExecuteState(&state->entityManager, &state->executeData, Execute_PathCreator);
         
      }break;
      case Game_Editor:
      {
         Assert(state->mode == Game_Execute);
         state->mode = Game_Editor;
         
         Level level = LoadLevel(entityManager->levelName, frameArena, assetHandler);
         Clear(state->currentStateArena);
         EditorLoadLevel(editor, currentStateArena, &level);
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
	ret.assetHandler = CreateAssetHandler(constantArena);
	ret.soundMixer = {};
	ret.editor = InitEditor(constantArena);
	
	u32 constantArenaRestCapacity = constantArena->capacity - (u32)(constantArena->current - constantArena->base) - 1;
   
	ret.currentStateArena = InitArena(PushData(constantArena, u8, constantArenaRestCapacity), constantArenaRestCapacity);
   
   // todo load this together with level
   Level level = LoadLevel(CreateString("bridge"), ret.currentStateArena, &ret.assetHandler);
	ret.entityManager = InitEntityManager(ret.currentStateArena, &level);
	ret.executeData = InitExecute();
   
	ChangeExecuteState(&ret.entityManager, &ret.executeData, Execute_PathCreator);
   
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
	
	
   
	SwitchGameMode(&ret, Game_Editor);
	
	return ret;
}

static void GameUpdateAndRender(GameState *state, RenderCommands *renderCommands, Input input, SoundBuffer *soundBuffer)
{	
	TimedBlock;
	AssetHandler *assetHandler = &state->assetHandler;
	Editor *editor = &state->editor;
	SoundMixer *soundMixer = &state->soundMixer;
	Arena *currentStateArena = state->currentStateArena;
	Font font = state->font;
	f32 dt = input.dt;
	ExecuteData *exe = &state->executeData;
   EntityManager *entityManager= &state->entityManager;
   
   
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
               
               ExecuteHandleEvents(entityManager, assetHandler, exe, message, input);
               
            }break;
            case Game_Editor:
            {
               EditorHandleEvents(editor, assetHandler, message, input, currentStateArena);
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
   
   
	RenderGroup renderGroup = InitRenderGroup(assetHandler, renderCommands);
	RenderGroup *rg = &renderGroup;
   PushClear(rg, V4(1.0f, 0.1f, 0.1f, 0.1f));
   
	switch (state->mode)
	{
      case Game_Execute:
      {
         
         //reset
         For(entityManager->entities)
         {
            it->frameColor = V4(1, 1, 1, 1);
         }
         
         Tweekable(b32, debugCam);
         Camera *cam = debugCam ? &exe->debugCamera : &exe->camera;
         
         f32 aspectRatio = cam->aspectRatio; // todo, this does not really belong in the camera.
         f32 focalLength = cam->focalLength;
         
         
         UpdateColorPickers(editor, input);
         GameExecuteUpdate(entityManager, exe, dt);
         
         // :ExecuteDraw
         PushRenderSetup(rg, *cam, exe->lightSource, (Setup_Projective | Setup_ShadowMapping));
         
         
         
         Tweekable(b32, DrawMeshOutlines);
         if (DrawMeshOutlines)
         {
            For(entityManager->entities)
            {
               RenderEntityAABBOutline(rg, assetHandler, it, exe->t);
            }
         }
         
         Tweekable(b32, drawEntityTree, 1);
         if (drawEntityTree)
         {
            RenderEntityQuadTree(rg, &entityManager->entityTree.root);
         }
         
         RenderExecute(rg, entityManager, exe, assetHandler, input);
         
      }break;
      case Game_Editor:
      {
         PushRenderSetup(rg, editor->camera, editor->levelInfo.lightSource, (Setup_Projective | Setup_ShadowMapping));
         
         UpdateEditor(editor, input);
         RenderEditor(rg, assetHandler, state->editor, input);
         
      }break;
   }
   
   
   { // animation test
      
      static f32 t = 0.0f;
      
      t += dt;
      
      u32 animationID               = RegisterAsset(assetHandler, Asset_Animation, "easy.animation");
      KeyFramedAnimation *animation = GetAnimation(assetHandler, animationID);
      u32 guyId          = RegisterAsset(assetHandler, Asset_Mesh, "easy.mesh");
      TriangleMesh *mesh = GetMesh(assetHandler, guyId);
      
      InterpolationDataArray local = GetLocalTransforms(animation, t);
      
      m4x4Array bones = LocalToSpace(&mesh->skeleton, local);
      
      { // Head IK test
         
         f32 timeScale = 0.1f;
         v3 headPos = V3(0, 0, 5);
         v3 orbit = 3.0f * V3(Cos(timeScale * t * PI), Sin(timeScale * t * PI), 0.0f);
         v3 point = headPos + orbit;
         PushDebugPointCuboid(rg, point);
#if 0
         InterpolationData data = MatrixToInterpolationData(bones[3]);
         
         v3 oldForward  = bones[3] * V3(0, 0, -1) - headPos; 
         
         PushLine(rg, headPos, bones[3] * V3(1, 0, 0), 0xFFFF0000); // forward : blue
         PushLine(rg, headPos, bones[3] * V3(0, 1, 0), 0xFF00FF00); // up      : green
         PushLine(rg, headPos, bones[3] * V3(0, 0, 1), 0xFF0000FF); // right   : red
         
         Quaternion newQ = LookAt(Normalize(oldForward), Normalize(orbit));
         
         data.orientation = newQ * data.orientation;
         
         bones[3] = InterpolationDataToMatrix(data);
         
         PushLine(rg, headPos, bones[3] * V3(0, -1, 0), 0xFF00FFFF);
         PushLine(rg, headPos, headPos +  CrossProduct(oldForward, point - headPos), 0xFFFFFF00);
#endif
         ApplyIK(mesh->skeleton.bones, local, bones, 3, point);
         
      }
      
      for (u32 i = 0; i < mesh->skeleton.bones.amount; i++)
      {
         bones[i] = bones[i] * mesh->skeleton.bones[i].inverseBindShapeMatrix;
      }
      
      PushAnimatedMesh(rg, mesh, QuaternionId(), v3(), 1.0f, V4(1, 1, 1, 1), bones);
   }
   
   
   Tweekable(b32, drawCamera);
   
   if (drawCamera)
   {
      
      Camera *camera = &exe->camera;
      
#if 0
      Input asd;
      asd.mouseDelta = V2(10, 0);
      EditorUpdateCamFocus(editor, camera, asd);
#endif
      
      m3x3 camM = QuaternionToMatrix3(camera->orientation);
      
      float camBoxSize = 0.4f;
      
      v3 camPos = camera->pos;
      
      PushDebugPointCuboid(rg, camPos);
      
      PushLine(rg, camera->pos, camera->pos + B1(camera->orientation), 0xFFFF0000); // blue
      PushLine(rg, camera->pos, camera->pos + B2(camera->orientation), 0xFF00FF00); // green
      PushLine(rg, camera->pos, camera->pos + B3(camera->orientation), 0xFF0000FF); // red
      
      
      
      //PushDebugPointCuboid(rg, camera->pos + B3(camera->orientation) *5.0f);
      
#if 0
      m4x4 proj = Projection(aspectRatio, focalLength) * CameraTransform(camera->orientation, camera->pos);
      m4x4 inv = InvOrId(proj);
      m4x4 id = proj * inv;
      v3 p1 = inv * V3(-1, -1, -1);
      v3 p2 = inv * V3(1, -1, -1);
      v3 p3 = inv * V3(-1, 1, -1);
      v3 p4 = inv * V3(1, 1, -1);
      PushQuadrilateral(rg, p1, p2, p3, p4, V4(0.5f, 1.0f, 0.0f, 0.0f));
      
      v3 pp1 = i12(ScreenZeroToOneToInGame(*camera, V2(0.0f, 0.0f)));
      v3 pp2 = i12(ScreenZeroToOneToInGame(*camera, V2(1.0f, 0.0f)));
      v3 pp3 = i12(ScreenZeroToOneToInGame(*camera, V2(0.0f, 1.0f)));
      v3 pp4 = i12(ScreenZeroToOneToInGame(*camera, V2(1.0f, 1.0f)));
      PushQuadrilateral(rg, pp1, pp2, pp3, pp4, V4(0.5f, 0.0f, 1.0f, 0.0f));
#endif
   }
   
   PushRenderSetup(rg, {}, {}, Setup_OrthoZeroToOne); 
   
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
   
   ToOutput(soundMixer, soundBuffer);
   
}
#endif