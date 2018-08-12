#ifndef RR_WORLD
#define RR_WORLD

#include "BasicTypes.h"
#include "Vector3.h"
#include "Input.h"
#include "Math.h"
#include "Arena.h"

struct AABB
{
	v3 minDim;
	v3 maxDim;
};

struct Camera
{
	v3 pos;
	Vector3Basis basis;
};


struct Triangle
{
	v3 p1;
	v3 p2;
	v3 p3;
};

struct TriangleArray
{
	u32 amount;
	Triangle *arr;
};

struct World
{
	struct IrradianceCache *cache;
	struct KdNode *kdTree;
	TriangleArray triangles;
	struct LightingTriangle *lightingTriangles;
	u32 amountOfTriangles;
	Camera camera;
	v3 lightSource;
};

struct RockCorner
{
	v3 p;
	union
	{
		RockCorner *adjacentCorners[2];
		struct
		{
			RockCorner *prev;
			RockCorner *next;
		};
	};

	
};

struct RockCornerListElement
{
	RockCorner data;
	RockCornerListElement *next;

};

struct RockCornerList
{
	RockCornerListElement *list;
	RockCornerListElement *freeList;
};

static u32 globalCornerIndex = 0;

static AABB CreateAABB(v3 minDim, v3 maxDim)
{
	return { minDim, maxDim };
}

static AABB InvertedInfinityAABB()
{
	AABB ret;
	ret.maxDim = V3(MINF32, MINF32, MINF32);
	ret.minDim = V3(MAXF32, MAXF32, MAXF32);
	return ret;
}

inline bool v3InAABB(v3 pos, AABB aabb)
{
	return
		aabb.minDim.x <= pos.x &&
		aabb.minDim.y <= pos.y &&
		aabb.minDim.z <= pos.z &&
		aabb.maxDim.x >= pos.x &&
		aabb.maxDim.y >= pos.y &&
		aabb.maxDim.z >= pos.z;
}

static u32 GetCubeNormalIndex(v3 normal)
{
	if (normal.x)
	{
		Assert(!normal.y & !normal.z);
		return (normal.x > 0) ? 0u : 1u;

	}
	if (normal.y)
	{
		Assert(!normal.x & !normal.z);
		return (normal.y > 0) ? 2u : 3u;
	}
	if (normal.z)
	{
		Assert(!normal.x & !normal.y);
		return (normal.z > 0) ? 4u : 5u;
	}
	else
	{
		Assert(!"invalide normal");
		return MAXU32;
	}
}

static v3 GetCubeNormalFromIndex(u32 index)
{
	switch (index)
	{
	case 0:
	{
		return V3(1, 0, 0);
	}break;
	case 1:
	{
		return V3(-1, 0, 0);
	}break;
	case 2:
	{
		return V3(0, 1, 0);
	}break;
	case 3:
	{
		return V3(0, -1, 0);
	}break;
	case 4:
	{
		return V3(0, 0, 1);
	}break;
	case 5:
	{
		return V3(0, 0, -1);
	}break;
	default:
	{

		Assert(!"invalideCubeNormalIndex");
		return V3();
	}break;
	}

}

static RockCorner CreateRockCorner(v3 p)
{
	RockCorner ret;
	ret.p = p;
	//ret.prev = NULL;
	//ret.next = NULL;
	return ret;
}

static void UpdateCamFocus(Input *input, v3 focusPoint, Camera *camera, f32 aspectRatio)
{
	f32 mouseXRot = 0.0f;
	f32 mouseZRot = 0.0f;
	f32 camZoffSet = 0.0f;

	if (input->keybord->u.isDown)
	{
		v2 mouseDelta = input->mouseDelta;
		float rotSpeed = 0.001f * 3.141592f;
		mouseZRot += mouseDelta.x * rotSpeed;
		mouseXRot += mouseDelta.y * rotSpeed;
	}
	if (input->keybord->s.isDown)
	{
		v2 mouseDelta = input->mouseDelta;
		float zoomSpeed = 0.008f;
		camZoffSet += (Abs(camera->pos.z) + camZoffSet) * mouseDelta.y * zoomSpeed;
	}

	m4x4 rotX = XRotation(mouseXRot);
	m4x4 rot = rotX * ZRotation(mouseZRot);
	m4x4 cam = CameraTransform(V3(1, 0, 0), V3(0, 1, 0), V3(0, 0, 1), camZoffSet * camera->basis.d3) * rot;

	camera->pos = cam * camera->pos;
	camera->basis = TransformBasis(camera->basis, rot);
}

