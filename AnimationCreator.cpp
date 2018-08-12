#include "AnimationCreator.h"

#include "Arena.h"

enum
{
	AnimationCreator_Play,
	AnimationCreator_Edit,
	AnimationCreator_SaveNLoad

};

enum
{
	AnimationEditMode_Rotate,
	AnimationEditMode_Move,
	AnimationEditMode_Scale,

};

enum ButtonEnum
{
	Button_Play,
	Button_SaveNLoad,

	Button_Rotate,
	Button_Scale,
	Button_Move,

	Button_NewElement,
	Button_DeleteFrame,

	Button_AssetUp,
	Button_AssetDown,
	Button_SpriteUp,
	Button_SpriteDown,

	Button_Save,
	Button_Load,
	Button_Cancel,

	Button_PlayLine,
	Button_PlayLineScrollBar,
	Button_FramePointer,

	Button_ButtonCount,
	Button_Invalid
};

bool IsValid(ButtonEnum test)
{
	return (test < Button_ButtonCount);
}

struct ButtonAnimationCreator
{
	ButtonEnum id;
	v2 pos;
	float width;
	float height;
	v4 color;
	u32 priority;
};

struct ButtonArrayAC
{
	ButtonAnimationCreator *base;
	u32 size;
	u32 capacity;
};

AssetHandler *assetHandler;
float time;
u32 mode;
u32 editMode;
AnimationFrameList *currentAnimation;
AnimationElementList *selectedElement;
String animationName;
String saveString;
AnimationFrameList *currentFrame;
ButtonArrayAC buttonArray;
float playLinePlayPosition;

#define POSBUTTONSIZE 20.0f

char *buttonTexts[Button_ButtonCount]
{
	"Button_Play",
	"Button_SaveNLoad",

	"Button_Rotate",
	"Button_Scale",
	"Button_Move",

	"Button_NewElement",
	"Button_DeleteFrame",

	"Button_AssetUp",
	"Button_AssetDown",
	"Button_SpriteUp",
	"Button_SpriteDown",

	"Button_Save",
	"Button_Load",
	"Button_Cancel",
	">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>",
	"",
	"",
};


void AnimationCreatorInit(Input *input)
{
	assetHandler = new AssetHandler();
	animationName = String();
	for (u32 assetIterator = 0; assetIterator < Asset_White; assetIterator++)
	{
		assetHandler->LoadAsset({ assetIterator });
	}

	time = 0.0f;
	mode = AnimationCreator_Edit;
	editMode = AnimationEditMode_Move;
	currentAnimation = new AnimationFrameList;
	*currentAnimation = { 0 };

	ButtonAnimationCreator *buttonBuffer = new ButtonAnimationCreator[100];

	buttonArray.base = buttonBuffer;
	buttonArray.size = 0;
	buttonArray.capacity = 100;

	
}


ButtonAnimationCreator InitButtonAnimationCreator(ButtonEnum id, v2 pos, float width, float height, u32 priority)
{
	ButtonAnimationCreator but;
	but.height = height;
	but.id = id;
	but.width = width;
	but.pos = pos;
	but.priority = priority;
	but.color = V4(1.0f, 1.0f, 1.0f, 1.0f);
	return but;
}

struct Quad
{
	v2 p1;
	v2 p2;
	v2 p3;
	v2 p4;
};


float GetAnimationLength(AnimationFrameList *animation)
{
	float ret = 0.0f;
	for (AnimationFrameList *it = animation; it; it = it->next)
	{
		ret = it->t;
	}
	return ret;
}

inline Quad LarpQuad(v2 cp1, v2 cp2, v2 cp3, v2 cp4, float ctime, v2 np1, v2 np2, v2 np3, v2 np4, float ntime, float time)
{
	Quad ret;

	float dt = ntime - ctime;
	float dti = 1.0f / dt;

	ret.p1 = dti * (-(ctime - time) * (np1) + ((ctime - time) + dt)* cp1);
	ret.p2 = dti * (-(ctime - time) * (np2) + ((ctime - time) + dt)* cp2);
	ret.p3 = dti * (-(ctime - time) * (np3) + ((ctime - time) + dt)* cp3);
	ret.p4 = dti * (-(ctime - time) * (np4) + ((ctime - time) + dt)* cp4);

	return ret;
}

bool RenderAnimationFrame(AnimationFrameList *animationFrame, float time, RenderGroup *rg)
{
	AnimationFrameList *lastFrame = animationFrame;
	while (lastFrame->next) lastFrame = lastFrame->next;
	if (time >= lastFrame->t && mode == AnimationCreator_Edit)
	{

		for (AnimationElementList *aeit = lastFrame->elements; aeit; aeit = aeit->next)
		{
			PushTexturedQuadrilateral(rg, i12(aeit->p1), i12(aeit->p2), i12(aeit->p3), i12(aeit->p4), V4(1.0f, 1.0f, 1.0f, 1.0f), { aeit->assetId }, aeit->assetIndex);
		}

		PushCenteredRectangle(rg, lastFrame->pos, POSBUTTONSIZE, POSBUTTONSIZE, V4(1.0f, 1.0f, 0.0f, 0.0f));

		return true;

	}
	for (AnimationFrameList *afit = animationFrame; afit->next; afit = afit->next)
	{
		if (afit->next->t >= time)
		{
			AnimationElementList *nextaeit = afit->next->elements;

			for (AnimationElementList *aeit = afit->elements; aeit; aeit = aeit->next, nextaeit = nextaeit->next)
			{
				Quad q = LarpQuad(aeit->p1, aeit->p2, aeit->p3, aeit->p4, afit->t, nextaeit->p1, nextaeit->p2, nextaeit->p3, nextaeit->p4, afit->next->t, time);

				PushTexturedQuadrilateral(rg, i12(q.p1), i12(q.p2), i12(q.p3), i12(q.p4), V4(1.0f, 1.0f, 1.0f, 1.0f), { aeit->assetId }, aeit->assetIndex);
			}

			PushCenteredRectangle(rg, afit->pos, POSBUTTONSIZE, POSBUTTONSIZE, V4(1.0f, 1.0f, 0.0f, 0.0f));
			return true;
		}
	}

	return false;
}

AnimationFrameList *GetCurrentAnimationFrame(AnimationFrameList *list, float time)
{
	if (!list->next) return list;
	AnimationFrameList *lastEntry = list;
	for (AnimationFrameList *it = list->next; it; it = it->next)
	{	
		if (it->t > time)
		{
			return lastEntry;
		}		
		lastEntry = it;
	}

	return lastEntry;
}

void CreateNewAnimationElement(AnimationFrameList *animation)
{
	v2 newElementP1 = { 200, 200 };
	v2 newElementP2 = { 300, 200 };
	v2 newElementP3 = { 200, 300 };
	v2 newElementP4 = { 300, 300 };

	for (AnimationFrameList *it = animation; it; it = it->next)
	{
		AnimationElementList *newElement = new AnimationElementList;
		newElement->assetId = { Asset_White };
		newElement->assetIndex = 0;
		newElement->next = it->elements;
		newElement->p1 = newElementP1;
		newElement->p2 = newElementP2;
		newElement->p3 = newElementP3;
		newElement->p4 = newElementP4;
		it->elements = newElement;
	}
}

