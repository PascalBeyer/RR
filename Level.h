
static World InitWorld(Arena *currentStateArena, AssetHandler *assetHandler, u32 screenWidth, u32 screenHeight)
{
	World ret = {};

	ret.lightSource = V3(20, 0, -20);
	ret.camera.pos = V3(0, 0, -5);
	ret.camera.basis = v3StdBasis;
	ret.camera.aspectRatio = (f32)screenWidth / (f32)screenHeight;
	ret.camera.focalLength = 1.0f;

	ret.debugCamera = ret.camera;
	ret.loadedLevel = EmptyLevel();
	ret.entityTree = InitOctTree(currentStateArena, 100);
	ret.entities = EntityCreateDynamicArray();
	ret.entitySerializer = 0;
	ret.entitySerialMap = u32CreateDynamicArray();

	b32 success = true;
	ret.blockMeshId = RegisterAsset(assetHandler, Asset_Mesh, "block.mesh", &success);
	ret.dudeMeshId = RegisterAsset(assetHandler, Asset_Mesh, "dude.mesh", &success);
	Assert(success);

	return ret;
}

static bool WriteLevel(char *fileName, World world, UnitHandler unitHandler, AssetHandler *assetHandler)
{
	u8 *mem = PushData(frameArena, u8, 0);

	*PushStruct(frameArena, u32) = 4; // Version number, do not remove

	*PushStruct(frameArena, v3) = world.lightSource;

	*PushStruct(frameArena, v3)	 = world.loadedLevel.camera.pos;
	*PushStruct(frameArena, v3)	 = world.loadedLevel.camera.b1;
	*PushStruct(frameArena, v3)	 = world.loadedLevel.camera.b2;
	*PushStruct(frameArena, v3)	 = world.loadedLevel.camera.b3;
	*PushStruct(frameArena, f32) = world.loadedLevel.camera.aspectRatio;
	*PushStruct(frameArena, f32) = world.loadedLevel.camera.focalLength;

	*PushStruct(frameArena, u32) = world.entities.amount;

	For(world.entities)
	{
		*PushStruct(frameArena, Quaternion) = it->orientation;
		*PushStruct(frameArena, v3i) = it->physicalPos;
		*PushStruct(frameArena, v3) = it->offset;
		*PushStruct(frameArena, v4) = it->color;
		*PushStruct(frameArena, f32) = it->scale;
		*PushStruct(frameArena, u32) = it->type;
		*PushStruct(frameArena, u64) = it->flags;
		String s = GetAssetInfo(assetHandler, it->meshId).name;
		*PushStruct(frameArena, u32) = s.length;
		Char *dest = PushData(frameArena, Char, s.length);
		memcpy(dest, s.data, s.length * sizeof(Char));
	}

	u32 size = (u32)((u8*)frameArena->current - mem);
	File file = CreateFile(mem, size);
	return WriteEntireFile(fileName, file);
}

#define PullOff(type) *(type *)at; at += sizeof(type);
static bool LoadLevelV1(u8 *at, UnitHandler *unitHandler, World *world, AssetHandler *assetHandler, Editor *editor, Arena *currentStateArena)
{
	u32 tileMapWidth = PullOff(u32);
	u32 tileMapHeight = PullOff(u32);

	//Assert(sizeof(Tile) == 8);
	//TileArray tiles = PushArray(currentStateArena, Tile, tileMapHeight * tileMapWidth);
	at += 8 * tileMapHeight * tileMapWidth;

	u32 meshAmount = PullOff(u32);
	Level *level = &world->loadedLevel;
	level->entities = PushArray(currentStateArena, EntityCopyData, meshAmount);

	for (u32 i = 0; i < meshAmount; i++)
	{
		Quaternion orientation = PullOff(Quaternion);
		v3 pos = PullOff(v3);
		v4 color = PullOff(v4);
		f32 scale = PullOff(f32);

		u32 nameLength = PullOff(u32);
		String name = CreateString((Char *)at, nameLength);
		u32 meshId = RegisterAsset(assetHandler, Asset_Mesh, name);
		at += nameLength * sizeof(Char);

		CreateEntity(world, meshId, scale, orientation, SnapToTileMap(pos), pos - V3(SnapToTileMap(pos)), color, Entity_None, 0);
	}

	u32 amountOfUnits = PullOff(u32);
	Assert(amountOfUnits < MaxUnitCount);

#if 0
	for (u32 i = 0; i < amountOfUnits; i++)
	{
		v2i pos = PullOff(v2i);
		Quaternion q = PullOff(Quaternion);
		CreateDude(unitHandler, pos, q);
	}
#endif

	ResetEditor(editor);

	editor->focusPoint = i12(ScreenZeroToOneToInGame(level->camera, V2(0.5f, 0.5f)));
	level->camera = level->camera;
	level->blocksNeeded = 10000;

	return true;
}

