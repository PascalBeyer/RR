#include "TileMap.h"

#include "File.h"
#include "Random.h"
#include "Math.h"

TileMap InitTileMap(u32 width, u32 height, u32 tileSize, Arena *arena)
{
	TileMap ret;
	ret.width = width;
	ret.height = height;
	ret.tileSize = tileSize;

	ret.tiles = PushArray(arena, Tile, width * height);
	
	for (u32 x = 0; x < width; x++)
	{	
		for (u32 y = 0; y < height; y++)
		{
			Tile *tempTile = ret.tiles + x + width * y;
			tempTile->pos = V2(x, y);
			tempTile->type = 0;
			tempTile->height = 1.0f;// randomSamples[(x*y + 1) % 100];
		}
	}
	return ret;
}


#pragma pack(push,1)
struct TileMapHeader
{
	u32 headerSize;
	u32 width;
	u32 height;
	u16 tileSize;
};
#pragma pack(pop)
#pragma pack(push,1)
struct TileReader
{
	u16 type;
	float posx;
	float posy;
};
#pragma pack(pop)



TileMap LoadTileMap(char *fileName, Arena *arena)
{
	TileMap ret;

	File file = File(fileName);
	void * memory = file.GetMemory();
	TileMapHeader *tileMapHeader = (TileMapHeader *)memory;

	ret.height = tileMapHeader->height;
	ret.width = tileMapHeader->width;
	ret.tileSize = tileMapHeader->tileSize;

	u8 *buffer = (u8 *)memory;
	buffer += tileMapHeader->headerSize;

	ret.tiles = PushArray(arena, Tile, ret.width * ret.height);;
	for (unsigned int x = 0; x < ret.width; x++)
	{
		for (unsigned int y = 0; y < ret.height; y++)
		{
			TileReader *tr = (TileReader *)buffer;
			buffer += sizeof(TileReader);
			Tile *tile = ret.tiles + x + ret.width *y;

			tile->pos.x = tr->posx;
			tile->pos.y = tr->posy;
			tile->type = tr->type;
		}
	}
	return ret;
}

bool InBounds(TileMap tileMap, v2 vec)
{
	return (0 <= vec.x) && (vec.x < tileMap.width) && (0 <= vec.y) && (vec.y < tileMap.height);
}

Tile* GetTile(TileMap tileMap, v2 pos)
{
	if (!InBounds(tileMap, pos))
	{
		return NULL;
	}

	u32 x = (u32)(pos.x);
	u32 y = (u32)(pos.y);

	return (tileMap.tiles + x + y * tileMap.width);
}

Tile* GetTile(TileMap tileMap, float x, float y)
{	
	return GetTile(tileMap, V2(x, y));
}

Tile* GetTile(TileMap tileMap, u32 x, u32 y)
{
	return GetTile(tileMap, V2(x, y));
}
/*
float TileMap::GetHeight(v2 pos)
{
	int x = (int)(pos.x);
	int y = (int)(pos.y);

	return tiles[x][y].height;
}
*/
static void LinearIntersectionOneDim(float *input, float speedX, float speedY, float minX, float posX, float posY, float minY, float maxY)
{
	if (!(speedX == 0))
	{
		float intersection = (minX - posX) / speedX;
		if ((0 <= intersection && intersection < *input))
		{
			float newY = posY + speedY * intersection;
			if (minY < newY && newY < maxY)
			{
				*input = intersection;
			}
		}
	}
}

float LineToFunctCapped(v2 a, v2 b, u32 x)
{
	float t = ((float)x - b.x) / a.x;
	if (t > 0.0f)
	{
		if (t < 1.0f)
		{
			return (a.y * t + b.y);
		}
		else
		{
			return a.y + b.y;
		}

	}
	else
	{
		return b.y;
	}

}
#if 0
u32 ceilf(float f)
{
	u32 fi = (int)f;
	if ((float)fi == f)
	{
		return fi;
	}
	else
	{
		return (u32)(fi + 1);
	}
}
u32 floorf(float f)
{
	u32 fi = (int)f;
	if ((float)fi == f)
	{
		return fi;
	}
	else
	{
		return (u32)(fi);
	}
}
#endif

