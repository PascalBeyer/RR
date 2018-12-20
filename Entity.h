#ifndef RR_ENTITY
#define RR_ENTITY

#define MaxUnitCount 100
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

	EntityFlag_IsFalling = 0x10,
	EntityFlag_IsMoving = 0x20,
	EntityFlag_InTree = 0x40,
	EntityFlag_MoveResolved = 0x80,

	EntityFlag_IsDynamic = 0x100,

	BlockFlag_TryingToSpawn = 0x200,
	BlockFlag_MovingToSpawn = 0x200,
};

enum InterpolationType
{
	Interpolation_Push,
	Interpolation_Move,
	Interpolation_Fall,
	Interpolation_PhaseMove,
};

struct EntityInterpolation
{
	u32 type;
	v3i dir;
};

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
	u64 temporaryFlags;

	EntityInterpolation interpolation;
};

static v3 GetRenderPos(Entity e)
{
	return V3(e.physicalPos) + e.offset;
}


static v3 GetRenderPos(Entity e, f32 interpolationT)
{
	f32 t = interpolationT;
	v3 moveOffset = V3();
	
	if (e.temporaryFlags & EntityFlag_IsMoving)
	{
		v3 delta = V3(e.interpolation.dir);
		f32 ease = -2.0f * t * t * t + 3.0f * t * t;
		moveOffset = ease * delta;
	}

	return V3(e.physicalPos) + e.offset + moveOffset;
}

typedef Entity* EntityPtr;

DefineDynamicArray(Entity);
DefineDynamicArray(EntityPtr);
DefineArray(Entity);

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

enum ExecuteEventType
{
	ExecuteEvent_SpawnedBlock,
	ExecuteEvent_ReceivedBlock,
};

struct ExecuteEvent
{
	ExecuteEventType type;
	u32 causingEntitySerial;
	u32 effectingEntitySerial;
};

DefineDynamicArray(ExecuteEvent);

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

struct World
{
	Level loadedLevel;

	Camera camera;
	Camera debugCamera;
	v3 lightSource; // maybe we want to change it on some change... not sure.

	EntityDynamicArray entities;

	u32 entitySerializer;
	u32DynamicArray entitySerialMap;
	
	TileOctTree entityTree;

	u32 at;

	u32 wallMeshId;
	u32 blockMeshId;
	u32 dudeMeshId;
};

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

static Entity *GetEntity(World *world, v3i tile)
{
	TileOctTreeNode *cur = &world->entityTree.root;
	Assert(PointInAABBi(cur->bound, tile));
	while (!cur->isLeaf)
	{
		for (u32 i = 0; i < 8; i++) // lazy
		{
			if (PointInAABBi(cur->children[i]->bound, tile))
			{
				cur = cur->children[i];
				break;
			}
		}
	}

	for (u32 i = 0; i < 16; i++)
	{
		if (cur->entitySerials[i] == 0xFFFFFFFF) continue;
		Entity *e = GetEntity(world, cur->entitySerials[i]);
		if (e->physicalPos == tile)
		{
			return e;
		}
	}

	return NULL;
}

static Entity *InsertEntity(World *world, TileOctTreeNode *cur, Entity *e, b32 swap)
{
	TimedBlock;
	if (!e) return NULL;

	TileOctTree *tree = &world->entityTree;

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

	Assert(cur);
	Assert(cur->isLeaf);

	//check for place allready taken, this is the only false case
	for (u32 i = 0; i < 16; i++)
	{
		if (cur->entitySerials[i] != 0xFFFFFFFF)
		{
			Entity *other = GetEntity(world, cur->entitySerials[i]);
			if (other->physicalPos == e->physicalPos)
			{
				if (swap) cur->entitySerials[i] = e->serialNumber;
				return other;
			}
		}
	}

	// try to insert
	for (u32 i = 0; i < 8; i++)
	{
		if (cur->entitySerials[i] == 0xFFFFFFFF)
		{
			cur->entitySerials[i] = e->serialNumber;
			return NULL;
		}
	}

	u32 oldSize = (cur->bound.maxDim.x - cur->bound.minDim.x);
	u32 newSize = oldSize >> 1;
	Assert(newSize);

	Entity *currentEntities[17];
	for (u32 i = 0; i < 16; i++)
	{
		currentEntities[i] = (cur->entitySerials[i] != 0xFFFFFFFF) ? GetEntity(world, cur->entitySerials[i]) : NULL;
	}
	currentEntities[16] = e;

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
		InsertEntity(world, cur, currentEntities[i], false);
	}

	return NULL;
}