//WARNING: apperantly this does not serch for the frame to insert it just assumes current frame  is right
AnimationFrameList *CreateNewAnimationFrame(AnimationFrameList *currentFrame, float time) 
{
	Assert(time != currentFrame->t);
	AnimationFrameList *newFrame = new AnimationFrameList;
	*newFrame = { 0 };
	newFrame->next = currentFrame->next;
	newFrame->t = time;

	if (currentFrame->elements)
	{
		AnimationElementList *copy = new AnimationElementList;
		newFrame->elements = copy;
		if (time > GetAnimationLength(currentFrame))
		{
			for (AnimationElementList *copyIt = currentFrame->elements; copyIt; copyIt = copyIt->next)
			{
				copy->assetId = copyIt->assetId;
				copy->assetIndex = copyIt->assetIndex;
				copy->p1 = copyIt->p1;
				copy->p2 = copyIt->p2;
				copy->p3 = copyIt->p3;
				copy->p4 = copyIt->p4;
				copy->next = NULL;
				if (copyIt->next)
				{
					AnimationElementList  *newCopyElement = new AnimationElementList;
					copy->next = newCopyElement;
					copy = newCopyElement;
					newCopyElement->next = NULL;
				}
			}

		}
		else
		{
			AnimationElementList *copyNextIt = currentFrame->next->elements;
			for (AnimationElementList *copyIt = currentFrame->elements; copyIt; copyIt = copyIt->next, copyNextIt = copyNextIt->next)
			{
				Quad copyCoord = LarpQuad(copyIt->p1, copyIt->p2, copyIt->p3, copyIt->p4, currentFrame->t,
					copyNextIt->p1, copyNextIt->p2, copyNextIt->p3, copyNextIt->p4, currentFrame->next->t, time);

				copy->assetId = copyIt->assetId;
				copy->assetIndex = copyIt->assetIndex;
				copy->p1 = copyCoord.p1;
				copy->p2 = copyCoord.p2;
				copy->p3 = copyCoord.p3;
				copy->p4 = copyCoord.p4;
				copy->next = NULL;
				if (copyIt->next)
				{
					AnimationElementList  *newCopyElement = new AnimationElementList;
					copy->next = newCopyElement;
					copy = newCopyElement;
					newCopyElement->next = NULL;
				}
			}
		}
	}
	
	currentFrame->next = newFrame;

	return newFrame;
}

//does not search trough frames, i.e. assumes list is the frame such that tframe <= time <= tnextFrame
AnimationElementList *GetSelectedElementOutOfCurrentFrame(AnimationFrameList* list, Input *input, float time)
{
	if (list->next)
	{
		AnimationElementList *nextIt = list->next->elements;
		for (AnimationElementList *it = list->elements; it; it = it->next, nextIt = nextIt->next)
		{
			Quad quadAtTime = LarpQuad(it->p1, it->p2, it->p3, it->p4, list->t, nextIt->p1, nextIt->p2, nextIt->p3, nextIt->p4, list->next->t, time);
			Bitmap *texture = assetHandler->GetAsset({ it->assetId })->sprites[it->assetIndex];

			float width = Norm(quadAtTime.p2 - quadAtTime.p1);
			float height = Norm(quadAtTime.p3 - quadAtTime.p1);

			float mouseRelX = Dot(((quadAtTime.p2 - quadAtTime.p1) / width), input->mousePos - quadAtTime.p1);
			float mouseRelY = Dot(((quadAtTime.p3 - quadAtTime.p1) / height), input->mousePos - quadAtTime.p1);

			u32 textureX = (u32)floorf((mouseRelX / width) * texture->width);
			u32 textureY = (u32)floorf((mouseRelY / height) * texture->height);

			if (PointInQuadraliteral(quadAtTime.p1, quadAtTime.p2, quadAtTime.p3, quadAtTime.p4, input->mousePos) && GetPixel(*texture, textureX, textureY))
			{
				return it;
			}
		}
	}
	else
	{
		for (AnimationElementList *it = list->elements; it; it = it->next)
		{
			Bitmap *texture = assetHandler->GetAsset({ it->assetId })->sprites[it->assetIndex];

			float width = Norm(it->p2 - it->p1);
			float height = Norm(it->p3 - it->p1);

			float mouseRelX = Dot(((it->p2 - it->p1) / width), input->mousePos - it->p1);
			float mouseRelY = Dot(((it->p3 - it->p1) / height), input->mousePos - it->p1);

			u32 textureX = Round((mouseRelX / width) * texture->width);
			u32 textureY = Round((mouseRelY / height) * texture->height);

			if (PointInQuadraliteral(it->p1, it->p2, it->p3, it->p4, input->mousePos) && GetPixel(*texture, textureX, textureY))
			{
				return it;
			}
		}
	}
	
	return NULL;
}

u32 GetAmountOfAnimationFrames(AnimationFrameList *list)
{
	u32 ret = 0;
	for (AnimationFrameList *it = list; it; it = it->next)
	{
		ret++;
	}
	return ret;
}

u32 GetAmountOfAnimationElements(AnimationFrameList *list)
{
	u32 ret = 0;
	for (AnimationElementList *it = list->elements; it; it = it->next)
	{
		ret++;
	}
	return ret;
}


void SaveAnimation(AnimationFrameList *animation, String name)
{
	u32 amountOfAnimationFrames = GetAmountOfAnimationFrames(animation);
	u32 amountOfAnimationElements = GetAmountOfAnimationElements(animation);

	u32 sizeOfAnimationElement = 2 * sizeof(u32) + 8 * sizeof(float);
	u32 sizeOfAnimationFrame = 3 * sizeof(float) + amountOfAnimationElements * sizeOfAnimationElement;
	u32 sizeOfAnimation = 2 * sizeof(u32) + amountOfAnimationFrames * sizeOfAnimationFrame;

	u8 *memory = new u8[sizeOfAnimation];

	File file = File(memory, sizeOfAnimation);

	u8 *it = memory;

	*((u32 *)it) = amountOfAnimationFrames;
	it += sizeof(u32);
	*((u32 *)it) = amountOfAnimationElements;
	it += sizeof(u32);

	for (AnimationFrameList *frameIt = animation; frameIt; frameIt = frameIt->next)
	{
		*((float* )it) = frameIt->t;
		it += sizeof(float);
		*((float*)it) = frameIt->pos.x;
		it += sizeof(float);
		*((float*)it) = frameIt->pos.y;
		it += sizeof(float);

		for (AnimationElementList *elementIt = frameIt->elements; elementIt;  elementIt = elementIt->next)
		{
			*((u32 *)it) = elementIt->assetId;
			it += sizeof(u32);
			*((u32 *)it) = elementIt->assetIndex;
			it += sizeof(u32);
			
			*((float *)it) = elementIt->p1.x;
			it += sizeof(float);
			*((float *)it) = elementIt->p1.y;
			it += sizeof(float);

			*((float *)it) = elementIt->p2.x;
			it += sizeof(float);
			*((float *)it) = elementIt->p2.y;
			it += sizeof(float);

			*((float *)it) = elementIt->p3.x;
			it += sizeof(float);
			*((float *)it) = elementIt->p3.y;
			it += sizeof(float);

			*((float *)it) = elementIt->p4.x;
			it += sizeof(float);
			*((float *)it) = elementIt->p4.y;
			it += sizeof(float);
		}
	}
	file.Write(ToZeroTerminated(workingArena, name));
	file.Free();
}

