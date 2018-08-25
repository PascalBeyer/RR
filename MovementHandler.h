#ifndef RR_MOVEMENTHANDLER
#define RR_MOVEMENTHANDLER

static f32 moveHandlerEps = 0.01f;
static float moveHandlerDt = 1.0f / 30.0f;

//TODO: implement weights, differant unit sizes, 
//make unit group a rolling buffer
//gather groups in the beginning, still needs a spatial gather tho
//for that gather look at quadTrees and k-d-trees
//study the avoidance algorithm and study linear programming for that matter.

struct UnitSteeringData
{
	v2 pos;
	float radius;
	v2 velocity;
	float speed;
	float maxSpeed;
	float acceleration;
	v2 goal;
	bool idle;
	u32 group;

	u32 workbits;
	v2 steering;
};

struct SteeringList
{
	UnitSteeringData *data;
	SteeringList *next;
};


static SteeringList *GetDistNeighborhood(UnitSteeringData *unit, UnitSteeringData *selection, u32 amount, float desiredDist)
{
	SteeringList *list = NULL;
	//u32 counter = 1;
	for (u32 i = 0; i < amount; i++)
	{
		if ((selection + i) != unit && !(selection + i)->idle  && BoxDist(selection[i].pos, unit->pos) < desiredDist && (selection + i)->group == unit->group)
		{
			SteeringList *elem = new SteeringList;
			elem->data = selection + i;
			elem->next = list;
			list = elem;
		}
	}
	return list;
}


static SteeringList *GetNeighborhood(UnitSteeringData *unit, UnitSteeringData *selection, u32 amount)
{
	//	SteeringList *ret;

	for (u32 i = 0; i < amount; i++)
	{
		selection->workbits = 0;
	}

	UnitSteeringData *cur = unit;

	for (u32 i = 0; i < amount; i++)
	{
		SteeringList *neigbors = GetDistNeighborhood(cur, selection, amount, 1.0f);

	}
	return NULL;
}

static void ApplyCollisionSteering(UnitSteeringData *from, UnitSteeringData *to)
{

	from->pos;
	to->pos;
	v2 relPos = to->pos - from->pos; // from becomes the origen
	float quadDist = QuadDist(from->pos, to->pos);

	from->radius;
	to->radius;
	float radSum = from->radius + to->radius;
	float quadRadSum = radSum * radSum;

	//if (quadRadSum > quadDist)
	{
		//float dist = sqrtf(quadDist);

		from->speed;
		to->speed;
		v2 fromVelocity = from->velocity + from->steering;
		v2 toVelocity = to->velocity + to->steering;

		float normFromSteerSpeed = Norm(fromVelocity);
		if (normFromSteerSpeed > from->maxSpeed)
		{
			fromVelocity = fromVelocity * (from->maxSpeed / normFromSteerSpeed);
		}
		float normToSteerSpeed = Norm(toVelocity);
		if (normToSteerSpeed > to->maxSpeed)
		{
			toVelocity = toVelocity * (to->maxSpeed / normToSteerSpeed);
		}

		v2 relVel = toVelocity - fromVelocity; //from stands still

		float directedSpeed = Dot(relVel, relPos) * -1.0f;

		float combinedSteeringForce;
		if (quadDist != 0)
		{
			//coef = ((((radSum - dist) / dt) - directedSpeed) / (2 * dist));
			//float currentPenetrationDepth = radSum;// -dist;

			float qNormRPos = quadDist;
			float p = Dot(relPos, relPos + relVel * moveHandlerDt) / qNormRPos;
			float q = (QuadNorm(relPos + relVel * moveHandlerDt) - radSum) / qNormRPos;
			float disc = p * p - q;
			combinedSteeringForce = sqrtf(p * p - q) - p;

			if (combinedSteeringForce < 0.0f)
			{
				combinedSteeringForce = 0.0f;
			}
			//coef = (radSum - quadDist) / (quadDist * dt);
		}
		else
		{
			combinedSteeringForce = 1.0f;
		}
		if ((from->idle ^ to->idle))
		{

			if (!from->idle)
			{
				to->steering += relPos * (combinedSteeringForce / moveHandlerDt);
			}

			if (!to->idle)
			{
				from->steering -= relPos * (combinedSteeringForce / moveHandlerDt);
			}
		}
		else
		{
			to->steering += relPos * (combinedSteeringForce / (2 * moveHandlerDt));
			from->steering -= relPos * (combinedSteeringForce / (2 * moveHandlerDt));

		}
	}
}

