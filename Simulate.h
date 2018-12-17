
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

enum ExecuteUndoType
{
	ExecuteUndo_None,

	//ExecuteUndo_FrameBegin,
	ExecuteUndo_FrameEnd,
	ExecuteUndo_Move,
	ExecuteUndo_Remove,
	ExecuteUndo_SpawnBlock,
	ExecuteUndo_ReceivedBlock,

	ExecuteUndo_Count,
};

struct ExecuteUndoRemove
{
	EntityType entityType;
	v3i position;
};

struct ExecuteUndoCreate
{
	EntityType entityType;
	v3i position;
};

// todo packing? todo less data?
struct ExecuteUndo
{
	u32 entitySerial;
	ExecuteUndoType type;

	union
	{
		EntityInterpolation move;
		ExecuteUndoRemove remove;
		ExecuteUndoCreate create;
	};
};

DefineDynamicArray(ExecuteUndo);

struct ExecuteData
{
	u32 state;

	ExecuteUndoDynamicArray undo;

	PathCreator pathCreator;
	SimData simData;
};
static ExecuteData InitExecute()
{
	ExecuteData ret;
	ret.undo = ExecuteUndoCreateDynamicArray(100);
	ret.state = Execute_None;
	return ret;
}
static void StartSimulation(World *world, UnitHandler *unitHandler)
{
	ResetDudeAts(unitHandler);
	ResetWorld(world);

	For(world->entities)
	{
		switch (it->type)
		{
		case Entity_Spawner:
		{
			v3i posToSpawn = it->physicalPos + V3i(0, 0, -1);
			Entity other = CreateBlock(world, posToSpawn);
			ExecuteEvent ev;
			ev.type = ExecuteEvent_SpawnedBlock;
			ev.effectingEntitySerial = other.serialNumber;
			ev.causingEntitySerial = it->serialNumber;
			ArrayAdd(&world->activeEvents, ev);
		}break;

		}
	}
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
		exe->undo.amount = 0;
		ResetWorld(world);
	}break;
	case Execute_Simulation:
	{
		exe->simData.timeScale = 1.0f;
		exe->simData.blocksCollected = 0;
		exe->simData.blocksNeeded = 1000;
		exe->undo.amount = 0;
		StartSimulation(world, unitHandler);
	}break;
	case Execute_Victory:
	{

	}break;
	
	InvalidDefaultCase;
	}
}

static Entity *TileBlocked(World *world, v3i tile)
{
	For(world->entities)
	{
		if (it->physicalPos == tile && (it->flags & EntityFlag_BlocksUnit))
		{
			return it;
		}
	}

	return NULL;
}