static void UpdateCamGodMode(Input *input, Camera *cam)
{
	f32 moveSpeed = 1.0f;
	if (input->keybord->shift.isDown)
	{
		moveSpeed = 0.25f;
	}

	if (input->keybord->w.isDown)
	{
		cam->pos += cam->basis.d3 * moveSpeed;
	}
	if (input->keybord->s.isDown)
	{
		cam->pos -= cam->basis.d3 * moveSpeed;
	}
	if (input->keybord->d.isDown)
	{
		cam->pos += cam->basis.d1 * moveSpeed;
	}
	if (input->keybord->a.isDown)
	{
		cam->pos -= cam->basis.d1 * moveSpeed;
	}

	if (input->keybord->space.isDown)
	{
		float rotSpeed = 0.001f * 3.141592f;
		m4x4 rot = XRotation(-rotSpeed * input->mouseDelta.y) * YRotation(rotSpeed * input->mouseDelta.x);

		m4x4 cameraMat = CamBasisToMat(cam->basis);
		m4x4 invCamMat = InvOrId(cameraMat);

		m4x4 hopefullyId = cameraMat * invCamMat;

		Vector3Basis basisTest1 = TransformBasis(v3StdBasis, cameraMat);
		Vector3Basis basisTest2 = TransformBasis(cam->basis, invCamMat);

		cam->basis = TransformBasis(cam->basis, cameraMat * rot * invCamMat);
	}
}


static RockCornerListElement *PushRockCornerToList(RockCornerList *list, Arena *arena, RockCorner corner)
{
	RockCornerListElement *mem = NULL;
	if (list->freeList)
	{
		mem = list->freeList;
		list->freeList = list->freeList->next;
	}
	else
	{
		mem = PushStruct(arena, RockCornerListElement);
	}

	mem->next = list->list;
	list->list = mem;
	mem->data = corner;
	return mem;
}

static void FreeRockCornerFromListFast(RockCornerList *list, RockCornerListElement *previousElement)
{
	RockCornerListElement *elementToFree;
	if (previousElement)
	{
		elementToFree = previousElement->next;
		previousElement->next = elementToFree->next;
	}
	else
	{
		elementToFree = list->list;
		list->list = elementToFree->next;
	}
	Assert(elementToFree);
	elementToFree->next = list->freeList;
	list->freeList = elementToFree;
}

static void FreeRockCornerFromList(RockCornerList *list, RockCornerListElement *element)
{
	Assert(element);
	RockCornerListElement *prev = NULL;
	for (RockCornerListElement *it = list->list; it; it = it->next)
	{
		if (it == element)
		{
			FreeRockCornerFromListFast(list, prev);
			return;
		}
		prev = it;
	}
	Assert(!"element was not in list");
}

static RockCornerList *InitRockCornerList(Arena *arena)
{
	RockCornerList *ret = PushStruct(arena, RockCornerList);
	ret->freeList = NULL;
	ret->list = NULL;
	return ret;
}


static v3 AABBCorner(AABB aabb, u32 index)
{
	Assert(index < 8);

	v3 corners[8]
	{
		aabb.minDim,
		V3(aabb.maxDim.x, p23(aabb.minDim)),
		V3(p12(aabb.minDim), aabb.maxDim.z),
		V3(aabb.minDim.x, aabb.maxDim.y, aabb.minDim.z),

		aabb.maxDim,
		V3(aabb.minDim.x, p23(aabb.maxDim)),
		V3(p12(aabb.maxDim), aabb.minDim.z),
		V3(aabb.maxDim.x, aabb.minDim.y, aabb.maxDim.z)
	};


	return corners[index];
}

static void PushTriangleToArena(Arena *arena, v3 p1, v3 p2, v3 p3)
{
	Triangle *tri = PushStruct(arena, Triangle);
	tri->p1 = p1;
	tri->p2 = p2;
	tri->p3 = p3;
}