static void ApplyGoalSeekSteering(UnitSteeringData *unit)
{
	v2 goal = unit->goal;
	v2 pos = unit->pos;
	float acceleration = unit->acceleration;

	v2 seekDirection = Normalize(goal - pos);

	v2 seekSteering = seekDirection * acceleration;

	unit->steering += seekSteering;
}

static void ApplyArrivalAndSeekSteering(UnitSteeringData *unit)
{
	v2 pos = unit->pos;
	v2 goal = unit->goal;
	v2 velocity = unit->velocity;
	float radius = unit->radius;
	float acceleration = unit->acceleration;
	float maxSpeed = unit->maxSpeed;

	float distToGoal = Dist(pos, goal);

	if (distToGoal)
	{
		v2 goalDirection = (goal - pos) / distToGoal;
		if (radius > distToGoal)
		{
			v2 desiredVelocity = goalDirection * (distToGoal / radius * maxSpeed);
			v2 arrivalSteering = (desiredVelocity - velocity);
			unit->steering += arrivalSteering;
		}
		else
		{
			v2 seekSteering = goalDirection * acceleration;
			unit->steering += seekSteering;
		}
	}
	else
	{
		v2 desiredVelocity = V2();
		v2 arrivalSteering = (desiredVelocity - velocity);
		unit->steering += arrivalSteering;

	}
}

static void ApplyAlignmentSteering(UnitSteeringData* unit, SteeringList *neighbors, float weight)
{
	v2 steer = V2();
	u32 count = 0;

	for (SteeringList *it = neighbors; it; it = it->next)
	{
		steer += it->data->velocity;
		count++;
	}

	if (count)
	{
		unit->steering += (1.0f / (float)count) * weight * steer;
	}
}

static void ApplyCohesionSteering(UnitSteeringData *unit, SteeringList *neighbors, float weight)
{
	v2 avgPos = V2();
	u32 count = 0;

	for (SteeringList *it = neighbors; it; it = it->next)
	{
		avgPos += it->data->pos;
		count++;
	}

	if (count)
	{
		v2 steer = Normalize((1.0f / (float)count) * avgPos - unit->pos);

		unit->steering += weight * steer;
	}

}

static void ApplySeparationSteering(UnitSteeringData *unit, SteeringList *neighbors, float weight)
{
	for (SteeringList *it = neighbors; it; it = it->next)
	{
		v2 relPos = (unit->pos - it->data->pos);
		float quadNorm = QuadNorm(relPos);

		unit->steering += weight * (1.0f / quadNorm) * relPos;
	}
}

static void ApplyCollisionSeparationSteeringNSquared(UnitSteeringData *selection, u32 amount)
{
	for (u32 i = 0; i < amount; i++)
	{
		for (u32 j = i + 1; j < amount; j++)
		{
			ApplyCollisionSteering(selection + i, selection + j);

		}
	}
}

static u32 SteeringListCount(SteeringList *list)
{
	if (!list) return 0;
	return (SteeringListCount(list->next) + 1);
}

