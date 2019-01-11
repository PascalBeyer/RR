

// todo copy and paste of editor.h, they will differ eventrually, because this shout utilize the entity tree.
static Entity *GetHotEntity(Camera cam, EntityManager *entityManager, AssetHandler *handler, v2 mousePosZeroToOne)
{
   
   
   v3 camP = cam.pos; // todo camera or debugCamera? Maybe we should again unify them
   v3 camD = ScreenZeroToOneToDirecion(cam, mousePosZeroToOne);
   
   f32 minDist = F32MAX;
   
   Entity *ret = NULL;
   
   For(entityManager->entities)
   {
      
      MeshInfo *info = GetMeshInfo(handler, it->meshId);
      
      if (!info) continue; // probably not on screen, if never rendered
      
      m4x4 mat = QuaternionToMatrix4(Inverse(it->orientation)); // todo save these?
      v3 rayP = mat * (camP - GetRenderPos(*it));
      v3 rayD = mat * camD; 
      // better rayCast system, right now this loads every mesh, to find out the aabb....
      
      AABB aabb = info->aabb;
      
      aabb.maxDim *= it->scale;
      aabb.minDim *= it->scale;
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
      
      
      if (PointInAABB(aabb, curExit))
      {
         f32 dist = Dist(curExit, rayP);
         if (dist < minDist)
         {
            minDist = dist;
            ret = it;
         }
      }
   }
   
   return ret;
}



enum ExecuteState
{
	Execute_None,
   
	Execute_LevelBegin,
	Execute_PlacingUnits,
	Execute_PathCreator,
	Execute_Simulation,
	Execute_Victory,
   
	Execute_Count,
};

struct SimData
{
	u32 blocksNeeded;
	u32 blocksCollected;
	f32 timeScale;
};

struct ExecuteData
{
	u32 state;
   
	Camera camera;
	Camera debugCamera;
   LightSource lightSource;
   
	u32 at; // execute data?
	f32 t;
   
	PathCreator pathCreator;
	SimData simData;
};
static ExecuteData InitExecute()
{
	ExecuteData ret;
	ret.state = Execute_None;
	return ret;
}

static void ChangeExecuteState(EntityManager *entityManager, ExecuteData *exe, ExecuteState state)
{
	exe->state = state;
	switch (state)
	{
      case Execute_LevelBegin:
      {
         
      }break;
      case Execute_PlacingUnits:
      {
         
      }break;
      case Execute_PathCreator:
      {
         exe->pathCreator = InitPathCreator();
         ResetEntityManager(entityManager);
      }break;
      case Execute_Simulation:
      {
         exe->simData.timeScale = 1.0f;
         exe->simData.blocksCollected = 0;
         exe->simData.blocksNeeded = 1000;
         ResetEntityManager(entityManager);
      }break;
      case Execute_Victory:
      {
         
      }break;
      
      InvalidDefaultCase;
	}
}

static void MaybeMoveEntity(Entity *e, v3i dir, EntityManager *entityManager, InterpolationType type);
static void MaybeCarryEntity(Entity *e, v3i dir, EntityManager *entityManager)
{
	EntityPtrArray entitiesToCarry = GetEntities(entityManager, e->physicalPos + V3i(0, 0, -1));
   
	For(_it, entitiesToCarry)
	{
		auto it = *_it;
		if (it->flags & EntityFlag_CanBeCarried)
		{
			MaybeMoveEntity(it, dir, entityManager, Interpolation_Carried);
		}
	}
}

