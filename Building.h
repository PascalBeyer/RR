#ifndef RR_BUILDING
#define RR_BUILDING

#include "Entity.h"
#include "Player.h"
#include "EntitySelection.h"

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


#endif