AnimationFrameList *LoadAnimation(String name)
{
	String inp = name;

	File file = File(ToZeroTerminated(workingArena, name));

	u8 *it = (u8 *)file.GetMemory();

	u32 amountOfAnimationFrames = *(u32 *)it;
	it += sizeof(u32);
	u32 amountOfAnimationElements = *(u32 *)it;
	it += sizeof(u32);

	AnimationFrameList *ret = new AnimationFrameList;
	*ret = { 0 };
	for (u32 i = 1; i < amountOfAnimationFrames; i++)
	{
		CreateNewAnimationFrame(ret, (float)i);
	}
	for (u32 i = 0; i < amountOfAnimationElements; i++)
	{
		CreateNewAnimationElement(ret);
	}


	for (AnimationFrameList *frameIt = ret; frameIt; frameIt = frameIt->next)
	{
		frameIt->t = *((float*)it);
		it += sizeof(float);
		frameIt->pos.x = *((float*)it);
		it += sizeof(float);
		frameIt->pos.y = *((float*)it);
		it += sizeof(float);
		
		for (AnimationElementList *elementIt = frameIt->elements; elementIt; elementIt = elementIt->next)
		{
			elementIt->assetId = *((u32 *)it);
			it += sizeof(u32);
			elementIt->assetIndex = *((u32 *)it);
			it += sizeof(u32);

			elementIt->p1.x = *((float *)it);
			it += sizeof(float);
			elementIt->p1.y = *((float *)it);
			it += sizeof(float);

			elementIt->p2.x = *((float *)it);
			it += sizeof(float);
			elementIt->p2.y = *((float *)it);
			it += sizeof(float);

			elementIt->p3.x = *((float *)it);
			it += sizeof(float);
			elementIt->p3.y = *((float *)it);
			it += sizeof(float);

			elementIt->p4.x = *((float *)it);
			it += sizeof(float);
			elementIt->p4.y = *((float *)it);
			it += sizeof(float);
		}
	}
	file.Free();
	return ret;
}

AnimationElementList *ShiftSelectedAnimationElementToNextFrame(AnimationFrameList *currentFrame, AnimationElementList *selectedElement)
{
	//if (!currentFrame || !currentFrame->next) return;
	AnimationElementList *nextFrameIt = currentFrame->next->elements;
	for (AnimationElementList *it = currentFrame->elements; it; it = it->next, nextFrameIt = nextFrameIt->next)
	{
		if (it == selectedElement)
		{
			return nextFrameIt;
		}
	}
	return NULL;
}

void FreeAnimation(AnimationFrameList *animation)
{
	for (AnimationFrameList *it = animation; it;)
	{
		for (AnimationElementList *eleIt = it->elements; eleIt; )
		{
			AnimationElementList *saveIt = eleIt->next;
			delete(eleIt);
			eleIt = saveIt;
		}
		AnimationFrameList *saveIt = it->next;
		delete(it);
		it = saveIt;
	}
}

AnimationFrameList *FindPreviousAnimationFrame(AnimationFrameList *list, AnimationFrameList *entry)
{
	AnimationFrameList *it = list;
	while (it && it->next != entry)
	{
		it = it->next;
	}
	return it;
}


ButtonAnimationCreator GetButtonAC(ButtonEnum id)
{
	switch (id)
	{
	case Button_Play:
	{
		return (InitButtonAnimationCreator(Button_Play, V2(1100, 100), 100, 100, 0));
	}break;
	case Button_SaveNLoad:
	{
		return (InitButtonAnimationCreator(Button_SaveNLoad, V2(1100, 300), 100, 100, 0));
	}break;
	case Button_Rotate:
	{
		return (InitButtonAnimationCreator(Button_Rotate, V2(50, 200), 50, 50, 0));
	}break;
	case Button_Scale:
	{
		return (InitButtonAnimationCreator(Button_Scale, V2(50, 340), 50, 50, 0));
	}break;
	case Button_Move:
	{
		return (InitButtonAnimationCreator(Button_Move, V2(50, 270), 50, 50, 0));
	}break;
	case Button_NewElement:
	{
		return (InitButtonAnimationCreator(Button_NewElement, V2(800, 500), 100, 50, 0));
	}break;
	case Button_DeleteFrame:
	{
		return (InitButtonAnimationCreator(Button_DeleteFrame, V2(900, 500), 100.0f, 50.0f, 0));
	}break;
	case Button_AssetUp:
	{
		return (InitButtonAnimationCreator(Button_AssetUp, V2(1100, 200), 200, 50, 0));
	}break;
	case Button_AssetDown:
	{
		return (InitButtonAnimationCreator(Button_AssetDown, V2(1100, 250), 200, 50, 0));
	}break;
	case Button_SpriteUp:
	{
		return (InitButtonAnimationCreator(Button_SpriteUp, V2(900, 200), 200, 50, 0));
	}break;
	case Button_SpriteDown:
	{
		return (InitButtonAnimationCreator(Button_SpriteDown, V2(900, 250), 200, 50, 0));
	}break;
	case Button_PlayLine:
	{
		return (InitButtonAnimationCreator(Button_PlayLine, V2(100, 720 - 50), 800, 50, 0));
	}break;

	case Button_PlayLineScrollBar:
	{
		float animationLength = GetAnimationLength(currentAnimation);
		float playLineScrollBarWidth = 0.5f * 800 / animationLength;
		float playLineScrollBarHeight = 15.0f;
		v2 playLineScrollBarPos = V2(100, 720 - 50) + V2(MapRangeToRangeCapped(playLinePlayPosition, 0.0f, animationLength, 0.0f, 50 - playLineScrollBarWidth), -playLineScrollBarHeight);
		return (InitButtonAnimationCreator(Button_PlayLineScrollBar, playLineScrollBarPos, playLineScrollBarWidth, playLineScrollBarHeight, 0));
	}break;

	case Button_Cancel:
	{
		return InitButtonAnimationCreator(Button_Cancel, V2(1100, 400), 100, 100, 0);
	}break;
	case Button_Save:
	{
		return InitButtonAnimationCreator(Button_Save, V2(1100, 250), 100, 100, 0);
	}break;
	case Button_Load:
	{
		return InitButtonAnimationCreator(Button_Load, V2(1100, 100), 100, 100, 0);
	}break;

	default:
		Assert(!"Invalide Button Id");
		return InitButtonAnimationCreator(Button_Invalid, V2(), 0, 0, 0);
		break;
	}
}


