#include "EntitySelection.h"


EntitySelection::EntitySelection()
{
	maxSize = 256;
	selectedEntities = new Entity*[maxSize];
	amountSelected = 0;
}
EntitySelection::EntitySelection(u32 maxSize)
{
	selectedEntities = new Entity*[maxSize];
	amountSelected = 0;
}
EntitySelection::~EntitySelection()
{
	//delete selectedEntities;
}
void EntitySelection::Clear()
{
	amountSelected = 0;
}
void EntitySelection::PushBack(Entity *back)
{
	selectedEntities[amountSelected++] = back;
}
Entity *EntitySelection::Get(u32 i)
{
	if (i < maxSize)
	{
		return selectedEntities[i];
	}
	else
	{
		return NULL;
	}

}

UnitSelection::UnitSelection()
{
	maxSize = 256;
	selectedUnits = new Unit*[maxSize];
	amountSelected = 0;
}
UnitSelection::UnitSelection(u32 maxSize)
{
	selectedUnits = new Unit*[maxSize];
	amountSelected = 0;
}
UnitSelection::~UnitSelection()
{
	//delete selectedEntities;
}
void UnitSelection::Clear()
{
	amountSelected = 0;
}
void UnitSelection::PushBack(Unit *back)
{
	selectedUnits[amountSelected++] = back;
}
Unit *UnitSelection::Get(u32 i)
{
	if (i < maxSize)
	{
		return selectedUnits[i];
	}
	else
	{
		return NULL;
	}

}
