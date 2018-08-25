#ifndef RR_ANIMATION
#define RR_ANIMATION

#include "String.h"

struct AnimationElementList
{
	AnimationElementList *next;
	u32 assetIndex;
	u32 assetId;
	v2 p1;
	v2 p2;
	v2 p3;
	v2 p4;
};

struct AnimationFrameList
{
	AnimationFrameList *next;
	AnimationElementList *elements;
	float t;
	v2 pos;
};

AnimationFrameList *LoadAnimation(String name);

static void DeleteFirstAnimationFrame(AnimationFrameList *list)
{
	if (!list) return;
	AnimationFrameList *saveCur = list;
	list = list->next;
	for (AnimationElementList *it = saveCur->elements; it;)
	{
		AnimationElementList *saveIt = it;
		it = it->next;

		delete(saveIt);
	}
	delete(saveCur);
}


#endif
