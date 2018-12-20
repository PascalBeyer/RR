
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

// place units on release, so we can use it both for moving em it the path creator and the inital thing
struct PlacingUnitsData 
{
	EntityPtrDynamicArray unitsToPlace;
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

	PathCreator pathCreator;
	SimData simData;
	PlacingUnitsData placingUnits;
};
static ExecuteData InitExecute()
{
	ExecuteData ret;
	ret.state = Execute_None;
	ret.placingUnits.unitsToPlace = EntityPtrCreateDynamicArray();
	return ret;
}

static void ChangeExecuteState(World *world, UnitHandler *unitHandler, ExecuteData *exe, ExecuteState state)
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
		ResetWorld(world);
	}break;
	case Execute_Simulation:
	{
		exe->simData.timeScale = 1.0f;
		exe->simData.blocksCollected = 0;
		exe->simData.blocksNeeded = 1000;
		ResetWorld(world);
	}break;
	case Execute_Victory:
	{

	}break;
	
	InvalidDefaultCase;
	}
}

static Entity *TileBlocked(World *world, v3i tile)
{
	Entity *e = GetEntity(world, tile);
	if (e && e->flags & EntityFlag_BlocksUnit)
	{
		return e;
	}

	return NULL;
}

// make dir an enum? so we cant push in something weird?
static void MaybeMoveEntity(Entity *e, v3i dir, World *world, InterpolationType type)
{
	if (e->temporaryFlags & EntityFlag_IsMoving)
	{
		if (type < e->interpolation.type) // '=' ?
		{
			return;
		}
	}
	if (dir == V3i()) return;

	v3i intendedPos = e->physicalPos + dir;

	Entity *blockingEntity = TileBlocked(world, intendedPos);
	if (blockingEntity && (blockingEntity->flags & EntityFlag_PushAble))
	{
		MaybeMoveEntity(blockingEntity, dir, world, Interpolation_Push);
	}

	e->interpolation.dir = dir;
	e->interpolation.type = type;

	Entity *below = GetEntity(world, e->physicalPos + V3i(0, 0, 1));
	e->interpolation.entityBelow = below ? below->serialNumber : 0xFFFFFFFF;
	Entity *above = GetEntity(world, e->physicalPos + V3i(0, 0, -1));
	e->interpolation.entityAbove = above ? above->serialNumber : 0xFFFFFFFF;

	if (below)
	{
		e->temporaryFlags |= EntityFlag_IsSupported;
	}

	e->temporaryFlags |= EntityFlag_IsMoving;
}

typedef Entity* EntityPtr;

DefineDynamicFrameArray(EntityPtr);

struct EntityCollision
{
	v3i tile;
	Entity *e;
	EntityCollision *next;
};

DefineDynamicFrameArray(EntityCollision);

static void MaybeStartFalling(World *world, Entity *e)
{
	Entity *below = GetEntity(world, e->physicalPos + V3i(0, 0, 1));

	if (!below)
	{
		e->temporaryFlags |= EntityFlag_IsFalling;
		e->temporaryFlags |= EntityFlag_IsMoving;
		e->interpolation.dir = V3i(0, 0, 1);
		Entity *above = GetEntity(world, e->physicalPos + V3i(0, 0, -1));
		e->interpolation.entityAbove = above ? above->serialNumber : 0xFFFFFFFF;
		e->interpolation.entityBelow = 0xFFFFFFFF;
		e->interpolation.type = Interpolation_Fall;
	}
	else
	{
		e->temporaryFlags |= EntityFlag_IsSupported; // this is kinda ironius with is falling...
	}

}

