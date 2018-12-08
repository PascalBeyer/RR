#ifndef RR_WORLD
#define RR_WORLD

struct AABB
{
	v3 minDim;
	v3 maxDim;
};


static AABB CreateAABB(v3 minDim, v3 maxDim)
{
	// todo assert if we have volume?

	return { minDim, maxDim };
}

static AABB InvertedInfinityAABB()
{
	AABB ret;
	ret.maxDim = V3(MINF32, MINF32, MINF32);
	ret.minDim = V3(MAXF32, MAXF32, MAXF32);
	return ret;
}

static b32 PointInAABB(AABB target, v3 point)
{
	return 
		(
			target.minDim.x <= point.x + 0.001f &&
			target.minDim.y <= point.y + 0.001f &&
			target.minDim.z <= point.z + 0.001f &&
			target.maxDim.x + 0.001f >= point.x &&
			target.maxDim.y + 0.001f >= point.y &&
			target.maxDim.z + 0.001f >= point.z
		);
}
//Righthanded coordinates, +Z going into the screen, +Y is "down" ? , +X is to the right
//todo just store the projection matrix instead of aspectratio and focal length?
struct Camera
{
	v3 pos;
	f32 aspectRatio;
	f32 focalLength;

	union
	{
		Vector3Basis basis;

		struct 
		{
			v3 b1;
			v3 b2;
			v3 b3;
		};
	};
};


static v2 ScreenZeroToOneToInGame(Camera cam, v2 point)
{

	m4x4 proj = Projection(cam.aspectRatio, cam.focalLength) * CameraTransform(cam.basis.d1, cam.basis.d2, cam.basis.d3, cam.pos);
	m4x4 inv = InvOrId(proj);

	v3 p1 = inv * V3(-1, -1, -1);
	v3 p2 = inv * V3(1, -1, -1);
	v3 p3 = inv * V3(-1, 1, -1);
	//v3 p4 = inv * V3(1, 1, -1);

	f32 posXinGame = point.x;
	f32 posYinGame =  1.0f - point.y;

	// this works as the screen in game is again a Rectangle
	v3 inGameP = p1 + posXinGame * (p2 - p1) + posYinGame * (p3 - p1);

	float ctPos = -cam.pos.z / (inGameP.z - cam.pos.z);
	return p12(ctPos * (inGameP - cam.pos) + cam.pos);
}

static v3 ScreenZeroToOneToScreenInGame(Camera cam, v2 point)
{

	m4x4 proj = Projection(cam.aspectRatio, cam.focalLength) * CameraTransform(cam.basis.d1, cam.basis.d2, cam.basis.d3, cam.pos);
	m4x4 inv = InvOrId(proj);

	v3 p1 = inv * V3(-1, -1, -1);
	v3 p2 = inv * V3(1, -1, -1);
	v3 p3 = inv * V3(-1, 1, -1);
	//v3 p4 = inv * V3(1, 1, -1);

	f32 posXinGame = point.x;
	f32 posYinGame = 1.0f - point.y;

	// this works as the screen in game is again a Rectangle
	v3 inGameP = p1 + posXinGame * (p2 - p1) + posYinGame * (p3 - p1);

	return inGameP;
}

static v3 ScreenZeroToOneToDirecion(Camera cam, v2 point)
{
	m4x4 proj = Projection(cam.aspectRatio, cam.focalLength) * CameraTransform(cam.basis.d1, cam.basis.d2, cam.basis.d3, cam.pos);
	m4x4 inv = InvOrId(proj);

	v3 p1 = inv * V3(-1, -1, -1);
	v3 p2 = inv * V3(1, -1, -1);
	v3 p3 = inv * V3(-1, 1, -1);
	v3 p4 = inv * V3(1, 1, -1);

	f32 posXinGame = point.x;
	f32 posYinGame = 1.0f - point.y;

	v3 inGameP = p1 + posXinGame * (p2 - p1) + posYinGame * (p3 - p1);

	return (inGameP - cam.pos);
}


struct Triangle
{
	union 
	{

