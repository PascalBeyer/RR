#ifndef RR_MARKINGRECT
#define RR_MARKINGRECT

#include "2DGeometry.h"
#include "EntitySelection.h"
#include <vector>
#include "Entity.h"
#include "Input.h"

class MarkingRect : public Shape::Rectangle
{
public:
	MarkingRect();
	~MarkingRect();

	void Update(EntitySelection *entitySelection, Input *input, Screen *screen, EntitySelection *entitys);

private:
	bool mouseCreatesBox;
	v2 mouseBoxPos;
};

#endif
