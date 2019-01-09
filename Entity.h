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
   EntityFlag_IsDynamic = 0x200,
   
   // dynamic flags
	EntityFlag_IsFalling = 0x10,
	EntityFlag_IsMoving = 0x20,
	EntityFlag_InTree = 0x40,
	
   EntityFlag_FrameResetFlag = EntityFlag_IsFalling | EntityFlag_IsMoving,
	
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

DefineDynamicArray(EntityInterpolation);

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
   // general entity data header if you will
   EntityType type;
	u64 flags;
   u32 serialNumber;
   
   // render stuff
   u32 meshId;
	f32 scale;
	Quaternion orientation;
   v3 offset;
	v4 color;
   v4 frameColor;
   
   // simulation stuff, also needed for render
	EntityInterpolation interpolation;
   v3i physicalPos;
	v3i initialPos;
	
   // specific stuff
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
   Quaternion orientation;
   v3i physicalPos;
   v3 offset;
   v4 color;
   f32 scale;
   EntityType type;
   u64 flags;
   
	u32 meshId;
};
DefineArray(EntityCopyData);


struct LightSource
{
   v3 pos;
   Quaternion orientation;
   v3 color;
};

// todo right now I do not see a reason why this should be an asset, so where should it live?
struct Level
{
   String name;
   
	Camera camera;
   LightSource lightSource;
	EntityCopyDataArray entities;
	u32 blocksNeeded;
   
	//u32 amountOfDudes; right now not used.
};


static v3 GetRenderPos(EntityCopyData e)
{
	return V3(e.physicalPos) + e.offset;
}


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



struct EntityManager
{
   String levelName;
   
	Camera camera;
	Camera debugCamera;
   LightSource lightSource;
   
   // todo make some sort of bucketed array.
   // todo make an array thing for each entity type
	EntityDynamicArray entities;
   
	u32 entitySerializer;
	u32DynamicArray entitySerialMap;
	
	TileOctTree entityTree;
   
	u32 at; // execute data?
	f32 t;
   
	u32 blockMeshId;
	u32 dudeMeshId;
};

static Entity *GetEntity(EntityManager *entityManager, u32 serialNumber)
{
	u32 index = entityManager->entitySerialMap[serialNumber];
	Entity *ret = entityManager->entities + index;
	return ret;
}

static void RemoveEntityFromTree(EntityManager *entityManager, Entity *e)
{
	if (!(e->flags & EntityFlag_InTree)) return;
   
	TileOctTree *tree = &entityManager->entityTree;
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

static EntityPtrArray GetEntities(EntityManager *entityManager, v3i tile, u64 flags = 0)
{
	TileOctTreeNode *cur = &entityManager->entityTree.root;
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
		Entity *e = GetEntity(entityManager, cur->entitySerials[i]);
		if (e->physicalPos == tile && ((e->flags & flags) == flags))
		{
			*PushStruct(frameArena, EntityPtr) = e;
		}
	}
	EndArray(frameArena, EntityPtr, ret);
   
	return ret;
}

static void InsertEntity(EntityManager *entityManager, TileOctTreeNode *cur, Entity *e)
{
	TimedBlock;
	if (!e) return;
   
	TileOctTree *tree = &entityManager->entityTree;
   
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
			Entity *other = GetEntities(entityManager, cur->entitySerials[i]);
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
		currentEntities[i] = (cur->entitySerials[i] != 0xFFFFFFFF) ? GetEntity(entityManager, cur->entitySerials[i]) : NULL;
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
		InsertEntity(entityManager, &entityManager->entityTree.root, e);
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
		InsertEntity(entityManager, cur, currentEntities[i]);
	}
   
	return;
}

static void ResetTreeHelper(EntityManager *entityManager, TileOctTree *tree, TileOctTreeNode *node)
{
	if (node->isLeaf) return;
   
	for (u32 i = 0; i < 8; i++)
	{
		auto c = node->children[i];
		ResetTreeHelper(entityManager, tree, c);
		c->next = tree->freeList;
		tree->freeList = c;
	}
}

static void ResetTree(EntityManager *entityManager, TileOctTree *tree) // todo speed can this be lineraized?
{
	ResetTreeHelper(entityManager, tree, &tree->root);
	tree->root.isLeaf = true;
	for (u32 i = 0; i < ArrayCount(tree->root.entitySerials); i++)
	{
		tree->root.entitySerials[i] = 0xFFFFFFFF;
	}
   
	For(entityManager->entities)
	{
		it->flags &= ~EntityFlag_InTree;
	}
}

