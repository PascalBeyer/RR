#ifndef RR_ENTITY
#define RR_ENTITY

#define MaxUnitCount 200

struct UnitInfo
{
	v2 pos;
	v2i phisicalPos;
};

enum UnitInstruction
{
	Unit_Wait,
	Unit_MoveUp,
	Unit_MoveDown,
	Unit_MoveLeft,
	Unit_MoveRight,

	Unit_Jump,
	
};

DefineArray(UnitInstruction);
struct UnitProgram
{
	UnitInstructionArray instructions;
	u32 at;
};

struct UnitHandler
{
	u32 amount;
	
	u32 serializer;

	UnitInfo *infos;
	Quaternion *orientations;
	UnitProgram *programs;
	u32 *ids;

	f32 t; // between 0 and 1

	f32 meshScale;
	TriangleMesh mesh; // for now all units have the same mesh
};

static UnitHandler CreateUnitHandler(Arena *arena, AssetHandler *assetHandler)
{
	UnitHandler ret;

	ret.amount = 0;
	ret.serializer = 0; // todo maybe start this at 1, so dead units have serialnumber 0
	ret.infos = PushData(arena, UnitInfo, MaxUnitCount);
	ret.mesh = LoadMesh(assetHandler, "obj/maja/unit.mesh");
	ret.orientations = PushData(arena, Quaternion, MaxUnitCount);
	ret.ids = PushData(arena, u32, MaxUnitCount);
	ret.programs = PushData(arena, UnitProgram, MaxUnitCount);

	return ret;
}

static void CreateUnit(UnitHandler *unitHandler, v2 pos, Quaternion orientation)
{
	if (unitHandler->amount < MaxUnitCount)
	{
		u32 index = unitHandler->amount++;
		unitHandler->t = 0.0f;
		unitHandler->ids[index] = unitHandler->serializer++;
		unitHandler->infos[index].pos = pos;
		unitHandler->infos[index].phisicalPos = V2i(pos);
		unitHandler->orientations[index] = orientation;
		unitHandler->programs[index].instructions = DynamicAllocArray(UnitInstruction, 10);
#if 1
		RandomSeries series = GetRandomSeries();
		For(unitHandler->programs[index].instructions)
		{
			*it = (UnitInstruction)(RandomU32(&series) % 5);
		}
#endif

	}
	else
	{
		Die;
	}

}

static void UpdateUnits(UnitHandler *handler, f32 dt)
{
	b32 instructionChange = false;

	handler->t += dt;
	if (handler->t > 1.0f)
	{
		handler->t -= 1.0f;
		instructionChange = true;
	}

	if (instructionChange)
	{
		for (u32 i = 0; i < handler->amount; i++)
		{
			UnitProgram *p = handler->programs + i;
			switch (p->instructions[p->at])
			{
			case Unit_Wait:
			{

			}break;
			case Unit_MoveDown:
			{
				handler->infos[i].phisicalPos.y--;
			}break;
			case Unit_MoveUp:
			{
				handler->infos[i].phisicalPos.y++;
			}break;
			case Unit_MoveLeft:
			{
				handler->infos[i].phisicalPos.x--;
			}break;
			case Unit_MoveRight:
			{
				handler->infos[i].phisicalPos.x++;
			}break;
			default:
			{
				Die;
			}break;

			}

			//todo resolve non movement commands.

			//todo here report collision error. Crash program.

			p->at = (p->at + 1) % p->instructions.amount;
		}
	}

	// todo: we could save the current instruction in the info, so we do not have to load the program every frame. for now we dont do that for sake of complexity
	for (u32 index = 0; index < handler->amount; index++) 
	{
		UnitProgram *p = handler->programs + index;
		UnitInfo *i = handler->infos + index;

		v2i nextPos = i->phisicalPos;
		switch (p->instructions[p->at])
		{
		case Unit_Wait:
		{

		}break;
		case Unit_MoveDown:
		{	
			nextPos += V2i(0, -1);
		}break;
		case Unit_MoveUp:
		{
			nextPos += V2i(0, +1);
		}break;
		case Unit_MoveLeft:
		{
			nextPos += V2i(-1, 0);
		}break;
		case Unit_MoveRight:
		{
			nextPos += V2i(1, 0);
		}break;
		default:
		{
			Die;
		}break;
		}


		i->pos = Lerp(V2(i->phisicalPos), handler->t * handler->t, V2(nextPos)) + V2(0.5f, 0.5f);
	}

}

static void RenderUnits(RenderGroup *rg, UnitHandler *handler)
{
	for (u32 i = 0; i < handler->amount; i++)
	{
		PushTriangleMesh(rg, handler->mesh, handler->orientations[i], i12(handler->infos[i].pos), 0.24f);
	}
}


#endif