static void SetNeighborPointerToNULL(RockCorner **neighbors, RockCorner *corner)
{

	for (u32 j = 0; j < 3; j++)
	{
		if (!neighbors[j])
		{
			continue;
		}
		RockCorner **arr = neighbors[j]->adjacentCorners;

		for (u32 i = 0; i < 3; i++)
		{
			if (arr[i] == corner)
			{
				arr[i] = NULL;
			}
		}
	}
}



static u32 faculty(u32 n)
{
	u32 ret = 1;
	for (u32 i = 1; i <= n; i++)
	{
		ret *= i;
	}
	return ret;
}

static u32 Over(u32 n, u32 k)
{
	Assert(n >= k);
	u32 ret = faculty(n) / (faculty(k) * faculty(n - k));
	return ret;
}

static TriangleArray CreateStoneAndPush(AABB aabb, f32 desiredVolume, Arena *arena, u32 iterations)
{
	Clear(workingArena);

	const u32 logAmountOfSamples = 6; 
	const u32 amountOfRings = logAmountOfSamples + 1;

	RockCorner *rings[amountOfRings];
	rings[0] = PushStruct(workingArena, RockCorner);
	*rings[0] = CreateRockCorner(V3(aabb.minDim.x, 0.5f * p23(aabb.maxDim + aabb.minDim)));
	
	rings[amountOfRings - 1] = PushStruct(workingArena, RockCorner);
	*rings[amountOfRings - 1] = CreateRockCorner(V3(aabb.maxDim.x, 0.5f * p23(aabb.maxDim + aabb.minDim)));


	for (u32 ringIndex = 1; ringIndex < logAmountOfSamples; ringIndex++)
	{
		u32 amountOfSamples = Over(logAmountOfSamples, ringIndex);
		f32 xPos = aabb.minDim.x + ((aabb.maxDim.x - aabb.minDim.x) / (f32)logAmountOfSamples) * ringIndex;
		RockCorner *lastCorner = NULL;
		for (u32 angleIndex = 0; angleIndex < amountOfSamples; angleIndex++)
		{
			f32 angle = ((f32)angleIndex) * 2.0f * PI / (f32)amountOfSamples;

			f32 xPercent = ringIndex / (f32)amountOfRings;

			f32 a = xPercent * (aabb.maxDim.y - aabb.minDim.y) * 0.5f; //these should be on an elipse, but what evs
			f32 b = xPercent * (aabb.maxDim.z - aabb.minDim.z) * 0.5f;;
			
			f32 sin = Sin(angle);
			f32 cos = Cos(angle);

			f32 r = a * b / (Sqrt(Square(a * sin) + Square(b *cos)));

			f32 yVal = r * cos + (aabb.maxDim.y + aabb.minDim.y) * 0.5f;
			f32 zVal = r * sin + (aabb.maxDim.z + aabb.minDim.z) * 0.5f;

			RockCorner *c = PushStruct(workingArena, RockCorner);
			*c = CreateRockCorner(V3(xPos, yVal, zVal));
			c->prev = lastCorner;
			lastCorner = c;
		}

		RockCorner *next = lastCorner;
		while (next->prev)
		{
			next->prev->next = next;
			next = next->prev;
		}
		next->prev = lastCorner;
		lastCorner->next = next;

		rings[ringIndex] = lastCorner;
	}

	TriangleArray ret;
	ret.arr = PushArray(arena, Triangle, 0);
	ret.amount = 0;
	for (u32 ringIndex = 1; ringIndex < amountOfRings - 2; ringIndex++) // does not deal with the corner points
	{
		RockCorner *leftCorner = rings[ringIndex];
		RockCorner *rightCorner = rings[ringIndex + 1];
		f32 minDist = QuadDist(rightCorner->p, leftCorner->p);
		for (RockCorner *it = rings[ringIndex + 1]->next; it != rings[ringIndex + 1]; it = it->next)
		{
			f32 dist = QuadDist(it->p, leftCorner->p);
			if (dist < minDist)
			{
				rightCorner = it;
				minDist = dist;
			}
		}
		RockCorner *leftIt = leftCorner;
		RockCorner *rightIt = rightCorner;

		do
		{
			while (QuadDist(rightIt->next->p, leftIt->p) < QuadDist(leftIt->next->p, rightIt->next->p))
			{
				PushTriangleToArena(arena, leftIt->p, rightIt->next->p, rightIt->p);
				ret.amount++;
				rightIt = rightIt->next;
			}
			PushTriangleToArena(arena, leftIt->p, rightIt->p, leftIt->next->p);
			leftIt = leftIt->next;
			ret.amount++;
		} while (leftIt != leftCorner);

		while (rightIt != rightCorner)
		{
			PushTriangleToArena(arena, leftIt->p, rightIt->next->p, rightIt->p);
			ret.amount++;
			rightIt = rightIt->next;
		}
	}

	RockCorner *zeroRock = rings[0];
	PushTriangleToArena(arena, rings[1]->p, rings[1]->next->p, zeroRock->p);
	ret.amount++;
	for (RockCorner *it = rings[1]->next; it != rings[1]; it = it->next)
	{
		PushTriangleToArena(arena, it->p, it->next->p, zeroRock->p);
		ret.amount++;
	}

	RockCorner *endRock = rings[amountOfRings - 1];
	PushTriangleToArena(arena, rings[amountOfRings - 2]->p, rings[amountOfRings - 2]->next->p, endRock->p);
	ret.amount++;
	for (RockCorner *it = rings[amountOfRings - 2]->next; it != rings[amountOfRings - 2]; it = it->next)
	{
		PushTriangleToArena(arena, it->p, it->next->p, endRock->p);
		ret.amount++;
	}

	return ret;
}



