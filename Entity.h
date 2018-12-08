#ifndef RR_ENTITY
#define RR_ENTITY

#define MaxUnitCount 200



struct UnitInfo
{
	v2i initialPos;
	v2 pos;
	v2i physicalPos;
};

enum UnitInstruction
{
	Unit_Wait,
	Unit_MoveUp,
	Unit_MoveDown,
	Unit_MoveLeft,
	Unit_MoveRight,
	
};

DefineDynamicArray(UnitInstruction);
struct UnitProgram
{
	UnitInstructionDynamicArray instructions;
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
	u32 meshId; // for now all units have the same mesh
	AABB meshAABB;
};

static UnitHandler CreateUnitHandler(Arena *arena, AssetHandler *assetHandler)
{
	UnitHandler ret;

	ret.amount = 0;
	ret.serializer = 0; // todo maybe start this at 1, so dead units have serialnumber 0
	ret.infos = PushData(arena, UnitInfo, MaxUnitCount);
	ret.meshId = RegisterAsset(assetHandler, Asset_Mesh, "dude.mesh");
	ret.orientations = PushData(arena, Quaternion, MaxUnitCount);
	ret.ids = PushData(arena, u32, MaxUnitCount);
	ret.programs = PushData(arena, UnitProgram, MaxUnitCount);
	ret.meshScale = 16.0f;
	ret.meshAABB = GetMesh(assetHandler, ret.meshId)->aabb;

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
		unitHandler->infos[index].physicalPos = V2i(pos);
		unitHandler->infos[index].initialPos = V2i(pos);
		unitHandler->orientations[index] = orientation;
		unitHandler->programs[index].instructions = UnitInstructionCreateDynamicArray();
#if 0
		unitHandler->programs[index].instructions.amount = unitHandler->programs[index].instructions.capacity;
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

static void ResetUnits(UnitHandler *handler)
{
	ConsoleOutputError("Handler Reset!");

	for (u32 i = 0; i < handler->amount; i++)
	{
		handler->infos[i].physicalPos = handler->infos[i].initialPos;
		handler->infos[i].pos = V2(handler->infos[i].initialPos);
		handler->programs[i].at = 0;
	}
}

static void UpdateUnits(UnitHandler *handler, TileMap tileMap, f32 dt)
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
			if (!p->instructions.amount) continue;
			switch (p->instructions[p->at])
			{
			case Unit_Wait:
			{

			}break;
			case Unit_MoveDown:
			{
				handler->infos[i].physicalPos.y--;
			}break;
			case Unit_MoveUp:
			{
				handler->infos[i].physicalPos.y++;
			}break;
			case Unit_MoveLeft:
			{
				handler->infos[i].physicalPos.x--;
			}break;
			case Unit_MoveRight:
			{
				handler->infos[i].physicalPos.x++;
			}break;
			default:
			{
				Die;
			}break;

			}

			//todo resolve non movement commands. exist?

			Tile *tile = GetTile(tileMap, V2(handler->infos[i].physicalPos));
			if (!tile || tile->type == Tile_Blocked)
			{
				ResetUnits(handler);
				return;
			}


			p->at = (p->at + 1) % p->instructions.amount;
		}
	}

	// todo unit collision here

	// todo: we could save the current instruction in the info, so we do not have to load the program every frame. for now we dont do that for sake of complexity
	for (u32 index = 0; index < handler->amount; index++) 
	{
		UnitProgram *p = handler->programs + index;
		UnitInfo *i = handler->infos + index;

		v2i nextPos = i->physicalPos;

		if (!p->instructions.amount) continue;

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


		i->pos = Lerp(V2(i->physicalPos), handler->t, V2(nextPos));
	}

}

static void RenderUnits(RenderGroup *rg, UnitHandler *handler)
{
	for (u32 i = 0; i < handler->amount; i++)
	{
		PushTriangleMesh(rg, handler->meshId, handler->orientations[i], i12(handler->infos[i].pos), handler->meshScale, V4(1, 1, 1, 1));
	}
}

enum PathCreatorState
{
	PathCreator_None,
	PathCreator_CreatingPath,
	PathCreator_PlacingUnits, // or something
};

