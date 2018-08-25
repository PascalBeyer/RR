#ifndef RR_UNIT
#define RR_UNIT

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


Unit::Unit()
{
}

Unit::Unit(v2 pos, float acceleration, float maxSpeed, float expectedSecondsPerFrame, float radius) : Entity(pos, radius)
{
	this->idle = true;
	this->acceleration = acceleration;
	this->maxSpeed = maxSpeed;
	this->expectedSecondsPerFrame = expectedSecondsPerFrame;
	facingDirection = 2;
}

Unit::~Unit()
{
}


/*
static Tile* RectIsCollidingWithTileMap(TileMap *tileMap, rr::Rectangle *rect)
{
unsigned int xmin = (int)rect->GetPos().x / tileMap->tileSize - 1;
if (xmin < 0) xmin = 0;
unsigned int ymin = (int)rect->GetPos().y / tileMap->tileSize - 1;
if (ymin < 0) ymin = 0;

unsigned int xmax = (int)rect->GetPos().x / tileMap->tileSize + 1;
if (xmax > tileMap->width) xmax = tileMap->width;
unsigned int ymax = (int)rect->GetPos().y / tileMap->tileSize + 1;
if (ymax > tileMap->height) ymax = tileMap->height;


for (unsigned int x = xmin; x <= xmax; x++)
{
for (unsigned int y = ymin; y <= ymax; y++)
{

Tile *tile = &tileMap->tiles[x][y];
//tile->type = 2;
if ((tile->type == 0) || (tile->type == 3))
{
tile->type = 3;

rr::Rectangle tileRect = rr::Rectangle(tile->pos, tileMap->tileSize, tileMap->tileSize);
if (rr::Rectangle::Colliding(*rect, tileRect))
return tile;
}
}
}
return NULL;
}*/
/*
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
static bool FindV3InVector(std::vector<v2> *v3Vector, v2 v)
{
for (unsigned int i = 0; i < v3Vector->size(); i++)
if (v == (*v3Vector)[i])
return true;
return false;
}
static void AStarAlgorithm(std::vector<v2> *outputWay, v2 goalVector, v2 position, TileMap *tileMap)
{
outputWay->clear();
//find Way
std::vector<v2> closedSet;
std::vector<RelativeTile*> openSet;

RelativeTile goal;
goal.tile = tileMap->GetTile(goalVector);

RelativeTile start;
start.tile = tileMap->GetTile(position);
start.lengthFromPos = 0;
start.prev = NULL;
openSet.push_back(&start);

while (!openSet.empty())
{
RelativeTile* current = openSet.front();
std::vector<RelativeTile*>::iterator cur = openSet.begin();
for (std::vector<RelativeTile*>::iterator i = openSet.begin(); i != openSet.end(); i++)
{
if (current->lengthFromPos + v2::Dist(current->tile->pos, goal.tile->pos) > (*i)->lengthFromPos + v2::Dist((*i)->tile->pos, goal.tile->pos))
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
outputWay->push_back(current->tile->pos);
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
neighboors.push_back(tileMap->GetTile(currPos + v2(tileMap->tileSize, 0)));
neighboors.push_back(tileMap->GetTile(currPos + v2(-tileMap->tileSize, 0)));
neighboors.push_back(tileMap->GetTile(currPos + v2(0, tileMap->tileSize)));
neighboors.push_back(tileMap->GetTile(currPos + v2(0, -tileMap->tileSize)));

for (std::vector<Tile*>::iterator i = neighboors.begin(); i != neighboors.end(); i++)
{
if ((*i)->type != 0)
{
if (!(FindV3InVector(&closedSet, (*i)->pos)))
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
*/

#endif