#if 0
static TriangleArray CreateStoneAndPushTry1(AABB aabb, f32 desiredVolume, Arena *arena, u32 iterations)
{

	Assert(false);
	//Assert(arena != transientArena);
	Arena *arenaT = transientArena;
	Clear(arenaT);

	TriangleArray ret;

	RockCornerList *list = InitRockCornerList(arenaT);
	RockCornerListElement *asArray = (RockCornerListElement *)arenaT->current;

	u32 sd = sizeof(u4);

	for (u32 i = 0; i < 8; i++)
	{
		RockCorner rockCorner = CreateRockCorner(AABBCorner(aabb, i));
		PushRockCornerToList(list, arenaT, rockCorner);
	}	
	asArray[0].data.adjacentCorners[0] = &asArray[1].data;
	asArray[0].data.adjacentCorners[1] = &asArray[7].data;
	asArray[0].data.adjacentCorners[2] = &asArray[2].data;
	asArray[0].data.adjacentCorners[3] = &asArray[5].data;
	asArray[0].data.adjacentCorners[4] = &asArray[3].data;
	asArray[0].data.adjacentCorners[5] = &asArray[6].data;
	
	asArray[1].data.adjacentCorners[0] = &asArray[0].data;
	asArray[1].data.adjacentCorners[1] = &asArray[3].data;
	asArray[1].data.adjacentCorners[2] = &asArray[6].data;
	asArray[1].data.adjacentCorners[3] = &asArray[4].data;
	asArray[1].data.adjacentCorners[4] = &asArray[7].data;
	asArray[1].data.adjacentCorners[5] = &asArray[2].data;
	
	asArray[2].data.adjacentCorners[0] = &asArray[0].data;
	asArray[2].data.adjacentCorners[1] = &asArray[1].data;
	asArray[2].data.adjacentCorners[2] = &asArray[7].data;
	asArray[2].data.adjacentCorners[3] = &asArray[4].data;
	asArray[2].data.adjacentCorners[4] = &asArray[5].data;
	asArray[2].data.adjacentCorners[5] = &asArray[3].data;

	asArray[3].data.adjacentCorners[0] = &asArray[0].data;
	asArray[3].data.adjacentCorners[1] = &asArray[1].data;
	asArray[3].data.adjacentCorners[2] = &asArray[6].data;
	asArray[3].data.adjacentCorners[3] = &asArray[4].data;
	asArray[3].data.adjacentCorners[4] = &asArray[5].data;
	asArray[3].data.adjacentCorners[5] = &asArray[2].data;
	
	asArray[4].data.adjacentCorners[0] = &asArray[7].data;
	asArray[4].data.adjacentCorners[1] = &asArray[2].data;
	asArray[4].data.adjacentCorners[2] = &asArray[5].data;
	asArray[4].data.adjacentCorners[3] = &asArray[3].data;
	asArray[4].data.adjacentCorners[4] = &asArray[6].data;
	asArray[4].data.adjacentCorners[5] = &asArray[1].data;

	asArray[5].data.adjacentCorners[0] = &asArray[2].data;
	asArray[5].data.adjacentCorners[1] = &asArray[7].data;
	asArray[5].data.adjacentCorners[2] = &asArray[4].data;
	asArray[5].data.adjacentCorners[3] = &asArray[6].data;
	asArray[5].data.adjacentCorners[4] = &asArray[3].data;
	asArray[5].data.adjacentCorners[5] = &asArray[0].data;

	asArray[6].data.adjacentCorners[0] = &asArray[1].data;
	asArray[6].data.adjacentCorners[1] = &asArray[0].data;
	asArray[6].data.adjacentCorners[2] = &asArray[3].data;
	asArray[6].data.adjacentCorners[3] = &asArray[5].data;
	asArray[6].data.adjacentCorners[4] = &asArray[4].data;
	asArray[6].data.adjacentCorners[5] = &asArray[7].data;

	asArray[7].data.adjacentCorners[0] = &asArray[1].data;
	asArray[7].data.adjacentCorners[1] = &asArray[0].data;
	asArray[7].data.adjacentCorners[2] = &asArray[2].data;
	asArray[7].data.adjacentCorners[3] = &asArray[5].data;
	asArray[7].data.adjacentCorners[4] = &asArray[4].data;
	asArray[7].data.adjacentCorners[5] = &asArray[6].data;

	
	RockCornerListElement *elementToChange = list->list;
	RockCorner *cornerToChange = &elementToChange->data;
	
	v3 newNodes[3] = 
	{
		0.5f * (cornerToChange->p + cornerToChange->adjacentCorners[0]->p),
		0.5f * (cornerToChange->p + cornerToChange->adjacentCorners[2]->p),
		0.5f * (cornerToChange->p + cornerToChange->adjacentCorners[4]->p),
	};
	
	RockCorner c1 = CreateRockCorner(newNodes[0]);
	RockCorner c2 = CreateRockCorner(newNodes[1]);
	RockCorner c3 = CreateRockCorner(newNodes[2]);
	
	RockCornerListElement *nc1 = PushRockCornerToList(list, arenaT, c1);
	RockCornerListElement *nc2 = PushRockCornerToList(list, arenaT, c2);
	RockCornerListElement *nc3 = PushRockCornerToList(list, arenaT, c3);


	ret.amount = 0;
	ret.arr = (Triangle *)arena->current;
	list->list->data.inSet = true;
	list->list->data.adjacentCorners[0]->inSet = true;
	
	while (RockCornerListElement *base = GetFirstRockCorner(list))
	{
		for (RockCornerListElement *it = base; it; it = it->next)
		{
			if (it->data.inSet)
			{
				continue;
			}
			RockCorner *currentCorner = &it->data;
			for (u32 firstNIndex = 0; firstNIndex < ArrayCount(currentCorner->adjacentCorners); firstNIndex += 2)
			{		
				RockCorner *firstN = currentCorner->adjacentCorners[firstNIndex];
				if (!firstN->inSet)
				{
					continue;
				}

				for (u32 secIt = 0; secIt < 4; secIt++)
				{
					u32 secondNIndex = (firstNIndex + secIt + (ArrayCount(currentCorner->adjacentCorners) - 2)) % ArrayCount(currentCorner->adjacentCorners);

					if (firstNIndex == secondNIndex) { continue; }

					RockCorner *secondN = currentCorner->adjacentCorners[secondNIndex];
					if (!secondN->inSet)
					{
						continue;
					}
					if (CheckAndRecordAdjacentTriangles(currentCorner, firstN, secondN)  && NeighborsContainNode(firstN->adjacentCorners, secondN))
					{
						PushTriangleToArena(arena, currentCorner->p, firstN->p, secondN->p);
						
						
						
						ret.amount++;
						if(ret.amount == iterations)
						{
							return ret;
						}
						currentCorner->inSet = true;
					}
				}
			}
		}
	}	

	return ret;

}