void HandleOnClick(ButtonEnum triggeredButton, RenderGroup *rg, Input *input)
{
	switch (triggeredButton)
	{
	case Button_Play:
	{
		Assert((mode == AnimationCreator_Play) | (mode == AnimationCreator_Edit));
		if (mode == AnimationCreator_Play)
		{
			mode = AnimationCreator_Edit;
		}
		else
		{
			mode = AnimationCreator_Play;
			selectedElement = NULL;
			time = 0.0f;
		}

	}break;
	case Button_SaveNLoad:
	{
		mode = AnimationCreator_SaveNLoad;
		selectedElement = NULL;
	}break;
	case Button_Rotate:
	{
		editMode = AnimationEditMode_Rotate;
	}break;
	case Button_Scale:
	{
		editMode = AnimationEditMode_Scale;
	}break;
	case Button_Move:
	{
		editMode = AnimationEditMode_Move;
	}break;
	case Button_NewElement:
	{
		CreateNewAnimationElement(currentAnimation);
	}break;
	case Button_DeleteFrame:
	{
		AnimationFrameList *prev = FindPreviousAnimationFrame(currentAnimation, currentFrame);
		Assert(prev);

		prev->next = currentFrame->next;

		for (AnimationElementList *it = currentFrame->elements; it;)
		{
			AnimationElementList *saveIt = it->next;
			delete it;
			it = saveIt;
		}
		delete currentFrame;
		currentFrame = prev;
	}break;
	case Button_AssetUp:
	{
		if (selectedElement)
		{
			if (++selectedElement->assetId == Asset_Count)
			{
				selectedElement->assetId = 0;
			}
			selectedElement->assetIndex = 0;
		}
	}break;
	case Button_AssetDown:
	{
		if (selectedElement)
		{
			if (selectedElement->assetId == 0)
			{
				selectedElement->assetId = Asset_Count - 1;
			}
			else
			{
				selectedElement->assetId--;
			}
			selectedElement->assetIndex = 0;
		}		
	}break;
	case Button_SpriteUp:
	{
		if (selectedElement)
		{
			if (++selectedElement->assetIndex == rg->assetHandler->GetAsset({ selectedElement->assetId })->spriteCount)
			{
				selectedElement->assetIndex = 0;
			}
		}		
	}break;
	case Button_SpriteDown:
	{
		if (selectedElement)
		{
			selectedElement->assetIndex = selectedElement->assetIndex ?
				selectedElement->assetIndex - 1 : rg->assetHandler->GetAsset({ selectedElement->assetId })->spriteCount - 1;
		}			
	}break;
	case Button_Save:
	{
		SaveAnimation(currentAnimation, Append(frameArena, saveString, CreateString(".ann")));
		animationName = saveString;
		mode = AnimationCreator_Edit;
	}break;
	case Button_Load:
	{
		FreeAnimation(currentAnimation);
		currentAnimation = LoadAnimation(Append(frameArena, animationName, CreateString(".ann")));
		animationName = saveString;
		mode = AnimationCreator_Edit;		
	}break;
	case Button_Cancel:
	{
		mode = AnimationCreator_Edit;
		return;
	}break;

	case Button_PlayLine:
	{
		v2 relPos = input->mousePos - GetButtonAC(Button_PlayLine).pos;
		time = relPos.x / GetButtonAC(Button_PlayLine).width + playLinePlayPosition;
		
	}break;
	case  Button_PlayLineScrollBar:
	{
		float animationLength = GetAnimationLength(currentAnimation);
		if (animationLength)
		{
			ButtonAnimationCreator playLine = GetButtonAC(Button_PlayLine);
			ButtonAnimationCreator bar = GetButtonAC(Button_PlayLineScrollBar);
			float playLineMin = playLine.pos.x;
			float playLineMax = playLine.pos.x + playLine.width;
			float inToRangeMin = playLine.pos.x + bar.width / 2.0f;
			float inToRangeMax = playLine.pos.x + playLine.width - bar.width / 2.0f;

			float mappedX = MapRangeToRangeCapped(input->mousePos.x, inToRangeMin, inToRangeMax, 0.0f, animationLength); //to animationLength so I get an additional second to edit
			time += (mappedX - playLinePlayPosition);
			playLinePlayPosition = mappedX;
		}		
	}break;
	default:
	{
		Assert(IsValid(triggeredButton));
	}break;

	}

}

void HandleOnDrag(ButtonEnum triggeredButton, RenderGroup *rg, Input *input)
{
	switch (triggeredButton)
	{
	case Button_PlayLine:
	{
		v2 relPos = input->mousePos - GetButtonAC(Button_PlayLine).pos;
		time = relPos.x / GetButtonAC(Button_PlayLine).width + playLinePlayPosition;
	}break;
	case  Button_PlayLineScrollBar:
	{
		float animationLength = GetAnimationLength(currentAnimation);
		if (animationLength)
		{
			ButtonAnimationCreator playLine = GetButtonAC(Button_PlayLine);
			ButtonAnimationCreator bar = GetButtonAC(Button_PlayLineScrollBar);
			float playLineMin = playLine.pos.x;
			float playLineMax = playLine.pos.x + playLine.width;
			float inToRangeMin = playLine.pos.x + bar.width / 2.0f;
			float inToRangeMax = playLine.pos.x + playLine.width - bar.width / 2.0f;

			float mappedX = MapRangeToRangeCapped(input->mousePos.x, inToRangeMin, inToRangeMax, 0.0f, GetAnimationLength(currentAnimation));
			time += (mappedX - playLinePlayPosition);
			playLinePlayPosition = mappedX;
		}
	}break;
	default:
	{
		Assert(IsValid(triggeredButton));
	}break;

	}

}

 
void BubbleButtons(ButtonAnimationCreator *first, ButtonAnimationCreator *second)
{
	ButtonAnimationCreator temp = *first;
	*first = *second;
	*second = temp;
}

void SortButtonsByPrority()
{
	if (!buttonArray.size) {
		return;
	}
	for (u32 j = 0; j < buttonArray.size; j++)
	for (u32 i = 0; i < buttonArray.size - 1; i++)
	{
		if (buttonArray.base[i + 1].priority < buttonArray.base[i].priority)
		{
			BubbleButtons(buttonArray.base + i, buttonArray.base + i + 1);
		}		
	}
}

ButtonEnum GetButtonTriggeredOnClick(v2 mousePos)
{
	for (u32 i = 0; i < buttonArray.size; i++)
	{
		v2 pos = buttonArray.base[i].pos;
		float width = buttonArray.base[i].width;
		float height = buttonArray.base[i].height;

		if (PointInRectangle(pos, width, height, mousePos))
		{
			return buttonArray.base[i].id;
		}
	}

	return Button_Invalid;
}

void PushButtonAC(ButtonAnimationCreator but)
{
	Assert(buttonArray.capacity > buttonArray.size + 1);
	buttonArray.base[buttonArray.size++] = but;
}

void PushButtonAC(ButtonEnum id)
{	
	PushButtonAC(GetButtonAC(id));
}