struct PathCreator
{
	u32 hotUnit;
	v2i lastPosOfCurrentPath;
	PathCreatorState state;

};

static u32 GetHotUnit(UnitHandler *unitHandler, v2 mousePosZeroToOne, f32 aspectRatio, f32 focalLength, Camera camera)
{
	v3 camP = camera.pos; // todo camera or debugCamera? Maybe we should again unify them
	v3 camD = ScreenZeroToOneToDirecion(camera, mousePosZeroToOne);

	for(u32 i = 0; i < unitHandler->amount; i++)
	{
		auto it = &unitHandler->infos[i];
		m4x4 mat = QuaternionToMatrix(Inverse(unitHandler->orientations[i]));
		v3 rayP = mat * (camP - i12(it->pos));
		v3 rayD = mat * camD;
		AABB aabb = unitHandler->meshAABB;
		aabb.maxDim *= unitHandler->meshScale; // todo can pre compute this
		aabb.minDim *= unitHandler->meshScale;
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

static v2i GetAdvanceForOneStep(UnitInstruction step)
{
	v2i advance = {};
	switch (step)
	{
	case Unit_Wait:
	{

	}break;
	case Unit_MoveDown:
	{
		advance = V2i(0, -1);
	}break;
	case Unit_MoveUp:
	{
		advance = V2i(0, 1);
	}break;
	case Unit_MoveLeft:
	{
		advance = V2i(-1, 0);
	}break;
	case Unit_MoveRight:
	{
		advance = V2i(1, 0);
	}break;
	default:
	{
		Die;
	}break;

	}
	return advance;
}

static v2i GetLastPosForPath(v2i unitPos, UnitInstructionDynamicArray path)
{
	v2i ret = unitPos;
	For(path)
	{
		switch (*it)
		{
		case Unit_Wait:
		{

		}break;
		case Unit_MoveDown:
		{
			ret.y--;
		}break;
		case Unit_MoveUp:
		{
			ret.y++;
		}break;
		case Unit_MoveLeft:
		{
			ret.x--;
		}break;
		case Unit_MoveRight:
		{
			ret.x++;
		}break;
		default:
		{
			Die;
		}break;

		}
	}
	return ret;
}

static void PathCreatorHandleEvent(PathCreator *pathCreator, UnitHandler *unitHandler, KeyStateMessage message, Input input, f32 focalLength, f32 aspectRatio, Camera cam)
{
	if (message.flag & KeyState_PressedThisFrame)
	{
		switch (message.key)
		{
		case Key_leftMouse:
		{
			switch (pathCreator->state)
			{
			case PathCreator_None:
			{
				u32 unitIndex = GetHotUnit(unitHandler, input.mouseZeroToOne, aspectRatio, focalLength, cam);
				if (unitIndex < unitHandler->amount)
				{
					pathCreator->hotUnit = unitIndex;
					Assert(unitHandler->infos[unitIndex].physicalPos == unitHandler->infos[unitIndex].initialPos);
					pathCreator->lastPosOfCurrentPath = GetLastPosForPath(unitHandler->infos[unitIndex].physicalPos, unitHandler->programs[unitIndex].instructions);
					pathCreator->state = PathCreator_CreatingPath;
				}
				return;
			}break;
			case PathCreator_CreatingPath:
			{
				Assert(pathCreator->hotUnit < unitHandler->amount);
				v2 p = ScreenZeroToOneToInGame(cam, input.mouseZeroToOne);
				v2 d = p - V2(pathCreator->lastPosOfCurrentPath);

				if (BoxNorm(d) < 0.5f)
				{
					ArrayAdd(&unitHandler->programs[pathCreator->hotUnit].instructions, Unit_Wait);
					break;
				}

				i32 xSign = (d.x > 0) ? 1 : -1;
				i32 ySign = (d.y > 0) ? 1 : -1;
				
				if ((f32)ySign * d.y > (f32)xSign * d.x)
				{
					if (ySign > 0)
					{
						ArrayAdd(&unitHandler->programs[pathCreator->hotUnit].instructions, Unit_MoveUp);
						pathCreator->lastPosOfCurrentPath.y++; // todo checking
					}
					else
					{
						ArrayAdd(&unitHandler->programs[pathCreator->hotUnit].instructions, Unit_MoveDown);
						pathCreator->lastPosOfCurrentPath.y--;
					}
				}
				else
				{
					if (xSign > 0)
					{
						ArrayAdd(&unitHandler->programs[pathCreator->hotUnit].instructions, Unit_MoveRight);
						pathCreator->lastPosOfCurrentPath.x++; // todo checking
					}
					else
					{
						ArrayAdd(&unitHandler->programs[pathCreator->hotUnit].instructions, Unit_MoveLeft);
						pathCreator->lastPosOfCurrentPath.x--;
					}
				}

			}break;
			case PathCreator_PlacingUnits:
			{

			}break;
			default:
			{
				Die;
			}break;
			}

		}break;

		}

	}
}

static void ColorForPathEditor(UnitHandler *unitHandler, PathCreator *pathCreator, World *world, Input input)
{
	if (pathCreator->hotUnit >= unitHandler->amount) return;
	
	Assert(pathCreator->hotUnit < unitHandler->amount);
	v2 mouseP = ScreenZeroToOneToInGame(world->camera, input.mouseZeroToOne);
	v2 mouseToPath = mouseP - V2(pathCreator->lastPosOfCurrentPath);

	TileMap tileMap = world->tileMap;
	Tweekable(v4, pathCreatorPathColor, V4(1.0f, 0.4f, 0.4f, 1.0f));
	Tweekable(v4, pathCreatorUnitPosColor, V4(1.0f, 1.0f, 1.0f, 0.4f));

	
	if (BoxNorm(mouseToPath) > 0.5f)
	{
		i32 xSign = (mouseToPath.x > 0) ? 1 : -1;
		i32 ySign = (mouseToPath.y > 0) ? 1 : -1;

		if ((f32)ySign * mouseToPath.y > (f32)xSign * mouseToPath.x)
		{
			if (ySign > 0)
			{
				v2i pos = pathCreator->lastPosOfCurrentPath + V2i(0, 1);
				Tile *tile = GetTile(tileMap, pos);
				if (tile)
				{
					PlacedMesh *mesh = world->placedMeshes.data + tile->meshIndex;

					mesh->frameColor *= pathCreatorPathColor;
				}
			}
			else
			{
				v2i pos = pathCreator->lastPosOfCurrentPath + V2i(0, -1);
				Tile *tile = GetTile(tileMap, pos);
				if (tile)
				{
					PlacedMesh *mesh = world->placedMeshes.data + tile->meshIndex;

					mesh->frameColor *= pathCreatorPathColor;
				}
			}
		}
		else
		{
			if (xSign > 0)
			{
				v2i pos = pathCreator->lastPosOfCurrentPath + V2i(1, 0);
				Tile *tile = GetTile(tileMap, pos);
				if (tile)
				{
					PlacedMesh *mesh = world->placedMeshes.data + tile->meshIndex;

					mesh->frameColor *= pathCreatorPathColor;
				}
			}
			else
			{
				v2i pos = pathCreator->lastPosOfCurrentPath + V2i(-1, 0);
				Tile *tile = GetTile(tileMap, pos);
				if (tile)
				{
					PlacedMesh *mesh = world->placedMeshes.data + tile->meshIndex;

					mesh->frameColor *= pathCreatorPathColor;
				}
			}
		}

	}
	
	
	UnitInstructionDynamicArray path = unitHandler->programs[pathCreator->hotUnit].instructions;
	
	v2i pos = unitHandler->infos[pathCreator->hotUnit].initialPos;

	{
		Tile *tile = GetTile(tileMap, pos);
		if (tile)
		{
			PlacedMesh *mesh = world->placedMeshes.data + tile->meshIndex;

			mesh->frameColor *= pathCreatorUnitPosColor;
		}
	}

	For(path)
	{
		pos += GetAdvanceForOneStep(*it);

		Tile *tile = GetTile(tileMap, pos);
		if (tile)
		{
			PlacedMesh *mesh = world->placedMeshes.data + tile->meshIndex;

			mesh->frameColor *= pathCreatorPathColor;
		}
	}
}


#endif