static void TileMapCollisionMovement(v2 *pos, float radius, v2 by, v2 currentGoal, TileMap *tileMap)
{
	v2 unitPos = *pos;
	float xMin;
	float xMax;
	if (unitPos.x < (unitPos.x + by.x))
	{
		xMin = unitPos.x - radius;
		xMax = unitPos.x + by.x + radius;
	}
	else
	{
		xMin = unitPos.x + by.x - radius;
		xMax = unitPos.x + radius;
	}

	float yMin;
	float yMax;
	if (unitPos.y < (unitPos.y + by.y))
	{
		yMin = unitPos.y - radius;
		yMax = unitPos.y + by.y + radius;
	}
	else
	{
		yMin = unitPos.y + by.y - radius;
		yMax = unitPos.y + radius;
	}

	u32 xMini = (u32)xMin;
	if (xMini < 0) xMini = 0;
	u32 xMaxi = (u32)xMax;
	if (xMaxi > tileMap->width) xMaxi = tileMap->width;
	u32 yMini = (u32)yMin;
	if (yMini < 0) yMini = 0;
	u32 yMaxi = (u32)yMax;
	if (yMaxi > tileMap->height) yMaxi = tileMap->height;

	float tRemaining = 1.0f;
	u32 amountOfIterations = 4;
	for (u32 iterations = 0; iterations < amountOfIterations && tRemaining != 0.0f; iterations++)
	{
		v2 wallNormal;
		float tMin = tRemaining;

		//gather tMin and wallNormal
		for (u32 x = xMini; x <= xMaxi; x++)
		{
			for (u32 y = yMini; y <= yMaxi; y++)
			{
				if (GetTile(*tileMap, x, y) != 0)
				{
					float xSpeed = by.x;
					float ySpeed = by.y;

					float tileXMin = (float)x - radius;
					float tileXMax = (float)(x + 1) + radius;
					float tileYMin = (float)y - radius;
					float tileYMax = (float)(y + 1) + radius;


					if (Dot(V2(-1, 0), by) < 0.0f)
					{//intersection with left wall
						float intersection = (tileXMin - unitPos.x) / xSpeed;

						if (intersection < tMin)
						{
							if (0 <= intersection)
							{
								float newY = unitPos.y + ySpeed * intersection;
								if (y <= newY && newY <= y + 1)
								{
									tMin = intersection;
									wallNormal = V2(-1, 0);
								}
							}
							else
							{
								if (intersection > -1.0f)
								{
									if (y <= unitPos.y && unitPos.y <= y + 1)
									{
										tMin = 0.0f;
										wallNormal = V2(-1, 0);
									}
								}
							}
						}
					}
					else if (Dot(V2(1, 0), by) < 0.0f)
					{//intersection with right wall
						float intersection = (tileXMax - unitPos.x) / xSpeed;

						if (intersection < tMin)
						{
							if (0 <= intersection)
							{
								float newY = unitPos.y + ySpeed * intersection;
								if (y <= newY && newY <= y + 1)
								{
									tMin = intersection;
									wallNormal = V2(1, 0);
								}
							}
							else
							{
								if (intersection > -1.0f)
								{
									if (y <= unitPos.y && unitPos.y <= y + 1)
									{
										tMin = 0.0f;
										wallNormal = V2(1, 0);
									}
								}
							}
						}
					}

					if (Dot(V2(0, -1), by) < 0.0f)
					{//intersection with upper wall
						float intersection = (tileYMin - unitPos.y) / ySpeed;
						if (intersection < tMin)
						{
							if (0 <= intersection)
							{
								float newX = unitPos.x + xSpeed * intersection;
								if (x <= newX && newX <= x + 1)
								{
									tMin = intersection;
									wallNormal = V2(0, -1);
								}
							}
							else
							{
								if (intersection > -1.0f)
								{
									if (x <= unitPos.x && unitPos.x <= x + 1)
									{
										tMin = 0.0f;
										wallNormal = V2(0, -1);
									}
								}
							}
						}

					}
					else if (Dot(V2(0, 1), by) < 0.0f)
					{//intersection with lower wall
						float intersection = (tileYMax - unitPos.y) / ySpeed;

						if (intersection < tMin)
						{
							if (0 <= intersection)
							{
								float newX = unitPos.x + xSpeed * intersection;
								if (x <= newX && newX <= x + 1)
								{
									tMin = intersection;
									wallNormal = V2(0, 1);
								}
							}
							else
							{
								if (intersection > -1.0f)
								{
									if (x <= unitPos.x && unitPos.x <= x + 1)
									{
										tMin = 0.0f;
										wallNormal = V2(0, 1);
									}
								}
							}
						}
					}
					Die;
#if 0
					Shape::Circle upperLeft = Shape::Circle(radius, V2(x, y));
					Shape::Circle upperRight = Shape::Circle(radius, V2((x + 1), y));
					Shape::Circle lowerLeft = Shape::Circle(radius, V2(x, (y + 1)));
					Shape::Circle lowerRight = Shape::Circle(radius, V2((x + 1), (y + 1)));

					float circleTime = upperLeft.GetIntersectionTimeForLine(unitPos, by);
					v2 circleNormal = unitPos - upperLeft.GetPos();
					if (circleTime <= tMin && Dot(by, circleNormal) < 0.0f)
					{
						tMin = circleTime;
						wallNormal = Normalize(circleNormal);
					}
					circleTime = upperRight.GetIntersectionTimeForLine(unitPos, by);
					circleNormal = unitPos - upperRight.GetPos();
					if (circleTime <= tMin && Dot(by, circleNormal) < 0.0f)
					{
						tMin = circleTime;
						wallNormal = Normalize(circleNormal);
					}
					circleTime = lowerLeft.GetIntersectionTimeForLine(unitPos, by);
					circleNormal = unitPos - lowerLeft.GetPos();
					if (circleTime <= tMin && Dot(by, circleNormal) < 0.0f)
					{
						tMin = circleTime;
						wallNormal = Normalize(circleNormal);
					}
					circleTime = lowerRight.GetIntersectionTimeForLine(unitPos, by);
					circleNormal = unitPos - lowerRight.GetPos();
					if (circleTime <= tMin && Dot(by, circleNormal) < 0.0f)
					{
						tMin = circleTime;
						wallNormal = Normalize(circleNormal);
					}
#endif // 0

				}
			}
		}

		{
			if (tMin != tRemaining)
			{
				unitPos += by * tMin;// + wallNormal * offset;
				float dotCoef = Dot(by, wallNormal);

				by -= wallNormal * dotCoef;
				tRemaining -= tMin;

			}
			else
			{
				unitPos += by * tMin;
				iterations = amountOfIterations;
			}
		}
	}
	*pos = unitPos;
}