void DrawButtons(RenderGroup *rg)
{
	for (u32 i = buttonArray.size - 1; i != -1; i--)
	{
		ButtonAnimationCreator but = buttonArray.base[i];
		PushButton(rg, but.pos, but.width, but.height, CreateString(buttonTexts[but.id]), but.color);
	}
}
#if 0
void AnimationCreatorHandleInput(Input *input, AnimationFrameList **currentFrame, float animationLength, v2 playLinePos, float playLineWidth, float playLineHeight, float *playLinePlayPosition, float playLineScrollBarWidth, float playLineScrollBarHeight,  RenderGroup *rg)
{
	if (input->mouse->leftButtonPressedThisFrame)
	{
		//UI
		if (mode == AnimationCreator_SaveNLoad) // this should be more of a differant _window_,  but there is no support
		{
			if (PointInRectangle(V2(1100, 100), 100, 100, input->mousePos)) //save
			{
				SaveAnimation(currentAnimation, saveString + ".ann");
				animationName = saveString;
				mode = AnimationCreator_Edit;
				return;
			}
			
			if (PointInRectangle(v2(1100, 250), 100, 100, input->mousePos)) //load 
			{
				FreeAnimation(currentAnimation);
				currentAnimation = LoadAnimation(animationName + ".ann");
				animationName = saveString;
				mode = AnimationCreator_Edit;
				return;
			}

			if (PointInRectangle(v2(1100, 400), 100, 100, input->mousePos)) //cancel 
			{
				mode = AnimationCreator_Edit;
				return;
			}
		}
		else
		{

			if (PointInRectangle(v2(1100, 300), 100, 100, input->mousePos)) //save and load
			{
				mode = AnimationCreator_SaveNLoad;
			}

			if (time == (*currentFrame)->t && time != 0.0f)
			{
				v2 deleteButtonPos = v2(900, 500);
				float deleteButtonWidth = 100.0f;
				float deleteButtonHeight = 50.0f;

				if (PointInRectangle(deleteButtonPos, deleteButtonWidth, deleteButtonHeight, input->mousePos)) //delete Frame
				{
					AnimationFrameList *prev = FindPreviousAnimationFrame(currentAnimation, *currentFrame);
					Assert(prev);

					prev->next = (*currentFrame)->next;

					for (AnimationElementList *it = (*currentFrame)->elements; it;)
					{
						AnimationElementList *saveIt = it->next;
						delete it;
						it = saveIt;
					}
					delete *currentFrame;
					*currentFrame = prev;

					return;
				}
			}

			v2 newImageButtonPos = v2(800, 500);
			float newImageButtonWidth = 100.0f;
			float newImageButtonHeight = 50.0f;

			if (PointInRectangle(newImageButtonPos, newImageButtonWidth, newImageButtonHeight, input->mousePos)) //create new AnimationElement
			{
				CreateNewAnimationElement(currentAnimation);
				return;
			}
			
			if (PointInRectangle(playLinePos, playLineWidth, playLineHeight, input->mousePos)) //PlayLine
			{
				v2 relPos = input->mousePos - playLinePos;
				time = relPos.x / playLineWidth + *playLinePlayPosition;
				return;
			}

			if (animationLength > 0.5f)
			{
				if (PointInRectangle(playLinePos - v2(0.0f, playLineScrollBarHeight), playLineWidth, playLineScrollBarHeight, input->mousePos)) //PlayLineScollBar
				{
					float playLineMin = playLinePos.x;
					float playLineMax = playLinePos.x + playLineWidth;
					float inToRangeMin = playLinePos.x + playLineScrollBarWidth / 2.0f;
					float inToRangeMax = playLinePos.x + playLineWidth - playLineScrollBarWidth / 2.0f;

					float mappedX = MapRangeToRangeCapped(input->mousePos.x, inToRangeMin, inToRangeMax, 0.0f, animationLength); //to animationLength so I get an additional second to edit
					time += (mappedX - *playLinePlayPosition);
					*playLinePlayPosition = mappedX;
					return;
				}
			}

			float playCursorWidth = 20.0f;
			float playCursorAdditionalY = 5.0f;
			float framePointerButtonWidth = playCursorWidth;
			for (AnimationFrameList *frames = currentAnimation; frames; frames = frames->next)
			{
				float buttPosX = (frames->t - *playLinePlayPosition) * playLineWidth + playLinePos.x - 0.5f * framePointerButtonWidth;
				float buttPosY = playLinePos.y - 8.0f * playCursorAdditionalY;
				if (*playLinePlayPosition <= frames->t &&  frames->t <= *playLinePlayPosition + 1.0f)
				{
					if (PointInRectangle(v2(buttPosX, buttPosY), framePointerButtonWidth, framePointerButtonWidth, input->mousePos))
					{
						time = frames->t;
					}
				}
			}
		}

		switch (mode)
		{
		case AnimationCreator_Play:
		{
			if (PointInRectangle(v2(1100, 100), 100, 100, input->mousePos))//play
			{
				mode = AnimationCreator_Edit;
				return;
			}
		}break;

		case AnimationCreator_Edit:
		{
			v2 currentFramePos;
			if ((*currentFrame)->next)
			{
				currentFramePos = LerpVector2((*currentFrame)->pos, (*currentFrame)->t, (*currentFrame)->next->pos, (*currentFrame)->next->t, time);
			}
			else
			{
				currentFramePos = (*currentFrame)->pos;
			}
			if((time != (*currentFrame)->t) && PointInRectangle(currentFramePos, POSBUTTONSIZE, POSBUTTONSIZE, input->mousePos))
			{
				CreateNewAnimationFrame((*currentFrame), time);
				break;
			}

			//general edit UI
			if (PointInRectangle(v2(1100, 100), 100, 100, input->mousePos))//play
			{
				mode = AnimationCreator_Play;
				selectedElement = NULL;
				return;
			}

			if (PointInRectangle(v2(50, 200), 50, 50, input->mousePos))//rotate
			{
				editMode = AnimationEditMode_Rotate;
				return;
			}
			if (PointInRectangle(v2(50, 270), 50, 50, input->mousePos))//move
			{
				editMode = AnimationEditMode_Move;
				return;
			}
			if (PointInRectangle(v2(50, 340), 50, 50, input->mousePos))//scale
			{
				editMode = AnimationEditMode_Scale;
				return;
			}

			if (selectedElement)
			{
				if (PointInRectangle(v2(1100, 200), 200, 50, input->mousePos)) //sprite Up
				{
					if (++selectedElement->assetIndex == rg->assetHandler->GetAsset({ selectedElement->assetId })->spriteCount)
					{
						selectedElement->assetIndex = 0;
					}
					return;
				}
				
				if (PointInRectangle(v2(1100, 250), 200, 50, input->mousePos)) //sprite down
				{
					if (selectedElement->assetIndex == 0)
					{
						selectedElement->assetIndex = rg->assetHandler->GetAsset({ selectedElement->assetId })->spriteCount - 1;
						
					}
					else
					{
						selectedElement->assetIndex--;
					}
					return;
				}

				if (PointInRectangle(v2(900, 200), 200, 50, input->mousePos)) //asset up
				{
					if (++selectedElement->assetId == Asset_Count)
					{
						selectedElement->assetId = 0;
					}
					selectedElement->assetIndex = 0;
					return;
				}

				if (PointInRectangle(v2(900, 250), 200, 50, input->mousePos)) //asset down
				{
					if (selectedElement->assetId == 0)
					{
						selectedElement->assetId = Asset_Count - 1;
					}
					else
					{
						selectedElement->assetId--;
					}
					selectedElement->assetIndex = 0;
					return;
				}
			}

			//spicific edit UI
			if (editMode == AnimationEditMode_Scale && selectedElement && time == (*currentFrame)->t)
			{
				float scaleButtonSize = 10.0f;

				//scale Buttons

				if (PointInCenteredRectangle(selectedElement->p1, scaleButtonSize, scaleButtonSize, input->oldMousePos))
				{
					return;
				}

				if (PointInCenteredRectangle(selectedElement->p2, scaleButtonSize, scaleButtonSize, input->oldMousePos))
				{
					return;
				}

				if (PointInCenteredRectangle(selectedElement->p3, scaleButtonSize, scaleButtonSize, input->oldMousePos))
				{
					return;
				}

				if (PointInCenteredRectangle(selectedElement->p4, scaleButtonSize, scaleButtonSize, input->oldMousePos))
				{
					return;
				}
			}

			AnimationElementList *oldSelectedElement = selectedElement;
			selectedElement = GetSelectedElementOutOfCurrentFrame((*currentFrame), input, time);

			if ((*currentFrame)->t != time && selectedElement != oldSelectedElement && selectedElement)
			{
				CreateNewAnimationFrame((*currentFrame), time);
				selectedElement = ShiftSelectedAnimationElementToNextFrame(currentAnimation, selectedElement);
				*currentFrame = (*currentFrame)->next;
				return;
			}
		}break;
		default:
		{
			//error handling
		}break;
		}
	}
	else if (input->mouse->leftButtonDown) //drag Button Input
	{

		float playCursorWidth = 20.0f;
		float playCursorAdditionalY = 5.0f;
		float framePointerButtonWidth = playCursorWidth;
		for (AnimationFrameList *frames = currentAnimation; frames; frames = frames->next) // frame Pointer buttons
		{
			float buttPosX = (frames->t - *playLinePlayPosition) * playLineWidth + playLinePos.x - 0.5f * framePointerButtonWidth;
			float buttPosY = playLinePos.y - 8.0f * playCursorAdditionalY;
			if (*playLinePlayPosition <= frames->t &&  frames->t <= *playLinePlayPosition + 1.0f)
			{		
				if (frames->t != 0.0f)
				{
					if(PointInRectangle(v2(buttPosX, buttPosY), framePointerButtonWidth, framePointerButtonWidth, input->oldMousePos))
					{
						frames->t = MapRangeToRangeCapped(input->mousePos.x, playLinePos.x, playLinePos.x + playLineWidth, *playLinePlayPosition, *playLinePlayPosition + 1);
						time = frames->t;
					}
				}
			}
		}


		switch (mode)
		{
		case AnimationCreator_Play:
		{
			if (PointInRectangle(playLinePos, playLineWidth, playLineHeight, input->mousePos)) // PlayLine
			{
				v2 relPos = input->mousePos - playLinePos;
				time = relPos.x / playLineWidth + *playLinePlayPosition;
				return;
			}

		}break;
		case AnimationCreator_Edit:
		{
			if (PointInRectangle(playLinePos, playLineWidth, playLineHeight, input->mousePos)) // PlayLine
			{
				v2 relPos = input->mousePos - playLinePos;
				time = relPos.x / playLineWidth + *playLinePlayPosition;
				return;
			}

			if (time == (*currentFrame)->t)
			{				
				if(PointInRectangle((*currentFrame)->pos, POSBUTTONSIZE, POSBUTTONSIZE, input->oldMousePos))
				{
					(*currentFrame)->pos += input->mouseDelta;
					return;
				}


				if (selectedElement)
				{
					switch (editMode)
					{
					case AnimationEditMode_Rotate:
					{

						if (PointInQuadraliteral(selectedElement->p1, selectedElement->p2, selectedElement->p3, selectedElement->p4, input->oldMousePos))
						{
							v2 middle = 0.25f * (selectedElement->p1 + selectedElement->p2 + selectedElement->p3 + selectedElement->p4);
							float angle = AngleBetween(input->mousePos - middle, input->oldMousePos - middle);

							selectedElement->p1 = RotateAround(middle, -angle, selectedElement->p1);
							selectedElement->p2 = RotateAround(middle, -angle, selectedElement->p2);
							selectedElement->p3 = RotateAround(middle, -angle, selectedElement->p3);
							selectedElement->p4 = RotateAround(middle, -angle, selectedElement->p4);
							return;
						}


					}break;
					case AnimationEditMode_Move:
					{
						if (PointInQuadraliteral(selectedElement->p1, selectedElement->p2, selectedElement->p3, selectedElement->p4, input->oldMousePos))
						{
							selectedElement->p1 += input->mouseDelta;
							selectedElement->p2 += input->mouseDelta;
							selectedElement->p3 += input->mouseDelta;
							selectedElement->p4 += input->mouseDelta;
							return;
						}
					}break;
					case AnimationEditMode_Scale:
					{
						float scaleButtonSize = 10.0f;

						//scale Buttons

						if (PointInCenteredRectangle(selectedElement->p1, scaleButtonSize, scaleButtonSize, input->oldMousePos))
						{
							selectedElement->p1 += input->mouseDelta;

							v2 vp2 = Normalize(selectedElement->p2 - selectedElement->p4);
							selectedElement->p2 += Dot(input->mouseDelta, vp2) * vp2;

							v2 vp3 = Normalize(selectedElement->p3 - selectedElement->p4);
							selectedElement->p3 += Dot(input->mouseDelta, vp3) * vp3;
							return;
						}

						if (PointInCenteredRectangle(selectedElement->p2, scaleButtonSize, scaleButtonSize, input->oldMousePos))
						{
							selectedElement->p2 += input->mouseDelta;

							v2 vp1 = Normalize(selectedElement->p1 - selectedElement->p3);
							selectedElement->p1 += Dot(input->mouseDelta, vp1) * vp1;

							v2 vp4 = Normalize(selectedElement->p4 - selectedElement->p3);
							selectedElement->p4 += Dot(input->mouseDelta, vp4) * vp4;
							return;
						}

						if (PointInCenteredRectangle(selectedElement->p3, scaleButtonSize, scaleButtonSize, input->oldMousePos))
						{
							selectedElement->p3 += input->mouseDelta;

							v2 vp1 = Normalize(selectedElement->p1 - selectedElement->p2);
							selectedElement->p1 += Dot(input->mouseDelta, vp1) * vp1;

							v2 vp4 = Normalize(selectedElement->p4 - selectedElement->p2);
							selectedElement->p4 += Dot(input->mouseDelta, vp4) * vp4;
							return;
						}

						if (PointInCenteredRectangle(selectedElement->p4, scaleButtonSize, scaleButtonSize, input->oldMousePos))
						{
							selectedElement->p4 += input->mouseDelta;

							v2 vp2 = Normalize(selectedElement->p2 - selectedElement->p1);
							selectedElement->p2 += Dot(input->mouseDelta, vp2) * vp2;

							v2 vp3 = Normalize(selectedElement->p3 - selectedElement->p1);
							selectedElement->p3 += Dot(input->mouseDelta, vp3) * vp3;
							return;
						}

					}break;
					default:
					{

					}break;
					}//switch aninationEditMode
				}
				
			}

		}break;
		case AnimationCreator_SaveNLoad:
		{
		}break;
		default:
		{
			
		}break;
		}
	}
}
//AnimationCreatorHandleInput(input, &currentFrame, animationLength, playLinePos, playLineWidth, playLineHeight, &playLinePlayPosition, playLineScrollBarWidth, playLineScrollBarHeight, &rg);
#endif

