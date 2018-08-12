#include "Animation.h"

void DeleteFirstAnimationFrame(AnimationFrameList *list)
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
