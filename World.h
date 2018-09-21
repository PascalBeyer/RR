#ifndef RR_WORLD
#define RR_WORLD

#include "buffers.h"
#include "LinearAlgebra.h"
#include "Color.h"

struct AABB
{
	v3 minDim;
	v3 maxDim;
};

//Righthanded coordinates, +Z going into the screen, +Y is "down" ? , +X is to the right
struct Camera
{
	v3 pos;
	union
	{
		Vector3Basis basis;

		struct 
		{
			v3 b1;
			v3 b2;
			v3 b3;
		};
	};
};


struct Triangle
{

	union 
	{
		struct
		{
			ColoredVertex cv1;
			ColoredVertex cv2;
			ColoredVertex cv3;
		};
		struct
		{
			v3 p1;
			u32 c1;
			v3 p2;
			u32 c2;
			v3 p3;
			u32 c3;
		};
		
	};
	
};

struct TriangleArray
{
	u32 amount;
	Triangle *arr;
	Triangle operator[] (u32 i)
	{
		return arr[i];
	}
};

struct World
{
	struct IrradianceCache *cache;
	struct KdNode *kdTree;
	TriangleArray triangles;
	struct LightingTriangle *lightingTriangles;
	u32 amountOfTriangles;
	Camera camera;
	Camera debugCamera;
	v3 lightSource;
};