static void ResetTreeHelper(TileOctTree *tree, TileOctTreeNode *node)
{
	if (node->isLeaf) return;

	for (u32 i = 0; i < 8; i++)
	{
		auto c = node->children[i];
		ResetTreeHelper(tree, c);
		c->next = tree->freeList;
		tree->freeList = c;
	}
}

static void ResetTree(TileOctTree *tree)
{
	ResetTreeHelper(tree, &tree->root);
	tree->root.isLeaf = true;
	for (u32 i = 0; i < 16; i++)
	{
		tree->root.entitySerials[i] = 0xFFFFFFFF;
	}
}

static Entity *InsertEntity(World *world, Entity *e, b32 swap = false) // the return feels weird.
{
	TileOctTree *tree = &world->entityTree;
	TileOctTreeNode *cur = &tree->root;
	Assert(PointInAABBi(cur->bound, e->physicalPos));
	// change this to isNotLeaf, to not have to do this not?

	Entity *blockingEntity = InsertEntity(world, cur, e, swap);
	if (blockingEntity)
	{
		if (swap)
		{
			blockingEntity->flags &= ~EntityFlag_InTree;
			e->flags |= EntityFlag_InTree;
		}
	}
	else
	{
		e->flags |= EntityFlag_InTree;
	}

	return blockingEntity;
}

static Entity CreateEntity(World *world, u32 meshID, f32 scale, Quaternion orientation, v3i pos, v3 offset, v4 color, EntityType type, u64 flags)
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
	ret.temporaryFlags = 0;
	ret.interpolation = {};
	
	u32 arrayIndex = ArrayAdd(&world->entities, ret);

	Assert(ret.serialNumber == world->entitySerialMap.amount);
	ArrayAdd(&world->entitySerialMap, arrayIndex);
	if (flags) // entity != none?
	{
		InsertEntity(world, world->entities + arrayIndex);
	}

	return ret;
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
	ret.temporaryFlags = 0;
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
	Entity *toRemove = GetEntity(world, index);
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

static Level EmptyLevel()
{
	Level ret;
	ret.camera.pos = V3(0, 0, -5);
	ret.camera.basis = v3StdBasis;
	ret.camera.aspectRatio = (f32)16 / (f32)9;
	ret.camera.focalLength = 1.0f;
	ret.blocksNeeded = 10000;

	ret.lightSource = V3(20, 0, -20);
	ret.name = {};
	ret.entities = {};

	return ret;
}

static void UnloadLevel(World *world)
{
	world->loadedLevel = EmptyLevel();
	world->at = 0;
	world->camera = world->loadedLevel.camera;
	ResetTree(&world->entityTree);
	world->entitySerialMap.amount = 0;
	world->entities.amount = 0;
	world->entitySerializer = 0;
	world->debugCamera = world->camera;
}

static void ResetWorld(World *world)
{
	world->camera = world->loadedLevel.camera;
	world->debugCamera = world->camera;
	world->lightSource = V3(0, 0, -10);
	world->at = 0;

	For(world->entities)
	{
		if (it->flags & EntityFlag_IsDynamic)
		{
			RemoveEntityFromTree(world, it);
			it->physicalPos = it->initialPos;
			it->interpolation = {};
			InsertEntity(world, it);
		}
		
	}

}
enum UnitInstruction
{
	Unit_Wait,
	Unit_MoveUp,
	Unit_MoveDown,
	Unit_MoveLeft,
	Unit_MoveRight,
};

DefineArray(UnitInstruction);

#define UnitIndexOffset (64 - 16)
struct UnitHandler
{
	u32 amount;

	u32 *entitySerials;
	UnitInstructionArray *programs;
	
	f32 t; // between 0 and 1
};