		struct
		{
			ColoredVertex cv1;
			ColoredVertex cv2;
			ColoredVertex cv3;
		};
		struct
		{
			v3 p1;
			u32 c1;
			v3 p2;
			u32 c2;
			v3 p3;
			u32 c3;
		};
		
	};
};

DefineArray(Triangle);

struct VertexFormat
{
	v3 p;
	v2 uv;
	v3 n;
	u32 c;
};

enum TriangleMeshType
{
	TriangleMeshType_List,
	TrianlgeMeshType_Strip,
};

struct Material
{
	f32 spectularExponent;		// Ns
	v3 ambientColor;			// Ka
	v3 diffuseColor;			// Kd
	v3 specularColor;			// Ks
	v3 ke;						// not in the spec and always (0,0,0)

	f32 indexOfReflection;		// Ni ("optical density")	0.001 - 10
	f32 dissolved;				// d						0 - 1	
								//v3 transmissionFilter;		// Tf						
	u32 illuminationModel;		// illum					0 - 10
	u32 bitmapID;
	String name;
	String texturePath;
};

DefineDynamicArray(Material);

struct IndexSet
{
	u32 amount;
	u32 offset;

	Material mat; 
};

DefineArray(IndexSet);
DefineArray(VertexFormat);
DefineDynamicArray(IndexSet);

struct TriangleMesh
{
	u32 type;
	VertexFormatArray vertices;
	u16Array indices;
	IndexSetArray indexSets;

	AABB aabb;
	
	u32 vertexVBO; // to init this call glBufferData and glBindData
	u32 indexVBO;
};

DefineArray(TriangleMesh);
DefineDynamicArray(TriangleMesh);

void RegisterTriangleMesh(TriangleMesh *mesh);

struct LightingSolution
{
	struct IrradianceCache *cache;
	struct KdNode *kdTree;
	struct LightingTriangle *lightingTriangles;
	u32 amountOfTriangles;

};

enum TileMapType
{
	Tile_Empty,
	Tile_Spawner,
	Tile_Goal,
	Tile_Blocked,
};

struct Tile
{
	TileMapType type;
	u32 meshIndex;
};

DefineArray(Tile);

struct TileMap
{
	Tile *tiles;
	u32 height, width;
	//u16 tileSize;
};

struct PlacedMesh
{
	u32 meshId;
	f32 scale;
	Quaternion orientation;
	v3 pos;
	v4 color;
	v4 frameColor;
	AABB untransformedAABB;
};

DefineDynamicArray(PlacedMesh);

static PlacedMesh CreatePlacedMesh(u32 meshID, f32 scale, Quaternion orientation, v3 pos, v4 color, AABB aabb)
{
	PlacedMesh ret;
	ret.meshId = meshID;
	ret.scale = scale;
	ret.orientation = orientation;
	ret.pos = pos;
	ret.color = color;
	ret.frameColor = V4(1, 1, 1, 1);
	ret.untransformedAABB = aabb;
	return ret;
};

struct World
{
	LightingSolution light;
	TriangleArray triangles;
	Camera camera;
	Camera debugCamera;
	v3 lightSource;
	PlacedMeshDynamicArray placedMeshes;

	TileMap tileMap;
};


static bool InBounds(TileMap tileMap, v2 vec)
{
	return (0.0f <= vec.x + 0.5f) && (vec.x + 0.5f < (f32)tileMap.width) && (0.0f <= vec.y + 0.5f) && (vec.y + 0.5f < (f32)tileMap.height);
}

static Tile* GetTile(TileMap tileMap, v2 pos)
{
	if (!InBounds(tileMap, pos))
	{
		return NULL;
	}

	u32 x = (u32)(pos.x + 0.5f);
	u32 y = (u32)(pos.y + 0.5f);

	return (tileMap.tiles + x + y * tileMap.width);
}


static bool InBounds(TileMap tileMap, v2i vec)
{
	return (0 <= vec.x) && (vec.x < (i32)tileMap.width) && (0 <= vec.y) && (vec.y < (i32)tileMap.height);
}