static void MoveSelection(UnitSelection *selection, UnitSteeringData *data, TileMap *tileMap)
{
	for (u32 i = 0; i < selection->amountSelected; i++)
	{

		Unit *cur = selection->Get(i);
		//if (!data[i].idle)
		{
			v2 steeredSpeed;
			v2 preNormalizedSteeredSpeed = data[i].velocity + data[i].steering;
			float normSteerSpeed = Norm(preNormalizedSteeredSpeed);
			if (normSteerSpeed > cur->maxSpeed)
			{
				steeredSpeed = preNormalizedSteeredSpeed * (cur->maxSpeed / normSteerSpeed);
			}
			else
			{
				steeredSpeed = preNormalizedSteeredSpeed;
			}
			v2 oldPos = cur->pos;
			v2 by = steeredSpeed * moveHandlerDt;

			TileMapCollisionMovement(&cur->pos, cur->radius, by, data[i].goal, tileMap);

			v2 traveledDist = cur->pos - oldPos;
			cur->velocity = traveledDist * (1.0f / moveHandlerDt);
			float newSpeed = Norm(traveledDist);

			float amountOfFacingDirections = 4.0f;

			float traveledAngle = AngleBetween(traveledDist, V2(1, 1));
			float normedTraveledAngle = traveledAngle / (PI);
			float traveledAngle4 = amountOfFacingDirections / 2.0f * normedTraveledAngle;
			float traveledAngleinRange = amountOfFacingDirections / 2.0f + traveledAngle4;


			if (traveledAngleinRange >= amountOfFacingDirections)
			{
				cur->facingDirection = 0;
			}
			else
			{
				cur->facingDirection = (u32)traveledAngleinRange;
			}


			//cur->speed = newSpeed * (1.0f / dt);
			bool collidedWithTileMap = !((oldPos + by) == cur->pos);

			bool steered = (data[i].steering == V2());

			//TODO: Put this into update + fix it (implenebt arrival)
			if (newSpeed == 0.0f && !collidedWithTileMap && cur->way.size() <= 1)
			{
				cur->idle = true;
				cur->way.clear();
			}
			else if (collidedWithTileMap)
			{
				cur->idle = false;
			}

			if (cur->idle)
			{
				cur->velocity = V2();
			}
		}
	}
}

