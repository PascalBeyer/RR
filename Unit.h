#ifndef RR_UNIT
#define RR_UNIT

#include "Entity.h"

class Unit : public Entity
{
public:
	Unit();
	Unit(v2 pos, float acceleration, float maxSpeed, float expectedSecondsPerFrame, float radius);
	~Unit();

	//static void FindPaths(std::vector<Unit> *unitSelection, TileMap *tileMap, std::vector<Entity> *entities, v2 targetPos);

	u32 facingDirection;
	std::vector<v2> way;

	//float idleTimer;
	//float idleTime;
	float expectedSecondsPerFrame;
	float acceleration;
	v2 velocity;
	u32 steeringGroupId;

	float maxSpeed;
	bool idle;

private:
	
};
#endif
