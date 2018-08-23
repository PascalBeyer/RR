#ifndef RR_GAME
#define RR_GAME

#include "SoundMixer.h"
#include "Renderer.h"
#include "Input.h"
#include "Unit.h"
#include "Player.h"
#include "Building.h"

#include "WorkHandler.h"
#include "MovementHandler.h"
#include "UI.h"
#include "Lighting.h"
#include "Arena.h"


//lets see if we need 3d
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
	
	UI *debugUI;

	float time;
};

GameState InitGame(int screenWidth, int screenHeight, WorkHandler *workHandler);
void GameUpdateAndRender(GameState *state, RenderCommands *renderCommands, Input *input, SoundBuffer *soundBuffer);
#endif