static void InsertEntity(EntityManager *entityManager, Entity *e)
{
	Assert(!(e->flags & EntityFlag_InTree));
	TileOctTree *tree = &entityManager->entityTree;
	TileOctTreeNode *cur = &tree->root;
	Assert(PointInAABBi(cur->bound, e->physicalPos));
	// change this to isNotLeaf, to not have to do this not?
   
	InsertEntity(entityManager, cur, e);
   
	e->flags |= EntityFlag_InTree;
}

static void MoveEntityInTree(EntityManager *entityManager, Entity *e, v3i by)
{
	RemoveEntityFromTree(entityManager, e);
	e->physicalPos += by;
	InsertEntity(entityManager, e);
}

static Entity *CreateEntityInternal(EntityManager *entityManager, u32 meshID, f32 scale, Quaternion orientation, v3i pos, v3 offset, v4 color, EntityType type, u64 flags)
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
	ret.serialNumber = entityManager->entitySerializer++;
	ret.type = type;
	ret.flags = flags;
	ret.interpolation = {};
	
	u32 arrayIndex = ArrayAdd(&entityManager->entities, ret);
   
	Assert(ret.serialNumber == entityManager->entitySerialMap.amount);
	ArrayAdd(&entityManager->entitySerialMap, arrayIndex);
	if (flags) // entity != none? // move this out?
	{
		InsertEntity(entityManager, entityManager->entities + arrayIndex);
	}
   
	return entityManager->entities + arrayIndex;
};

static void RestoreEntity(EntityManager *entityManager, u32 serial, u32 meshID, f32 scale, Quaternion orientation, v3i pos, v3 offset, v4 color, EntityType type, u64 flags)
{
	Entity ret;
	ret.meshId = meshID;
	ret.scale = scale;
	ret.orientation = orientation;
	ret.physicalPos = pos;
	ret.offset = offset;
	ret.color = color;
	ret.frameColor = V4(1, 1, 1, 1);
	ret.serialNumber = entityManager->entitySerializer++;
	ret.type = type;
	ret.flags = flags;
	ret.interpolation = {};
   
	u32 arrayIndex = ArrayAdd(&entityManager->entities, ret);
   
	Assert(entityManager->entitySerialMap[serial] == 0xFFFFFFFF);
	entityManager->entitySerialMap[serial] = arrayIndex;
	if (flags) // entity != none?
	{
		InsertEntity(entityManager, entityManager->entities + arrayIndex);
	}
}

static void RemoveEntity(EntityManager *entityManager, u32 serial)
{
	u32 index = entityManager->entitySerialMap[serial];
	Entity *toRemove = entityManager->entities + index;
	RemoveEntityFromTree(entityManager, toRemove);
   
	entityManager->entitySerialMap[serial] = 0xFFFFFFFF;
   
	u32 lastIndex = entityManager->entities.amount - 1;
	if (index != lastIndex)
	{
		u32 serialNumberToChange = entityManager->entities[lastIndex].serialNumber;
		entityManager->entitySerialMap[serialNumberToChange] = index;
	}
	UnorderedRemove(&entityManager->entities, index);
}

static void UnloadLevel(EntityManager *entityManager)
{
	entityManager->at = 0;
	entityManager->camera.orientation = QuaternionId();
   entityManager->camera.pos = V3(0, 0, -5);
   entityManager->camera.focalLength = 1.0f;
   entityManager->camera.aspectRatio = 16.0f / 9.0f;
	entityManager->entities.amount = 0; // this before ResetTree, makes the reset faster
	ResetTree(entityManager, &entityManager->entityTree);
	entityManager->entitySerialMap.amount = 0;
	entityManager->entitySerializer = 0;
	entityManager->debugCamera = entityManager->camera;
}