static void CollectSteeringData(UnitSelection *selection, UnitSteeringData *data)
{
	if (selection->amountSelected > 0)
	{
		moveHandlerDt = selection->Get(0)->expectedSecondsPerFrame;
	}

	for (u32 i = 0; i < selection->amountSelected; i++)
	{
		Unit *cur = selection->Get(i);
		data[i].pos = cur->pos;
		data[i].radius = cur->radius;
		data[i].steering = v2();
		data[i].idle = cur->idle;
		data[i].group = cur->steeringGroupId;
		data[i].maxSpeed = cur->maxSpeed;
		data[i].acceleration = cur->acceleration;
		data[i].speed = Norm(cur->velocity);
		data[i].velocity = cur->velocity;
		if (!cur->way.empty())
		{
			data[i].goal = cur->way.back();
		}
		else
		{
			data[i].goal = cur->pos;
		}
	}
}

static Tile *FirstPathBlockate(TileMap tileMap, v2 pos, v2 goal)
{
	return GetFirstTileWithType(tileMap, pos, goal, 1);
}

static bool SolveLineIntersectoion(float *u, float *v, v2 a, v2 b, v2 c, v2 d)
{
	v2 s = d - b;

	if (a.x * c.y - c.x * a.y == 0)
	{
		return false;
	}
	if (a.x != 0)
	{
		{
			*v = (s.y - (a.y * s.x) / a.x) / ((a.y * c.x) / a.x - c.y);
			*u = (s.x + c.x * *v) / a.x;
		}
		return true;
	}
	else
	{
		return SolveLineIntersectoion(v, u, c, d, a, b);
	}
}
/*
void FixPath(std::vector<v3> *way, v2 pos, TileMap *tileMap, float radius)
{
if (!way->empty())
{
v3 back = (way->back());

v2 currentGoal = p12(way->back());
way->pop_back();
if (!way->empty())
{

v2 nextGoal = p12(way->back());

v2 relativeGoal = nextGoal - currentGoal;
v2 normedRelativeGoal = Normalize(relativeGoal);
v2 relCur = currentGoal - pos;
v2 relNext = nextGoal - pos;
v2 perp = PerpendicularVector(relNext);
float directionSign = Dot(perp, relCur);
v2 normalizedPerp = Normalize(perp);

v2 adjustedPos;
v2 adjustedNextGoal;

if (directionSign > 0)
{
adjustedPos = pos - normalizedPerp * radius;
adjustedNextGoal = nextGoal - normalizedPerp * radius;
}
else
{
adjustedPos = pos + normalizedPerp *radius;
adjustedNextGoal = nextGoal + normalizedPerp * radius;

normalizedPerp *= -1.0f;
}

Tile *currentObstical = FirstPathBlockate(adjustedPos, tileMap, adjustedNextGoal);

if (!currentObstical)
{
FixPath(way, pos, tileMap, radius);
return;
}
u32 iteration = 0;
v2 ret = p12(back);
while(currentObstical && iteration < 10){
iteration++;
u32 x = (u32)currentObstical->pos.x;
u32 y = (u32)currentObstical->pos.y;
v2 pointToAvoid = v2(x, y);
float prod = Dot(pointToAvoid, normalizedPerp);

float check = Dot(v2(x + 1, y), normalizedPerp);
if (prod < check)
{
pointToAvoid = v2(x + 1, y);
prod = check;
}
check = Dot(v2(x, y + 1), normalizedPerp);
if (prod < check)
{
pointToAvoid = v2(x, y + 1);
prod = check;
}
check = Dot(v2(x + 1, y + 1), normalizedPerp);
if (prod < check)
{
pointToAvoid = v2(x + 1, y + 1);
prod = check;
}

v2 relativePoint = pointToAvoid - adjustedPos;

//float u1;
//float v1;

//SolveLineIntersectoion(&u1, &v1, normedRelativeGoal, currentGoal, relativePoint, adjustedPos);
//v2 intersectionAdj = normedRelativeGoal * u1 + currentGoal;
float u2;
float v2;

v2 intersection;
if (SolveLineIntersectoion(&u2, &v2, normedRelativeGoal, currentGoal, relativePoint, pos))
{

intersection = normedRelativeGoal * u2 + currentGoal;
ret = intersection;

}
else
{
way->push_back(v3::Inclusion12(ret));
return;

}
adjustedNextGoal = intersection - (normalizedPerp * radius);

currentObstical = FirstPathBlockate(adjustedPos, tileMap, adjustedNextGoal);
}

way->push_back(v3::Inclusion12(ret));
}
else
{
way->push_back(back);
}
}
}

void DetermineVisablePath(UnitSelection *selection, TileMap *tileMap)
{
for (u32 i = 0; i < selection->amountSelected; i++)
{
Unit *cur = selection->Get(i);
FixPath(&cur->way, p12(cur->pos), tileMap, cur->radius);
}
}*/