static void MaybeMoveEntity(Entity *e, v3i dir, EntityManager *entityManager, InterpolationType type)
{
	if (e->flags & EntityFlag_IsMoving)
	{
		if (type < e->interpolation.type) // '=' ?
		{
			return;
		}
	}
	if (dir == V3i()) return;
   
	v3i intendedPos = e->physicalPos + dir;
   
	// todo for now we just push if it is pushable, maybe we should only push if all are pushable...
	EntityPtrArray blockingEntities = GetEntities(entityManager, intendedPos, EntityFlag_PushAble);
   
	For(blockingEntities)
	{
		auto b = *it;
		MaybeMoveEntity(b, dir, entityManager, Interpolation_Push);
	}
   
	MaybeCarryEntity(e, dir, entityManager);
   
	e->interpolation.dir = dir;
	e->interpolation.type = type;
   
	Assert(type);
   
	e->flags |= EntityFlag_IsMoving;
}


struct EntityCollision
{
	v3i tile;
	Entity *e;
	EntityCollision *next;
};

DefineDFArray(EntityCollision);

static void MaybeStartFalling(EntityManager *entityManager, Entity *e)
{
	EntityPtrArray below = GetEntities(entityManager, e->physicalPos + V3i(0, 0, 1), EntityFlag_SupportsUnit);
   
	if (!below)
	{
		e->flags |= EntityFlag_IsFalling;
		e->flags |= EntityFlag_IsMoving;
		e->interpolation.dir = V3i(0, 0, 1);
		e->interpolation.type = Interpolation_Fall;
		MaybeCarryEntity(e, V3i(0, 0, 1), entityManager);
	}
	
}