void PushAnimationCreatorButtons()
{
	
	switch (mode)
	{
	case AnimationCreator_Play:
	{
		PushButtonAC(Button_Play);
		PushButtonAC(Button_SaveNLoad);
		PushButtonAC(Button_PlayLine);
		PushButtonAC(Button_PlayLineScrollBar);

	}break;
	case AnimationCreator_Edit:
	{
		PushButtonAC(Button_Play);
		PushButtonAC(Button_SaveNLoad);
		PushButtonAC(Button_PlayLine);
		PushButtonAC(Button_PlayLineScrollBar);

		PushButtonAC(Button_Rotate);
		PushButtonAC(Button_Move);
		PushButtonAC(Button_Scale);

		PushButtonAC(Button_NewElement);
		
		if (selectedElement)
		{
			PushButtonAC(Button_AssetDown);
			PushButtonAC(Button_AssetUp);
			PushButtonAC(Button_SpriteDown);
			PushButtonAC(Button_SpriteUp);

			switch (editMode)
			{
			case AnimationEditMode_Move:
			{

			}break;
			case AnimationEditMode_Scale:
			{

			}break;
			case AnimationEditMode_Rotate:
			{

			}break;
			default:
				Assert(!"Invalide Edit Mode");
			}break;
		}
				
	}break;//case AnimationCreator_Edit
	case AnimationCreator_SaveNLoad:
	{
		PushButtonAC(Button_Cancel);
		PushButtonAC(Button_Save);
		PushButtonAC(Button_Load);
	}break;

	default:
		Assert(!"Invalide Mode")
			break;
	}

}

