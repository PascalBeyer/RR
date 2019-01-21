
#define MaxUnitInstructions 100
enum UnitInstruction
{
   Unit_Wait,
	Unit_MoveUp,
	Unit_MoveDown,
	Unit_MoveLeft,
	Unit_MoveRight,
   
   Unit_Instruction_Count,
};
DefineDynamicArray(UnitInstruction);

struct UnitData
{
   u32 serial;
   UnitInstructionDynamicArray instructions;
   UnitInstruction currentInstruction;
   u32 at;
   f32 t;
};
DefineDynamicArray(UnitData);


struct EntitySerialResult
{
   u16 type; // this could be a u8... but there are no u 24... maybe we find some kind of static data...
   u16 index;
};
DefineDynamicArray(EntitySerialResult);
static EntitySerialResult InvalidEntitySerial___()
{
   return {0xFFFF, 0xFFFF};
}
#define InvalidEntitySerial InvalidEntitySerial___()



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
   
   TileOctTree entityTree;
   UnitDataDynamicArray unitData;
   
   
   BuddyAllocator alloc;
   union
   {
      EntityDynamicArray entityArrays[Entity_Count];
      struct
      {
         EntityDynamicArray noneArray;
         EntityDynamicArray unitArray;
         EntityDynamicArray wallArray;
      };
   };
   AnimationStateDynamicArray animationStates; // parralel to unitArray
   
   u32 serializer;
   EntitySerialResultDynamicArray entitySerialMap; // this is just huge and that fine???
};