static bool LoadLevelV2(u8 *at, UnitHandler *unitHandler, World *world, AssetHandler *assetHandler, Editor *editor, Arena *currentStateArena)
{
	Level *level = &world->loadedLevel;

	level->lightSource = PullOff(v3);

	level->camera.pos = PullOff(v3);
	level->camera.b1 = PullOff(v3);
	level->camera.b2 = PullOff(v3);
	level->camera.b3 = PullOff(v3);
	level->camera.aspectRatio = PullOff(f32);
	level->camera.focalLength = PullOff(f32);

	u32 meshAmount = PullOff(u32);

	level->entities = PushArray(currentStateArena, EntityCopyData, meshAmount);

	for (u32 i = 0; i < meshAmount; i++)
	{
		Quaternion orientation = PullOff(Quaternion);
		v3 pos = PullOff(v3);
		v4 color = PullOff(v4);
		f32 scale = PullOff(f32);

		u32 nameLength = PullOff(u32);
		String name = CreateString((Char *)at, nameLength);
		u32 meshId = RegisterAsset(assetHandler, Asset_Mesh, name);
		at += nameLength * sizeof(Char);

		CreateEntity(world, meshId, scale, orientation, SnapToTileMap(pos), pos - V3(SnapToTileMap(pos)), color, Entity_None, 0);
	}

	u32 amountOfUnits = PullOff(u32);
	Assert(amountOfUnits < MaxUnitCount);

#if 0
	for (u32 i = 0; i < amountOfUnits; i++)
	{
		v2i pos = PullOff(v2i);
		Quaternion q = PullOff(Quaternion);
		CreateDude(unitHandler, pos, q);
	}
#endif

	ResetEditor(editor);

	editor->focusPoint = i12(ScreenZeroToOneToInGame(level->camera, V2(0.5f, 0.5f)));
	world->debugCamera = world->camera = level->camera;

	level->blocksNeeded = 10000;
	return true;
}

static EntityCopyData EntityToData(Entity e)
{
	EntityCopyData ret;

	ret.color = e.color;
	ret.flags = e.flags;
	ret.orientation = e.orientation;
	ret.meshId = e.meshId;
	ret.scale = e.scale;
	ret.type = e.type;
	ret.physicalPos = e.physicalPos;
	ret.offset = e.offset;
	return ret;

}

static bool LoadLevelV3(u8 *at, UnitHandler *unitHandler, World *world, AssetHandler *assetHandler, Editor *editor, Arena *currentStateArena)
{
	Level *level = &world->loadedLevel;

	level->lightSource = PullOff(v3);

	level->camera.pos = PullOff(v3);
	level->camera.b1 = PullOff(v3);
	level->camera.b2 = PullOff(v3);
	level->camera.b3 = PullOff(v3);
	level->camera.aspectRatio = PullOff(f32);
	level->camera.focalLength = PullOff(f32);

	u32 meshAmount = PullOff(u32);

	level->entities = PushArray(currentStateArena, EntityCopyData, meshAmount);

	For (level->entities)
	{
		Quaternion orientation = PullOff(Quaternion);
		v3 pos = PullOff(v3);
		v4 color = PullOff(v4);
		f32 scale = PullOff(f32);
		EntityType entityType = (EntityType)PullOff(u32);
		u64 flags = PullOff(u64);

		u32 nameLength = PullOff(u32);
		String name = CreateString((Char *)at, nameLength);
		u32 meshId = RegisterAsset(assetHandler, Asset_Mesh, name);
		at += nameLength * sizeof(Char);

		v3i physicalPos = SnapToTileMap(pos); 
		v3 offset = pos - V3(SnapToTileMap(pos));
		Entity e;
		switch (entityType)
		{
		case Entity_Wall:
		{
			e = CreateWall(world, meshId, physicalPos, scale, orientation, offset, color);
		}break;
		case Entity_Dude:
		{
			e = CreateDude(world, unitHandler, physicalPos, scale, orientation, offset, color);
		}break;
		case Entity_Block:
		{
			e = CreateBlock(world, physicalPos, scale, orientation, offset, color);
		}break;
		case Entity_Goal:
		{
			e = CreateGoal(world, meshId, physicalPos, scale, orientation, offset, color);
		}break;
		case Entity_Spawner:
		{
			e = CreateSpawner(world, meshId, physicalPos, scale, orientation, offset, color);
		}break;
		case Entity_None:
		{
			e = CreateEntity(world, meshId, scale, orientation, physicalPos, offset, color, Entity_None, 0);
		}break;
		InvalidDefaultCase;
		}
		*it = EntityToData(e);

	}

	ResetEditor(editor);

	level->blocksNeeded = 10000;
	editor->focusPoint = i12(ScreenZeroToOneToInGame(level->camera, V2(0.5f, 0.5f)));
	world->debugCamera = world->camera = level->camera;

	return true;
}

