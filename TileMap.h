#ifndef RR_TILEMAP
#define RR_TILEMAP

#include "buffers.h"
#include "Input.h"
#include "BasicTypes.h"
#include "Arena.h"

enum
{
	TileType_Empty,
	TileType_Full,
	TileType_UpperLeft,
	TileType_UpperRight,	
	TileType_LowerLeft,
	TileType_LowerRight,

	TileType_Count
};

struct Tile
{
	u16 type;
	v2 pos; //at top left
	float height;
};

struct TileMap
{
	Tile *tiles;
	u32 height, width;
	u16 tileSize;
};


TileMap LoadTileMap(char *fileName, Arena *arena);
TileMap InitTileMap(u32 width, u32 height, u32 tileSize, Arena *arena);

//void Render(ImageBuffer *imageBuffer, Screen *screen);
float GetIntersectionTimeForLine(TileMap tileMap, v2 pos, v2 speedVector, Tile tile);

Tile* GetFirstTileWithType(TileMap tilemap, v2 pos, v2 Goal, u16 type);
Tile* GetTile(TileMap tilemap, v2 pos);
Tile* GetTile(TileMap tileMap, float x, float y);
Tile* GetTile(TileMap tileMap, u32 x, u32 y);
bool InBounds(TileMap tileMap, v2 vec);

#if WriteMode

void WriteUpdate(Input *input, v2 screenPos);
void Save(char *fileName, void* memory);
#endif


#endif