void HandleAnimationElementsOnClick(Input *input)
{
	AnimationElementList *oldSelectedElement = selectedElement;
	selectedElement = GetSelectedElementOutOfCurrentFrame(currentFrame, input, time);

	if (currentFrame->t != time && selectedElement) //creates a new Frame iff clicked and still have selected Element
	{
		CreateNewAnimationFrame(currentFrame, time);
		selectedElement = ShiftSelectedAnimationElementToNextFrame(currentAnimation, selectedElement);
		currentFrame = currentFrame->next;
	}
}


void HandleAnimationElementsOnDrag(Input *input)
{
	Assert(selectedElement);

	switch (editMode)
	{
	case AnimationEditMode_Move:
	{
		selectedElement->p1 += input->mouseDelta;
		selectedElement->p2 += input->mouseDelta;
		selectedElement->p3 += input->mouseDelta;
		selectedElement->p4 += input->mouseDelta;

	}break;
	case AnimationEditMode_Rotate:
	{
		v2 middle = 0.25f * (selectedElement->p1 + selectedElement->p2 + selectedElement->p3 + selectedElement->p4);
		float angle = AngleBetween(input->mousePos - middle, input->oldMousePos - middle);

		selectedElement->p1 = RotateAround(middle, -angle, selectedElement->p1);
		selectedElement->p2 = RotateAround(middle, -angle, selectedElement->p2);
		selectedElement->p3 = RotateAround(middle, -angle, selectedElement->p3);
		selectedElement->p4 = RotateAround(middle, -angle, selectedElement->p4);

	}break;
	case AnimationEditMode_Scale:
	{
		//nuttin: this _is_/will be handled via Scale Buttons
	}break;
	default:
	{
		Assert(!"Invalid editMode");
	}break;		
	}
}