struct RelativeTile
{
	Tile *tile;
	float lengthFromPos;
	RelativeTile *prev;
};
static bool FindTileInVector(std::vector<RelativeTile*> *tileVector, Tile* tile)
{
	for (unsigned int i = 0; i < tileVector->size(); i++)
		if (tile == (*tileVector)[i]->tile)
			return true;
	return false;
}
static bool FindV2InVector(std::vector<v2> *v2Vector, v2 v)
{
	for (unsigned int i = 0; i < v2Vector->size(); i++)
		if (v == (*v2Vector)[i])
			return true;
	return false;
}
static void AStarAlgorithm(std::vector<v2> *outputWay, v2 goalVector, v2 position, TileMap tileMap)
{
	//find Way
	std::vector<v2> closedSet;
	std::vector<RelativeTile*> openSet;

	RelativeTile goal;
	goal.tile = GetTile(tileMap, goalVector);

	RelativeTile start;
	start.tile = GetTile(tileMap, position);
	start.lengthFromPos = 0;
	start.prev = NULL;
	openSet.push_back(&start);

	while (!openSet.empty())
	{
		RelativeTile* current = openSet.front();
		std::vector<RelativeTile*>::iterator cur = openSet.begin();
		for (std::vector<RelativeTile*>::iterator i = openSet.begin(); i != openSet.end(); i++)
		{
			if (current->lengthFromPos + Dist(current->tile->pos, goal.tile->pos) > (*i)->lengthFromPos + Dist((*i)->tile->pos, goal.tile->pos))
			{
				current = *i;
				cur = i;
			}
		}
		if (current->tile == goal.tile)
		{
			//find way back
			while (current)
			{
				outputWay->push_back(current->tile->pos + V2(0.5f, 0.5f));
				current = current->prev;
			}

			break;
		}
		else
		{

			v2 currPos = current->tile->pos;

			openSet.erase(cur);
			closedSet.push_back(currPos);

			std::vector<Tile*> neighboors;

			v2 neighboor = currPos + V2(1.0f, 0.0f);
			if (InBounds(tileMap, neighboor))
			{
				neighboors.push_back(GetTile(tileMap, neighboor));
			}
			neighboor = currPos + V2(-1.0f, 0.0f);
			if (InBounds(tileMap, neighboor))
			{
				neighboors.push_back(GetTile(tileMap, neighboor));
			}
			neighboor = currPos + V2(0.0f, 1.0f);
			if (InBounds(tileMap, neighboor))
			{
				neighboors.push_back(GetTile(tileMap, neighboor));
			}
			neighboor = currPos + V2(0.0f, -1.0f);
			if (InBounds(tileMap, neighboor))
			{
				neighboors.push_back(GetTile(tileMap, neighboor));
			}
			for (std::vector<Tile*>::iterator i = neighboors.begin(); i != neighboors.end(); i++)
			{
				if ((*i)->type == 0)
				{
					if (!(FindV2InVector(&closedSet, (*i)->pos)))
					{
						if (!(FindTileInVector(&openSet, *i)))
						{
							RelativeTile *neighboorTile = new RelativeTile;
							neighboorTile->tile = *i;
							neighboorTile->lengthFromPos = current->lengthFromPos + 1;
							neighboorTile->prev = current;
							openSet.push_back(neighboorTile);
						}
					}
				}
			}
		}
	}
}

