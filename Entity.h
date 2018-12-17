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
	//EntityFlag_BeingPushed = 0x20,
};

enum InterpolationType
{
	Interpolation_Push,
	Interpolation_Move,
	Interpolation_Fall,
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
	v3 offset;
	v4 color;

	v4 frameColor;

	u32 serialNumber;

	EntityType type;
	u64 flags;

	EntityInterpolation interpolation;
};


static v3 GetRenderPos(Entity e)
{
	return V3(e.physicalPos) + e.offset;
}


static void ApplyStandardtFlags(Entity *e)
{
	switch (e->type)
	{
	case Entity_None:
	{
		e->flags = 0;
	}break;
	case Entity_Dude:
	{
		e->flags = EntityFlag_SupportsUnit | EntityFlag_BlocksUnit;
	}break;
	case Entity_Wall:
	{
		e->flags = EntityFlag_SupportsUnit | EntityFlag_BlocksUnit;
	}break;
	case Entity_Block:
	{
		e->flags = EntityFlag_SupportsUnit | EntityFlag_PushAble;
	}break;
	case Entity_Spawner:
	{
		e->flags = 0;
	}break;
	case Entity_Goal:
	{
		e->flags = 0;
	}break;
	InvalidDefaultCase;
	};
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

struct World
{
	Level loadedLevel;

	Camera camera;
	Camera debugCamera;
	v3 lightSource; // maybe we want to change it on some change... not sure.
	EntityDynamicArray entities;
	ExecuteEventDynamicArray activeEvents;

	u32 entitySerializer;
	u32DynamicArray entitySerialMap;
	
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

static void RemoveEntity(World *world, u32 serial)
{
	u32 index = world->entitySerialMap[serial];
	world->entitySerialMap[serial] = 0xFFFFFFFF;

	u32 lastIndex = world->entities.amount - 1;
	if (index != lastIndex)
	{
		u32 serialNumberToChange = world->entities[lastIndex].serialNumber;
		world->entitySerialMap[serialNumberToChange] = index;
	}
	UnorderedRemove(&world->entities, index);
}

static Entity CreateEntity(World *world, u32 meshID, f32 scale, Quaternion orientation, v3i pos, v3 offset, v4 color, EntityType type, u64 flags)
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

	u32 arrayIndex = ArrayAdd(&world->entities, ret);
	Assert(ret.serialNumber == world->entitySerialMap.amount);
	ArrayAdd(&world->entitySerialMap, arrayIndex);
	return ret;
};

static Entity RestoreEntity(World *world, u32 serial, u32 meshID, f32 scale, Quaternion orientation, v3i pos, v3 offset, v4 color, EntityType type, u64 flags)
{
	u32 index = world->entitySerialMap[serial];
	Assert(index == 0xFFFFFFFF); // maybe handle this and allow it, just returning the curently loaded one
	Entity ret;
	ret.meshId = meshID;
	ret.scale = scale;
	ret.orientation = orientation;
	ret.physicalPos = pos;
	ret.offset = offset;
	ret.color = color;
	ret.frameColor = V4(1, 1, 1, 1);
	ret.serialNumber = serial;
	ret.type = type;
	ret.flags = flags;

	u32 arrayIndex = ArrayAdd(&world->entities, ret);
	world->entitySerialMap[serial] = arrayIndex;
	return ret;
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

static void ResetLevel(World *world)
{
	world->loadedLevel = EmptyLevel();
}

static void ResetWorld(World *world)
{
	world->camera = world->loadedLevel.camera;
	world->debugCamera = world->camera;
	world->entitySerialMap.amount = 0;
	world->entitySerializer = 0;
	world->entities.amount = 0;
	world->lightSource = V3(0, 0, -10);
	world->activeEvents.amount = 0;

	For(world->loadedLevel.entities)
	{
		CreateEntity(world, it->meshId, it->scale, it->orientation, it->physicalPos, it->offset, it->color, it->type, it->flags);
	}
}


enum UnitInstruction
{
	Unit_Wait,
	Unit_MoveUp,
	Unit_MoveDown,
	Unit_MoveLeft,
	Unit_MoveRight,
	Unit_Fall,
};

DefineArray(UnitInstruction);

#define UnitIndexOffset (64 - 16)
struct UnitHandler
{
	u32 amount;

	u32 *entitySerials;
	UnitInstructionArray *programs;
	
	u64 at;

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
		v3 rayP = mat * (camP - GetRenderPos(*e));
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

static Entity CreateDude(World *world, UnitHandler *handler, v3i pos, f32 scale = 1.0f, Quaternion orientation = {1, 0, 0, 0}, v3 offset = V3(), v4 color = V4(1, 1, 1, 1))
{
	u32 index = handler->amount++;
	if (index > MaxUnitCount) return {};

	u64 flags = EntityFlag_BlocksUnit | EntityFlag_PushAble | EntityFlag_SupportsUnit | ((u64)index << UnitIndexOffset);
	Entity e = CreateEntity(world, world->dudeMeshId, scale, orientation, pos, offset, color, Entity_Dude, flags);

	handler->entitySerials[index] = e.serialNumber;
	handler->programs[index].amount = 0;
	//handler->programs[index].at = 0x0;

	return e;
}

static Entity CreateBlock(World *world, v3i pos, f32 scale = 1.0f, Quaternion orientation = { 0.707106769f, -0.707106769f, 0, 0 }, v3 offset = V3(0, 0, 0.27f), v4 color = V4(1, 1, 1, 1))
{
	u64 flags = EntityFlag_BlocksUnit | EntityFlag_PushAble | EntityFlag_SupportsUnit;
	return CreateEntity(world, world->blockMeshId, scale, orientation, pos, offset, color, Entity_Block, flags);
}

static Entity RestoreBlock(World *world, u32 serial, v3i pos, f32 scale = 1.0f, Quaternion orientation = { 0.707106769f, -0.707106769f, 0, 0 }, v3 offset = V3(0, 0, 0.27f), v4 color = V4(1, 1, 1, 1))
{
	u64 flags = EntityFlag_BlocksUnit | EntityFlag_PushAble | EntityFlag_SupportsUnit;
	return RestoreEntity(world, serial, world->blockMeshId, scale, orientation, pos, offset, color, Entity_Block, flags);
}

static Entity CreateWall(World *world, u32 meshId, v3i pos, f32 scale = 1.0f, Quaternion orientation = { 1, 0, 0, 0 }, v3 offset = V3(), v4 color = V4(1, 1, 1, 1))
{
	u64 flags = EntityFlag_BlocksUnit | EntityFlag_SupportsUnit;
	return CreateEntity(world, meshId, scale, orientation, pos, offset, color, Entity_Wall, flags);
}

static Entity CreateSpawner(World *world, u32 meshId, v3i pos, f32 scale = 1.0f, Quaternion orientation = { 1, 0, 0, 0 }, v3 offset = V3(), v4 color = V4(1, 1, 1, 1))
{
	u64 flags = EntityFlag_BlocksUnit | EntityFlag_SupportsUnit;
	return CreateEntity(world, meshId, scale, orientation, pos, offset, color, Entity_Spawner, flags);
}

static Entity CreateGoal(World *world, u32 meshId, v3i pos, f32 scale = 1.0f, Quaternion orientation = { 1, 0, 0, 0 }, v3 offset = V3(), v4 color = V4(1, 1, 1, 1))
{
	u64 flags = EntityFlag_BlocksUnit | EntityFlag_SupportsUnit;
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
	case Unit_Fall:
	{
		advance = V3i(0, 0, 1);
	}break;
	default:
	{
		Die;
	}break;

	}
	return advance;
}


static void ResetDudeAts(UnitHandler *handler)
{
	handler->at = 0;
}

#endif