static Tile* GetTile(TileMap tileMap, v2i pos)
{
	if (!InBounds(tileMap, pos))
	{
		return NULL;
	}

	u32 x = pos.x;
	u32 y = pos.y;

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



struct RockCorner
{
	u32 color;
	v3 p;
	union
	{
		RockCorner *adjacentCorners[2];
		struct
		{
			RockCorner *prev;
			RockCorner *next;
		};
	};

	
};

struct RockCornerListElement
{
	RockCorner data;
	RockCornerListElement *next;

};

struct RockCornerList
{
	RockCornerListElement *list;
	RockCornerListElement *freeList;
};

static u32 globalCornerIndex = 0;

inline bool v3InAABB(v3 pos, AABB aabb)
{
	return
		aabb.minDim.x <= pos.x &&
		aabb.minDim.y <= pos.y &&
		aabb.minDim.z <= pos.z &&
		aabb.maxDim.x >= pos.x &&
		aabb.maxDim.y >= pos.y &&
		aabb.maxDim.z >= pos.z;
}

static u32 GetCubeNormalIndex(v3 normal)
{
	if (normal.x)
	{
		Assert(!normal.y & !normal.z);
		return (normal.x > 0) ? 0u : 1u;

	}
	if (normal.y)
	{
		Assert(!normal.x & !normal.z);
		return (normal.y > 0) ? 2u : 3u;
	}
	if (normal.z)
	{
		Assert(!normal.x & !normal.y);
		return (normal.z > 0) ? 4u : 5u;
	}
	else
	{
		Assert(!"invalide normal");
		return MAXU32;
	}
}

static v3 GetCubeNormalFromIndex(u32 index)
{
	switch (index)
	{
	case 0:
	{
		return V3(1, 0, 0);
	}break;
	case 1:
	{
		return V3(-1, 0, 0);
	}break;
	case 2:
	{
		return V3(0, 1, 0);
	}break;
	case 3:
	{
		return V3(0, -1, 0);
	}break;
	case 4:
	{
		return V3(0, 0, 1);
	}break;
	case 5:
	{
		return V3(0, 0, -1);
	}break;
	default:
	{

		Assert(!"invalideCubeNormalIndex");
		return V3();
	}break;
	}

}

static u32 GrayFromU32(u32 i)
{
	u32 c = i & 0xFF;
	u32 ret = (0xFF << 24) | (c << 16) | (c << 8) | (c << 0);
	return ret;
}


static RockCorner CreateRockCorner(v3 p, u32 colorSeed)
{
	RockCorner ret;
	ret.p = p;
	ret.color = GrayFromU32(colorSeed);

	//ret.prev = NULL;
	//ret.next = NULL;
	return ret;
}

static void ColorForTileMap(World *world)
{
	v3 screenPos = world->camera.pos;

	v2 screenUL = ScreenZeroToOneToInGame(world->camera, V2(0.0f, 0.0f));
	v2 screenUR = ScreenZeroToOneToInGame(world->camera, V2(1.0f, 0.0f));
	v2 screenBL = ScreenZeroToOneToInGame(world->camera, V2(0.0f, 1.0f));
	v2 screenBR = ScreenZeroToOneToInGame(world->camera, V2(1.0f, 1.0f));

	float minScreenX = Min(Min(screenUL.x, screenUR.x), Min(screenBL.x, screenBR.x));
	float minScreenY = Min(Min(screenUL.y, screenUR.y), Min(screenBL.y, screenBR.y));

	float maxScreenX = Max(Max(screenUL.x, screenUR.x), Max(screenBL.x, screenBR.x));
	float maxScreenY = Max(Max(screenUL.y, screenUR.y), Max(screenBL.y, screenBR.y));

	u32 minScreenXi = (u32)Max((int)floorf(minScreenX) - 1.0f, 0.0f);
	u32 minScreenYi = (u32)Max((int)floorf(minScreenY) - 1.0f, 0.0f);

	u32 maxScreenXi = (u32)Min((int)ceilf(maxScreenX) + 1.0f, (float)world->tileMap.width);
	u32 maxScreenYi = (u32)Min((int)ceilf(maxScreenY) + 1.0f, (float)world->tileMap.height);

	Tweekable(b32, DrawWholeMap);
	if (DrawWholeMap)
	{
		minScreenXi = 0;
		minScreenYi = 0;
		maxScreenXi = world->tileMap.width;
		maxScreenYi = world->tileMap.height;
	}
	for (u32 x = minScreenXi; x < maxScreenXi; x++)
	{
		for (u32 y = minScreenYi; y < maxScreenYi; y++)
		{
			Tile *tile = GetTile(world->tileMap, V2(x, y));
			
			v4 slightlyRed = V4(1.0f, 1.0f, 0.55f, 0.55f);
			v4 slightlyGreen = V4(1.0f, 0.55f, 1.0f, 0.55f);
			v4 slightlyYellow = V4(1.0f, 1.0f, 1.0f, 0.55f);
			v4 slightlyBlue = V4(1.0f, 0.55f, 0.55f, 1.0f);

			if (tile)
			{
				switch (tile->type)
				{
				case Tile_Blocked:
				{
					world->placedMeshes[tile->meshIndex].frameColor *= slightlyRed;
				}break;
				case Tile_Empty:
				{
					world->placedMeshes[tile->meshIndex].frameColor *= slightlyGreen;
				}break;
				case Tile_Goal:
				{
					world->placedMeshes[tile->meshIndex].frameColor *= slightlyYellow;
				}break;
				case Tile_Spawner:
				{
					world->placedMeshes[tile->meshIndex].frameColor *= slightlyBlue;
				}break;

				}
			}
		}
	}
}


static void UpdateCamGame(Input *input, Camera *camera)
{
	Tweekable(f32, screenScrollBorder, 0.05f);
	Tweekable(f32, screenScrollSpeed, 1.0f);

	v2 asd = input->mouseZeroToOne - V2(0.5f, 0.5f);

	f32 boxNorm = BoxNorm(asd) + screenScrollBorder;

	if (boxNorm > 0.5f)
	{
		camera->pos += 2.0f * boxNorm * screenScrollSpeed * i12(asd);
	}

}

static void UpdateCamFocus(Input *input, Camera *camera, DEBUGKeyTracker *tracker)
{
	v3 focusPoint = V3(ScreenZeroToOneToInGame(*camera, V2(0.5f, 0.5f)), 0.0f);

	f32 factor = 1.0f + tracker->middleBack * 0.1f - tracker->middleForward * 0.1f;

	tracker->middleForward = false;
	tracker->middleBack = false;

	camera->pos = focusPoint + factor * (camera->pos - focusPoint);

	if (!tracker->middleDown)
	{
		return;
		
	}

	v2 mouseDelta = input->mouseDelta;
	
	float rotSpeed = 0.001f * 3.141592f;
	
	f32 mouseZRot = -mouseDelta.y * rotSpeed; // this should rotate around the z axis
	f32 mouseCXRot = mouseDelta.x * rotSpeed; // this should rotate around the camera x axis

	m3x3 cameraT = Rows3x3(camera->b1, camera->b2, camera->b3);
	m3x3 cameraTInv = Invert(cameraT);

	m3x3 id = cameraT * cameraTInv;
	m3x3 rotX = XRotation3x3(mouseZRot);
	m3x3 rotZ = ZRotation3x3(mouseCXRot);
	m3x3 rot = cameraTInv * rotX * cameraT * rotZ;

	v3 delta = camera->pos - focusPoint;

	camera->pos = focusPoint + rot * delta;

	camera->basis = TransformBasis(camera->basis, rot);
}

static void UpdateCamGodMode(Input *input, Camera *cam, DEBUGKeyTracker tracker)
{
	Tweekable(f32, cameraMoveSpeed);
	//GET_MACRO3(f32, cameraMoveSpeed, Tweekable1, Tweekable2)(f32, cameraMoveSpeed);
	//Tweekable2(f32, cameraMoveSpeed);
#if 0
	if (input->keybord[Key_shift].flag & KeyState_Down)
	{
		moveSpeed = 0.25f;
	}
#endif

	if (tracker.wDown)
	{
		cam->pos += cam->basis.d3 * cameraMoveSpeed;
	}
	if (tracker.sDown)
	{
		cam->pos -= cam->basis.d3 * cameraMoveSpeed;
	}
	if (tracker.dDown)
	{
		cam->pos += cam->basis.d1 * cameraMoveSpeed;
	}
	if (tracker.aDown)
	{
		cam->pos -= cam->basis.d1 * cameraMoveSpeed;
	}

	if (tracker.spaceDown)
	{
		float rotSpeed = 0.001f * 3.141592f;
		m4x4 rot = XRotation(-rotSpeed * input->mouseDelta.y) * YRotation(rotSpeed * input->mouseDelta.x);

		m4x4 cameraMat = CamBasisToMat(cam->basis);
		m4x4 invCamMat = InvOrId(cameraMat);

		cam->basis = TransformBasis(cam->basis, cameraMat * rot * invCamMat);
	}

	
}

static v3 AABBCorner(AABB aabb, u32 index)
{
	Assert(index < 8);

	v3 corners[8]
	{
		aabb.minDim,
		V3(aabb.maxDim.x, p23(aabb.minDim)),
		V3(p12(aabb.minDim), aabb.maxDim.z),
		V3(aabb.minDim.x, aabb.maxDim.y, aabb.minDim.z),

		aabb.maxDim,
		V3(aabb.minDim.x, p23(aabb.maxDim)),
		V3(p12(aabb.maxDim), aabb.minDim.z),
		V3(aabb.maxDim.x, aabb.minDim.y, aabb.maxDim.z)
	};


	return corners[index];
}

static void PushTriangleToArena(Arena *arena, v3 p1, v3 p2, v3 p3, u32 c1, u32 c2, u32 c3, v3 normal)
{
	Triangle *tri = PushStruct(arena, Triangle);
	tri->p1 = p1;
	tri->c1 = c1;
	tri->p2 = p2;
	tri->c2 = c2;
	tri->p3 = p3;
	tri->c3 = c3;
}
static void PushTriangleToArena(Arena *arena, ColoredVertex cv1, ColoredVertex cv2, ColoredVertex cv3, v3 normal)
{
	Triangle *tri = PushStruct(arena, Triangle);
	tri->cv1 = cv1;
	tri->cv2 = cv2;
	tri->cv3 = cv3;
}
static void PushTriangleToArena(Arena *arena, v3 p1, v3 p2, v3 p3, u32 c, v3 normal)
{
	PushTriangleToArena(arena, p1, p2, p3, c, c, c, normal);
}
static void PushTriangleToArenaIntendedNormal(Arena *arena, v3 p1, v3 p2, v3 p3, u32 c1, u32 c2, u32 c3, v3 intendedPosNormalDot)
{
	v3 proposedNormal = Normalize(CrossProduct(p2 - p1, p3 - p1));
	//v3 normal = Dot(intendedPosNormal, proposedNormal) > 0 ? proposedNormal : -proposedNormal;
	Assert(proposedNormal != V3());
	if (Dot(intendedPosNormalDot, proposedNormal) > 0)
	{
		PushTriangleToArena(arena, p1, p2, p3, c1, c2, c3, proposedNormal);
	}
	else
	{
		//adjust for face culling
		PushTriangleToArena(arena, p1, p3, p2, c1, c3, c2, -proposedNormal);
	}
}
static void PushTriangleToArenaIntendedNormal(Arena *arena, v3 p1, v3 p2, v3 p3, u32 c, v3 intendedPosNormalDot)
{
	v3 proposedNormal = Normalize(CrossProduct(p2 - p1, p3 - p1));
	//v3 normal = Dot(intendedPosNormal, proposedNormal) > 0 ? proposedNormal : -proposedNormal;
	Assert(proposedNormal != V3());
	if (Dot(intendedPosNormalDot, proposedNormal) > 0)
	{
		PushTriangleToArena(arena, p1, p2, p3, c, c, c, proposedNormal);
	}
	else
	{
		//adjust for face culling
		PushTriangleToArena(arena, p1, p3, p2, c, c, c, -proposedNormal);
	}
}

static void PushTriangleToArenaIntendedNormal(Arena *arena, ColoredVertex cv1, ColoredVertex cv2, ColoredVertex cv3, v3 intendedPosNormal)
{
	PushTriangleToArenaIntendedNormal(arena, cv1.pos, cv2.pos, cv3.pos, cv1.color, cv2.color, cv3.color, intendedPosNormal);
}
static void PushTriangleToArena(Arena *arena, v3 p1, v3 p2, v3 p3, v3 normal)
{
	u32 c = 0xFFFFFFFF;
	PushTriangleToArena(arena, p1, p2, p3, c, c, c, normal);
}

static void SetNeighborPointerToNULL(RockCorner **neighbors, RockCorner *corner)
{

	for (u32 j = 0; j < 3; j++)
	{
		if (!neighbors[j])
		{
			continue;
		}
		RockCorner **arr = neighbors[j]->adjacentCorners;

		for (u32 i = 0; i < 3; i++)
		{
			if (arr[i] == corner)
			{
				arr[i] = NULL;
			}
		}
	}
}


static u32 faculty(u32 n)
{
	u32 ret = 1;
	for (u32 i = 1; i <= n; i++)
	{
		ret *= i;
	}
	return ret;
}

static u32 Over(u32 n, u32 k)
{
	Assert(n >= k);
	u32 ret = faculty(n) / (faculty(k) * faculty(n - k));
	return ret;
}

static TriangleArray CreateSkyBoxAndPush(AABB aabb, u32 color, Arena *arena)
{
	TriangleArray ret;
	ret.data = PushData(arena, Triangle, 0);
	ret.amount = 12;
	v3 d1 = V3(aabb.maxDim.x - aabb.minDim.x, V2());
	v3 d2 = V3(0.0f, aabb.maxDim.y - aabb.minDim.y, 0.0f);
	v3 d3 = V3(V2(), aabb.maxDim.z - aabb.minDim.z);

	v3 p = aabb.minDim;

	v3 p1 = p + d1;
	v3 p2 = p + d2;
	v3 p3 = p + d3;

	v3 p12 = p + d1 + d2;
	v3 p13 = p + d1 + d3;
	v3 p23 = p + d2 + d3;

	v3 p123 = p + d1 + d2 + d3;
	RandomSeries series = {RandomSeed()};

	u32 randomGray = GrayFromU32(RandomU32(&series) % 15);
	PushTriangleToArenaIntendedNormal(arena, p, p1, p2, color + randomGray, V3(0, 0, 1)); //front
	PushTriangleToArenaIntendedNormal(arena, p12, p1, p2, color + randomGray, V3(0, 0, 1));

	randomGray = GrayFromU32(RandomU32(&series) % 15);
	PushTriangleToArenaIntendedNormal(arena, p, p1, p3, color + randomGray, V3(0, 1, 0));//bottom
	PushTriangleToArenaIntendedNormal(arena, p13, p1, p3, color + randomGray, V3(0, 1, 0));

	randomGray = GrayFromU32(RandomU32(&series) % 15);
	PushTriangleToArenaIntendedNormal(arena, p, p2, p3, color + randomGray, V3(1, 0, 0)); //left
	PushTriangleToArenaIntendedNormal(arena, p23, p2, p3, color + randomGray, V3(1, 0, 0));

	randomGray = GrayFromU32(RandomU32(&series) % 15);
	PushTriangleToArenaIntendedNormal(arena, p3, p13, p23, color + randomGray, V3(0, 0, -1)); //back
	PushTriangleToArenaIntendedNormal(arena, p123, p13, p23, color + randomGray, V3(0, 0, -1));

	randomGray = GrayFromU32(RandomU32(&series) % 15);
	PushTriangleToArenaIntendedNormal(arena, p2, p12, p23, color + randomGray, V3(0, -1, 0)); //top
	PushTriangleToArenaIntendedNormal(arena, p123, p12, p23, color + randomGray, V3(0, -1, 0));

	randomGray = GrayFromU32(RandomU32(&series) % 15);
	PushTriangleToArenaIntendedNormal(arena, p1, p12, p13, color + randomGray, V3(-1, 0, 0)); //right
	PushTriangleToArenaIntendedNormal(arena, p123, p12, p13, color + randomGray, V3(-1, 0, 0));

	return ret;

}

static TriangleArray CreateStoneAndPush(AABB aabb, f32 desiredVolume, Arena *arena, u32 iterations, Arena *workingArena)
{
	Clear(workingArena);

	TriangleArray ret = PushArray(arena, Triangle, 0);

	v3 aabbMidPoint = 0.5f * (aabb.minDim + aabb.maxDim);

	const u32 logAmountOfSamples = 6; 
	const u32 amountOfRings = logAmountOfSamples + 1;

	RandomSeries series = { RandomSeed() };

	RockCorner *rings[amountOfRings];
	rings[0] = PushStruct(workingArena, RockCorner);
	*rings[0] = CreateRockCorner(V3(aabb.minDim.x, 0.5f * p23(aabb.maxDim + aabb.minDim)), RandomU32(&series) % 10 + 40);
	
	rings[amountOfRings - 1] = PushStruct(workingArena, RockCorner);
	*rings[amountOfRings - 1] = CreateRockCorner(V3(aabb.maxDim.x, 0.5f * p23(aabb.maxDim + aabb.minDim)), RandomU32(&series) % 10 + 40);


	for (u32 ringIndex = 1; ringIndex < logAmountOfSamples; ringIndex++)
	{
		u32 amountOfSamples = Over(logAmountOfSamples, ringIndex);
		f32 xPos = aabb.minDim.x + ((aabb.maxDim.x - aabb.minDim.x) / (f32)logAmountOfSamples) * ringIndex;
		RockCorner *lastCorner = NULL;
		for (u32 angleIndex = 0; angleIndex < amountOfSamples; angleIndex++)
		{
			f32 angle = ((f32)angleIndex) * 2.0f * PI / (f32)amountOfSamples;

			f32 xPercent = ringIndex / (f32)amountOfRings;

			f32 a = xPercent * (aabb.maxDim.y - aabb.minDim.y) * 0.5f; //these should be on an elipse, but what evs
			f32 b = xPercent * (aabb.maxDim.z - aabb.minDim.z) * 0.5f;;
			
			f32 sin = Sin(angle);
			f32 cos = Cos(angle);

			f32 r = a * b / (Sqrt(Square(a * sin) + Square(b *cos)));

			f32 yVal = r * cos + (aabb.maxDim.y + aabb.minDim.y) * 0.5f;
			f32 zVal = r * sin + (aabb.maxDim.z + aabb.minDim.z) * 0.5f;

			RockCorner *c = PushStruct(workingArena, RockCorner);
			*c = CreateRockCorner(V3(xPos, yVal, zVal), RandomU32(&series) % 10 + 40);
			c->prev = lastCorner;
			lastCorner = c;
		}

		RockCorner *next = lastCorner;
		while (next->prev)
		{
			next->prev->next = next;
			next = next->prev;
		}
		next->prev = lastCorner;
		lastCorner->next = next;

		rings[ringIndex] = lastCorner;
	}

	for (u32 ringIndex = 1; ringIndex < amountOfRings - 2; ringIndex++) // does not deal with the corner points
	{
		RockCorner *leftCorner = rings[ringIndex];
		RockCorner *rightCorner = rings[ringIndex + 1];
		f32 minDist = QuadDist(rightCorner->p, leftCorner->p);
		for (RockCorner *it = rings[ringIndex + 1]->next; it != rings[ringIndex + 1]; it = it->next)
		{
			f32 dist = QuadDist(it->p, leftCorner->p);
			if (dist < minDist)
			{
				rightCorner = it;
				minDist = dist;
			}
		}
		RockCorner *leftIt = leftCorner;
		RockCorner *rightIt = rightCorner;

		do
		{
			while (QuadDist(rightIt->next->p, leftIt->p) < QuadDist(leftIt->next->p, rightIt->next->p))
			{
				PushTriangleToArenaIntendedNormal(arena, leftIt->p, rightIt->next->p, rightIt->p, leftIt->color, rightIt->next->color, rightIt->color, leftIt->p - aabbMidPoint);
				ret.amount++;
				rightIt = rightIt->next;
			}
			PushTriangleToArenaIntendedNormal(arena, leftIt->p, rightIt->p, leftIt->next->p, leftIt->color, rightIt->color, leftIt->next->color, leftIt->p - aabbMidPoint);
			leftIt = leftIt->next;
			ret.amount++;
		} while (leftIt != leftCorner);

		while (rightIt != rightCorner)
		{
			PushTriangleToArenaIntendedNormal(arena, leftIt->p, rightIt->next->p, rightIt->p, leftIt->color, rightIt->next->color, rightIt->color, leftIt->p - aabbMidPoint);
			ret.amount++;
			rightIt = rightIt->next;
		}
	}

	RockCorner *zeroRock = rings[0];
	PushTriangleToArenaIntendedNormal(arena, rings[1]->p, rings[1]->next->p, zeroRock->p, rings[1]->color, rings[1]->next->color, zeroRock->color, zeroRock->p - aabbMidPoint);
	ret.amount++;
	for (RockCorner *it = rings[1]->next; it != rings[1]; it = it->next)
	{
		PushTriangleToArenaIntendedNormal(arena, it->p, it->next->p, zeroRock->p, it->color, it->next->color, zeroRock->color, zeroRock->p - aabbMidPoint);
		ret.amount++;
	}

	RockCorner *endRock = rings[amountOfRings - 1];
	PushTriangleToArenaIntendedNormal(arena, rings[amountOfRings - 2]->p, rings[amountOfRings - 2]->next->p, endRock->p, rings[amountOfRings - 2]->color, rings[amountOfRings - 2]->next->color, endRock->color, endRock->p - aabbMidPoint);
	ret.amount++;
	
	for (RockCorner *it = rings[amountOfRings - 2]->next; it != rings[amountOfRings - 2]; it = it->next)
	{
		PushTriangleToArenaIntendedNormal(arena, it->p, it->next->p, endRock->p, it->color, it->next->color, endRock->color, endRock->p - aabbMidPoint);
		ret.amount++;
	}

	return ret;
	
}


#if 0
static TriangleMesh GenerateAndPushTriangleFloorMesh(AABB aabb, Arena* arena, u32 meshSize = 255)
{
	TriangleMesh ret;

	ret.type = TrianlgeMeshType_Strip;
	u32 blackBrown = RGBAfromHEX(0x553A26);
	u32 grassGreen = RGBAfromHEX(0x4B6F44);

	ret.amountOfVerticies = meshSize * meshSize; // 1600
	Assert(ret.amountOfVerticies < 65536);
	ret.vertices = PushData(arena, v3, ret.amountOfVerticies);
	ret.colors = PushData(arena, u32, ret.amountOfVerticies);

	RandomSeries series = { RandomSeed() };

	for (u32 x = 0; x < meshSize; x++)
	{
		for (u32 y = 0; y < meshSize; y++)
		{
			f32 xEntropy = 0.15f * RandomSignedPercent(&series);
			f32 yEntropy = 0.15f * RandomSignedPercent(&series);
			f32 zEntropy = 0.3f * RandomPercent(&series);

			f32 xVal = (xEntropy + (f32)x) / (meshSize) * (aabb.maxDim.x - aabb.minDim.x) + (aabb.minDim.x);
			f32 yVal = (yEntropy + (f32)y) / (meshSize) * (aabb.maxDim.y - aabb.minDim.y) + (aabb.minDim.y);

			ret.vertices[x + meshSize * y] = 0.5f * V3(xVal, yVal, zEntropy + aabb.minDim.z);
			ret.colors  [x + meshSize * y] = Pack3x8(LerpVector3(Unpack3x8(blackBrown), Unpack3x8(grassGreen), zEntropy * 3.0f));
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

#endif

#endif 

