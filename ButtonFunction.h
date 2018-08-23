#ifndef RR_BUTTONFUNCTIONS
#define RR_BUTTONFUNCTIONS

#include "BasicTypes.h"

static void ButtonEventStub(void *data)
{

}

static void ButtomToggleBool(void* data)
{
	bool *theBool = (bool *)data;
	*theBool = !*theBool;
}

#pragma pack(push,1)
struct ButtonScrollDatum
{
	float *t;
	v2 *mousePos;
	v2 buttonPos;
	float width;
	float height;
};
#pragma pack(pop)

static void ButtonHorizontalScroll(void* data)
{	
	ButtonScrollDatum *datum = (ButtonScrollDatum *)data;

	bool inX = (datum->buttonPos.x < datum->mousePos->x) && (datum->buttonPos.x + datum->width > datum->mousePos->x);
	bool inY = (datum->buttonPos.y < datum->mousePos->y) && (datum->buttonPos.y + datum->height > datum->mousePos->y);

	bool inside = inX & inY;

	if (inside)
	{
		*datum->t = ((datum->mousePos->x - datum->buttonPos.x) / datum->width);
	}
}

#pragma pack(push,1)
struct ButtonFloatAssignDatum
{
	float *t;
	float value;
};
#pragma pack(pop)


static void ButtonFloatAssign(void *data)
{
	ButtonFloatAssignDatum *datum = (ButtonFloatAssignDatum *)data;
	*datum->t = datum->value;
}

#pragma pack(push,1)
struct ButtonUint32AssignDatum
{
	u32 *toAssign;
	u32 value;
};
#pragma pack(pop)

static void ButtonUint32Assign(void *data)
{	
	ButtonUint32AssignDatum *datum = (ButtonUint32AssignDatum *)data;
	*datum->toAssign = datum->value;	
}



#endif