static RockCornerListElement *GetFirstRockCorner(RockCornerList *list)
{
	for (RockCornerListElement *it = list->list; it; it = it->next)
	{
		if (!it->data.inSet)
		{
			return it;
		}
	}
	return NULL;
}

static bool NeighborsContainNode(RockCorner **adjacentCorners, RockCorner *elem)
{
	for (u32 i = 0; i < 6; i++)
	{
		if (adjacentCorners[i] == elem)
		{
			return true;
		}
	}
	return false;
}

static u32 GetIndex(RockCorner **arr, RockCorner *corner, u32 amount)
{
	for (u32 i = 0; i < amount; i++)
	{
		if (arr[i] == corner)
		{
			return i;
		}
	}
	return MAXU32;
}

static void SwitchValues(u32 *u1, u32 *u2)
{
	u32 save = *u1;
	*u1 = *u2;
	*u2 = save;
}

static bool CheckRecordsWorker(RockCorner *currentCorner, RockCorner *firstN, RockCorner *secondN)
{
	u32 count = ArrayCount(currentCorner->adjacentCorners);

	u32 index1 = GetIndex(currentCorner->adjacentCorners, firstN, count);
	u32 index2 = GetIndex(currentCorner->adjacentCorners, secondN, count);

	Assert(index1 < count);
	Assert(index2 < count);

	if (AbsDiff(index1, index2) < 3u)
	{
		if (index2 < index1)
		{
			SwitchValues(&index1, &index2);
		}
	}
	else
	{
		u32 shifted1 = (index1 + 2) % count;
		u32 shifted2 = (index2 + 2) % count;
		if (shifted2 < shifted1)
		{
			SwitchValues(&index1, &index2);
		}
	}

	u4 *mem = (u4 *)&currentCorner->adjacentTriangles;

	for (u32 index = index1; index != index2; index = (index + 1) % count)
	{
		if (mem[index] == 1 || mem[index] == 3)
		{
			return false;
		}
	} // nothing in there

	for (u32 index = index1; index != index2; index = (index + (count - 1)) % count)
	{
		if (mem[index] == 2)
		{
			return true;
		}
		if (mem[index] == 3 || mem[index] == 1)
		{
			return false;
		}
	}

	return true;
}

