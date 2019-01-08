#ifndef RR_ENTITY
#define RR_ENTITY

#define MaxUnitInstructions 100

enum EntityType
{
	Entity_None,
   
	Entity_Dude,
	Entity_Wall,
	Entity_Block,
	Entity_Spawner,
	Entity_Goal,
   
	Entity_Count,
};

enum EntityFlags
{
	EntityFlag_SupportsUnit = 0x1,
	EntityFlag_BlocksUnit = 0x2,
	EntityFlag_Dead = 0x4,
	EntityFlag_PushAble = 0x8,
	EntityFlag_CanBeCarried = 0x800,
   
   
	EntityFlag_IsFalling = 0x10,
	EntityFlag_IsMoving = 0x20,
	EntityFlag_InTree = 0x40,
	
	EntityFlag_IsDynamic = 0x200,
   
	BlockFlag_TryingToSpawn = 0x400,
	
};

enum InterpolationType
{
	Interpolation_None,
	Interpolation_Blocked,
	Interpolation_Push,
	Interpolation_Carried,
	Interpolation_Move,
	Interpolation_Fall,
	Interpolation_Teleport,
};

struct EntityInterpolation
{
	InterpolationType type;
	v3i dir;
};

enum UnitInstruction
{
	Unit_Wait,
	Unit_MoveUp,
	Unit_MoveDown,
	Unit_MoveLeft,
	Unit_MoveRight,
};

DefineDynamicArray(UnitInstruction);


struct Entity
{
	u32 meshId;
	f32 scale;
	Quaternion orientation;
	v3i physicalPos;
	v3i initialPos;
	v3 offset;
	v4 color;
   
	v4 frameColor;
   
	u32 serialNumber;
   
	EntityType type;
	u64 flags;
   
	EntityInterpolation interpolation;
   
	union
	{
		UnitInstructionDynamicArray instructions;
	};
	
};

typedef Entity* EntityPtr;

DefineDynamicArray(Entity);
DefineDynamicArray(EntityPtr);
DefineArray(Entity);
DefineArray(EntityPtr);
DefineDFArray(EntityPtr);

static v3 GetRenderPos(Entity e)
{
	return V3(e.physicalPos) + e.offset;
}


static v3 GetRenderPos(Entity e, f32 interpolationT)
{
	f32 t = interpolationT;
	v3 moveOffset = V3();
	
	if (e.flags & EntityFlag_IsMoving)
	{
		switch (e.interpolation.type)
		{
         case Interpolation_Carried:
         case Interpolation_Move:
         case Interpolation_Push:
         {
            v3 delta = -V3(e.interpolation.dir);
            f32 ease = -2.0f * t * t * t + 3.0f * t * t;
            moveOffset = (1.0f - ease) * delta;
         }break;
         case Interpolation_Fall:
         {
            v3 delta = -V3(e.interpolation.dir);
            f32 lin = 1.0f - t;
            moveOffset = lin * delta;
         }break;
         case Interpolation_Blocked:
         {
            v3 delta = V3(e.interpolation.dir);
            f32 ease = t * (1.0f - t);
            moveOffset = ease * delta;
         }break;
         case Interpolation_Teleport:
         {
            v3 delta = -V3(e.interpolation.dir);
            f32 ease = (t < 0.5f);
            moveOffset = ease * delta;
         }break;
         InvalidDefaultCase;
		}
	}
   
	return V3(e.physicalPos) + e.offset + moveOffset;
}


struct EntityCopyData
{
	u32 meshId;
	f32 scale;
	Quaternion orientation;
	v3i physicalPos;
	v3 offset;
	v4 color;
   
	EntityType type;
	u64 flags;
};

static v3 GetRenderPos(EntityCopyData e)
{
	return V3(e.physicalPos) + e.offset;
}

DefineArray(EntityCopyData);

DefineDynamicArray(EntityInterpolation);

struct Level
{
	Camera camera;
	v3 lightSource;
	EntityCopyDataArray entities;
	String name;
	u32 blocksNeeded;
   
	u32 amountOfDudes;
};

struct AABBi
{
	v3i minDim;
	v3i maxDim;
};

static AABBi CreateAABBi(v3i minDim, v3i maxDim)
{
	AABBi ret;
	ret.minDim = minDim;
	ret.maxDim = maxDim;
	return ret;
}

