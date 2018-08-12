#ifndef RR_PLAYER
#define RR_PLAYER

#include "EntitySelection.h"
#include "Screen.h"
#include "Entity.h"
#include "MarkingRect.h"
#include "Input.h"
#include "TileMap.h"
#include "BasicTypes.h"

struct Player
{


	MarkingRect markingRect;
	EntitySelection entitySelection;

	Screen *screen;	
	
	u8 playerIndex;
	u32 minerals;
	u32 gas;

	u32 steeringGroupingIndex;


};

Player InitPlayer(u32 screenPixelWidth, u32 screenPixelHeight, float screenWidth, TileMap *tileMap, float screenScrollSpeed);

void HandleRightClick(Player player, EntitySelection *entitys, v2 targetPos, TileMap tileMap);

void Update(Player player, Input *input, TileMap tileMap, EntitySelection *entitys);




#endif