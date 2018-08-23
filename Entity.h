#ifndef RR_ENTITY
#define RR_ENTITY

#include "Bitmap.h"
#include "Screen.h"
#include "buffers.h"
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



#endif
