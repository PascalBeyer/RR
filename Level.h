
static void WriteLevel(char *fileName, World world, UnitHandler unitHandler, AssetHandler *assetHandler)
{
	u8 *mem = PushData(frameArena, u8, 0);

	*PushStruct(frameArena, u32) = 1; // Version number, do not remove

	*PushStruct(frameArena, u32) = world.tileMap.width;
	*PushStruct(frameArena, u32) = world.tileMap.height;

	for (u32 i = 0; i < world.tileMap.width * world.tileMap.height; i++)
	{
		*PushStruct(frameArena, u32) = world.tileMap.tiles[i].type;
		*PushStruct(frameArena, u32) = world.tileMap.tiles[i].meshIndex;
	}

	*PushStruct(frameArena, u32) = world.placedMeshes.amount;
	For(world.placedMeshes)
	{
		//*PushStruct(frameArena, u32) = EntityType_Mesh;
		*PushStruct(frameArena, Quaternion) = it->orientation;
		*PushStruct(frameArena, v3) = it->pos;
		*PushStruct(frameArena, v4) = it->color;
		*PushStruct(frameArena, f32) = it->scale;
		String s = GetAssetInfo(assetHandler, it->meshId).name;
		*PushStruct(frameArena, u32) = s.length;
		Char *dest = PushData(frameArena, Char, s.length);
		memcpy(dest, s.data, s.length * sizeof(Char));
	}

	*PushStruct(frameArena, u32) = unitHandler.amount;
	for (u32 i = 0; i < unitHandler.amount; i++)
	{
		//*PushStruct(frameArena, u32) = EntityType_Unit;
		*PushStruct(frameArena, v2i) = unitHandler.infos[i].initialPos;
		*PushStruct(frameArena, Quaternion) = unitHandler.orientations[i];
	}

	u32 size = (u32)((u8*)frameArena->current - mem);
	File file = CreateFile(mem, size);
	WriteEntireFile(fileName, file);
}


#define PullOff(type) *(type *)at; at += sizeof(type);
static bool LoadLevel(String fileName, UnitHandler *unitHandler, World *world, Arena *currentStateArena, AssetHandler *assetHandler)
{
	unitHandler->amount = 0;
	world->placedMeshes.amount = 0;
	Clear(currentStateArena);
	File file = LoadFile((char *)FormatCString("level/%s.level", fileName).data);
	if (!file.fileSize) return false;
	defer(FreeFile(file));
	u8 *at = (u8 *)file.memory;

	u32 version = PullOff(u32);

	u32 tileMapWidth = PullOff(u32);
	u32 tileMapHeight = PullOff(u32);

	Assert(sizeof(Tile) == 8);
	TileArray tiles = PushArray(currentStateArena, Tile, tileMapHeight * tileMapWidth);

	For(tiles)
	{
		it->type = PullOff(TileMapType);
		it->meshIndex = PullOff(u32);
	}

	u32 meshAmount = PullOff(u32);
	Reserve(&world->placedMeshes, meshAmount);
	world->placedMeshes.amount = meshAmount;

	For(world->placedMeshes)
	{
		it->orientation = PullOff(Quaternion);
		it->pos = PullOff(v3);
		it->color = PullOff(v4);
		it->scale = PullOff(f32);

		u32 nameLength = PullOff(u32);
		String name = CreateString((Char *)at, nameLength);
		it->meshId = RegisterAsset(assetHandler, Asset_Mesh, name);
		it->untransformedAABB = GetMesh(assetHandler, it->meshId)->aabb;
		at += nameLength * sizeof(Char);
	}

	unitHandler->amount = PullOff(u32);
	Assert(unitHandler->amount < MaxUnitCount);

	for (u32 i = 0; i < unitHandler->amount; i++)
	{
		unitHandler->infos[i].initialPos = PullOff(v2i);
		unitHandler->orientations[i] = PullOff(Quaternion);
	}

	return true;
}

#undef PullOff