static void ResetEntityManager(EntityManager *entityManager)
{
	entityManager->at = 0;
	entityManager->t = 0.0f;
   
	For(entityManager->entities)
	{
		if (it->flags & EntityFlag_IsDynamic)
		{
			RemoveEntityFromTree(entityManager, it);
		}
	}
   
	For(entityManager->entities)
	{
		if (it->flags & EntityFlag_IsDynamic)
		{
			it->physicalPos = it->initialPos;
			it->flags &= ~(EntityFlag_IsFalling | EntityFlag_IsMoving);
			it->interpolation = {};
			InsertEntity(entityManager, it);
		}
		
	}
   
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

static Entity CreateDude(EntityManager *entityManager, v3i pos, f32 scale = 1.0f, Quaternion orientation = {1, 0, 0, 0}, v3 offset = V3(), v4 color = V4(1, 1, 1, 1), u32 meshId = 0xFFFFFFFF)
{
	u64 flags = GetStandardFlags(Entity_Dude);
	if (meshId == 0xFFFFFFFF) meshId = entityManager->blockMeshId;
	Entity *e = CreateEntityInternal(entityManager, meshId, scale, orientation, pos, offset, color, Entity_Dude, flags);
   
	e->instructions = UnitInstructionCreateDynamicArray(100);
   
	return *e;
}

static Entity CreateBlock(EntityManager *entityManager, v3i pos, f32 scale = 1.0f, Quaternion orientation = { 0.707106769f, -0.707106769f, 0, 0 }, v3 offset = V3(0, 0, 0.27f), v4 color = V4(1, 1, 1, 1), u32 meshId = 0xFFFFFFFF)
{
	if (meshId == 0xFFFFFFFF) meshId = entityManager->blockMeshId;
	u64 flags = GetStandardFlags(Entity_Block);
	return *CreateEntityInternal(entityManager, meshId, scale, orientation, pos, offset, color, Entity_Block, flags);
}

static Entity CreateWall(EntityManager *entityManager, u32 meshId, v3i pos, f32 scale = 1.0f, Quaternion orientation = { 1, 0, 0, 0 }, v3 offset = V3(), v4 color = V4(1, 1, 1, 1))
{
	u64 flags = GetStandardFlags(Entity_Wall);
	return *CreateEntityInternal(entityManager, meshId, scale, orientation, pos, offset, color, Entity_Wall, flags);
}

static Entity CreateSpawner(EntityManager *entityManager, u32 meshId, v3i pos, f32 scale = 1.0f, Quaternion orientation = { 1, 0, 0, 0 }, v3 offset = V3(), v4 color = V4(1, 1, 1, 1))
{
	u64 flags = GetStandardFlags(Entity_Spawner);
	return *CreateEntityInternal(entityManager, meshId, scale, orientation, pos, offset, color, Entity_Spawner, flags);
}

static Entity CreateGoal(EntityManager *entityManager, u32 meshId, v3i pos, f32 scale = 1.0f, Quaternion orientation = { 1, 0, 0, 0 }, v3 offset = V3(), v4 color = V4(1, 1, 1, 1))
{
	u64 flags = GetStandardFlags(Entity_Goal);
	return *CreateEntityInternal(entityManager, meshId, scale, orientation, pos, offset, color, Entity_Goal, flags);
}

// this seems pretty bad in retrospect
static Entity CreateEntity(EntityManager *entityManager, EntityType type, u32 meshId, v3i pos, f32 scale, Quaternion orientation, v3 offset, v4 color, u64 flags)
{
	switch (type)
	{
      case Entity_None:
      {
         return *CreateEntityInternal(entityManager, meshId, scale, orientation, pos, offset, color, type, flags);
      }break;
      case Entity_Dude:
      {
         return CreateDude(entityManager, pos, scale, orientation, offset, color, meshId);
      }break;
      case Entity_Wall:
      {
         return CreateWall(entityManager, meshId, pos, scale, orientation, offset, color);
      }break;
      case Entity_Spawner:
      {
         return CreateSpawner(entityManager, meshId, pos, scale, orientation, offset, color);
      }break;
      
      case Entity_Goal:
      {
         return CreateGoal(entityManager, meshId, pos, scale, orientation, offset, color);
      }break;
      
      case Entity_Block:
      {
         return CreateBlock(entityManager, pos, scale, orientation, offset, color, meshId);
      }break;
      
      InvalidDefaultCase;
	}
   
	return {};
}

static EntityManager InitEntityManager(Arena *currentStateArena, Level level)
{
	EntityManager ret;
   
   ret.levelName = CopyString(level.name, currentStateArena);
   ret.camera = level.camera;
   ret.debugCamera = level.camera;
   ret.lightSource = level.lightSource;
   ret.entitySerializer = 0;
	ret.entityTree = InitOctTree(currentStateArena, 100); // todo hardcoded.
	ret.entities = EntityCreateDynamicArray(level.entities.amount);
   ret.entitySerialMap = u32CreateDynamicArray(level.entities.amount);
   u32 at = 0;
   f32 t = 0.0f;
   
   For(level.entities)
   {
      CreateEntity(&ret, it->type, it->meshId, it->physicalPos, it->scale, it->orientation, it->offset, it->color, it->flags);
   }
   
	return ret;
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