static void AdvanceGameState(EntityManager *entityManager, ExecuteData *exe, b32 checkForVictory = true)
{
	TimedBlock;
	SimData *sim = exe ? &exe->simData : NULL;
   
	EntityPtrDFArray movingEntities = EntityPtrCreateDFArray();
   
	For(entityManager->entities) // this should only be dynamic entities
	{
		it->flags &= ~(EntityFlag_IsFalling | EntityFlag_IsMoving);
		it->interpolation = {};
	}
   
	// this should get "passed in" and what ever.
	u64 at = exe->at++;
   
	For(entityManager->entities)
	{
		switch (it->type)
		{
         case Entity_None:
         case Entity_Wall:
         {
            
         }break;
         case Entity_Block:
         {			
            if (it->flags & BlockFlag_TryingToSpawn)
            {
               if (it->physicalPos == it->initialPos)
               {
                  it->flags &= ~BlockFlag_TryingToSpawn; 
                  //it->flags |= EntityFlag_PushAble; 
                  //this does not work this way, as it might be pushed _before_ this gets set.maybe move the pushing code into the eval ?
                  break;
               }
               
               if (checkForVictory && ++sim->blocksCollected >= sim->blocksNeeded)
               {
                  ChangeExecuteState(entityManager, exe, Execute_Victory);
                  return;
               }
               
               v3i dir = it->initialPos - it->physicalPos;
               MaybeMoveEntity(it, dir, entityManager, Interpolation_Teleport);
            }
            
            MaybeStartFalling(entityManager, it);
            EntityPtrArray supportingEntities = GetEntities(entityManager, it->physicalPos + V3i(0, 0, 1), EntityFlag_SupportsUnit);
            
            EntityArray arr = {};
            
            For(_s, supportingEntities)
            {
               auto s = *_s;
               if (s->type == Entity_Goal)
               {
                  it->flags |= BlockFlag_TryingToSpawn;
                  //it->flags &= ~EntityFlag_PushAble;
                  break;
               }
            }
            
         }break;
         case Entity_Dude:
         {
            UnitInstructionDynamicArray *p = &it->instructions;
            MaybeStartFalling(entityManager, it);
            if (!p->amount) continue;
            
            u32 unitAt = (at % p->amount);
            
            auto command = (*p)[unitAt];
            
            MaybeMoveEntity(it, GetAdvanceForOneStep(command), entityManager, Interpolation_Move); 
            
         }break;
         case Entity_Spawner:
         {
            
         }break;
         case Entity_Goal:
         {
            
         }break;
         
		}
	}
   
	// can't be welded in above, because they might get pushed or smth, after they update, could weld it into the maybe move sort of thing
	For(entityManager->entities)
	{
		if (it->flags & EntityFlag_IsMoving)
		{
			ArrayAdd(&movingEntities, it);
		}
      
	}
   
	EntityPtrDFArray resetEntities = EntityPtrCreateDFArray();
	EntityCollisionDFArray collisions = EntityCollisionCreateDFArray();
   
	For(movingEntities)
	{
		Entity *e = *it;
		RemoveEntityFromTree(entityManager, e);
		
		v3i dir = e->interpolation.dir;
      
		v3i nextPos = e->physicalPos + dir;
      
		b32 found = false;
		For(c, collisions)// maybe this can be made faster?
		{
			if (nextPos == c->tile)
			{
				found = true;
				EntityCollision *collision = PushStruct(frameArena, EntityCollision);
				collision->e = e;
				collision->next = c->next;
				collision->tile = c->tile;
            
				c->next = collision;
				break;
			}
		}
      
		if (!found)
		{
			EntityCollision collision;
			collision.e = e;
			collision.next = NULL;
			collision.tile = nextPos;
         
			ArrayAdd(&collisions, collision);
		}
	}
   
	For(collisions)
	{
		
		if (!it->next)
		{
			//move for non-colliding Entities
			Assert(it->tile == it->e->physicalPos + it->e->interpolation.dir);
         
			// todo, speed, we are walking the tree two times, for no particular reason.
			EntityPtrArray blockingEntities = GetEntities(entityManager, it->tile, EntityFlag_BlocksUnit);
         
			if (blockingEntities.amount)
			{
				ArrayAdd(&resetEntities, it->e);
				it->e->interpolation.type = Interpolation_Blocked;
			}
			else
			{
				it->e->physicalPos = it->tile;
				InsertEntity(entityManager, it->e);
			}
			continue;
		}
      
		for (EntityCollision *c = it; c; c = c->next)
		{
			c->e->interpolation.type = Interpolation_Blocked;
			ArrayAdd(&resetEntities, c->e);
		}
	}
   
	For(e, resetEntities)
	{
		EntityPtrArray blockingEntities = GetEntities(entityManager, (*e)->physicalPos, EntityFlag_BlocksUnit);
		
		For(_it, blockingEntities)
		{
			auto it = *_it;
			Assert(it->flags & EntityFlag_IsMoving);
			Assert(it->interpolation.type != Interpolation_Blocked);
			it->interpolation.type = Interpolation_Blocked;
			RemoveEntityFromTree(entityManager, it);
			it->physicalPos -= it->interpolation.dir;
			ArrayAdd(&resetEntities, it);
		}
      
		if (blockingEntities.amount)
		{
			ArrayAdd(&resetEntities, *e);
		}
		else
		{
			InsertEntity(entityManager, *e);
		}
	}
}

static void UpdateSimulation(EntityManager *entityManager, ExecuteData *exe, f32 dt)
{
	exe->t += dt * exe->simData.timeScale;
   
	while (exe->t > 1.0f)
	{
		exe->t -= 1.0f;
      
		AdvanceGameState(entityManager, exe);
		if (exe->state == Execute_Victory) break;
	}
}

static void GameExecuteUpdate(EntityManager *entityManager, ExecuteData *exe, f32 dt)
{
	SimData *sim = &exe->simData;
   
	switch (exe->state)
	{
      case Execute_None:
      {
         Die;
      }break;
      case Execute_LevelBegin:
      {
         
      }break;
      case Execute_PlacingUnits:
      {
         
      }break;
      case Execute_PathCreator:
      {
         //UpdatePathCreator();
      }break;
      case Execute_Simulation:
      {
         UpdateSimulation(entityManager, exe, dt);
      }break;
      case Execute_Victory:
      {
         
      }break;
      
      InvalidDefaultCase;
      
	}
   
   
}

struct ProgressBar
{
	v2 pos;
	f32 border;
	f32 width;
	f32 heigth;
   
	v4 unfilledColor;
	v4 filledColor;
};