static b32 PointInAABBi(AABBi aabb, v3i point)
{
	return
      (
		aabb.minDim.x <  point.x &&
		aabb.minDim.y <  point.y &&
		aabb.minDim.z <  point.z &&
		aabb.maxDim.x >= point.x &&
		aabb.maxDim.y >= point.y &&
		aabb.maxDim.z >= point.z
      );
}

struct TileOctTreeNode
{
	b32 isLeaf;
	AABBi bound;
   
	union
	{
		TileOctTreeNode *next; // for clear list
      
		TileOctTreeNode *children[8];
		struct
		{
			TileOctTreeNode *TopUpLeft; // top = minZ, Up = minY, Left = minX
			TileOctTreeNode *TopUpRight;
			TileOctTreeNode *TopDownLeft;
			TileOctTreeNode *TopDownRight;
         
			TileOctTreeNode *BotUpLeft;
			TileOctTreeNode *BotUpRight;
			TileOctTreeNode *BotDownLeft;
			TileOctTreeNode *BotDownRight;
         
		};
      
		u32 entitySerials[16];
	};	
};

struct TileOctTree
{
	TileOctTreeNode root;
	TileOctTreeNode *freeList;
	Arena *arena;
};


static TileOctTree InitOctTree(Arena *currentStateArena, u32 initialCapacity)
{
	TileOctTree ret;
	ret.root.bound = CreateAABBi(V3i(-128, -128, -128), V3i(128, 128, 128));
	ret.root.isLeaf = true;
	for (u32 i = 0; i < 16; i++)
	{
		ret.root.entitySerials[i] = 0xFFFFFFFF;
	}
   
	ret.arena = currentStateArena;
	ret.freeList = NULL;
	for (u32 i = 0; i < initialCapacity; i++)
	{
		TileOctTreeNode *node = PushStruct(currentStateArena, TileOctTreeNode);
		node->next = ret.freeList;
		ret.freeList = node;
	}
   
	return  ret;
}


static TileOctTreeNode *GetANode(TileOctTree *tree)
{
	if (tree->freeList)
	{
		TileOctTreeNode *ret = tree->freeList;
		tree->freeList = tree->freeList->next;
		*ret = {};
		return ret;
	}
   
	return PushZeroStruct(tree->arena, TileOctTreeNode);
}

static TileOctTreeNode *GetALeaf(TileOctTree *tree)
{
	if (tree->freeList)
	{
		TileOctTreeNode *ret = tree->freeList;
		tree->freeList = tree->freeList->next;
		for (u32 i = 0; i < 16; i++)
		{
			ret->entitySerials[i] = 0xFFFFFFFF;
		}
		ret->isLeaf = true;
		return ret;
	}
   
	TileOctTreeNode *ret = PushStruct(tree->arena, TileOctTreeNode);
   
	for (u32 i = 0; i < 16; i++)
	{
		ret->entitySerials[i] = 0xFFFFFFFF;
	}
	ret->isLeaf = true;
	return ret;
}
#if 0
static void RemoveAllEntities(TileOctTreeNode *node)
{
	if (node->isLeaf)
	{
		for (u32 i = 0; i < 16; i++)
		{
			node->entitySerials[i] = 0xFFFFFFFF;
		}
		return;
	}
   
	for (u32 i = 0; i < 8; i++)
	{
		RemoveAllEntities(node->children[i]);
	}
}

static void RemoveAllEntities(TileOctTree *tree)
{
	RemoveAllEntities(&tree->root);
}
#endif

struct LightSource
{
   v3 pos;
   Quaternion orientation;
   v3 color;
};

struct World
{
	Level loadedLevel;
   
	Camera camera;
	Camera debugCamera;
   LightSource lightSource;
   
	EntityDynamicArray entities;
   
	u32 entitySerializer;
	u32DynamicArray entitySerialMap;
	
	TileOctTree entityTree;
   
	u32 at; // execute data?
	f32 t;
   
	u32 wallMeshId;
	u32 blockMeshId;
	u32 dudeMeshId;
};

static Level EmptyLevel()
{
	Level ret;
	ret.camera.pos = V3(0, 0, -5);
	ret.camera.orientation = {1, 0, 0, 0};
	ret.camera.aspectRatio = (f32)16 / (f32)9;
	ret.camera.focalLength = 1.0f;
	ret.blocksNeeded = 10000;
   
	ret.lightSource = V3(20, 0, -20);
	ret.name = {};
	ret.entities = {};
   
	return ret;
}