void AnimationCreatorUpdateAndRender(RenderCommands *renderComands, Input *input)
{
	RenderGroup renderGroup = InitRenderGroup(assetHandler, renderComands);
	RenderGroup *rg = &renderGroup;
	ClearPushBuffer(rg);
	PushOrthogonalTransform(rg, 1280, 720);
	PushClear(rg, V4(1.0f, 0.4f, 0.4f, 0.4f));	
	
	float animationLength = GetAnimationLength(currentAnimation);

	
	float playLineWidth = 800;
	float playLineHeight = 50;
	float playLineX = 100;
	float playLineY = 720 - 50;
	v2 playLinePos = V2(playLineX, playLineY);
	float playLineScrollBarWidth = 0.5f * playLineWidth / animationLength;
	float playLineScrollBarHeight = 15.0f;
	v2 playLineScrollBarPos = playLinePos + V2(MapRangeToRangeCapped(playLinePlayPosition, 0.0f, animationLength, 0.0f, playLineWidth - playLineScrollBarWidth), -playLineScrollBarHeight);

	currentFrame = GetCurrentAnimationFrame(currentAnimation, time);
	AnimationFrameList *nextFrame = currentFrame->next;
	
	buttonArray.size = 0;
	PushAnimationCreatorButtons();
	SortButtonsByPrority();

	if (input->mouse->leftButtonPressedThisFrame)
	{
		ButtonEnum triggeredButton = GetButtonTriggeredOnClick(input->mousePos);

		if (IsValid(triggeredButton))
		{
			HandleOnClick(triggeredButton, rg, input);
		}
		else
		{
			HandleAnimationElementsOnClick(input);
		}
		
	}
	else if (input->mouse->leftButtonDown)
	{
		ButtonEnum triggeredButton = GetButtonTriggeredOnClick(input->oldMousePos);
		if (IsValid(triggeredButton))
		{
			HandleOnDrag(triggeredButton, rg, input);
		}
		else
		{
			if (selectedElement)
			{
				HandleAnimationElementsOnDrag(input);
			}			
		}
	}

	RenderAnimationFrame(currentFrame, time, rg);
	DrawButtons(rg);

#if 0
	//draw
	switch (mode)
	{
	case AnimationCreator_Play:
	{
		rg.PushButton(v2(1100, 100), 100, 100, "Play", v4(1, 1, 1, 1));
		
		rg.PushButton(v2(playLineX, playLineY), playLineWidth, playLineHeight, ">>>", v4(1, 1, 1, 1));
		
		if (animationLength > 0.5f)
		{
			float playLineScrollBarWidth = 0.5f * playLineWidth / animationLength;
			float playLineScrollBarHeight = 15.0f;
			v2 playLineScrollBarPos = playLinePos + v2(MapRangeToRangeCapped(playLinePlayPosition, 0.0f, animationLength, 0.0f, playLineWidth - playLineScrollBarWidth), -playLineScrollBarHeight);
			rg.PushButton(playLinePos - v2(0.0f, playLineScrollBarHeight), playLineWidth, playLineScrollBarHeight, "", v4(0.0f, 0.5f, 0.5f, 1.0f));
			
			rg.PushRectangle(playLineScrollBarPos, playLineScrollBarWidth, playLineScrollBarHeight, v4(1.0f, 0.2f, 0.2f, 1.0f));

		}

		time += input->mouse->expectedTimePerFrame;
		if (!currentAnimation)
		{
			time = 0;
			mode = AnimationCreator_Edit;
			break;
		}
		if (!RenderAnimationFrame(currentAnimation, time, &rg))
		{
			time = 0;
			playLinePlayPosition = 0.0f;
			mode = AnimationCreator_Edit;
		}
		else
		{
			if (time - playLinePlayPosition > 1.0f)
			{
				playLinePlayPosition += 1.0f;
			}
		}

	}break;
	case AnimationCreator_Edit:
	{
		rg.PushButton(v2(1100, 100), 100, 100, "Play", v4(1, 1, 1, 1));
		rg.PushButton(v2(50, 200), 50, 50, "Rotate", v4(1, 1, 1, 1));
		rg.PushButton(v2(50, 270), 50, 50, "Move", v4(1, 1, 1, 1));
		rg.PushButton(v2(50, 340), 50, 50, "Scale", v4(1, 1, 1, 1));
		rg.PushButton(v2(playLineX, playLineY), playLineWidth, playLineHeight, ">>>", v4(1, 1, 1, 1));
		rg.PushButton(v2(1100, 300), 100, 100, "Save and Load", v4(1, 1, 1, 1));
		

		if (animationLength > 0.5f)
		{
			float playLineScrollBarWidth = 0.5f * playLineWidth / animationLength;
			float playLineScrollBarHeight = 15.0f;
			v2 playLineScrollBarPos = playLinePos + v2(MapRangeToRangeCapped(playLinePlayPosition, 0.0f, animationLength, 0.0f, playLineWidth - playLineScrollBarWidth), -playLineScrollBarHeight);
			rg.PushButton(playLinePos - v2(0.0f, playLineScrollBarHeight), playLineWidth, playLineScrollBarHeight, "", v4(0.0f, 0.5f, 0.5f, 1.0f));
			rg.PushRectangle(playLineScrollBarPos, playLineScrollBarWidth, playLineScrollBarHeight, v4(1.0f, 0.2f, 0.2f, 1.0f));
		}

		v2 currentFramePos;
		if (currentFrame->next)
		{
			currentFramePos = LarpVector2(currentFrame->pos, currentFrame->t, currentFrame->next->pos, currentFrame->next->t, time);
		}
		else
		{
			currentFramePos = currentFrame->pos;
		}		
		rg.PushCenteredRectangle(currentFramePos, POSBUTTONSIZE, POSBUTTONSIZE, v4(1.0f, 1.0f, 0.0f, 0.0f));		
		
		if (selectedElement)
		{
			rg.PushLine(selectedElement->p1, selectedElement->p2);
			rg.PushLine(selectedElement->p2, selectedElement->p4);
			rg.PushLine(selectedElement->p3, selectedElement->p4);
			rg.PushLine(selectedElement->p3, selectedElement->p1);

			rg.PushButton(v2(1100, 200), 200, 50, "Sprite Up", v4(1.0f, 0.3f, 0.0f, 1.0f));
			rg.PushButton(v2(1100, 250), 200, 50, "Sprite Down", v4(1.0f, 0.3f, 0.0f, 1.0f));
			rg.PushButton(v2(900, 200), 200, 50, "Asset Up", v4(1.0f, 0.3f, 0.0f, 1.0f));
			rg.PushButton(v2(900, 250), 200, 50, "Asset Down", v4(1.0f, 0.3f, 0.0f, 1.0f));
		}

		switch (editMode)
		{
		case (AnimationEditMode_Move):
		{

		}break;

		case (AnimationEditMode_Rotate):
		{

		}break;
		case (AnimationEditMode_Scale):
		{
			float scaleButtonSize = 10.0f;

			if (time >= animationLength)
			{
				for (AnimationElementList *it = currentFrame->elements; it; it = it->next)
				{				
					rg.PushCenteredRectangle(it->p1, scaleButtonSize, scaleButtonSize, v4(1, 1, 1, 1));
					rg.PushCenteredRectangle(it->p2, scaleButtonSize, scaleButtonSize, v4(1, 1, 1, 1));
					rg.PushCenteredRectangle(it->p3, scaleButtonSize, scaleButtonSize, v4(1, 1, 1, 1));
					rg.PushCenteredRectangle(it->p4, scaleButtonSize, scaleButtonSize, v4(1, 1, 1, 1));

				}
			}
			else
			{
				AnimationElementList *nextIt = currentFrame->next->elements;
				for (AnimationElementList *it = currentFrame->elements; it; it = it->next, nextIt = nextIt->next)
				{
					Quad quadAtTime = LarpQuad(it->p1, it->p2, it->p3, it->p4, currentFrame->t, nextIt->p1, nextIt->p2, nextIt->p3, nextIt->p4, currentFrame->next->t, time);
					rg.PushCenteredRectangle(quadAtTime.p1, scaleButtonSize, scaleButtonSize, v4(1, 1, 1, 1));
					rg.PushCenteredRectangle(quadAtTime.p2, scaleButtonSize, scaleButtonSize, v4(1, 1, 1, 1));
					rg.PushCenteredRectangle(quadAtTime.p3, scaleButtonSize, scaleButtonSize, v4(1, 1, 1, 1));
					rg.PushCenteredRectangle(quadAtTime.p4, scaleButtonSize, scaleButtonSize, v4(1, 1, 1, 1));

				}
			}
		}

		default:
		{

		}break;
		}//switch renderEditMode

		if (time == currentFrame->t && time != 0.0f)
		{
			v2 deleteButtonPos = v2(900, 500);
			float deleteButtonWidth = 100.0f;
			float deleteButtonHeight = 50.0f;

			rg.PushButton(deleteButtonPos, deleteButtonWidth, deleteButtonHeight, "Delete Frame", v4(1.0f, 1.0f, 0.0f, 0.0f));
		}

		v2 newImageButtonPos = v2(800, 500);
		float newImageButtonWidth = 100.0f;
		float newImageButtonHeight = 50.0f;

		rg.PushButton(newImageButtonPos, newImageButtonWidth, newImageButtonHeight, "new", v4(1, 1, 1, 1));

		RenderAnimationFrame(currentAnimation, time, &rg);

		//button handles at bitmapCorners
		//multiple Modes (rotate, move whole, drag around buttonHandles)/scale
		//ctrl Z list 
		//show layers,  be able to switch order / make visible 
		//loading, saving
		//drawing?
		//zooming?
		//swich to a system, that only saves changes? instead of having all copy usw. (needs more calc to figure out how it looks)


	}break;
	case AnimationCreator_SaveNLoad:
	{
		
		rg.PushButton(v2(1100, 100), 100, 100, "Save", v4(1, 1, 1, 1));

		rg.PushButton(v2(1100, 250), 100, 100, "Load", v4(1, 1, 1, 1));
		
		rg.PushButton(v2(1100, 400), 100, 100, "Cancel", v4(1, 1, 1, 1));

		if (saveString.size == 0) saveString = animationName;

		char pressed = GetPressedChar(input);
		if (pressed)
		{
			saveString = saveString + String(pressed);
		}

		rg.PushDebugString(i12(500, 500), saveString, 30);

	}break;
	default:
	{
		Assert(false);
	}break;

	} //end of drawing



	
	if (mode != AnimationCreator_SaveNLoad)
	{
		float framePointerButtonWidth = playCursorWidth;
		for (AnimationFrameList *frames = currentAnimation; frames; frames = frames->next)
		{
			float buttPosX = (frames->t - playLinePlayPosition) * playLineWidth + playLineX - 0.5f * framePointerButtonWidth;
			float buttPosY = playLineY - 8.0f * playCursorAdditionalY;
			if (playLinePlayPosition <= frames->t &&  frames->t <= playLinePlayPosition + 1.0f)
			{
				rg.PushButton(v2(buttPosX, buttPosY), framePointerButtonWidth, framePointerButtonWidth, String(frames->t, 2), v4(1, 1, 1, 1));
			}
		}

	
#endif
	float playCursorWidth = 20.0f;
	float playCursorAdditionalY = 5.0f;
	if (mode != AnimationCreator_SaveNLoad)
	{
		float playCursorPos = (time - playLinePlayPosition) * playLineWidth + playLineX;
		PushRectangle(rg, V2(playCursorPos - playCursorWidth * 0.5f, playLineY - playCursorAdditionalY), playCursorWidth, 2.0f*playCursorAdditionalY + playLineHeight, V4(1.0f, 0.9f, 0.9f, 1.0f));
	}


}