// make dir an enum? so we cant push in something weird?
static void MaybeMoveEntity(Entity *e, v3i dir, World *world, InterpolationType type)
{
	if (e->flags & EntityFlag_IsMoving)
	{
		if ((u32)type < e->interpolation.type) // '=' ?
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

	e->flags |= EntityFlag_IsMoving;
}

typedef Entity* EntityPtr;

DefineDynamicFrameArray(EntityPtr);

static u32 SpawnerTryToSpawn(World *world, u32 serial, u32 oldSerial = 0xFFFFFFFF)
{
	Entity *e = GetEntity(world, serial); 
	Assert(e->type == Entity_Spawner);
	v3i posToSpawn = e->physicalPos + V3i(0, 0, -1);

	if (!TileBlocked(world, posToSpawn))
	{
		Entity other;
		if (oldSerial != 0xFFFFFFFF)
		{
			other = RestoreBlock(world, oldSerial, posToSpawn);
		}
		else
		{
			other = CreateBlock(world, posToSpawn);
		}
		
		return other.serialNumber;

	}

	return 0xFFFFFFFF;
}

static Entity *GetEntityInTile(World *world, v3i tile)
{
	For(world->entities)
	{
		if (it->physicalPos == tile && (it->flags & EntityFlag_SupportsUnit))
		{
			return it;
		}
	}
	return NULL;
}

static Entity *EntityIsSupported(World *world, Entity *e)
{
	v3i tile = e->physicalPos + V3i(0, 0, 1);
	For(world->entities)
	{
		if (it->physicalPos == tile && (it->flags & EntityFlag_SupportsUnit))
		{
			return it;
		}
	}
	return NULL;
	

}

static void MaybeFallEntity(World *world, Entity *e)
{
	if (EntityIsSupported(world, e)) return;

	e->flags |= EntityFlag_IsFalling;
	v3i dir = V3i(0, 0, 1);
	MaybeMoveEntity(e, dir, world, Interpolation_Fall);

}

static void AdvanceGameState(World *world, UnitHandler *unitHandler, ExecuteData *exe, b32 checkForVictory = true)
{
	SimData *sim = exe ? &exe->simData : NULL;

	For(world->entities)
	{
		if (it->flags & EntityFlag_IsMoving)
		{
			it->physicalPos += it->interpolation.dir;
			it->flags &= ~EntityFlag_IsMoving;
			it->offset = V3();

			ExecuteUndo undo;
			undo.entitySerial = it->serialNumber;
			undo.type = ExecuteUndo_Move;
			undo.move = it->interpolation;
			ArrayAdd(&exe->undo, undo);

			if (it->flags & EntityFlag_IsFalling) // falling implies moving right now, as we go through the maybe move entity path
			{
				it->flags &= ~EntityFlag_IsFalling;
				if (it->physicalPos.z > 10)
				{
					ExecuteUndo undo;
					undo.entitySerial = it->serialNumber;
					undo.type = ExecuteUndo_Remove;
					ExecuteUndoRemove remove;
					remove.entityType = it->type;
					remove.position = it->physicalPos;
					undo.remove = remove;
					ArrayAdd(&exe->undo, undo);

					RemoveEntity(world, it->serialNumber);
					it--;
				}
			}
		}
		
	}

	{
		ExecuteUndo end;
		end.type = ExecuteUndo_FrameEnd;
		ArrayAdd(&exe->undo, end);
	}

	u64 at = unitHandler->at++;

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
			MaybeFallEntity(world, it);
		}break;
		case Entity_Dude:
		{
			u32 unitIndex = GetUnitIndex(it);
			Assert(unitIndex < unitHandler->amount);
			UnitInstructionArray *p = unitHandler->programs + unitIndex;
			if (!p->amount) continue;

			u32 unitAt = (at % p->amount);

			auto command = (*p)[unitAt];

			MaybeFallEntity(world, it);

			// this does nothing if allready falling.
			MaybeMoveEntity(it, GetAdvanceForOneStep(command), world, Interpolation_Move); 

		}break;
		case Entity_Spawner:
		{
			v3i posToSpawn = it->physicalPos + V3i(0, 0, -1);
			For(ev, world->activeEvents)
			{
				if (ev->type == ExecuteEvent_ReceivedBlock && ev->effectingEntitySerial == it->serialNumber)
				{
					u32 placedSerial = SpawnerTryToSpawn(world, it->serialNumber);
					if( placedSerial != 0xFFFFFFFF)
					{
						ev->type = ExecuteEvent_SpawnedBlock;
						ev->effectingEntitySerial = placedSerial;
						ev->causingEntitySerial = it->serialNumber;

					}
					break;
				}
			}
		}break;
		case Entity_Goal:
		{
			v3i posToSpawn = it->physicalPos + V3i(0, 0, -1);
			Entity *above = TileBlocked(world, posToSpawn);
			if (above && above->type == Entity_Block)
			{
				if (checkForVictory && ++sim->blocksCollected > sim->blocksNeeded)
				{
					ChangeExecuteState(world, unitHandler, exe, Execute_Victory);
					
					return;
				}
				For(ev, world->activeEvents)
				{
					if (ev->type == ExecuteEvent_SpawnedBlock && ev->effectingEntitySerial == above->serialNumber)
					{
						{
							ExecuteUndo undo;
							undo.entitySerial = it->serialNumber;
							undo.type = ExecuteUndo_ReceivedBlock;
							ExecuteUndoRemove remove;
							remove.entityType = it->type;
							remove.position = it->physicalPos;
							undo.remove = remove;
							ArrayAdd(&exe->undo, undo);
						}

						u32 aboveSerial = above->serialNumber;
						RemoveEntity(world, aboveSerial);
						

						u32 newSerial = SpawnerTryToSpawn(world, ev->causingEntitySerial, aboveSerial);
						if (newSerial != 0xFFFFFFFF)
						{
							ev->effectingEntitySerial = newSerial;

							{
								ExecuteUndo undo;
								undo.entitySerial = it->serialNumber;
								undo.type = ExecuteUndo_SpawnBlock;
								ExecuteUndoCreate create;
								create.entityType = it->type;
								create.position = it->physicalPos;
								undo.create = create;
								ArrayAdd(&exe->undo, undo);
							}


							break;
						}

						
						ev->type = ExecuteEvent_ReceivedBlock;
						ev->effectingEntitySerial = ev->causingEntitySerial;
						ev->causingEntitySerial = it->serialNumber;
						break;
					}
				}
			}
		}break;

		}
	}
}



static void UpdateSimulation(World *world, UnitHandler *unitHandler, ExecuteData *execute, f32 dt)
{
	unitHandler->t += dt * execute->simData.timeScale;

	while (unitHandler->t > 1.0f)
	{
		unitHandler->t -= 1.0f;

		AdvanceGameState(world, unitHandler, execute);
		if (execute->state == Execute_Victory) break;
	}

	f32 t = unitHandler->t;
	For(world->entities)
	{
		if (it->flags & EntityFlag_IsMoving)
		{
			v3 delta = V3(it->interpolation.dir);
			f32 ease = -2.0f * t * t * t + 3.0f * t * t;
			it->offset = ease * delta;
		}
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