struct RockCorner
{
	u32 color;
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

static u32 GrayFromU32(u32 i)
{
	u32 c = i & 0xFF;
	u32 ret = (0xFF << 24) | (c << 16) | (c << 8) | (c << 0);
	return ret;
}


static RockCorner CreateRockCorner(v3 p, u32 colorSeed)
{
	RockCorner ret;
	ret.p = p;
	ret.color = GrayFromU32(colorSeed);

	//ret.prev = NULL;
	//ret.next = NULL;
	return ret;
}

static void UpdateCamFocus(Input *input, v3 focusPoint, Camera *camera, f32 aspectRatio, DEBUGKeyTracker tracker)
{
	f32 mouseXRot = 0.0f;
	f32 mouseZRot = 0.0f;
	f32 camZoffSet = 0.0f;

	Die;
	//todo : you know
	if (tracker.aDown)
	{
		v2 mouseDelta = input->mouseDelta;
		float rotSpeed = 0.001f * 3.141592f;
		mouseZRot += mouseDelta.x * rotSpeed;
		mouseXRot += mouseDelta.y * rotSpeed;
	}

	if (tracker.sDown)
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

static void UpdateCamGodMode(Input *input, Camera *cam, DEBUGKeyTracker tracker)
{
	f32 moveSpeed = 0.5f;
#if 0
	if (input->keybord[Key_shift].flag & KeyState_Down)
	{
		moveSpeed = 0.25f;
	}
#endif

	if (tracker.wDown)
	{
		cam->pos += cam->basis.d3 * moveSpeed;
	}
	if (tracker.sDown)
	{
		cam->pos -= cam->basis.d3 * moveSpeed;
	}
	if (tracker.dDown)
	{
		cam->pos += cam->basis.d1 * moveSpeed;
	}
	if (tracker.aDown)
	{
		cam->pos -= cam->basis.d1 * moveSpeed;
	}

	if (tracker.spaceDown)
	{
		float rotSpeed = 0.001f * 3.141592f;
		m4x4 rot = XRotation(-rotSpeed * input->mouseDelta.y) * YRotation(rotSpeed * input->mouseDelta.x);

		m4x4 cameraMat = CamBasisToMat(cam->basis);
		m4x4 invCamMat = InvOrId(cameraMat);

		cam->basis = TransformBasis(cam->basis, cameraMat * rot * invCamMat);
	}

	
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

static void PushTriangleToArena(Arena *arena, v3 p1, v3 p2, v3 p3, u32 c1, u32 c2, u32 c3, v3 normal)
{
	Triangle *tri = PushStruct(arena, Triangle);
	tri->p1 = p1;
	tri->c1 = c1;
	tri->p2 = p2;
	tri->c2 = c2;
	tri->p3 = p3;
	tri->c3 = c3;
}
static void PushTriangleToArena(Arena *arena, ColoredVertex cv1, ColoredVertex cv2, ColoredVertex cv3, v3 normal)
{
	Triangle *tri = PushStruct(arena, Triangle);
	tri->cv1 = cv1;
	tri->cv2 = cv2;
	tri->cv3 = cv3;
}
static void PushTriangleToArena(Arena *arena, v3 p1, v3 p2, v3 p3, u32 c, v3 normal)
{
	PushTriangleToArena(arena, p1, p2, p3, c, c, c, normal);
}
static void PushTriangleToArenaIntendedNormal(Arena *arena, v3 p1, v3 p2, v3 p3, u32 c1, u32 c2, u32 c3, v3 intendedPosNormalDot)
{
	v3 proposedNormal = Normalize(CrossProduct(p2 - p1, p3 - p1));
	//v3 normal = Dot(intendedPosNormal, proposedNormal) > 0 ? proposedNormal : -proposedNormal;
	Assert(proposedNormal != V3());
	if (Dot(intendedPosNormalDot, proposedNormal) > 0)
	{
		PushTriangleToArena(arena, p1, p2, p3, c1, c2, c3, proposedNormal);
	}
	else
	{
		//adjust for face culling
		PushTriangleToArena(arena, p1, p3, p2, c1, c3, c2, -proposedNormal);
	}
}
static void PushTriangleToArenaIntendedNormal(Arena *arena, v3 p1, v3 p2, v3 p3, u32 c, v3 intendedPosNormalDot)
{
	v3 proposedNormal = Normalize(CrossProduct(p2 - p1, p3 - p1));
	//v3 normal = Dot(intendedPosNormal, proposedNormal) > 0 ? proposedNormal : -proposedNormal;
	Assert(proposedNormal != V3());
	if (Dot(intendedPosNormalDot, proposedNormal) > 0)
	{
		PushTriangleToArena(arena, p1, p2, p3, c, c, c, proposedNormal);
	}
	else
	{
		//adjust for face culling
		PushTriangleToArena(arena, p1, p3, p2, c, c, c, -proposedNormal);
	}
}

static void PushTriangleToArenaIntendedNormal(Arena *arena, ColoredVertex cv1, ColoredVertex cv2, ColoredVertex cv3, v3 intendedPosNormal)
{
	PushTriangleToArenaIntendedNormal(arena, cv1.pos, cv2.pos, cv3.pos, cv1.color, cv2.color, cv3.color, intendedPosNormal);
}
static void PushTriangleToArena(Arena *arena, v3 p1, v3 p2, v3 p3, v3 normal)
{
	u32 c = 0xFFFFFFFF;
	PushTriangleToArena(arena, p1, p2, p3, c, c, c, normal);
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

static TriangleArray CreateSkyBoxAndPush(AABB aabb, u32 color, Arena *arena)
{
	TriangleArray ret;
	ret.arr = PushArray(arena, Triangle, 0);
	ret.amount = 12;
	v3 d1 = V3(aabb.maxDim.x - aabb.minDim.x, V2());
	v3 d2 = V3(0.0f, aabb.maxDim.y - aabb.minDim.y, 0.0f);
	v3 d3 = V3(V2(), aabb.maxDim.z - aabb.minDim.z);

	v3 p = aabb.minDim;

	v3 p1 = p + d1;
	v3 p2 = p + d2;
	v3 p3 = p + d3;

	v3 p12 = p + d1 + d2;
	v3 p13 = p + d1 + d3;
	v3 p23 = p + d2 + d3;

	v3 p123 = p + d1 + d2 + d3;
	RandomSeries series = {RandomSeed()};

	u32 randomGray = GrayFromU32(RandomU32(&series) % 15);
	PushTriangleToArenaIntendedNormal(arena, p, p1, p2, color + randomGray, V3(0, 0, 1)); //front
	PushTriangleToArenaIntendedNormal(arena, p12, p1, p2, color + randomGray, V3(0, 0, 1));

	randomGray = GrayFromU32(RandomU32(&series) % 15);
	PushTriangleToArenaIntendedNormal(arena, p, p1, p3, color + randomGray, V3(0, 1, 0));//bottom
	PushTriangleToArenaIntendedNormal(arena, p13, p1, p3, color + randomGray, V3(0, 1, 0));

	randomGray = GrayFromU32(RandomU32(&series) % 15);
	PushTriangleToArenaIntendedNormal(arena, p, p2, p3, color + randomGray, V3(1, 0, 0)); //left
	PushTriangleToArenaIntendedNormal(arena, p23, p2, p3, color + randomGray, V3(1, 0, 0));

	randomGray = GrayFromU32(RandomU32(&series) % 15);
	PushTriangleToArenaIntendedNormal(arena, p3, p13, p23, color + randomGray, V3(0, 0, -1)); //back
	PushTriangleToArenaIntendedNormal(arena, p123, p13, p23, color + randomGray, V3(0, 0, -1));

	randomGray = GrayFromU32(RandomU32(&series) % 15);
	PushTriangleToArenaIntendedNormal(arena, p2, p12, p23, color + randomGray, V3(0, -1, 0)); //top
	PushTriangleToArenaIntendedNormal(arena, p123, p12, p23, color + randomGray, V3(0, -1, 0));

	randomGray = GrayFromU32(RandomU32(&series) % 15);
	PushTriangleToArenaIntendedNormal(arena, p1, p12, p13, color + randomGray, V3(-1, 0, 0)); //right
	PushTriangleToArenaIntendedNormal(arena, p123, p12, p13, color + randomGray, V3(-1, 0, 0));

	return ret;

}

static TriangleArray CreateStoneAndPush(AABB aabb, f32 desiredVolume, Arena *arena, u32 iterations)
{
	Clear(workingArena);

	TriangleArray ret;
	ret.arr = PushArray(arena, Triangle, 0);
	ret.amount = 0;


	v3 aabbMidPoint = 0.5f * (aabb.minDim + aabb.maxDim);

	const u32 logAmountOfSamples = 6; 
	const u32 amountOfRings = logAmountOfSamples + 1;

	RandomSeries series = { RandomSeed() };

	RockCorner *rings[amountOfRings];
	rings[0] = PushStruct(workingArena, RockCorner);
	*rings[0] = CreateRockCorner(V3(aabb.minDim.x, 0.5f * p23(aabb.maxDim + aabb.minDim)), RandomU32(&series) % 10 + 40);
	
	rings[amountOfRings - 1] = PushStruct(workingArena, RockCorner);
	*rings[amountOfRings - 1] = CreateRockCorner(V3(aabb.maxDim.x, 0.5f * p23(aabb.maxDim + aabb.minDim)), RandomU32(&series) % 10 + 40);


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
			*c = CreateRockCorner(V3(xPos, yVal, zVal), RandomU32(&series) % 10 + 40);
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
				PushTriangleToArenaIntendedNormal(arena, leftIt->p, rightIt->next->p, rightIt->p, leftIt->color, rightIt->next->color, rightIt->color, leftIt->p - aabbMidPoint);
				ret.amount++;
				rightIt = rightIt->next;
			}
			PushTriangleToArenaIntendedNormal(arena, leftIt->p, rightIt->p, leftIt->next->p, leftIt->color, rightIt->color, leftIt->next->color, leftIt->p - aabbMidPoint);
			leftIt = leftIt->next;
			ret.amount++;
		} while (leftIt != leftCorner);

		while (rightIt != rightCorner)
		{
			PushTriangleToArenaIntendedNormal(arena, leftIt->p, rightIt->next->p, rightIt->p, leftIt->color, rightIt->next->color, rightIt->color, leftIt->p - aabbMidPoint);
			ret.amount++;
			rightIt = rightIt->next;
		}
	}

	RockCorner *zeroRock = rings[0];
	PushTriangleToArenaIntendedNormal(arena, rings[1]->p, rings[1]->next->p, zeroRock->p, rings[1]->color, rings[1]->next->color, zeroRock->color, zeroRock->p - aabbMidPoint);
	ret.amount++;
	for (RockCorner *it = rings[1]->next; it != rings[1]; it = it->next)
	{
		PushTriangleToArenaIntendedNormal(arena, it->p, it->next->p, zeroRock->p, it->color, it->next->color, zeroRock->color, zeroRock->p - aabbMidPoint);
		ret.amount++;
	}

	RockCorner *endRock = rings[amountOfRings - 1];
	PushTriangleToArenaIntendedNormal(arena, rings[amountOfRings - 2]->p, rings[amountOfRings - 2]->next->p, endRock->p, rings[amountOfRings - 2]->color, rings[amountOfRings - 2]->next->color, endRock->color, endRock->p - aabbMidPoint);
	ret.amount++;
	
	for (RockCorner *it = rings[amountOfRings - 2]->next; it != rings[amountOfRings - 2]; it = it->next)
	{
		PushTriangleToArenaIntendedNormal(arena, it->p, it->next->p, endRock->p, it->color, it->next->color, endRock->color, endRock->p - aabbMidPoint);
		ret.amount++;
	}

	return ret;
	
}

static TriangleArray GenerateAndPushTriangleFloor(AABB aabb, Arena* arena)
{
	u32 blackBrown = RGBAfromHEX(0x553A26);
	v4 groundColor = Unpack4x8(&blackBrown);

	RandomSeries series = { RandomSeed() };

	const u32 meshSize = 40;
	ColoredVertex groundMesh[meshSize][meshSize];

	for (u32 x = 0; x < meshSize; x++)
	{
		for (u32 y = 0; y < meshSize; y++)
		{
			f32 xEntropy = 0.15f * RandomSignedPercent(&series);
			f32 yEntropy = 0.15f * RandomSignedPercent(&series);
			f32 zEntropy = 1.0f * RandomPercent(&series);

			f32 xVal = (xEntropy + (f32)x) / (meshSize) * (aabb.maxDim.x - aabb.minDim.x) + (aabb.minDim.x);
			f32 yVal = (yEntropy + (f32)y) / (meshSize) * (aabb.maxDim.y - aabb.minDim.y) + (aabb.minDim.y);

			groundMesh[x][y].pos = 0.5f * V3(xVal, yVal, zEntropy + aabb.minDim.z);
			groundMesh[x][y].color =  GrayFromU32(RandomU32(&series) % 100) & 0xFFFFFF00;
		}
	}

	TriangleArray ret;
	ret.arr = PushArray(arena, Triangle, 0);
	ret.amount = 0;
	for (u32 x = 0; x < meshSize - 1; x++)
	{
		for (u32 y = 0; y < meshSize - 1; y++)
		{
			PushTriangleToArenaIntendedNormal(arena, groundMesh[x][y], groundMesh[x + 1][y], groundMesh[x + 1][y + 1], V3(0, 0, -1));
			PushTriangleToArenaIntendedNormal(arena, groundMesh[x][y], groundMesh[x][y + 1], groundMesh[x + 1][y + 1], V3(0, 0, -1));
			ret.amount += 2;
		}
	}
	//ret.amount = 2 * Square((meshSize - 1));
	return ret;
}

#endif // !RR_WORLD