static void RecordAdjacentTriangles(RockCorner *currentCorner, RockCorner *firstN, RockCorner *secondN)
{
	u32 count = ArrayCount(currentCorner->adjacentCorners);

	u32 index1 = GetIndex(currentCorner->adjacentCorners, firstN, count);
	u32 index2 = GetIndex(currentCorner->adjacentCorners, secondN, count);
	Assert(index1 < count);
	Assert(index2 < count);

	if (AbsDiff(index1, index2) < 3)
	{
		if (index2 < index1)
		{
			SwitchValues(&index1, &index2);
		}
	}
	else
	{
		u32 shifted1 = (index1 + 2) % count;
		u32 shifted2 = (index2 + 2) % count;
		if (shifted2 < shifted1)
		{
			SwitchValues(&index1, &index2);
		}
	}

	u4 *mem = (u4 *)&currentCorner->adjacentTriangles;

	if (mem[index1] == 2)
	{
		mem[index1] = 3;
	}
	else
	{
		mem[index1] = 1;
	}
	if (mem[index2] == 1)
	{
		mem[index2] = 3;
	}
	else
	{
		mem[index2] = 2;
	}
}

static bool CheckAndRecordAdjacentTriangles(RockCorner *currentCorner, RockCorner *firstN, RockCorner *secondN)
{
	if (CheckRecordsWorker(currentCorner, firstN, secondN) &&
		CheckRecordsWorker(firstN, currentCorner, secondN) &&
		CheckRecordsWorker(secondN, currentCorner, firstN))
	{
		RecordAdjacentTriangles(currentCorner, firstN, secondN);
		RecordAdjacentTriangles(firstN, currentCorner, secondN);
		RecordAdjacentTriangles(secondN, currentCorner, firstN);
		return true;
	}

	return false;

}

#endif


#endif // !RR_WORLD