static World InitWorld(Arena *currentStateArena, AssetHandler *assetHandler, u32 screenWidth, u32 screenHeight)
{
	World ret = {};
   
	ret.lightSource.pos = V3(20, 0, -20);
   ret.lightSource.orientation = QuaternionId();
   ret.lightSource.color = V3(1, 1, 1);
	ret.camera.pos = V3(0, 0, -5);
	ret.camera.orientation = QuaternionId();
	ret.camera.aspectRatio = (f32)screenWidth / (f32)screenHeight;
	ret.camera.focalLength = 1.0f;
   
	ret.debugCamera = ret.camera;
	ret.loadedLevel = EmptyLevel();
	ret.entityTree = InitOctTree(currentStateArena, 100);
	ret.entities = EntityCreateDynamicArray();
	ret.entitySerializer = 0;
	ret.entitySerialMap = u32CreateDynamicArray();
	ret.t = 0.0f;
   
	b32 success = true;
	ret.blockMeshId = RegisterAsset(assetHandler, Asset_Mesh, "block.mesh", &success);
	ret.dudeMeshId = RegisterAsset(assetHandler, Asset_Mesh, "dude.mesh", &success);
	//Assert(success);
   
	return ret;
}

static Entity *GetEntity(World *world, u32 serialNumber)
{
	u32 index = world->entitySerialMap[serialNumber];
	Entity *ret = world->entities + index;
	return ret;
}

static void RemoveEntityFromTree(World *world, Entity *e)
{
	if (!(e->flags & EntityFlag_InTree)) return;
   
	TileOctTree *tree = &world->entityTree;
	TileOctTreeNode *cur = &tree->root;
	
	e->flags &= ~EntityFlag_InTree;
   
	while (!cur->isLeaf)
	{
		for (u32 i = 0; i < 8; i++) // lazy
		{
			if (PointInAABBi(cur->children[i]->bound, e->physicalPos))
			{
				cur = cur->children[i];
				break;
			}
		}
	}
   
	for (u32 i = 0; i < 16; i++)
	{
		if (e->serialNumber == cur->entitySerials[i])
		{
			cur->entitySerials[i] = 0xFFFFFFFF;
			return;
		}
	}
   
	Die;
}

static EntityPtrArray GetEntities(World *world, v3i tile, u64 flags = 0)
{
	TileOctTreeNode *cur = &world->entityTree.root;
	Assert(PointInAABBi(cur->bound, tile));
	while (!cur->isLeaf)
	{
		for (u32 i = 0; i < ArrayCount(cur->children); i++)
		{
			if (PointInAABBi(cur->children[i]->bound, tile))
			{
				cur = cur->children[i];
				break;
			}
		}
	}
   
	BeginArray(frameArena, EntityPtr, ret);
   
	for (u32 i = 0; i < ArrayCount(cur->entitySerials); i++)
	{
		if (cur->entitySerials[i] == 0xFFFFFFFF) continue;
		Entity *e = GetEntity(world, cur->entitySerials[i]);
		if (e->physicalPos == tile && ((e->flags & flags) == flags))
		{
			*PushStruct(frameArena, EntityPtr) = e;
		}
	}
	EndArray(frameArena, EntityPtr, ret);
   
	return ret;
}

