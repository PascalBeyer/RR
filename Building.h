#ifndef RR_BUILDING
#define RR_BUILDING

struct Building
{
	Entity e;
	
	bool producing;
	u32 runningFrameIndex;
	float expectedSecondsPerFrame;
	float unitProduceTime;

};

static Building CreateBuilding(v2 pos, float radius, float unitProduceTime, float expectedSecondsPerFrame)
{
	Building ret;
	ret.unitProduceTime = unitProduceTime;
	ret.expectedSecondsPerFrame = expectedSecondsPerFrame;

	ret.runningFrameIndex = 0;
	return ret;
}

static void Update(Building *building, u16 tileSize, EntitySelection *entitys)
{
	Entity e = building->e;

	if (building->producing)
	{
		building->runningFrameIndex++;
		if (building->unitProduceTime < (float)building->runningFrameIndex * building->expectedSecondsPerFrame)
		{
			building->runningFrameIndex = 0;
			building->producing = false;

			v2 unitPos = building->e.pos + V2((e.radius + 3), (e.radius + 3));
			float acceleration = 0.5f;
			float maxSpeed = 1.0f;
			float speed = 0;

			//Unit *unit = new Unit(unitPos, acceleration, maxSpeed, expectedSecondsPerFrame, 1.0f);
			//entitys->PushBack(unit);
		}
	}
}

#endif

