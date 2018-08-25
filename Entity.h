#ifndef RR_ENTITY
#define RR_ENTITY
#include <vector>

class Entity
{
public:
	Entity();
	Entity(v2 pos, float radius);
	~Entity();

	v2 GetPos();
	v2 GetBitmapPos();
	float GetSize();

	virtual void Update(std::vector<Entity*> *entitys);
	virtual void Render(ImageBuffer *imageBuffer, Screen *screen);
	float radius;
	v2 pos;
protected:
	
};


Entity::Entity()
{
}

Entity::~Entity()
{
}

Entity::Entity(v2 pos, float radius)
{
	this->pos = pos + V2(0.5f, 0.5f);
	this->radius = radius - 0.01f;
}

v2 Entity::GetPos()
{
	return pos;
}
v2 Entity::GetBitmapPos()
{
	return pos - V2(radius, radius);
}
float Entity::GetSize()
{
	return radius;
}

void Entity::Update(std::vector<Entity*> *entitys)
{
}
void Entity::Render(ImageBuffer *imageBuffer, Screen *screen)
{
	//sprite->Render(imageBuffer, screen);
}


#endif