static void InsertEntity(World *world, TileOctTreeNode *cur, Entity *e)
{
	TimedBlock;
	if (!e) return;
   
	TileOctTree *tree = &world->entityTree;
   
	while (!cur->isLeaf)
	{
		for (u32 i = 0; i < ArrayCount(cur->children); i++) // lazy
		{
			if (PointInAABBi(cur->children[i]->bound, e->physicalPos))
			{
				cur = cur->children[i];
				break;
			}
		}
	}
   
	Assert(cur);
	Assert(cur->isLeaf);
   
	//check for place allready taken, this is the only false case
#if 0
	for (u32 i = 0; i < 16; i++)
	{
		if (cur->entitySerials[i] != 0xFFFFFFFF)
		{
			Entity *other = GetEntities(world, cur->entitySerials[i]);
			if (other->physicalPos == e->physicalPos)
			{
				if (swap) cur->entitySerials[i] = e->serialNumber;
				return other;
			}
		}
	}
#endif
   
	// try to insert
	for (u32 i = 0; i < ArrayCount(cur->entitySerials); i++)
	{
		if (cur->entitySerials[i] == 0xFFFFFFFF)
		{
			cur->entitySerials[i] = e->serialNumber;
			return;
		}
	}
   
	u32 oldSize = (cur->bound.maxDim.x - cur->bound.minDim.x);
	u32 newSize = oldSize >> 1;
	Assert(newSize);
   
	Entity *currentEntities[ArrayCount(cur->entitySerials) + 1];
	for (u32 i = 0; i < ArrayCount(cur->entitySerials); i++)
	{
		currentEntities[i] = (cur->entitySerials[i] != 0xFFFFFFFF) ? GetEntity(world, cur->entitySerials[i]) : NULL;
	}
	currentEntities[ArrayCount(cur->entitySerials)] = e;
   
	b32 allTheSame = true;
	for (u32 i = 0; i < ArrayCount(cur->entitySerials); i++)
	{
		if (e->physicalPos != currentEntities[i]->physicalPos)
		{
			allTheSame = false;
			break;
		}
	}
   
	if (allTheSame)
	{
		Die;
		e->physicalPos -= V3i(0, 0, 1);
		InsertEntity(world, &world->entityTree.root, e);
		return;
	}
   
	//split
	TileOctTreeNode *TopUpLeft = GetALeaf(tree);
	TopUpLeft->bound = CreateAABBi(cur->bound.minDim, cur->bound.minDim + V3i(newSize, newSize, newSize));
	cur->TopUpLeft = TopUpLeft;
   
	TileOctTreeNode *TopUpRight = GetALeaf(tree);
	TopUpRight->bound = CreateAABBi(cur->bound.minDim + V3i(newSize, 0, 0), cur->bound.minDim + V3i(oldSize, newSize, newSize));
	cur->TopUpRight = TopUpRight;
   
	TileOctTreeNode *TopDownLeft = GetALeaf(tree);
	TopDownLeft->bound = CreateAABBi(cur->bound.minDim + V3i(0, newSize, 0), cur->bound.minDim + V3i(newSize, oldSize, newSize));
	cur->TopDownLeft = TopDownLeft;
   
	TileOctTreeNode *TopDownRight = GetALeaf(tree);
	TopDownRight->bound = CreateAABBi(cur->bound.minDim + V3i(newSize, newSize, 0), cur->bound.minDim + V3i(oldSize, oldSize, newSize));
	cur->TopDownRight = TopDownRight;
   
	TileOctTreeNode *BotUpLeft = GetALeaf(tree);
	BotUpLeft->bound = CreateAABBi(cur->bound.minDim + V3i(0, 0, newSize), cur->bound.minDim + V3i(newSize, newSize, oldSize));
	cur->BotUpLeft = BotUpLeft;
   
	TileOctTreeNode *BotUpRight = GetALeaf(tree);
	BotUpRight->bound = CreateAABBi(cur->bound.minDim + V3i(newSize, 0, newSize), cur->bound.minDim + V3i(oldSize, newSize, oldSize));
	cur->BotUpRight = BotUpRight;
   
	TileOctTreeNode *BotDownLeft = GetALeaf(tree);
	BotDownLeft->bound = CreateAABBi(cur->bound.minDim + V3i(0, newSize, newSize), cur->bound.minDim + V3i(newSize, oldSize, oldSize));
	cur->BotDownLeft = BotDownLeft;
   
	TileOctTreeNode *BotDownRight = GetALeaf(tree);
	BotDownRight->bound = CreateAABBi(cur->bound.minDim + V3i(newSize, newSize, newSize), cur->bound.minDim + V3i(oldSize, oldSize, oldSize));
	cur->BotDownRight = BotDownRight;
   
	cur->isLeaf = false;
   
	for (u32 i = 0; i < 17; i++)
	{
		InsertEntity(world, cur, currentEntities[i]);
	}
   
	return;
}

static void ResetTreeHelper(World *world, TileOctTree *tree, TileOctTreeNode *node)
{
	if (node->isLeaf) return;
   
	for (u32 i = 0; i < 8; i++)
	{
		auto c = node->children[i];
		ResetTreeHelper(world, tree, c);
		c->next = tree->freeList;
		tree->freeList = c;
	}
}