static Entity *GetEntity(EntityManager *entityManager, u32 serial)
{
   EntitySerialResult result = entityManager->entitySerialMap[serial];
   Entity *ret = entityManager->entityArrays[result.type] + result.index;
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
      if (e->serial == cur->entitySerials[i])
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
         cur->entitySerials[i] = e->serial;
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

// not used right now
static void ResetTree(EntityManager *entityManager, TileOctTree *tree) // todo speed can this be lineraized?
{
   ResetTreeHelper(entityManager, tree, &tree->root);
   tree->root.isLeaf = true;
   for (u32 i = 0; i < ArrayCount(tree->root.entitySerials); i++)
   {
      tree->root.entitySerials[i] = 0xFFFFFFFF;
   }
   
   for(u32 i = 1; i < Entity_Count; i++)
   {
      For(entityManager->entityArrays[i])
      {
         it->flags &= ~EntityFlag_InTree;
      }
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

static Entity *CreateEntityInternal(EntityManager *entityManager, EntityData data)
{
   Entity ret;
   ApplyEntityDataToEntity(&ret, &data);
   
   ret.serial = entityManager->serializer++;
   ret.initialPos = ret.physicalPos;
   Assert(ret.type < Entity_Count);
   
   u32 arrayIndex = ArrayAdd(&entityManager->entityArrays[ret.type], ret);
   
   Assert(ret.serial == entityManager->entitySerialMap.amount);
   EntitySerialResult toAdd;
   toAdd.index = (u16)arrayIndex;
   toAdd.type = (u16)ret.type;
   ArrayAdd(&entityManager->entitySerialMap, toAdd);
   if (ret.flags) // entity != none? // move this out?
   {
      InsertEntity(entityManager, entityManager->entityArrays[ret.type] + arrayIndex);
   }
   
   return entityManager->entityArrays[ret.type] + arrayIndex;
};

static void RestoreEntity(EntityManager *entityManager, EntityData data)
{
   Entity ret;
   ApplyEntityDataToEntity(&ret, &data);
   
   u32 arrayIndex = ArrayAdd(&entityManager->entityArrays[ret.type], ret);
   EntitySerialResult toAdd;
   toAdd.index = (u16)arrayIndex;
   toAdd.type = (u16)ret.type;
   Assert(entityManager->entitySerialMap[ret.serial].index == 0xFFFF);
   
   entityManager->entitySerialMap[ret.serial] = toAdd;
   if (ret.flags) // entity != none?
   {
      InsertEntity(entityManager, entityManager->entityArrays[ret.type] + arrayIndex);
   }
}

static void RemoveEntity(EntityManager *entityManager, u32 serial)
{
   EntitySerialResult result = entityManager->entitySerialMap[serial];
   Assert(result.type < Entity_Count);
   Entity *toRemove = entityManager->entityArrays[result.type] + result.index;
   RemoveEntityFromTree(entityManager, toRemove);
   
   u16 index = result.index;
   entityManager->entitySerialMap[serial] = InvalidEntitySerial;
   
   u32 lastIndex = entityManager->entityArrays[result.type].amount - 1;
   if (index != lastIndex)
   {
      u32 serialNumberToChange = entityManager->entityArrays[result.type][lastIndex].serial;
      entityManager->entitySerialMap[serialNumberToChange].index = index;
   }
   UnorderedRemove(&entityManager->entityArrays[result.type], index);
}

static void ResetEntityManager(EntityManager *entityManager)
{
   For(entityManager->unitData)
   {
      it->t = 1.0f;
      it->at = 0;
   }
   
   For(entityManager->unitArray)
   {
      RemoveEntityFromTree(entityManager, it);
   }
   
   For(entityManager->unitArray)
   {
      it->physicalPos = it->initialPos;
      it->flags &= ~EntityFlag_FrameResetFlag;
      InsertEntity(entityManager, it);
      it->visualPos = V3(it->initialPos);
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
         return EntityFlag_BlocksUnit | EntityFlag_SupportsUnit | EntityFlag_IsDynamic;
      }break;
      case Entity_Wall:
      {
         return EntityFlag_BlocksUnit | EntityFlag_SupportsUnit;
      }break;
      InvalidDefaultCase;
   };
   return 0;
}

static Entity CreateDude(EntityManager *entityManager, v3i pos, f32 scale = 1.0f, Quaternion orientation = {1, 0, 0, 0}, v3 offset = V3(), v4 color = V4(1, 1, 1, 1), u32 meshId = 0xFFFFFFFF)
{
   EntityData entityData;
   entityData.orientation = orientation;
   entityData.physicalPos = pos;
   entityData.offset      = offset;
   entityData.color       = color;
   entityData.scale       = scale;
   
   entityData.flags       = GetStandardFlags(Entity_Dude);
   entityData.type        = Entity_Dude;
   
   entityData.meshId      = meshId;
   
   Entity *e = CreateEntityInternal(entityManager, entityData);
   
   UnitData data = {};
   data.serial = e->serial;
   data.instructions = UnitInstructionCreateDynamicArray(&entityManager->alloc);
   ArrayAdd(&entityManager->unitData, data);
   
   AnimationState anim = {};
   anim.meshId = e->meshId;
   anim.serial = e->serial;
   ForC(anim.animations) 
   {
      it->animationId = 0xFFFFFFFF;
   }
   ForC(anim.iks)
   {
      it->boneIndex = 0xFFFFFFFF;
   }
   ArrayAdd(&entityManager->animationStates, anim);
   
   return *e;
}

static Entity CreateWall(EntityManager *entityManager, v3i pos, f32 scale = 1.0f, Quaternion orientation = {1, 0, 0, 0}, v3 offset = V3(), v4 color = V4(1, 1, 1, 1), u32 meshId = 0xFFFFFFFF)
{
   EntityData entityData;
   entityData.orientation = orientation;
   entityData.physicalPos = pos;
   entityData.offset      = offset;
   entityData.color       = color;
   entityData.scale       = scale;
   
   entityData.type        = Entity_Wall;
   entityData.flags       = GetStandardFlags(Entity_Wall);
   
   entityData.meshId      = meshId;
   
   return *CreateEntityInternal(entityManager, entityData);
}

static Entity CreateEntityNone(EntityManager *entityManager, v3i pos, f32 scale = 1.0f, Quaternion orientation = {1, 0, 0, 0}, v3 offset = V3(), v4 color = V4(1, 1, 1, 1), u32 meshId = 0xFFFFFFFF)
{
   EntityData entityData;
   entityData.orientation = orientation;
   entityData.physicalPos = pos;
   entityData.offset      = offset;
   entityData.color       = color;
   entityData.scale       = scale;
   
   entityData.type        = Entity_None;
   entityData.flags       = GetStandardFlags(Entity_None);
   
   entityData.meshId      = meshId;
   
   return *CreateEntityInternal(entityManager, entityData);
}

// this seems pretty bad in retrospect
static Entity CreateEntity(EntityManager *entityManager, EntityType type, u32 meshId, v3i pos, f32 scale, Quaternion orientation, v3 offset, v4 color, u64 flags)
{
   switch (type)
   {
      case Entity_None:
      {
         return CreateEntityNone(entityManager, pos, scale, orientation, offset, color,  meshId);
      }break;
      case Entity_Dude:
      {
         return CreateDude(entityManager, pos, scale, orientation, offset, color, meshId);
      }break;
      case Entity_Wall:
      {
         return CreateWall(entityManager, pos, scale, orientation, offset, color, meshId);
      }break;
      
      InvalidDefaultCase;
   }
   
   return {};
}

static void InitEntityManager(EntityManager *entityManager, Arena *currentStateArena, Level *level)
{
   
   entityManager->levelName = CopyString(level->name, currentStateArena);
   entityManager->serializer = 0;
   entityManager->entityTree = InitOctTree(currentStateArena, 100); // todo hardcoded.
   
   entityManager->alloc = CreateBuddyAllocator(currentStateArena, MegaBytes(64), 1024);
   // todo something something, probabaly do not allocate level->entites for each one...
   entityManager->noneArray = EntityCreateDynamicArray(&entityManager->alloc, 10);
   entityManager->unitArray = EntityCreateDynamicArray(&entityManager->alloc, 10);
   entityManager->wallArray = EntityCreateDynamicArray(&entityManager->alloc, level->entities.amount);
   
   entityManager->animationStates = AnimationStateCreateDynamicArray(&entityManager->alloc);
   
   entityManager->entitySerialMap = EntitySerialResultCreateDynamicArray(&entityManager->alloc, level->entities.amount);
   entityManager->unitData = UnitDataCreateDynamicArray(&entityManager->alloc);
   
   For(level->entities)
   {
      CreateEntity(entityManager, it->type, it->meshId, it->physicalPos, it->scale, it->orientation, it->offset, it->color, it->flags);
   }
   
#if 1
   RandomSeries series = GetRandomSeries();
   For(entityManager->unitData)
   {
      for(u32 i = 0; i < 10; i++)
      {
         ArrayAdd(&it->instructions, (UnitInstruction)((RandomU32(&series)>>10) % Unit_Instruction_Count));
      }
   }
#endif
}

static v3i GetAdvanceForOneStep(UnitInstruction step)
{
   v3i advance;
   switch (step)
   {
      case Unit_Wait:
      {
         advance = {};
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
         advance = {};
         Die;
      }break;
      
   }
   return advance;
}

static UnitInstruction GetOneStepForDir(v3i dir)
{
   
   if(dir == V3i(0, 0, 0))
   {
      return Unit_Wait;
   }else if(dir == V3i(1, 0, 0))
   {
      return Unit_MoveRight;
   }else if(dir == V3i(-1, 0, 0))
   {
      return Unit_MoveLeft;
   }else if(dir == V3i(0, 1, 0))
   {
      return Unit_MoveUp;
   }else if(dir == V3i(0, -1, 0))
   {
      return Unit_MoveDown;
   }
   else
   {
      Die;
   }
   
   return Unit_Wait;
}

static v3i SnapToTileMap(v3 pos)
{
   return V3i((i32)pos.x, (i32)pos.y, (i32)pos.z);
}