static void SimpleStupidFunnelAlgorithm(Unit *unit, TileMap tileMap)
{
	std::vector<v2> *way = &unit->way;
	float radius = unit->radius;

	v2 pos = unit->pos;

	if (!way->empty())
	{
		v2 firstGoal = way->back();
		if (GetFirstTileWithType(tileMap, pos, firstGoal, 1))
		{
			AStarAlgorithm(way, firstGoal, pos, tileMap);
		}

		Tile *tile = GetTile(tileMap, firstGoal);
		way->pop_back();

		v2 goal = firstGoal;

		if (way->empty())
		{
			way->push_back(firstGoal);
			return;
		}
		v2 saveLast;

		u32 sizeOfWay = (u32)way->size();
		for (u32 i = 0; i < sizeOfWay - 1; i++)
		{
			saveLast = goal;
			goal = way->back();

			v2 offset = Normalize(PerpendicularVector(goal - pos)) * radius;

			v2 left = pos + offset;
			v2 right = pos - offset;

			v2 goalLeft = goal + offset;
			v2 goalRight = goal - offset;

			if (GetFirstTileWithType(tileMap, pos, goalLeft, 1) || GetFirstTileWithType(tileMap, pos, goalRight, 1))
			{
				way->push_back((saveLast));
				return;
			}
			way->pop_back();
		}
	}
}

static void ClearSteeringList(SteeringList *list)
{
	if (list)
	{
		ClearSteeringList(list->next);
		delete list;
	}
}

static void MoveUnits(UnitSelection *selection, TileMap tileMap)
{

	UnitSteeringData *data = new UnitSteeringData[selection->amountSelected];

	//precalc
	for (u32 i = 0; i < selection->amountSelected; i++)
	{
		SimpleStupidFunnelAlgorithm(selection->Get(i), tileMap);
	}
	CollectSteeringData(selection, data);

	//steering
	for (u32 i = 0; i < selection->amountSelected; i++)
	{
		if (!data[i].idle)
		{
			//collect neighbors
			float neighborCollectionRadius = 2.0f;
			SteeringList *neighbors = GetDistNeighborhood(data + i, data, selection->amountSelected, neighborCollectionRadius);

			//apply steering
			ApplyArrivalAndSeekSteering(data + i);

			ApplyAlignmentSteering(data + i, neighbors, 0.7f);
			ApplyCohesionSteering(data + i, neighbors, 0.3f);
			ApplySeparationSteering(data + i, neighbors, 0.1f);

			ClearSteeringList(neighbors);
		}
	}




	ApplyCollisionSeparationSteeringNSquared(data, selection->amountSelected);

	//move
	MoveSelection(selection, data, &tileMap);

	delete[] data;
}


