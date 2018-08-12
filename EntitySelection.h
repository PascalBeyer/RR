#ifndef RR_ENTITYSELECTION
#define RR_ENTITYSELECTION

#include "Entity.h"
#include "Unit.h"
class EntitySelection
{
public:
	EntitySelection();
	EntitySelection(u32 maxSize);
	~EntitySelection();
	void Clear();
	void PushBack(Entity *a);
	Entity *Get(u32 i);
	u8 amountSelected;

private:
	Entity **selectedEntities;
	u32 maxSize;

};
class UnitSelection
{
public:
	UnitSelection();
	UnitSelection(u32 maxSize);
	~UnitSelection();
	void Clear();
	void PushBack(Unit *a);
	Unit *Get(u32 i);
	u8 amountSelected;

private:
	Unit **selectedUnits;
	u32 maxSize;
};

#endif