static u32 GetHotUnit(World *world, UnitHandler *unitHandler, AssetHandler *assetHandler, v2 mousePosZeroToOne, Camera camera)
{
	v3 camP = camera.pos; // todo camera or debugCamera? Maybe we should again unify them
	v3 camD = ScreenZeroToOneToDirecion(camera, mousePosZeroToOne);

	for (u32 i = 0; i < unitHandler->amount; i++)
	{
		Entity *e = GetEntity(world, unitHandler->entitySerials[i]);
		m4x4 mat = QuaternionToMatrix(Inverse(e->orientation));
		v3 rayP = mat * (camP - GetRenderPos(*e, unitHandler->t));
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

		if (PointInAABB(aabb, curExit)) return i;
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
		return EntityFlag_BlocksUnit | EntityFlag_PushAble | EntityFlag_SupportsUnit | EntityFlag_IsDynamic;
	}break;
	case Entity_Wall:
	{
		return EntityFlag_BlocksUnit | EntityFlag_SupportsUnit;
	}break;
	case Entity_Block:
	{
		return EntityFlag_BlocksUnit | EntityFlag_PushAble | EntityFlag_SupportsUnit | EntityFlag_IsDynamic;
	}break;
	case Entity_Spawner:
	{
		return EntityFlag_BlocksUnit | EntityFlag_SupportsUnit | EntityFlag_IsDynamic;
	}break;
	case Entity_Goal:
	{
		return EntityFlag_BlocksUnit | EntityFlag_SupportsUnit | EntityFlag_IsDynamic;
	}break;
	InvalidDefaultCase;
	};
	return 0;
}

static Entity CreateDude(World *world, UnitHandler *handler, v3i pos, f32 scale = 1.0f, Quaternion orientation = {1, 0, 0, 0}, v3 offset = V3(), v4 color = V4(1, 1, 1, 1))
{
	u32 index = handler->amount++;
	if (index > MaxUnitCount) return {};

	u64 flags = GetStandardFlags(Entity_Dude) | ((u64)index << UnitIndexOffset);
	Entity e = CreateEntity(world, world->dudeMeshId, scale, orientation, pos, offset, color, Entity_Dude, flags);

	handler->entitySerials[index] = e.serialNumber;
	handler->programs[index].amount = 0;
	//handler->programs[index].at = 0x0;

	return e;
}

static Entity CreateBlock(World *world, v3i pos, f32 scale = 1.0f, Quaternion orientation = { 0.707106769f, -0.707106769f, 0, 0 }, v3 offset = V3(0, 0, 0.27f), v4 color = V4(1, 1, 1, 1))
{
	u64 flags = GetStandardFlags(Entity_Block);
	return CreateEntity(world, world->blockMeshId, scale, orientation, pos, offset, color, Entity_Block, flags);
}

static Entity CreateWall(World *world, u32 meshId, v3i pos, f32 scale = 1.0f, Quaternion orientation = { 1, 0, 0, 0 }, v3 offset = V3(), v4 color = V4(1, 1, 1, 1))
{
	u64 flags = GetStandardFlags(Entity_Wall);
	return CreateEntity(world, meshId, scale, orientation, pos, offset, color, Entity_Wall, flags);
}

static Entity CreateSpawner(World *world, u32 meshId, v3i pos, f32 scale = 1.0f, Quaternion orientation = { 1, 0, 0, 0 }, v3 offset = V3(), v4 color = V4(1, 1, 1, 1))
{
	u64 flags = GetStandardFlags(Entity_Spawner);
	return CreateEntity(world, meshId, scale, orientation, pos, offset, color, Entity_Spawner, flags);
}

static Entity CreateGoal(World *world, u32 meshId, v3i pos, f32 scale = 1.0f, Quaternion orientation = { 1, 0, 0, 0 }, v3 offset = V3(), v4 color = V4(1, 1, 1, 1))
{
	u64 flags = GetStandardFlags(Entity_Goal);
	return CreateEntity(world, meshId, scale, orientation, pos, offset, color, Entity_Goal, flags);
}


static u32 GetUnitIndex(Entity *e)
{
	return (e->flags >> UnitIndexOffset);
}

static void ResetUnitHandler(UnitHandler *unitHandler)
{
	unitHandler->amount = 0;
	unitHandler->t = 0;
	// I guess this is everything?
}
static UnitHandler CreateUnitHandler(Arena *arena, AssetHandler *assetHandler)
{
	UnitHandler ret;

	ret.amount = 0;
	ret.entitySerials = PushData(arena, u32, MaxUnitCount);
	ret.programs = PushData(arena, UnitInstructionArray, MaxUnitCount);
	for (u32 i = 0; i < MaxUnitCount; i++)
	{
		ret.programs[i] = PushArray(arena, UnitInstruction, MaxUnitInstructions);
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