static bool LoadLevelV4(u8 *at, UnitHandler *unitHandler, World *world, AssetHandler *assetHandler, Editor *editor, Arena *currentStateArena)
{
	Level *level = &world->loadedLevel;

	level->lightSource = PullOff(v3);

	level->camera.pos = PullOff(v3);
	level->camera.b1 = PullOff(v3);
	level->camera.b2 = PullOff(v3);
	level->camera.b3 = PullOff(v3);
	level->camera.aspectRatio = PullOff(f32);
	level->camera.focalLength = PullOff(f32);

	u32 meshAmount = PullOff(u32);

	level->entities = PushArray(currentStateArena, EntityCopyData, meshAmount);

	For(level->entities)
	{
		Quaternion orientation = PullOff(Quaternion);
		v3i physicalPos = PullOff(v3i);
		v3 offset = PullOff(v3);
		v4 color = PullOff(v4);
		f32 scale = PullOff(f32);
		EntityType entityType = (EntityType)PullOff(u32);
		u64 flags = PullOff(u64);

		u32 nameLength = PullOff(u32);
		String name = CreateString((Char *)at, nameLength);
		u32 meshId = RegisterAsset(assetHandler, Asset_Mesh, name);
		at += nameLength * sizeof(Char);

		Entity e;
		switch (entityType)
		{
		case Entity_Wall:
		{
			e = CreateWall(world, meshId, physicalPos, scale, orientation, offset, color);
		}break;
		case Entity_Dude:
		{
			e = CreateDude(world, unitHandler, physicalPos, scale, orientation, offset, color);
		}break;
		case Entity_Block:
		{
			e = CreateBlock(world, physicalPos, scale, orientation, offset, color);
		}break;
		case Entity_Goal:
		{
			e = CreateGoal(world, meshId, physicalPos, scale, orientation, offset, color);
		}break;
		case Entity_Spawner:
		{
			e = CreateSpawner(world, meshId, physicalPos, scale, orientation, offset, color);
		}break;
		case Entity_None:
		{
			e = CreateEntity(world, meshId, scale, orientation, physicalPos, offset, color, Entity_None, 0);
		}break;
		InvalidDefaultCase;
		}
		*it = EntityToData(e);
	}

	level->blocksNeeded = 10000;

	ResetEditor(editor);

	editor->focusPoint = i12(ScreenZeroToOneToInGame(level->camera, V2(0.5f, 0.5f)));
	world->debugCamera = world->camera = level->camera;

	return true;
}


static bool LoadLevel(String fileName, UnitHandler *unitHandler, World *world, Arena *currentStateArena, AssetHandler *assetHandler, Editor *editor)
{
	File file = LoadFile(FormatCString("level/%s.level", fileName));
	if (!file.fileSize) return false;
	defer(FreeFile(file));

	UnloadLevel(world);
	ResetWorld(world);
	ResetUnitHandler(unitHandler);
	ResetEditor(editor);

	u8 *at = (u8 *)file.memory;
	u32 version = PullOff(u32);

	world->loadedLevel.name = CopyString(fileName, currentStateArena);

	switch (version)
	{
	case 1: 
	{
		return LoadLevelV1(at, unitHandler, world, assetHandler, editor, currentStateArena);
	}break;
	case 2:
	{
		return LoadLevelV2(at, unitHandler, world, assetHandler, editor, currentStateArena);
	}break;
	case 3:
	{
		return LoadLevelV3(at, unitHandler, world, assetHandler, editor, currentStateArena);
	}break;
	case 4:
	{
		return LoadLevelV4(at, unitHandler, world, assetHandler, editor, currentStateArena);
	}break;
	InvalidDefaultCase;
	}

	return false;
}

#undef PullOff