static void ResetTree(World *world, TileOctTree *tree) // todo speed can this be lineraized?
{
	ResetTreeHelper(world, tree, &tree->root);
	tree->root.isLeaf = true;
	for (u32 i = 0; i < ArrayCount(tree->root.entitySerials); i++)
	{
		tree->root.entitySerials[i] = 0xFFFFFFFF;
	}
   
	For(world->entities)
	{
		it->flags &= ~EntityFlag_InTree;
	}
}

static void InsertEntity(World *world, Entity *e)
{
	Assert(!(e->flags & EntityFlag_InTree));
	TileOctTree *tree = &world->entityTree;
	TileOctTreeNode *cur = &tree->root;
	Assert(PointInAABBi(cur->bound, e->physicalPos));
	// change this to isNotLeaf, to not have to do this not?
   
	InsertEntity(world, cur, e);
   
	e->flags |= EntityFlag_InTree;
}

static void MoveEntityInTree(World *world, Entity *e, v3i by)
{
	RemoveEntityFromTree(world, e);
	e->physicalPos += by;
	InsertEntity(world, e);
}

static Entity *CreateEntityInternal(World *world, u32 meshID, f32 scale, Quaternion orientation, v3i pos, v3 offset, v4 color, EntityType type, u64 flags)
{
	Entity ret;
	ret.meshId = meshID;
	ret.scale = scale;
	ret.orientation = orientation;
	ret.physicalPos = pos;
	ret.initialPos = pos;
	ret.offset = offset;
	ret.color = color;
	ret.frameColor = V4(1, 1, 1, 1);
	ret.serialNumber = world->entitySerializer++;
	ret.type = type;
	ret.flags = flags;
	ret.interpolation = {};
	
	u32 arrayIndex = ArrayAdd(&world->entities, ret);
   
	Assert(ret.serialNumber == world->entitySerialMap.amount);
	ArrayAdd(&world->entitySerialMap, arrayIndex);
	if (flags) // entity != none? // move this out?
	{
		InsertEntity(world, world->entities + arrayIndex);
	}
   
	return world->entities + arrayIndex;
};

static void RestoreEntity(World *world, u32 serial, u32 meshID, f32 scale, Quaternion orientation, v3i pos, v3 offset, v4 color, EntityType type, u64 flags)
{
	Entity ret;
	ret.meshId = meshID;
	ret.scale = scale;
	ret.orientation = orientation;
	ret.physicalPos = pos;
	ret.offset = offset;
	ret.color = color;
	ret.frameColor = V4(1, 1, 1, 1);
	ret.serialNumber = world->entitySerializer++;
	ret.type = type;
	ret.flags = flags;
	ret.interpolation = {};
   
	u32 arrayIndex = ArrayAdd(&world->entities, ret);
   
	Assert(world->entitySerialMap[serial] == 0xFFFFFFFF);
	world->entitySerialMap[serial] = arrayIndex;
	if (flags) // entity != none?
	{
		InsertEntity(world, world->entities + arrayIndex);
	}
}

static void RemoveEntity(World *world, u32 serial)
{
	u32 index = world->entitySerialMap[serial];
	Entity *toRemove = world->entities + index;
	RemoveEntityFromTree(world, toRemove);
   
	world->entitySerialMap[serial] = 0xFFFFFFFF;
   
	u32 lastIndex = world->entities.amount - 1;
	if (index != lastIndex)
	{
		u32 serialNumberToChange = world->entities[lastIndex].serialNumber;
		world->entitySerialMap[serialNumberToChange] = index;
	}
	UnorderedRemove(&world->entities, index);
}

static void UnloadLevel(World *world)
{
	world->loadedLevel = EmptyLevel();
	world->at = 0;
	world->camera = world->loadedLevel.camera;
	world->entities.amount = 0; // this before ResetTree, makes the reset faster
	ResetTree(world, &world->entityTree);
	world->entitySerialMap.amount = 0;
	world->entitySerializer = 0;
	world->debugCamera = world->camera;
}

