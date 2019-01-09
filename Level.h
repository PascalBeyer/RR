
static bool WriteLevel(char *fileName, World world, AssetHandler *assetHandler)
{
	u8 *mem = PushData(frameArena, u8, 0);
   
	*PushStruct(frameArena, u32) = 5; // Version number, do not remove
   
	*PushStruct(frameArena, LightSource) = world.lightSource;
   
	*PushStruct(frameArena, v3) = world.loadedLevel.camera.pos;
	*PushStruct(frameArena, Quaternion) = world.loadedLevel.camera.orientation;
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

static bool LoadLevelV3(u8 *at, World *world, AssetHandler *assetHandler, Editor *editor, Arena *currentStateArena)
{
	Level *level = &world->loadedLevel;
   
	level->lightSource.pos = PullOff(v3);
   level->lightSource.orientation = QuaternionId();
   
	level->camera.pos = PullOff(v3);
   PullOff(v3);
   PullOff(v3);
   PullOff(v3);
	level->camera.orientation = QuaternionId();
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
      
		Entity e = CreateEntity(world, entityType, meshId, physicalPos, scale, orientation, offset, color, 0);
      
		*it = EntityToData(e);
      
	}
   
	ResetEditor(editor);
   
	level->blocksNeeded = 10000;
	editor->focusPoint = i12(ScreenZeroToOneToInGame(level->camera, V2(0.5f, 0.5f)));
	world->debugCamera = world->camera = level->camera;
   
	return true;
}

static bool LoadLevelV4(u32 version, u8 *at, World *world, AssetHandler *assetHandler, Editor *editor, Arena *currentStateArena)
{
	Level *level = &world->loadedLevel;
   
	level->lightSource = PullOff(LightSource);
   
	level->camera.pos = PullOff(v3);
   
   if(version < 5)
   {
      PullOff(v3);
      PullOff(v3);
      PullOff(v3);
      level->camera.orientation = QuaternionId();
   }
   else 
   {
      level->camera.orientation = PullOff(Quaternion);
   }
   
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
      
		Entity e = CreateEntity(world, entityType, meshId, physicalPos, scale, orientation, offset, color, 0);
		*it = EntityToData(e);
	}
   
	level->blocksNeeded = 10000;
   
	ResetEditor(editor);
   
	editor->focusPoint = i12(ScreenZeroToOneToInGame(level->camera, V2(0.5f, 0.5f)));
	world->debugCamera = world->camera = level->camera;
   
	return true;
}


static bool LoadLevel(String fileName, World *world, Arena *currentStateArena, AssetHandler *assetHandler, Editor *editor)
{
	File file = LoadFile(FormatCString("level/%s.level", fileName));
	if (!file.fileSize) return false;
	defer(FreeFile(file));
   
	UnloadLevel(world);
	ResetWorld(world);
	ResetEditor(editor);
   
	u8 *at = (u8 *)file.memory;
	u32 version = PullOff(u32);
   
	world->loadedLevel.name = CopyString(fileName, currentStateArena);
   
	switch (version)
	{
      case 1: 
      {
         Die;
      }break;
      case 2:
      {
         Die;
      }break;
      case 3:
      {
         return LoadLevelV3(at, world, assetHandler, editor, currentStateArena);
      }break;
      case 4:
      {
         return LoadLevelV4(4, at, world, assetHandler, editor, currentStateArena);
      }break;
      case 5:
      {
         return LoadLevelV4(version, at, world, assetHandler, editor, currentStateArena);
      }break;
      InvalidDefaultCase;
	}
   
	return false;
}

#undef PullOff

