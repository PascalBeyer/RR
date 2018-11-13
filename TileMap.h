#ifndef RR_TILEMAP
#define RR_TILEMAP

// todo move this into world?

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
	//u16 tileSize;
};


static TriangleMesh GenerateMeshForFlatTile(AssetHandler *assetHandler, AABB aabb, Arena* arena)
{
	TriangleMesh ret;
	Material mat;
	mat.bitmapID = RegisterAsset(assetHandler, Asset_Texture, "obj/stone.texture");
	mat.spectularExponent = 96.078431f;
	mat.ambientColor   = V3(1.000000f, 1.000000f, 1.000000f);
	mat.diffuseColor   = V3(0.640000f, 0.640000f, 0.640000f);
	mat.specularColor  = V3(0.500000f, 0.500000f, 0.500000f);
	mat.ke             = V3(0.000000f, 0.000000f, 0.000000f);
	mat.indexOfReflection = 1;
	mat.dissolved = 1.000000;
	mat.illuminationModel = 2;
	

	u32 blackBrown = 0xFFFFFFFF; //RGBAfromHEX(0x553A26);
	u32 grassGreen = 0xFFFFFFFF; //RGBAfromHEX(0x4B6F44);

	u32 meshSize = 9; // @hardcoded

	ret.type = TrianlgeMeshType_Strip;
	u32 amountOfVerticies = meshSize * meshSize; 
	Assert(amountOfVerticies < 65536);
	ret.vertices = PushArray(arena, VertexFormat, amountOfVerticies);

	RandomSeries series = { RandomSeed() };
	
	f32 xFac = (aabb.maxDim.x - aabb.minDim.x) / (meshSize - 1);
	f32 yFac = (aabb.maxDim.y - aabb.minDim.y) / (meshSize - 1);

	f32 meshInv = 1.0f / (f32)(meshSize - 1);
	for (u32 i = 0; i < meshSize; i++)
	{
		f32 percent = (f32)i * meshInv;

		ret.vertices[0 + meshSize * i].p  = V3(0, yFac * i, 0) + aabb.minDim;
		ret.vertices[0 + meshSize * i].c  = blackBrown;
		ret.vertices[0 + meshSize * i].uv = V2(0, percent);
			
		ret.vertices[(meshSize - 1) + meshSize * i].p = V3(xFac * (meshSize - 1), yFac * i, 0) + aabb.minDim;
		ret.vertices[(meshSize - 1) + meshSize * i].c = blackBrown;
		ret.vertices[(meshSize - 1) + meshSize * i].uv = V2(1.0f, percent);
		
		ret.vertices[i + meshSize * (meshSize - 1)].p = V3(xFac * i, yFac * (meshSize - 1), 0) + aabb.minDim;
		ret.vertices[i + meshSize * (meshSize - 1)].c = blackBrown;
		ret.vertices[i + meshSize * (meshSize - 1)].uv = V2(percent, 1.0f);
		
		ret.vertices[i + meshSize * 0].p = V3(xFac * i, 0, 0) + aabb.minDim;
		ret.vertices[i + meshSize * 0].c = blackBrown;
		ret.vertices[i + meshSize * 0].uv = V2(percent, 0.0f);

	}	


	for (u32 x = 1; x < meshSize - 1; x++)
	{
		f32 zVariance = aabb.maxDim.z - aabb.minDim.z;
		for (u32 y = 1; y < meshSize - 1; y++)
		{
			f32 xEntropy = 0.5f * RandomSignedPercent(&series);
			f32 yEntropy = 0.5f * RandomSignedPercent(&series);
			f32 zEntropy = 0.5f * RandomPercent(&series);

			f32 xVal = (xEntropy + (f32)x) * xFac + (aabb.minDim.x);
			f32 yVal = (yEntropy + (f32)y) * yFac + (aabb.minDim.y);
			f32 zVal = zVariance * zEntropy + aabb.minDim.z;

			f32 lerpFactor = zEntropy;

			ret.vertices[x + meshSize * y].p = V3(xVal, yVal, zVal);
			ret.vertices[x + meshSize * y].c = Pack3x8(LerpVector3(Unpack3x8(blackBrown), Unpack3x8(grassGreen), lerpFactor));
			ret.vertices[x + meshSize * y].uv = V2((f32)x * meshInv, (f32)y * meshInv);
		}
	}

	u32 amountOfIndecies = (meshSize - 1) * (3 + 2 * (meshSize - 1));
	ret.indices = PushArray(arena, u16, amountOfIndecies);
	ret.indexSets = PushArray(arena, IndexSet, 1);
	ret.indexSets[0].mat = mat;
	ret.indexSets[0].amount = amountOfIndecies;
	ret.indexSets[0].offset = 0;

	u32 index = 0;

	for (u32 x = 0; x < meshSize - 1; x++)
	{
		ret.indices[index++] = 0xFFFF; // reset Index

		ret.indices[index++] = x * meshSize + 0; // (x, 0)

												  //move up
		for (u32 y = 0; y < meshSize - 1; y++) // 2* (meshSize - 1)
		{
			ret.indices[index++] = (x + 1) * meshSize + y;
			ret.indices[index++] = x       * meshSize + (y + 1);
		}

		ret.indices[index++] = (x + 1) * meshSize + (meshSize - 1);
	}

	Assert(ret.indices.amount == index);

	// calculating vertices
	{
		v3 p = ret.vertices[0 + meshSize * 0].p;

		v3 p1 = ret.vertices[0 + meshSize * 1].p;
		v3 p2 = ret.vertices[1 + meshSize * 1].p;
		v3 p3 = ret.vertices[1 + meshSize * 0].p;
		
		v3 n1 = Normalize(CrossProduct(p1 - p, p2 - p));
		v3 n2 = Normalize(CrossProduct(p2 - p, p3 - p));
		
		ret.vertices[0 + meshSize * 0].n = Normalize((n1 + n2));
	}


	{
		v3 p = ret.vertices[(meshSize - 1) + meshSize * 0].p;

		v3 p3 = ret.vertices[(meshSize - 1) + meshSize * 1].p;
		v3 p2 = ret.vertices[(meshSize - 2) + meshSize * 1].p;
		v3 p1 = ret.vertices[(meshSize - 2) + meshSize * 0].p;

		v3 n1 = Normalize(CrossProduct(p1 - p, p2 - p));
		v3 n2 = Normalize(CrossProduct(p2 - p, p3 - p));

		ret.vertices[(meshSize - 1) + meshSize * 0].n = Normalize((n1 + n2));
	}


	{
		v3 p = ret.vertices[0 + meshSize * (meshSize - 1)].p;

		v3 p3 = ret.vertices[0 + meshSize * (meshSize - 2)].p;
		v3 p2 = ret.vertices[1 + meshSize * (meshSize - 2)].p;
		v3 p1 = ret.vertices[1 + meshSize * (meshSize - 1)].p;

		v3 n1 = Normalize(CrossProduct(p1 - p, p2 - p));
		v3 n2 = Normalize(CrossProduct(p2 - p, p3 - p));

		ret.vertices[0 + meshSize * (meshSize - 1)].n = Normalize((n1 + n2));
	}


	{
		v3 p = ret.vertices[(meshSize - 1) + meshSize * (meshSize - 1)].p;

		v3 p1 = ret.vertices[(meshSize - 1) + meshSize * (meshSize - 2)].p;
		v3 p2 = ret.vertices[(meshSize - 2) + meshSize * (meshSize - 2)].p;
		v3 p3 = ret.vertices[(meshSize - 2) + meshSize * (meshSize - 1)].p;

		v3 n1 = Normalize(CrossProduct(p1 - p, p2 - p));
		v3 n2 = Normalize(CrossProduct(p2 - p, p3 - p));

		ret.vertices[(meshSize - 1) + meshSize * (meshSize - 1)].n = Normalize((n1 + n2));
	}


	for (u32 i = 1; i < meshSize - 1; i++)
	{
		f32 percent = (f32)i * meshInv;

		{
			v3 p = ret.vertices[0 + meshSize * (i + 0)].p;

			//the 5 points top and clockwise
			v3 p1 = ret.vertices[0 + meshSize * (i + 1)].p;
			v3 p2 = ret.vertices[1 + meshSize * (i + 1)].p;
			v3 p3 = ret.vertices[1 + meshSize * (i + 0)].p;
			v3 p4 = ret.vertices[1 + meshSize * (i - 1)].p;
			v3 p5 = ret.vertices[0 + meshSize * (i - 1)].p;

			//the 4 triangle vertices
			v3 n1 = Normalize(CrossProduct(p1 - p, p2 - p));
			v3 n2 = Normalize(CrossProduct(p2 - p, p3 - p));
			v3 n3 = Normalize(CrossProduct(p3 - p, p4 - p));
			v3 n4 = Normalize(CrossProduct(p4 - p, p5 - p));

			ret.vertices[0 + meshSize * i].n = Normalize((n1 + n2 + n3 + n4));

		}

		{
			v3 p = ret.vertices[(meshSize - 1) + meshSize * (i + 0)].p;

			//the 5 points top and clockwise
			v3 p1 = ret.vertices[((meshSize - 1) + 0) + meshSize * (i + 1)].p;
			v3 p5 = ret.vertices[((meshSize - 1) + 0) + meshSize * (i - 1)].p;
			v3 p6 = ret.vertices[((meshSize - 1) - 1) + meshSize * (i - 1)].p;
			v3 p7 = ret.vertices[((meshSize - 1) - 1) + meshSize * (i + 0)].p;
			v3 p8 = ret.vertices[((meshSize - 1) - 1) + meshSize * (i + 1)].p;


			//the 4 triangle vertices
			v3 n5 = Normalize(CrossProduct(p5 - p, p6 - p));
			v3 n6 = Normalize(CrossProduct(p6 - p, p7 - p));
			v3 n7 = Normalize(CrossProduct(p7 - p, p8 - p));
			v3 n8 = Normalize(CrossProduct(p8 - p, p1 - p));

			ret.vertices[(meshSize - 1) + meshSize * i].n = Normalize((n5 + n6 + n7 + n8));
		}
		
		

		{
			v3 p = ret.vertices[(i + 0) + meshSize * (meshSize - 1)].p;

			//the 8 points top and clockwise
			v3 p3 = ret.vertices[(i + 1) + meshSize * (meshSize - 1 + 0)].p;
			v3 p4 = ret.vertices[(i + 1) + meshSize * (meshSize - 1 - 1)].p;
			v3 p5 = ret.vertices[(i + 0) + meshSize * (meshSize - 1 - 1)].p;
			v3 p6 = ret.vertices[(i - 1) + meshSize * (meshSize - 1 - 1)].p;
			v3 p7 = ret.vertices[(i - 1) + meshSize * (meshSize - 1 + 0)].p;
			

			//the 8 triangle normals
			v3 n3 = Normalize(CrossProduct(p3 - p, p4 - p));
			v3 n4 = Normalize(CrossProduct(p4 - p, p5 - p));
			v3 n5 = Normalize(CrossProduct(p5 - p, p6 - p));
			v3 n6 = Normalize(CrossProduct(p6 - p, p7 - p));
			
			ret.vertices[i + meshSize * (meshSize - 1)].n = Normalize((n3 + n4 + n5 + n6));
		}
		


		{
			v3 p = ret.vertices[(i + 0) + meshSize * 0].p;

			//the 5 points top and clockwise
			v3 p1 = ret.vertices[(i + 0) + meshSize * (0 + 1)].p;
			v3 p2 = ret.vertices[(i + 1) + meshSize * (0 + 1)].p;
			v3 p3 = ret.vertices[(i + 1) + meshSize * (0 + 0)].p;
			v3 p7 = ret.vertices[(i - 1) + meshSize * (0 + 0)].p;
			v3 p8 = ret.vertices[(i - 1) + meshSize * (0 + 1)].p;


			//the 4 triangle normals
			v3 n1 = Normalize(CrossProduct(p1 - p, p2 - p));
			v3 n2 = Normalize(CrossProduct(p2 - p, p3 - p));
			v3 n7 = Normalize(CrossProduct(p7 - p, p8 - p));
			v3 n8 = Normalize(CrossProduct(p8 - p, p1 - p));

			ret.vertices[i + meshSize * 0].n = Normalize((n1 + n2 + n7 + n8));
		}
	}

	for (u32 x = 1; x < meshSize - 1; x++)
	{
		for (u32 y = 1; y < meshSize - 1; y++)
		{
			v3 p = ret.vertices[(x + 0) + meshSize * (y + 0)].p;

			//the 8 points top and clockwise
			v3 p1 = ret.vertices[(x + 0) + meshSize * (y + 1)].p;
			v3 p2 = ret.vertices[(x + 1) + meshSize * (y + 1)].p;
			v3 p3 = ret.vertices[(x + 1) + meshSize * (y + 0)].p;
			v3 p4 = ret.vertices[(x + 1) + meshSize * (y - 1)].p;
			v3 p5 = ret.vertices[(x + 0) + meshSize * (y - 1)].p;
			v3 p6 = ret.vertices[(x - 1) + meshSize * (y - 1)].p;
			v3 p7 = ret.vertices[(x - 1) + meshSize * (y + 0)].p;
			v3 p8 = ret.vertices[(x - 1) + meshSize * (y + 1)].p;


			//the 8 triangle normals
			v3 n1 = Normalize(CrossProduct(p1 - p, p2 - p));
			v3 n2 = Normalize(CrossProduct(p2 - p, p3 - p));
			v3 n3 = Normalize(CrossProduct(p3 - p, p4 - p));
			v3 n4 = Normalize(CrossProduct(p4 - p, p5 - p));
			v3 n5 = Normalize(CrossProduct(p5 - p, p6 - p));
			v3 n6 = Normalize(CrossProduct(p6 - p, p7 - p));
			v3 n7 = Normalize(CrossProduct(p7 - p, p8 - p));
			v3 n8 = Normalize(CrossProduct(p8 - p, p1 - p));


			ret.vertices[x + meshSize * y].n = Normalize((n1 + n2 + n3 + n4 + n5 + n6 + n7 + n8));
		}
	}


	RegisterTriangleMesh(&ret);

	return ret;
}



