#ifndef RR_BUILDING
#define RR_BUILDING

class Building : public Entity
{
public:
	Building();
	Building(v2 pos, float radius, float unitProduceTime, float expectedSecondsPerFrame);
	~Building();

	void Update(u16 tileSize, EntitySelection *entitys);

	bool producing;
private:
	u32 runningFrameIndex;
	float expectedSecondsPerFrame;
	float unitProduceTime;

};

Building::Building()
{

}
Building::~Building()
{

}

Building::Building(v2 pos, float radius, float unitProduceTime, float expectedSecondsPerFrame) : Entity(pos, radius)
{
	this->unitProduceTime = unitProduceTime;
	this->expectedSecondsPerFrame = expectedSecondsPerFrame;

	runningFrameIndex = 0;
}

void Building::Update(u16 tileSize, EntitySelection *entitys)
{
	if (producing)
	{
		runningFrameIndex++;
		if (unitProduceTime < (float)runningFrameIndex * expectedSecondsPerFrame)
		{
			runningFrameIndex = 0;
			producing = false;

			v2 unitPos = pos + V2((GetSize() + 3), (GetSize() + 3));
			float acceleration = 0.5f;
			float maxSpeed = 1.0f;
			float speed = 0;

			Unit *unit = new Unit(unitPos, acceleration, maxSpeed, expectedSecondsPerFrame, 1.0f);
			entitys->PushBack(unit);
		}
	}
}

#endif