static void ResetWorld(World *world)
{
	world->camera = world->loadedLevel.camera;
	world->debugCamera = world->camera;
	
	world->at = 0;
	world->t = 0.0f;
   
	For(world->entities)
	{
		if (it->flags & EntityFlag_IsDynamic)
		{
			RemoveEntityFromTree(world, it);
		}
	}
   
	For(world->entities)
	{
		if (it->flags & EntityFlag_IsDynamic)
		{
			it->physicalPos = it->initialPos;
			it->flags &= ~(EntityFlag_IsFalling | EntityFlag_IsMoving);
			it->interpolation = {};
			InsertEntity(world, it);
		}
		
	}
   
}

static u32 GetHotUnit(World *world, AssetHandler *assetHandler, v2 mousePosZeroToOne, Camera camera)
{
	v3 camP = camera.pos; // todo camera or debugCamera? Maybe we should again unify them
	v3 camD = ScreenZeroToOneToDirecion(camera, mousePosZeroToOne);
   
	For(world->entities)
	{
		if (it->type != Entity_Dude) continue;
		Entity *e = it;
		m4x4 mat = QuaternionToMatrix4(Inverse(e->orientation));
		v3 rayP = mat * (camP - GetRenderPos(*e, world->t));
		v3 rayD = mat * camD;
      
		MeshInfo *info = GetMeshInfo(assetHandler, e->meshId);
		if (!info)continue;
		AABB aabb = info->aabb;
		aabb.maxDim *= e->scale; // todo can pre compute this
		aabb.minDim *= e->scale;
		f32 curIntersectionMin = MAXF32;
      
		f32 x = rayP.x;
		f32 dx = rayD.x;
		f32 y = rayP.y;
		f32 dy = rayD.y;
		f32 z = rayP.z;
		f32 dz = rayD.z;
      
		f32 aabbMinX = aabb.minDim.x;
		f32 aabbMaxX = aabb.maxDim.x;
		f32 aabbMinY = aabb.minDim.y;
		f32 aabbMaxY = aabb.maxDim.y;
		f32 aabbMinZ = aabb.minDim.z;
		f32 aabbMaxZ = aabb.maxDim.z;
      
		f32 t1x = (aabbMaxX - x) / dx;
		if (dx > 0 && t1x <= curIntersectionMin)
		{
			curIntersectionMin = t1x;
		}
      
		f32 t2x = (aabbMinX - x) / dx;
		if (dx < 0 && t2x <= curIntersectionMin)
		{
			curIntersectionMin = t2x;
		}
      
		f32 t1y = (aabbMaxY - y) / dy;
		if (dy > 0 && t1y <= curIntersectionMin)
		{
			curIntersectionMin = t1y;
		}
      
		f32 t2y = (aabbMinY - y) / dy;
		if (dy < 0 && t2y <= curIntersectionMin)
		{
			curIntersectionMin = t2y;
		}
      
		f32 t1z = (aabbMaxZ - z) / dz;
		if (dz > 0 && t1z <= curIntersectionMin)
		{
			curIntersectionMin = t1z;
		}
      
		f32 t2z = (aabbMinZ - z) / dz;
		if (dz < 0 && t2z <= curIntersectionMin)
		{
			curIntersectionMin = t2z;
		}
		v3 curExit = rayD * curIntersectionMin + rayP;
      
		if (PointInAABB(aabb, curExit)) return e->serialNumber;
	}
   
	return 0xFFFFFFFF;
}


static u64 GetStandardFlags(EntityType type)
{
	switch (type)
	{
      case Entity_None:
      {
         return 0;
      }break;
      case Entity_Dude:
      {
         return EntityFlag_BlocksUnit | EntityFlag_PushAble | EntityFlag_SupportsUnit | EntityFlag_IsDynamic | EntityFlag_CanBeCarried;
      }break;
      case Entity_Wall:
      {
         return EntityFlag_BlocksUnit | EntityFlag_SupportsUnit;
      }break;
      case Entity_Block:
      {
         return EntityFlag_BlocksUnit | EntityFlag_PushAble | EntityFlag_SupportsUnit | EntityFlag_IsDynamic | EntityFlag_CanBeCarried;
      }break;
      case Entity_Spawner:
      {
         return EntityFlag_BlocksUnit | EntityFlag_SupportsUnit;
      }break;
      case Entity_Goal:
      {
         return EntityFlag_BlocksUnit | EntityFlag_SupportsUnit;
      }break;
      InvalidDefaultCase;
	};
	return 0;
}