/*
TODO: Are building circles or rectangular? probably rectangular if I want to stick to a tilemap system
//this is collision code/move code.
u32 amountOfIterations = 4;
for (u32 iterations = 0; iterations < amountOfIterations && tRemaining != 0.0f; iterations++)
{
v3 wallNormal;
float tmin = tRemaining;
for (u32 i = 0; i < entities->size(); i++)
{
if (!(entities->at(i) == this))
{
Shape::Circle bounding = entities->at(i)->GetBoundingCircle();

float tResult = this->boundingCircle.GetIntersectionTimeWithCircle(bounding, currentSpeedVector, &wallNormal);
if (tResult < tmin)
{
tmin = tResult;
}
}

}
if (iterations == 0 && v3::Norm(currentSpeedVector) * tmin > v3::Dist(pos, currentGoal))
{
pos = currentGoal;
way.pop_back();
iterations = amountOfIterations;
}
else
{
if (tmin != tRemaining)
{
if (wallNormal != v3())
{

}
pos += currentSpeedVector * tmin + wallNormal * offSet;
currentSpeedVector -= wallNormal * v3::Dot(currentSpeedVector, wallNormal);
speed = v3::Norm(currentSpeedVector);
tRemaining -= tmin;

}
else
{
pos += currentSpeedVector * tmin;
iterations = amountOfIterations;
}
}
}
boundingCircle.SetPos(pos);

if (currentSpeedVector.x > 0)
{
if (currentSpeedVector.y > 0)
{
if (currentSpeedVector.y > currentSpeedVector.x)
{
facingDirection = 0;
}
else
{
facingDirection = 1;
}
}
else
{

if (-currentSpeedVector.y > currentSpeedVector.x)
{
facingDirection = 2;
}
else
{
facingDirection = 1;
}
}
}
else
{
if (currentSpeedVector.y > 0)
{
if (currentSpeedVector.y > -currentSpeedVector.x)
{
facingDirection = 0;
}
else
{
facingDirection = 3;
}
}
else
{

if (-currentSpeedVector.y > -currentSpeedVector.x)
{
facingDirection = 2;
}
else
{
facingDirection = 3;
}
}
}
}
/*
int amountOfIterations = 4;
for (int i = 0; i < amountOfIterations && tRemaining != 0.0f; i++)
{
float tmin = tRemaining;

int xmin = (int)(moveRect.GetUL().x / tileMap->tileSize);
int ymin = (int)(moveRect.GetUL().y / tileMap->tileSize);
int xmax = (int)(moveRect.GetLR().x / tileMap->tileSize) + 1;
int ymax = (int)(moveRect.GetLR().y / tileMap->tileSize) + 1;

v2 wallNormal;

for (int x = xmin; x <= xmax; x++)
{
for (int y = ymin; y <= ymax; y++)
{
Tile* tile = &tileMap->tiles[x][y];
if (tile->type == 0)
{
shapes::RoundedRectangle intersectionRoundingRect = boundingCircle + shapes::Rectangle(tile->pos, tileMap->tileSize, tileMap->tileSize);

float tResult = intersectionRoundingRect.GetIntersectionTimeForLine(pos, currentSpeedVector, &wallNormal);
if (tResult < tmin)
{
tmin = tResult;
}
}
}
}
//check if there
//create an offset

if (i == 0 && Norm(currentSpeedVector) * tmin > Dist(pos, currentGoal))
{
pos = currentGoal;
way.pop_back();
i = amountOfIterations;
}
else
{
if (tmin != tRemaining)
{
pos += currentSpeedVector * tmin + wallNormal * offSet;
currentSpeedVector -= wallNormal * Dot(currentSpeedVector, wallNormal);
speed = Norm(currentSpeedVector);
tRemaining -= tmin;

}
else
{
pos += currentSpeedVector * tmin;
i = amountOfIterations;
}
}
}
sprite->SetPos(pos + v3(-radius, -radius, 0));
boundingRect.SetPos(pos);



void Unit::Update(EntitySelection *selection, TileMap *tileMap)
{
/*
v2 mouseVector = v2((float)mouseInput->x + (float)screen->GetX(), (float)mouseInput->y + (float)screen->GetY());

//handle new Input
if (mouseInput->rightButtonDown && marked && (way.empty() || way.front() != mouseVector ))
{
#if AStar
AStarAlgorithm(&way, v2((float)mouseInput->x + (float)screen->GetX(), (float)mouseInput->y + (float)screen->GetY(), 0), pos, tileMap);

#else
way.clear();
way.push_back(mouseVector);
#endif

}

//move
/*for (int i = 0; i < way.size(); i++)
{
tileMap->GetTile(way[i])->type = 2;
}

if (way.empty())
{
//maybe here we should accelerate down
speed = 0;
}
else


if (way.empty())
{
speed = 0;
}
else if (way.back() != pos)
{
//v2 currentSpeedVector = v2(0.1f, 0.0f);
v3 currentGoal = way.back();
//shapes::Rectangle moveRect = shapes::Rectangle(pos, currentGoal);

if (speed < maxSpeed)
{
speed += (acceleration * expectedSecondsPerFrame);
}
float tRemaining = 1.0f;

v3 relUnitTarget = currentGoal - pos;
float norm = v3::Norm(relUnitTarget);
v3 currentSpeedVector = relUnitTarget*(speed / norm);
if (v3::quadNorm(currentSpeedVector) > v3::quadDist(pos, currentGoal))
{
pos = currentGoal;
way.pop_back();
}
else
{
pos = pos + currentSpeedVector;
}

Unit *other = (Unit *)selection->Get(1);

float sumRad = radius + other->radius;
float quadSumRad = sumRad * sumRad;
float quadDist = v3::quadDist(pos, other->pos);

if ((quadSumRad > quadDist))
{
v3 dP = other->pos - pos;
float dPNorm = v3::Norm(dP);
float distToTravel = sumRad - dPNorm;
other->pos += dP * distToTravel;
}

}
}

*/





#endif // !RR_COLLISIONHANDLER