static void AdvanceGameState(World *world, UnitHandler *unitHandler, ExecuteData *exe, b32 checkForVictory = true)
{
	TimedBlock;
	SimData *sim = exe ? &exe->simData : NULL;

	EntityPtrDFArray movingEntities = EntityPtrCreateDFArray();

	For(world->entities)
	{
		it->temporaryFlags = 0;
		it->interpolation = {};
	}

	// this should get "passed in" and what ever.
	u64 at = world->at++;

	For(world->entities)
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
					ChangeExecuteState(world, unitHandler, exe, Execute_Victory);
					return;
				}

				v3i dir = it->initialPos - it->physicalPos;
				MaybeMoveEntity(it, dir, world, Interpolation_PhaseMove);
			}

			MaybeStartFalling(world, it);
			Entity *supportingEntity = GetEntity(world, it->physicalPos + V3i(0, 0, 1));

			if (supportingEntity && supportingEntity->type == Entity_Goal)
			{
				it->flags |= BlockFlag_TryingToSpawn;
				//it->flags &= ~EntityFlag_PushAble;
				break;
			}

		}break;
		case Entity_Dude:
		{
			u32 unitIndex = GetUnitIndex(it);
			Assert(unitIndex < unitHandler->amount);
			UnitInstructionArray *p = unitHandler->programs + unitIndex;
			MaybeStartFalling(world, it);
			if (!p->amount) continue;

			u32 unitAt = (at % p->amount);

			auto command = (*p)[unitAt];
			MaybeMoveEntity(it, GetAdvanceForOneStep(command), world, Interpolation_Move); 
			
		}break;
		case Entity_Spawner:
		{

		}break;
		case Entity_Goal:
		{

		}break;

		}
	}

	// can't be welded in above, because they might get pushed or smth, after they update
	For(world->entities)
	{
		if (it->temporaryFlags & EntityFlag_IsMoving)
		{
			ArrayAdd(&movingEntities, it);
		}

	}

	EntityPtrDFArray resetEntities = EntityPtrCreateDFArray();
	EntityCollisionDFArray collisions = EntityCollisionCreateDFArray();

	For(movingEntities)
	{
		Entity *e = *it;
		if (e->temporaryFlags & EntityFlag_IsSupported)
		{
			Entity *below = GetEntity(world, e->interpolation.entityBelow);
			if ((below->temporaryFlags & EntityFlag_IsMoving) && !(below->temporaryFlags & EntityFlag_MoveResolved))
			{
				ArrayAdd(&movingEntities, e);
				continue;
			}
		}
		else
		{
			u32 above = e->interpolation.entityAbove;
			if (above != 0xFFFFFFFF)
			{
				Entity *entityAbove = GetEntity(world, above);
				if (!(entityAbove->temporaryFlags & EntityFlag_IsMoving))
				{
					entityAbove->interpolation.dir = V3i(0, 0, 1);
					entityAbove->interpolation.type = Interpolation_Fall;
					entityAbove->interpolation.entityBelow = e->serialNumber;
					Entity *aboveAbove = GetEntity(world, entityAbove->physicalPos + V3i(0, 0, -1));
					entityAbove->interpolation.entityAbove = aboveAbove ? aboveAbove->serialNumber : 0xFFFFFFFF;
					ArrayAdd(&movingEntities, entityAbove);
				}
			}
		}

		RemoveEntityFromTree(world, e);
		e->temporaryFlags |= EntityFlag_MoveResolved;

		v3i dir = e->interpolation.dir;

		while(e)
		{
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

			if (e != *it) 
			{
				e->interpolation.dir += dir;
			}

			if (e->interpolation.entityAbove == 0xFFFFFFFF)
			{
				break;
			}

			Entity *above = GetEntity(world, e->interpolation.entityAbove);
			if (!(above->temporaryFlags & EntityFlag_IsMoving))
			{
				RemoveEntityFromTree(world, above);
				above->interpolation.dir = V3i();
				above->interpolation.type = Interpolation_Carried;
				above->interpolation.entityBelow = e->serialNumber;
				Entity *aboveAbove = GetEntity(world, above->physicalPos + V3i(0, 0, -1));
				above->interpolation.entityAbove = aboveAbove ? aboveAbove->serialNumber : 0xFFFFFFFF;
				
			}

			e = above;
		}
		
	}

	For(collisions)
	{
		
		if (!it->next)
		{
			if (it->e->temporaryFlags & EntityFlag_QueuedForReset) continue;
			//move for non-colliding Entities
			v3i oldP = it->e->physicalPos;
			Entity *blockingEntity;
			if (it->tile == it->e->physicalPos + it->e->interpolation.dir)
			{
				it->e->physicalPos = it->tile;
				it->e->temporaryFlags |= EntityFlag_IsMoving;
				blockingEntity = InsertEntity(world, it->e);
			}
			else
			{
				blockingEntity = TileBlocked(world, it->tile);
			}

			if (blockingEntity)
			{
				it->e->physicalPos = oldP;
				it->e->temporaryFlags |= EntityFlag_QueuedForReset;
				ArrayAdd(&resetEntities, it->e);
			}
			continue;
		}

		for (EntityCollision *c = it; c; c = c->next)
		{
			if (c->e->temporaryFlags & EntityFlag_QueuedForReset) continue;
			c->e->temporaryFlags |= EntityFlag_QueuedForReset;
			ArrayAdd(&resetEntities, c->e);
		}
	}

	For(resetEntities)
	{
		Entity *e = *it;
		Assert(e->temporaryFlags & EntityFlag_QueuedForReset);

		Entity *blockingEntity = InsertEntity(world, *it, true);
		if (blockingEntity)
		{
			blockingEntity->physicalPos -= blockingEntity->interpolation.dir;
			*it = blockingEntity;
			blockingEntity->temporaryFlags |= EntityFlag_QueuedForReset;
		}
		else
		{
			UnorderedRemove(&resetEntities, (u32)(it - resetEntities.data));
		}

		it--;
	}

}

static u32 GetLeastCommonMultipleForUnits(UnitHandler *unitHandler)
{
	if (unitHandler->amount == 0) return 0;

	u32 ret = unitHandler->programs->amount;
	for (u32 i = 1; i < unitHandler->amount; i++)
	{
		ret = LCM(ret, unitHandler->programs[i].amount);
	}
	return ret;
}

static void UpdateSimulation(World *world, UnitHandler *unitHandler, ExecuteData *execute, f32 dt)
{
	//dt = 0.333333f;
	unitHandler->t += dt * execute->simData.timeScale;
	//unitHandler->t = 1.1f;
#if 0
	u32 amountOfIterationPerCycles = GetLeastCommonMultipleForUnits(unitHandler);
	u32 utime = (u32)unitHandler->t;
	u32 relativeTime = utime % amountOfIterationPerCycles; // we would have to do this with _all_ entities

	u32 currentAt = unitHandler->at % amountOfIterationPerCycles;
#endif

	while (unitHandler->t > 1.0f)
	{
		unitHandler->t -= 1.0f;

		AdvanceGameState(world, unitHandler, execute);
		if (execute->state == Execute_Victory) break;
	}
}

static void GameExecuteUpdate(World *world, UnitHandler *unitHandler, ExecuteData *exe, f32 dt)
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
		UpdateSimulation(world, unitHandler, exe, dt);
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