static Entity CreateDude(World *world, v3i pos, f32 scale = 1.0f, Quaternion orientation = {1, 0, 0, 0}, v3 offset = V3(), v4 color = V4(1, 1, 1, 1), u32 meshId = 0xFFFFFFFF)
{
	u64 flags = GetStandardFlags(Entity_Dude);
	if (meshId == 0xFFFFFFFF) meshId = world->blockMeshId;
	Entity *e = CreateEntityInternal(world, meshId, scale, orientation, pos, offset, color, Entity_Dude, flags);
   
	e->instructions = UnitInstructionCreateDynamicArray(100);
   
	return *e;
}

static Entity CreateBlock(World *world, v3i pos, f32 scale = 1.0f, Quaternion orientation = { 0.707106769f, -0.707106769f, 0, 0 }, v3 offset = V3(0, 0, 0.27f), v4 color = V4(1, 1, 1, 1), u32 meshId = 0xFFFFFFFF)
{
	if (meshId == 0xFFFFFFFF) meshId = world->blockMeshId;
	u64 flags = GetStandardFlags(Entity_Block);
	return *CreateEntityInternal(world, meshId, scale, orientation, pos, offset, color, Entity_Block, flags);
}

static Entity CreateWall(World *world, u32 meshId, v3i pos, f32 scale = 1.0f, Quaternion orientation = { 1, 0, 0, 0 }, v3 offset = V3(), v4 color = V4(1, 1, 1, 1))
{
	u64 flags = GetStandardFlags(Entity_Wall);
	return *CreateEntityInternal(world, meshId, scale, orientation, pos, offset, color, Entity_Wall, flags);
}

static Entity CreateSpawner(World *world, u32 meshId, v3i pos, f32 scale = 1.0f, Quaternion orientation = { 1, 0, 0, 0 }, v3 offset = V3(), v4 color = V4(1, 1, 1, 1))
{
	u64 flags = GetStandardFlags(Entity_Spawner);
	return *CreateEntityInternal(world, meshId, scale, orientation, pos, offset, color, Entity_Spawner, flags);
}

static Entity CreateGoal(World *world, u32 meshId, v3i pos, f32 scale = 1.0f, Quaternion orientation = { 1, 0, 0, 0 }, v3 offset = V3(), v4 color = V4(1, 1, 1, 1))
{
	u64 flags = GetStandardFlags(Entity_Goal);
	return *CreateEntityInternal(world, meshId, scale, orientation, pos, offset, color, Entity_Goal, flags);
}

static Entity CreateEntity(World *world, EntityType type, u32 meshId, v3i pos, f32 scale, Quaternion orientation, v3 offset, v4 color, u64 flags)
{
	switch (type)
	{
      case Entity_None:
      {
         return *CreateEntityInternal(world, meshId, scale, orientation, pos, offset, color, type, flags);
      }break;
      case Entity_Dude:
      {
         return CreateDude(world, pos, scale, orientation, offset, color, meshId);
      }break;
      case Entity_Wall:
      {
         return CreateWall(world, meshId, pos, scale, orientation, offset, color);
      }break;
      case Entity_Spawner:
      {
         return CreateSpawner(world, meshId, pos, scale, orientation, offset, color);
      }break;
      
      case Entity_Goal:
      {
         return CreateGoal(world, meshId, pos, scale, orientation, offset, color);
      }break;
      
      case Entity_Block:
      {
         return CreateBlock(world, pos, scale, orientation, offset, color, meshId);
      }break;
      
      InvalidDefaultCase;
	}
   
	return {};
}

static v3i GetAdvanceForOneStep(UnitInstruction step)
{
	v3i advance = {};
	switch (step)
	{
      case Unit_Wait:
      {
         
      }break;
      case Unit_MoveDown:
      {
         advance = V3i(0, -1, 0);
      }break;
      case Unit_MoveUp:
      {
         advance = V3i(0, 1, 0);
      }break;
      case Unit_MoveLeft:
      {
         advance = V3i(-1, 0, 0);
      }break;
      case Unit_MoveRight:
      {
         advance = V3i(1, 0, 0);
      }break;
      
      default:
      {
         Die;
      }break;
      
	}
	return advance;
}

#endif