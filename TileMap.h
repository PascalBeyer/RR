#ifndef RR_TILEMAP
#define RR_TILEMAP

enum
{
	Tile_Passible = 0x1,

};

struct Tile
{
	u16 type;
	v2 pos; //at top left
	TriangleMesh mesh;
	u32 tag;
};

struct TileMap
{
	Tile *tiles;
	u32 height, width;
	u16 tileSize;
};


static TriangleMesh GenerateMeshForFlatTile(AABB aabb, Arena* arena)
{
	TriangleMesh ret;

	u32 blackBrown = RGBAfromHEX(0x553A26);
	u32 grassGreen = RGBAfromHEX(0x4B6F44);

	u32 meshSize = 10; // @hardcoded

	ret.amountOfVerticies = meshSize * meshSize; 
	Assert(ret.amountOfVerticies < 65536);
	ret.vertices = PushData(arena, v3, ret.amountOfVerticies);
	ret.colors = PushData(arena, u32, ret.amountOfVerticies);

	RandomSeries series = { RandomSeed() };
	
	f32 xFac = (aabb.maxDim.x - aabb.minDim.x) / (meshSize - 1);
	f32 yFac = (aabb.maxDim.y - aabb.minDim.y) / (meshSize - 1);

	for (u32 i = 0; i < meshSize; i++)
	{
		ret.vertices[0 + meshSize * i] = V3(0, yFac * i, 0) + aabb.minDim;
		ret.colors	[0 + meshSize * i] = blackBrown;

		ret.vertices[(meshSize - 1) + meshSize * i] = V3(xFac * (meshSize - 1), yFac * i, 0) + aabb.minDim;
		ret.colors	[(meshSize - 1) + meshSize * i] = blackBrown;

		ret.vertices[i + meshSize * (meshSize - 1)] = V3(xFac * i, yFac * (meshSize - 1), 0) + aabb.minDim;
		ret.colors	[i + meshSize * (meshSize - 1)] = blackBrown;

		ret.vertices[i + meshSize * 0] = V3(xFac * i, 0, 0) + aabb.minDim;
		ret.colors	[i + meshSize * 0] = blackBrown;
	}

	f32 zVariance = aabb.maxDim.z - aabb.minDim.z;

	for (u32 x = 1; x < meshSize - 1; x++)
	{
		for (u32 y = 1; y < meshSize - 1; y++)
		{
			f32 xEntropy = RandomSignedPercent(&series);
			f32 yEntropy = RandomSignedPercent(&series);
			f32 zEntropy = RandomPercent(&series);

			f32 xVal = (xEntropy + (f32)x) * xFac + (aabb.minDim.x);
			f32 yVal = (yEntropy + (f32)y) * yFac + (aabb.minDim.y);
			f32 zVal = zVariance * zEntropy + aabb.minDim.z;

			ret.vertices[x + meshSize * y] = V3(xVal, yVal, zVal);
			f32 lerpFactor = zEntropy;
			ret.colors[x + meshSize * y] = Pack3x8(LerpVector3(Unpack3x8(blackBrown), Unpack3x8(grassGreen), lerpFactor));
		}
	}

	ret.amountOfIndicies = (meshSize - 1) * (3 + 2 * (meshSize - 1));
	ret.indecies = PushData(constantArena, u16, ret.amountOfIndicies);

	u32 index = 0;

	for (u32 x = 0; x < meshSize - 1; x++)
	{
		ret.indecies[index++] = 0xFFFF; // reset Index

		ret.indecies[index++] = x * meshSize + 0; // (x, 0)

												  //move up
		for (u32 y = 0; y < meshSize - 1; y++) // 2* (meshSize - 1)
		{
			ret.indecies[index++] = (x + 1) * meshSize + y;
			ret.indecies[index++] = x       * meshSize + (y + 1);
		}

		ret.indecies[index++] = (x + 1) * meshSize + (meshSize - 1);
	}

	Assert(ret.amountOfIndicies == index);

	RegisterTriangleMesh(&ret);

	return ret;
}



static TileMap InitTileMap(u32 width, u32 height, u32 tileSize, Arena *arena)
{
	TileMap ret;
	ret.width = width;
	ret.height = height;
	ret.tileSize = tileSize;

	ret.tiles = PushData(arena, Tile, width * height);

	RandomSeries series = GetRandomSeries();

	for (u32 x = 0; x < width; x++)
	{
		for (u32 y = 0; y < height; y++)
		{
			Tile *tempTile = ret.tiles + x + width * y;
			tempTile->pos = V2(x, y);
			tempTile->type = 0;
			tempTile->tag = RandomU32(&series) % 2;
			AABB aabb = CreateAABB(V3(x, y, 0.1f), V3(x+1, y+1, 0));
			//tempTile->mesh = GenerateMeshForFlatTile(aabb, arena);
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


static TileMap LoadTileMap(char *fileName, Arena *arena)
{
	TileMap ret;

	File file = LoadFile(fileName);
	void * memory = file.memory;
	TileMapHeader *tileMapHeader = (TileMapHeader *)memory;

	ret.height = tileMapHeader->height;
	ret.width = tileMapHeader->width;
	ret.tileSize = tileMapHeader->tileSize;

	u8 *buffer = (u8 *)memory;
	buffer += tileMapHeader->headerSize;

	ret.tiles = PushData(arena, Tile, ret.width * ret.height);;
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

static bool InBounds(TileMap tileMap, v2 vec)
{
	return (0 <= vec.x) && (vec.x < tileMap.width) && (0 <= vec.y) && (vec.y < tileMap.height);
}

static Tile* GetTile(TileMap tileMap, v2 pos)
{
	if (!InBounds(tileMap, pos))
	{
		return NULL;
	}

	u32 x = (u32)(pos.x);
	u32 y = (u32)(pos.y);

	return (tileMap.tiles + x + y * tileMap.width);
}

static Tile* GetTile(TileMap tileMap, float x, float y)
{
	return GetTile(tileMap, V2(x, y));
}

static Tile* GetTile(TileMap tileMap, u32 x, u32 y)
{
	return GetTile(tileMap, V2(x, y));
}

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

static float LineToFunctCapped(v2 a, v2 b, u32 x)
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

static Tile *GetFirstTileWithType(TileMap tilemap, v2 pos, v2 goal, u16 type)
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


static float GetIntersectionTimeForLine(TileMap tileMap, v2 pos, v2 speedVector, Tile tile)
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



#endif