Tile *GetFirstTileWithType(TileMap tilemap, v2 pos, v2 goal, u16 type)
{
	TileMap *tileMap = &tilemap;
	v2 a = goal - pos;
	if (a.x != 0) {
		int signx;
		if (a.x > 0)
		{
			signx = 1;
		}
		else
		{
			signx = -1;
		}

		int signy;
		if (a.y > 0)
		{
			signy = 1;
		}
		else
		{
			signy = -1;
		}

		u32 xBegin = (u32)pos.x;
		u32 xEnd = ((u32)goal.x + signx);

		for (u32 x = xBegin; x != xEnd; x += signx)
		{
			float yx = LineToFunctCapped(a, pos, x);
			float yx1 = LineToFunctCapped(a, pos, (x + 1));

			u32 yBegin;
			u32 yEnd;

			if ((signy > 0))
			{
				if (yx > yx1)
				{

					yEnd = (u32)ceilf(yx);
					yBegin = (u32)yx1;
				}
				else
				{
					yEnd = (u32)ceilf(yx1);
					yBegin = (u32)yx;
				}
			}
			else
			{

				if (yx > yx1)
				{
					yBegin = (u32)yx;
					yEnd = (u32)floorf(yx1 - 1.0f);
				}
				else
				{
					yBegin = (u32)yx1;
					yEnd = (u32)floorf(yx - 1.0f);
				}
			}

			if (yBegin == yEnd)
			{
				int asda = 123;
			}

			for (u32 y = yBegin; y != yEnd; y += signy)
			{
				Tile *tile = GetTile(tilemap, x, y);
				if (tile->type == type)
				{
					return tile;
				}
			}
		}
	}
	else
	{
		int signy;
		if (a.y > 0)
		{
			signy = 1;
		}
		else
		{
			signy = -1;
		}
		float ybegin = pos.y;
		float yend = a.y + pos.y;

		for (u32 y = (u32)ybegin; y != (u32)yend; y += signy)
		{
			Tile* tile = GetTile(tilemap, V2(pos.x, y));
			if (tile->type == type)
			{
				return tile;
			}
		}
	}

	return NULL;
}


float GetIntersectionTimeForLine(TileMap tileMap, v2 pos, v2 speedVector, Tile tile)
{
	v2 halfTileDimension = V2(tileMap.tileSize / 2.0f, tileMap.tileSize / 2.0f);
	v2 minVector = tile.pos - halfTileDimension;
	v2 maxVector = tile.pos + halfTileDimension;

	float ret = 1.0f;

	LinearIntersectionOneDim(&ret, speedVector.x, speedVector.y, minVector.x, pos.x, pos.y, minVector.y, maxVector.y);
	LinearIntersectionOneDim(&ret, speedVector.x, speedVector.y, maxVector.x, pos.x, pos.y, minVector.y, maxVector.y);
	LinearIntersectionOneDim(&ret, speedVector.y, speedVector.x, minVector.y, pos.y, pos.x, minVector.x, maxVector.x);
	LinearIntersectionOneDim(&ret, speedVector.y, speedVector.x, maxVector.y, pos.y, pos.x, minVector.x, maxVector.x);

	return ret;
}

#if WriteMode

void TileMap::WriteUpdate(Input *input, v2 screenPos)
{
	if (input->mouse->leftButtonPressedThisFrame)
	{
		Tile *tile = GetTile(input->relativeMousePos + screenPos);
		tile->type = (tile->type + 1) % 2;//TileType_Count;
	}
}


void TileMap::Save(char* fileName, void *memory)
{
	u32 constMemorySize = 4 * sizeof(u32); //headerSize, width, height
	u32 tileMapMemorySize = width * height * (2 * sizeof(float) + sizeof(u16)); // tileSize for over all tiles
	u32 totalMemorySize = constMemorySize + tileMapMemorySize;

	u32 headerSize = sizeof(TileMapHeader);

	u8 *fileMemory = (u8 *)memory;
	*((u32 *)fileMemory) = headerSize;
	fileMemory += sizeof(u32);
	*((u32 *)fileMemory) = width;
	fileMemory += sizeof(u32);
	*((u32 *)fileMemory) = height;
	fileMemory += sizeof(u32);
	*((u16 *)fileMemory) = tileSize;
	fileMemory += sizeof(u16);
	for (unsigned int x = 0; x < width; x++)
	{
		for (unsigned int y = 0; y < height; y++)
		{
			*((u16 *)fileMemory) = tiles[x][y].type;
			fileMemory += sizeof(u16);
			*((float *)fileMemory) = tiles[x][y].pos.x;
			fileMemory += sizeof(float);
			*((float *)fileMemory) = tiles[x][y].pos.y;
			fileMemory += sizeof(float);
		}
	}

	File file = File(memory, totalMemorySize);
	file.Write(fileName);
}

#endif