static TileMap InitTileMap(AssetHandler *assetHandler, u32 width, u32 height, Arena *arena)
{
	TileMap ret;
	ret.width = width;
	ret.height = height;

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
			//if (tempTile->tag)
			{
				tempTile->mesh = GenerateMeshForFlatTile(assetHandler, aabb, arena);
			}
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

	Die;

	File file = LoadFile(fileName);
	void * memory = file.memory;
	TileMapHeader *tileMapHeader = (TileMapHeader *)memory;

	ret.height = tileMapHeader->height;
	ret.width = tileMapHeader->width;

	u8 *buffer = (u8 *)memory;
	buffer += tileMapHeader->headerSize;

	ret.tiles = PushData(arena, Tile, ret.width * ret.height);;
	for (unsigned int x = 0; x < ret.width; x++)
	{
		for (unsigned int y = 0; y < ret.height; y++)
		{
			TileReader *tr = (TileReader *)buffer;
			buffer += sizeof(TileReader);
			Tile *tile = ret.tiles + x + ret.width * y;

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
	v2 halfTileDimension = V2(1.0f / 2.0f, 1.0f / 2.0f);
	v2 minVector = tile.pos - halfTileDimension;
	v2 maxVector = tile.pos + halfTileDimension;

	float ret = 1.0f;

	LinearIntersectionOneDim(&ret, speedVector.x, speedVector.y, minVector.x, pos.x, pos.y, minVector.y, maxVector.y);
	LinearIntersectionOneDim(&ret, speedVector.x, speedVector.y, maxVector.x, pos.x, pos.y, minVector.y, maxVector.y);
	LinearIntersectionOneDim(&ret, speedVector.y, speedVector.x, minVector.y, pos.y, pos.x, minVector.x, maxVector.x);
	LinearIntersectionOneDim(&ret, speedVector.y, speedVector.x, maxVector.y, pos.y, pos.x, minVector.x, maxVector.x);

	return ret;
}

/*
static Tile* RectIsCollidingWithTileMap(TileMap *tileMap, rr::Rectangle *rect)
{
unsigned int xmin = (int)rect->GetPos().x / tileMap->tileSize - 1;
if (xmin < 0) xmin = 0;
unsigned int ymin = (int)rect->GetPos().y / tileMap->tileSize - 1;
if (ymin < 0) ymin = 0;

unsigned int xmax = (int)rect->GetPos().x / tileMap->tileSize + 1;
if (xmax > tileMap->width) xmax = tileMap->width;
unsigned int ymax = (int)rect->GetPos().y / tileMap->tileSize + 1;
if (ymax > tileMap->height) ymax = tileMap->height;


for (unsigned int x = xmin; x <= xmax; x++)
{
for (unsigned int y = ymin; y <= ymax; y++)
{

Tile *tile = &tileMap->tiles[x][y];
//tile->type = 2;
if ((tile->type == 0) || (tile->type == 3))
{
tile->type = 3;

rr::Rectangle tileRect = rr::Rectangle(tile->pos, tileMap->tileSize, tileMap->tileSize);
if (rr::Rectangle::Colliding(*rect, tileRect))
return tile;
}
}
}
return NULL;
}*/
/*
struct RelativeTile
{
Tile *tile;
float lengthFromPos;
RelativeTile *prev;
};
static bool FindTileInVector(std::vector<RelativeTile*> *tileVector, Tile* tile)
{
for (unsigned int i = 0; i < tileVector->size(); i++)
if (tile == (*tileVector)[i]->tile)
return true;
return false;
}
static bool FindV3InVector(std::vector<v2> *v3Vector, v2 v)
{
for (unsigned int i = 0; i < v3Vector->size(); i++)
if (v == (*v3Vector)[i])
return true;
return false;
}
static void AStarAlgorithm(std::vector<v2> *outputWay, v2 goalVector, v2 position, TileMap *tileMap)
{
outputWay->clear();
//find Way
std::vector<v2> closedSet;
std::vector<RelativeTile*> openSet;

RelativeTile goal;
goal.tile = tileMap->GetTile(goalVector);

RelativeTile start;
start.tile = tileMap->GetTile(position);
start.lengthFromPos = 0;
start.prev = NULL;
openSet.push_back(&start);

while (!openSet.empty())
{
RelativeTile* current = openSet.front();
std::vector<RelativeTile*>::iterator cur = openSet.begin();
for (std::vector<RelativeTile*>::iterator i = openSet.begin(); i != openSet.end(); i++)
{
if (current->lengthFromPos + v2::Dist(current->tile->pos, goal.tile->pos) > (*i)->lengthFromPos + v2::Dist((*i)->tile->pos, goal.tile->pos))
{
current = *i;
cur = i;
}
}
if (current->tile == goal.tile)
{
//find way back
while (current)
{
outputWay->push_back(current->tile->pos);
current = current->prev;
}

break;
}
else
{

v2 currPos = current->tile->pos;


openSet.erase(cur);
closedSet.push_back(currPos);

std::vector<Tile*> neighboors;
neighboors.push_back(tileMap->GetTile(currPos + v2(tileMap->tileSize, 0)));
neighboors.push_back(tileMap->GetTile(currPos + v2(-tileMap->tileSize, 0)));
neighboors.push_back(tileMap->GetTile(currPos + v2(0, tileMap->tileSize)));
neighboors.push_back(tileMap->GetTile(currPos + v2(0, -tileMap->tileSize)));

for (std::vector<Tile*>::iterator i = neighboors.begin(); i != neighboors.end(); i++)
{
if ((*i)->type != 0)
{
if (!(FindV3InVector(&closedSet, (*i)->pos)))
{
if (!(FindTileInVector(&openSet, *i)))
{
RelativeTile *neighboorTile = new RelativeTile;
neighboorTile->tile = *i;
neighboorTile->lengthFromPos = current->lengthFromPos + 1;
neighboorTile->prev = current;
openSet.push_back(neighboorTile);
}
}
}
}
}
}
}
*/


#endif