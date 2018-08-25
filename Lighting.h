#ifndef RR_Lighting
#define RR_Lighting

//todo: optimize kd-tree build
//		gpu stuff
//		gradients
//		think of a way to remove the default triangle

struct ClipRect
{
	u32 xMin;
	u32 yMin;
	u32 xMax;
	u32 yMax;
};


static ClipRect Intersect(ClipRect first, ClipRect second)
{
	ClipRect ret;
	ret.xMin = (u32)Max((float)first.xMin, (float)second.xMin);
	ret.yMin = (u32)Max((float)first.yMin, (float)second.yMin);
	ret.xMax = (u32)Min((float)first.xMax, (float)second.xMax);
	ret.yMax = (u32)Min((float)first.yMax, (float)second.yMax);
	return ret;
}
static u32 Area(ClipRect clipRect)
{
	s32 width = clipRect.xMax - clipRect.xMin;
	s32 height = clipRect.yMax - clipRect.yMin;
	if (width > 0 && height > 0)
		return (width * height);
	else
		return 0;
}


//Globals
static const u32 globalLightingImageSizeFactor = 80; // 80 = 1280 by 720 (actual size)
static const u32 globalLightingImageWidth = globalLightingImageSizeFactor * 16;
static const u32 globalLightingImageHeight = globalLightingImageSizeFactor * 9;
static Bitmap globalLightingBitmap;
static u32 image[globalLightingImageWidth][globalLightingImageHeight];

static u32 iteration = 0;

static f32 ExtractLinearTosRGB(f32 linear)
{
	if (linear < 0.0f)
	{
		linear = 0.0f;
	}

	if (linear > 1.0f)
	{
		linear = 1.0f;
	}

	f32 S = linear*12.92f;
	if (linear > 0.0031308f)
	{
		S = 1.055f * powf(linear, 1.0f / 2.4f) - 0.055f;
	}

	return(S);
}

static v3 ExtractLinearTosRGB(v3 linear)
{
	return V3(
		ExtractLinearTosRGB(linear.x),
		ExtractLinearTosRGB(linear.y),
		ExtractLinearTosRGB(linear.z)
	);
}

static f32Wide RandomPercentWide(u32Wide *seed)
{
	u32Wide x32 = *seed;
	x32 = x32 ^ (x32 << 13);
	x32 = x32 ^ (x32 >> 17);
	x32 = x32 ^ (x32 << 5);
	*seed = x32;

	return CastToF32(x32) / (f32)MAXU32;
}

struct RayCastWork
{
	World world;
	Bitmap bitmap;
	ClipRect clipRect;
};

static u32 GetTriangleIndex(LightingTriangle* arr, LightingTriangle* triangle)
{
	return (u32)(triangle - arr);
}

static IrradianceSampleArray *GetSampleArray(IrradianceCache *cache, LightingTriangle *arr, LightingTriangle *t)
{
	u32 index = GetTriangleIndex(arr, t);	
	return cache->triangleSamples + index;
}


static Interval GetIntersectionRayAABB(v3 rayP, v3 rayD, AABB aabb)
{
	Interval ret = InvertedInfinityInterval();

	if (v3InAABB(rayP, aabb))
	{
		ret.min = 0.0f;
	}

	float x = rayP.x;
	float dx = rayD.x;
	float y = rayP.y;
	float dy = rayD.y;
	float z = rayP.z;
	float dz = rayD.z;

	float aabbMinX = aabb.minDim.x;
	float aabbMaxX = aabb.maxDim.x;
	float aabbMinY = aabb.minDim.y;
	float aabbMaxY = aabb.maxDim.y;
	float aabbMinZ = aabb.minDim.z;
	float aabbMaxZ = aabb.maxDim.z;

	{
		float t1 = (aabbMaxX - x) / dx;
		float intersection1y = dy * t1 + y;
		float intersection1z = dz * t1 + z;
		if (t1 >= 0 && aabbMinY <= intersection1y && intersection1y <= aabbMaxY && aabbMinZ <= intersection1z && intersection1z <= aabbMaxZ)
		{
			ret.min = Min(ret.min, t1);
			ret.max = Max(ret.max, t1);
		}

		float t2 = (aabbMinX - x) / dx;
		float intersection2y = dy * t2 + y;
		float intersection2z = dz * t2 + z;
		if (t2 >= 0 && aabbMinY <= intersection2y && intersection2y <= aabbMaxY && aabbMinZ <= intersection2z && intersection2z <= aabbMaxZ)
		{
			ret.min = Min(ret.min, t2);
			ret.max = Max(ret.max, t2);
		}
	}

	{
		float t1 = (aabbMaxY - y) / dy;
		float intersection1x = dx * t1 + x;
		float intersection1z = dz * t1 + z;
		if (t1 >= 0 && aabbMinX <= intersection1x && intersection1x <= aabbMaxX && aabbMinZ <= intersection1z && intersection1z <= aabbMaxZ)
		{
			ret.min = Min(ret.min, t1);
			ret.max = Max(ret.max, t1);
		}

		float t2 = (aabbMinY - y) / dy;
		float intersection2x = dx * t2 + x;
		float intersection2z = dz * t2 + z;
		if (t2 >= 0 && aabbMinX <= intersection2x && intersection2x <= aabbMaxX && aabbMinZ <= intersection2z && intersection2z <= aabbMaxZ)
		{
			ret.min = Min(ret.min, t2);
			ret.max = Max(ret.max, t2);
		}
	}

	{
		float t1 = (aabbMaxZ - z) / dz;
		float intersection1x = dx * t1 + x;
		float intersection1y = dy * t1 + y;
		if (t1 >= 0 && aabbMinX <= intersection1x && intersection1x <= aabbMaxX && aabbMinY <= intersection1y && intersection1y <= aabbMaxY)
		{
			ret.min = Min(ret.min, t1);
			ret.max = Max(ret.max, t1);
		}

		float t2 = (aabbMinZ - z) / dz;
		float intersection2x = dx * t2 + x;
		float intersection2y = dy * t2 + y;
		if (t2 >= 0 && aabbMinX <= intersection2x && intersection2x <= aabbMaxX && aabbMinY <= intersection2y && intersection2y <= aabbMaxY)
		{
			ret.min = Min(ret.min, t2);
			ret.max = Max(ret.max, t2);
		}
	}

	return ret;
}

static f32 QuadDistToInterval(f32 val, f32 min, f32 max)
{
	f32 ret = 0;
	if (max < val)
	{
		ret = Square(max - val);
	}
	if (min > val)
	{
		ret = Square(val - min);
	}
	return ret;
}

static f32 QuadDistPointAABB(v3 p, AABB aabb)
{
	f32 ret = 0;

	ret += QuadDistToInterval(p.x, aabb.minDim.x, aabb.maxDim.x);
	ret += QuadDistToInterval(p.y, aabb.minDim.y, aabb.maxDim.y);
	ret += QuadDistToInterval(p.z, aabb.minDim.z, aabb.maxDim.z);

	return ret;
}

static bool SampleIntersectsAABB(IrradianceSample *sample, AABB aabb)
{
	return (QuadDistPointAABB(sample->pos, aabb) < sample->quadR);
}


static bool TriangleIntersectsSample(LightingTriangle *t, IrradianceSample *sample)
{
	v3 P = sample->pos;
	v3 A = t->pos - P;
	v3 B = t->pos + t->d1 - P;
	v3 C = t->pos + t->d2 - P;
	f32 rr = sample->quadR;
	v3 V = CrossProduct(B - A, C - A);
	f32 d = Dot(A, V);
	f32 e = Dot(V, V);
	bool sep1 = d * d > rr * e;
	f32 aa = Dot(A, A);
	f32 ab = Dot(A, B);
	f32 ac = Dot(A, C);
	f32 bb = Dot(B, B);
	f32 bc = Dot(B, C);
	f32 cc = Dot(C, C);
	bool sep2 = (aa > rr) & (ab > aa) & (ac > aa);
	bool sep3 = (bb > rr) & (ab > bb) & (bc > bb);
	bool sep4 = (cc > rr) & (ac > cc) & (bc > cc);
	v3 AB = B - A;
	v3 BC = C - B;
	v3 CA = A - C;
	f32 d1 = ab - aa;
	f32 d2 = bc - bb;
	f32 d3 = ac - cc;
	f32 e1 = Dot(AB, AB);
	f32 e2 = Dot(BC, BC);
	f32 e3 = Dot(CA, CA);
	v3 Q1 = A * e1 - d1 * AB;
	v3 Q2 = B * e2 - d2 * BC;
	v3 Q3 = C * e3 - d3 * CA;
	v3 QC = C * e1 - Q1;
	v3 QA = A * e2 - Q2;
	v3 QB = B * e3 - Q3;
	bool sep5 = (Dot(Q1, Q1) > rr * e1 * e1) & (Dot(Q1, QC) > 0);
	bool sep6 = (Dot(Q2, Q2) > rr * e2 * e2) & (Dot(Q2, QA) > 0);
	bool sep7 = (Dot(Q3, Q3) > rr * e3 * e3) & (Dot(Q3, QB) > 0);
	bool separated = sep1 | sep2 | sep3 | sep4 | sep5 | sep6 | sep7;

	return !separated;
}


static u32 InsertSample(KdNode *node, IrradianceSample *sample, LightingTriangle *triangles, IrradianceCache *cache)
{
	if (!SampleIntersectsAABB(sample, node->aabb))
	{
		return 0;
	}

	if (node->isLeaf)
	{
		u32 added = 0;
		for (u32 tI = 0; tI < node->amountOfTriangles; tI++)
		{
			LightingTriangle *t = node->triangles[tI];

			IrradianceSampleArray *samples = GetSampleArray(cache, triangles, t);

			if (TriangleIntersectsSample(t, sample))
			{

				if (samples->amount + 1 > cache->maxEntriesPerTriangle)
				{
					continue;
				}

				bool isNewEntry = true;

				for (u32 i = 0; i < samples->amount; i++) //todo: inset a bool to test against?
				{
					if (samples->entries + i == sample)
					{
						isNewEntry = false;
						break;
					}
				}

				if (isNewEntry)
				{
					samples->entries[samples->amount++] = *sample;
					added++;
				}
			}
		}
		return added;
	}

	u32 addedNeg = InsertSample(node->negative, sample, triangles, cache);
	u32 addedPos = InsertSample(node->positive, sample, triangles, cache);
	return addedNeg + addedPos;
}

struct RayCastResult
{
	u64 bouncesComputed;
	u64 averageCyclesPerBounce;
	u64 trianglesIntersectionCalcuted;
	f64 averageCyclesPerTriangleIntersection;
	u64 raysCast;
	u64 averageCyclesPerRayCast;
	u64 leafsTraversed;
};

static v3Wide FitV3ToAABB(v3Wide v, AABB aabb)
{
	v3Wide ret;
	ret.x = Clamp(v.x, aabb.minDim.x, aabb.maxDim.x);
	ret.y = Clamp(v.y, aabb.minDim.y, aabb.maxDim.y);
	ret.z = Clamp(v.z, aabb.minDim.z, aabb.maxDim.z);
	return ret;
}
static v3 FitV3ToAABB(v3 v, AABB aabb)
{
	v3 ret;
	ret.x = Clamp(v.x, aabb.minDim.x, aabb.maxDim.x);
	ret.y = Clamp(v.y, aabb.minDim.y, aabb.maxDim.y);
	ret.z = Clamp(v.z, aabb.minDim.z, aabb.maxDim.z);
	return ret;
}

struct TriangleSearchResult
{
	LightingTriangle *initialT;
	v3 initialP;
	KdNode *initialLeaf;
};

struct TriangleSearchResultWide
{
	LightingTriangle *initialT[4];
	v3Wide initialP;
	KdNode *initialLeaf[4];
};



static KdNode *dummyLeaf;
static TriangleSearchResult GetInitialTriangle(v3 rayEntry, v3 xyD, KdNode *entryLeaf)
{
	TriangleSearchResult ret;

	f32 epsilon = 0.001f;

	KdNode *currentLeaf = entryLeaf;

	v3 rayP = rayEntry;
	v3 rayD = xyD;

	while (true)
	{
		LightingTriangle **triangles = currentLeaf->triangles;
		LightingTriangle *hitTriangle = NULL;

		AABB aabb = currentLeaf->aabb;

		f32 min = (MAXFLOAT);

		for (u32 i = 0; i < currentLeaf->amountOfTriangles; i++)
		{
			LightingTriangle *t = triangles[i];

			v3 taP = (t->pos);
			v3 taD1 = (t->d1);
			v3 taD2 = (t->d2);
			v3 taN = (t->normal);

			f32 rayPTerm = Dot(taN, rayP);
			f32 taPTerm = Dot(taN, taP);
			f32 directionTerm = Dot(taN, rayD);

			f32 constantTerm = taPTerm - rayPTerm;

			f32 intersectionT = constantTerm / directionTerm;
			bool intersectionGood = (directionTerm < 0.0f) & (intersectionT < min) & (intersectionT >= -0);

			v3 intersection = intersectionT * rayD + rayP;
			v3 relInt = intersection - taP;

			//todo :should we precalc these?
			f32 lenD1Sq = Dot(taD1, taD1);
			f32 lenD2Sq = Dot(taD2, taD2);
			f32 d12 = Dot(taD1, taD2);
			f32 denomInv = (1.0f / (lenD1Sq * lenD2Sq - d12 * d12));

			f32 dotV1 = Dot(relInt, taD1);
			f32 dotV2 = Dot(relInt, taD2);
			f32 bary1 = (lenD2Sq * dotV1 - d12 * dotV2) * denomInv; //barycentric coordinates
			f32 bary2 = (lenD1Sq * dotV2 - d12 * dotV1) * denomInv;

			bool hitInTriangleBounds = (0.0f <= bary1) & (0.0f <= bary2) & (bary1 + bary2 <= 1.001f);

			//todo: fix this w/o eps?
			bool hitInAABBBounds =
				(aabb.minDim.x <= intersection.x + epsilon) &
				(aabb.minDim.y <= intersection.y + epsilon) &
				(aabb.minDim.z <= intersection.z + epsilon) &
				(aabb.maxDim.x >= intersection.x - epsilon) &
				(aabb.maxDim.y >= intersection.y - epsilon) &
				(aabb.maxDim.z >= intersection.z - epsilon);

			if (intersectionGood & hitInTriangleBounds & hitInAABBBounds)
			{
				min = intersectionT;
				hitTriangle = triangles[i];
			}
		}

		bool hit = (min < F32MAX);

		if (!hit)
		{
			//we are complaring dot products to 0, so we only get the _second_ intersection, so we only need to take the minimal second intersection
			f32 curIntersectionMin = MAXF32;
			KdNode *nextNode = NULL;

			f32 x = rayP.x;
			f32 dx = rayD.x;
			f32 y = rayP.y;
			f32 dy = rayD.y;
			f32 z = rayP.z;
			f32 dz = rayD.z;

			f32 aabbMinX = aabb.minDim.x;
			f32 aabbMaxX = aabb.maxDim.x;
			f32 aabbMinY = aabb.minDim.y;
			f32 aabbMaxY = aabb.maxDim.y;
			f32 aabbMinZ = aabb.minDim.z;
			f32 aabbMaxZ = aabb.maxDim.z;

			f32 t1x = (aabbMaxX - x) / dx;
			if (dx > 0 && t1x <= curIntersectionMin)
			{
				nextNode = currentLeaf->posXNeighboor;
				curIntersectionMin = t1x;
			}

			f32 t2x = (aabbMinX - x) / dx;
			if (dx < 0 && t2x <= curIntersectionMin)
			{
				nextNode = currentLeaf->negXNeighboor;
				curIntersectionMin = t2x;
			}

			f32 t1y = (aabbMaxY - y) / dy;
			if (dy > 0 && t1y <= curIntersectionMin)
			{
				nextNode = currentLeaf->posYNeighboor;
				curIntersectionMin = t1y;
			}

			f32 t2y = (aabbMinY - y) / dy;
			if (dy < 0 && t2y <= curIntersectionMin)
			{
				nextNode = currentLeaf->negYNeighboor;
				curIntersectionMin = t2y;
			}

			f32 t1z = (aabbMaxZ - z) / dz;
			if (dz > 0 && t1z <= curIntersectionMin)
			{
				nextNode = currentLeaf->posZNeighboor;
				curIntersectionMin = t1z;
			}

			f32 t2z = (aabbMinZ - z) / dz;
			if (dz < 0 && t2z <= curIntersectionMin)
			{
				nextNode = currentLeaf->negZNeighboor;
				curIntersectionMin = t2z;
			}
			v3 curExit = rayD * curIntersectionMin + rayP;
			while (!nextNode->isLeaf)
			{
				v3 planePos = nextNode->planePos;

				if (Dot(nextNode->normal, curExit - planePos) > 0)
				{
					nextNode = nextNode->positive;
				}
				else
				{
					nextNode = nextNode->negative;
				}
			}
			currentLeaf = nextNode;

		}
		else
		{
			ret.initialT = hitTriangle;
			ret.initialP = rayD * min + rayP;
			ret.initialLeaf = currentLeaf;
			break;
		}
	}

	return ret;
}
static v3 CastRayToLightSource(v3 initialP, v3 lightSource, KdNode *initialLeaf, f32 bounceWeight, v3 initialAttenuation)
{
	f32 epsilon = 0.001f;

	v3 relLightSource = lightSource - initialP;
	float lightSourceNormSq = NormSquared(relLightSource);
	v3 lightD = FastNormalize(relLightSource);

	v3 rayP = initialP;
	v3 rayD = lightD;
	KdNode *currentLeaf = initialLeaf;

	while (true)
	{
		LightingTriangle **triangles = currentLeaf->triangles;

		AABB aabb = currentLeaf->aabb;

		f32 min = (MAXFLOAT);

		for (u32 i = 0; i < currentLeaf->amountOfTriangles; i++)
		{
			LightingTriangle *t = triangles[i];

			v3 taP = (t->pos);
			v3 taD1 = (t->d1);
			v3 taD2 = (t->d2);
			v3 taN = (t->normal);

			f32 rayPTerm = Dot(taN, rayP);
			f32 taPTerm = Dot(taN, taP);
			f32 directionTerm = Dot(taN, rayD);

			f32 constantTerm = taPTerm - rayPTerm;

			f32 intersectionT = constantTerm / directionTerm;
			bool intersectionGood = (directionTerm < 0.0f) & (intersectionT < min) & (intersectionT >= -0);

			v3 intersection = intersectionT * rayD + rayP;
			v3 relInt = intersection - taP;

			//todo :should we precalc these?
			f32 lenD1Sq = Dot(taD1, taD1);
			f32 lenD2Sq = Dot(taD2, taD2);
			f32 d12 = Dot(taD1, taD2);
			f32 denomInv = (1.0f / (lenD1Sq * lenD2Sq - d12 * d12));

			f32 dotV1 = Dot(relInt, taD1);
			f32 dotV2 = Dot(relInt, taD2);
			f32 bary1 = (lenD2Sq * dotV1 - d12 * dotV2) * denomInv; //barycentric coordinates
			f32 bary2 = (lenD1Sq * dotV2 - d12 * dotV1) * denomInv;

			bool hitInTriangleBounds = (0.0f <= bary1) & (0.0f <= bary2) & (bary1 + bary2 <= 1.0f);

			//todo: fix this w/o eps?
			bool hitInAABBBounds =
				(aabb.minDim.x <= intersection.x + epsilon) &
				(aabb.minDim.y <= intersection.y + epsilon) &
				(aabb.minDim.z <= intersection.z + epsilon) &
				(aabb.maxDim.x >= intersection.x - epsilon) &
				(aabb.maxDim.y >= intersection.y - epsilon) &
				(aabb.maxDim.z >= intersection.z - epsilon);

			if (intersectionGood & hitInTriangleBounds & hitInAABBBounds)
			{
				min = intersectionT;
			}
		}

		bool hit = (min < F32MAX);

		if (!hit)
		{
			//we are complaring dot products to 0, so we only get the _second_ intersection, so we only need to take the minimal second intersection
			f32 curIntersectionMin = MAXF32;
			KdNode *nextNode = NULL;

			f32 x = rayP.x;
			f32 dx = rayD.x;
			f32 y = rayP.y;
			f32 dy = rayD.y;
			f32 z = rayP.z;
			f32 dz = rayD.z;

			f32 aabbMinX = aabb.minDim.x;
			f32 aabbMaxX = aabb.maxDim.x;
			f32 aabbMinY = aabb.minDim.y;
			f32 aabbMaxY = aabb.maxDim.y;
			f32 aabbMinZ = aabb.minDim.z;
			f32 aabbMaxZ = aabb.maxDim.z;

			f32 t1x = (aabbMaxX - x) / dx;
			if (dx > 0 && t1x <= curIntersectionMin)
			{
				nextNode = currentLeaf->posXNeighboor;
				curIntersectionMin = t1x;
			}

			f32 t2x = (aabbMinX - x) / dx;
			if (dx < 0 && t2x <= curIntersectionMin)
			{
				nextNode = currentLeaf->negXNeighboor;
				curIntersectionMin = t2x;
			}

			f32 t1y = (aabbMaxY - y) / dy;
			if (dy > 0 && t1y <= curIntersectionMin)
			{
				nextNode = currentLeaf->posYNeighboor;
				curIntersectionMin = t1y;
			}

			f32 t2y = (aabbMinY - y) / dy;
			if (dy < 0 && t2y <= curIntersectionMin)
			{
				nextNode = currentLeaf->negYNeighboor;
				curIntersectionMin = t2y;
			}

			f32 t1z = (aabbMaxZ - z) / dz;
			if (dz > 0 && t1z <= curIntersectionMin)
			{
				nextNode = currentLeaf->posZNeighboor;
				curIntersectionMin = t1z;
			}

			f32 t2z = (aabbMinZ - z) / dz;
			if (dz < 0 && t2z <= curIntersectionMin)
			{
				nextNode = currentLeaf->negZNeighboor;
				curIntersectionMin = t2z;
			}
			v3 curExit = rayD * curIntersectionMin + rayP;
			while (!nextNode->isLeaf)
			{
				v3 planePos = nextNode->planePos;

				if (Dot(nextNode->normal, curExit - planePos) > 0)
				{
					nextNode = nextNode->positive;
				}
				else
				{
					nextNode = nextNode->negative;
				}
			}
			currentLeaf = nextNode;
		}
		else
		{
			if (min * min >= lightSourceNormSq - epsilon)
			{
				return (bounceWeight * initialAttenuation);
			}

			return V3();
		}
	}
}


#define ConditionalPointerAssign(left, mask, right) \
for (u32 assignIndex = 0; assignIndex < 4; assignIndex++)\
{\
	if (Lane(mask, assignIndex))\
	{\
		left[assignIndex] = right[assignIndex];\
	}\
}

#define ConditionalPointerAssignSpecial(left, mask, right, rightExt) \
for (u32 assignIndex= 0; assignIndex < 4; assignIndex++)\
{\
	if (Lane(mask, assignIndex))\
	{\
		left[assignIndex] = right[assignIndex]->rightExt;\
	}\
}

static KdNode *TraverseDown(KdNode *nextNode, v3 curExit)
{
	while (!nextNode->isLeaf)
	{
		v3 planePos = nextNode->planePos;

		if (Dot(nextNode->normal, curExit - planePos) > 0)
		{
			nextNode = nextNode->positive;
		}
		else
		{
			nextNode = nextNode->negative;
		}
	}

	return nextNode;
}



static TriangleSearchResultWide GetInitialTriangleWide(v3Wide rayEntry, v3Wide xyD, KdNode *entryLeaf)
{

	TriangleSearchResultWide ret = {};

	f32Wide epsilon = Load(0.001f);

	KdNode *currentLeaf[4] = { entryLeaf, entryLeaf, entryLeaf, entryLeaf };

	v3Wide rayP = rayEntry;
	v3Wide rayD = xyD;

	u32Wide finishedMask = Load(0u);

	//u32Wide trianglesHit;

	//Triangle defaultT = {}; is now handled in creation

	while (AnyFalse(finishedMask))
	{
		LightingTriangle *hitTriangle[4] = {};

		v3Wide aabbMin = LoadFromStruct(currentLeaf, aabb.minDim);
		v3Wide aabbMax = LoadFromStruct(currentLeaf, aabb.maxDim);

		f32Wide min = Load(MAXFLOAT);

		u32Wide triangleAmount = LoadFromStruct(currentLeaf, amountOfTriangles);

		KdNode *dummiedLeafs[4];
		for (u32 i = 0; i < 4; i++)
		{
			dummiedLeafs[i] = currentLeaf[i]->amountOfTriangles ? currentLeaf[i] : dummyLeaf;
		}

		u32 i = 0;
		u32Wide iMask = (Load(i) < triangleAmount);
		while (AnyTrue(AndNot(iMask, finishedMask)))
		{
			LightingTriangle *t[4];
			for (u32 ti = 0; ti < 4; ti++)
			{
				t[ti] = dummiedLeafs[ti]->triangles[Min(i, dummiedLeafs[ti]->amountOfTriangles - 1)];
			}

			v3Wide taP = LoadFromStruct(t, pos);
			v3Wide taD1 = LoadFromStruct(t, d1);
			v3Wide taD2 = LoadFromStruct(t, d2);
			v3Wide taN = LoadFromStruct(t, normal);

			//todo :should we precalc these?
			f32Wide lenD1Sq = Dot(taD1, taD1);
			f32Wide lenD2Sq = Dot(taD2, taD2);
			f32Wide d12 = Dot(taD1, taD2);
			f32Wide denomInv = (1.0f / (lenD1Sq * lenD2Sq - d12 * d12));

			f32Wide rayPTerm = Dot(taN, rayP);
			f32Wide taPTerm = Dot(taN, taP);
			f32Wide directionTerm = Dot(taN, rayD);

			f32Wide constantTerm = taPTerm - rayPTerm;

			f32Wide intersectionT = constantTerm / directionTerm;
			u32Wide intersectionGood = (directionTerm < Zerof32Wide()) & (intersectionT < min) & (intersectionT >= Zerof32Wide());

			v3Wide intersection = intersectionT * rayD + rayP;
			v3Wide relInt = intersection - taP;

			f32Wide dotV1 = Dot(relInt, taD1);
			f32Wide dotV2 = Dot(relInt, taD2);
			f32Wide bary1 = (lenD2Sq * dotV1 - d12 * dotV2) * denomInv; //barycentric coordinates
			f32Wide bary2 = (lenD1Sq * dotV2 - d12 * dotV1) * denomInv;

			u32Wide hitInTriangleBounds = (Zerof32Wide() <= bary1) & (Zerof32Wide() <= bary2) & (bary1 + bary2 <= Load(1.0f));

			//todo: fix this w/o eps?
			u32Wide hitInAABBBounds =
				(aabbMin.x <= intersection.x + epsilon) &
				(aabbMin.y <= intersection.y + epsilon) &
				(aabbMin.z <= intersection.z + epsilon) &
				(aabbMax.x >= intersection.x - epsilon) &
				(aabbMax.y >= intersection.y - epsilon) &
				(aabbMax.z >= intersection.z - epsilon);

			u32Wide hitTMask = (intersectionGood & hitInTriangleBounds & hitInAABBBounds);

			u32Wide assignMask = AndNot(hitTMask, finishedMask);
			if (AnyTrue(assignMask))
			{
				ConditionalAssign(&min, assignMask, intersectionT);
				ConditionalPointerAssign(hitTriangle, assignMask, t);
				finishedMask = finishedMask | assignMask;
			}

			i++;
			iMask = (Load(i) < triangleAmount);
		}

		u32Wide hit = (min < Load(F32MAX));

		if (AnyTrue(hit))
		{
			ConditionalPointerAssign(ret.initialT, hit, hitTriangle);
			ConditionalAssign(&ret.initialP, hit, rayD * min + rayP);
			ConditionalPointerAssign(ret.initialLeaf, hit, currentLeaf);
		}
		//we are complaring dot products to 0, so we only get the _second_ intersection, so we only need to take the minimal second intersection
		f32Wide curIntersectionMin = Load(MAXF32);
		u32Wide nextNodeIndex = ZeroU32Wide();

		f32Wide x = rayP.x;
		f32Wide dx = rayD.x;
		f32Wide y = rayP.y;
		f32Wide dy = rayD.y;
		f32Wide z = rayP.z;
		f32Wide dz = rayD.z;

		f32Wide aabbMinX = aabbMin.x;
		f32Wide aabbMaxX = aabbMax.x;
		f32Wide aabbMinY = aabbMin.y;
		f32Wide aabbMaxY = aabbMax.y;
		f32Wide aabbMinZ = aabbMin.z;
		f32Wide aabbMaxZ = aabbMax.z;

		f32Wide t1x = (aabbMaxX - x) / dx;
		{
			u32Wide hitAABB = dx > Zerof32Wide() & t1x <= curIntersectionMin;
			ConditionalAssign(&nextNodeIndex, hitAABB, Load(0u));
			ConditionalAssign(&curIntersectionMin, hitAABB, t1x);
		}

		f32Wide t2x = (aabbMinX - x) / dx;
		{
			u32Wide hitAABB = (dx < Zerof32Wide() & t2x <= curIntersectionMin);
			ConditionalAssign(&nextNodeIndex, hitAABB, Load(1u));
			ConditionalAssign(&curIntersectionMin, hitAABB, t2x);
		}

		f32Wide t1y = (aabbMaxY - y) / dy;
		{
			u32Wide hitAABB = (dy > Zerof32Wide() & t1y <= curIntersectionMin);
			ConditionalAssign(&nextNodeIndex, hitAABB, Load(2u));
			ConditionalAssign(&curIntersectionMin, hitAABB, t1y);
		}

		f32Wide t2y = (aabbMinY - y) / dy;
		{
			u32Wide hitAABB = (dy < Zerof32Wide() & t2y <= curIntersectionMin);
			ConditionalAssign(&nextNodeIndex, hitAABB, Load(3u));
			ConditionalAssign(&curIntersectionMin, hitAABB, t2y);
		}

		f32Wide t1z = (aabbMaxZ - z) / dz;
		{
			u32Wide hitAABB = (dz > Zerof32Wide() & t1z <= curIntersectionMin);
			ConditionalAssign(&nextNodeIndex, hitAABB, Load(4u));
			ConditionalAssign(&curIntersectionMin, hitAABB, t1z);
		}

		f32Wide t2z = (aabbMinZ - z) / dz;
		{
			u32Wide hitAABB = (dz < Zerof32Wide() & t2z <= curIntersectionMin);
			ConditionalAssign(&nextNodeIndex, hitAABB, Load(5u));
			ConditionalAssign(&curIntersectionMin, hitAABB, t2z);
		}
		v3Wide curExit = rayD * curIntersectionMin + rayP;

		for (u32 laneIndex = 0; laneIndex < 4; laneIndex++)
		{
			if (!Lane(finishedMask, laneIndex))
			{
				currentLeaf[laneIndex] = TraverseDown(currentLeaf[laneIndex]->neighboors[Lane(nextNodeIndex, laneIndex)], Lane(curExit, laneIndex));
			}
		}
	}

	return ret;
}

static v3Wide CastRayToLightSourceWide(v3Wide initialP, v3Wide lightSource, KdNode **initialLeaf, f32Wide bounceWeight, v3Wide initialAttenuation)
{
	f32Wide epsilon = Load(0.001f);

	f32Wide returnTime = Zerof32Wide();

	v3Wide relLightSource = lightSource - initialP;
	f32Wide lightSourceNormSq = NormSquared(relLightSource);
	v3Wide lightD = FastNormalize(relLightSource);


	KdNode *currentLeaf[4] = { initialLeaf[0], initialLeaf[1], initialLeaf[2], initialLeaf[3] };

	v3Wide rayP = initialP;
	v3Wide rayD = lightD;

	u32Wide finishedMask = Load(0u);
	LightingTriangle defaultT = {};

	while (AnyFalse(finishedMask))
	{
		LightingTriangle *hitTriangle[4] = {};

		v3Wide aabbMin = LoadFromStruct(currentLeaf, aabb.minDim);
		v3Wide aabbMax = LoadFromStruct(currentLeaf, aabb.maxDim);

		f32Wide min = Load(MAXFLOAT);

		u32Wide triangleAmount = LoadFromStruct(currentLeaf, amountOfTriangles);

		KdNode *dummiedLeafs[4];
		for (u32 i = 0; i < 4; i++)
		{
			dummiedLeafs[i] = currentLeaf[i]->amountOfTriangles ? currentLeaf[i] : dummyLeaf;
		}

		u32 i = 0;
		u32Wide iMask = (Load(i) < triangleAmount);
		while (AnyTrue(AndNot(iMask, finishedMask)))
		{
			LightingTriangle *t[4];
			for (u32 ti = 0; ti < 4; ti++)
			{
				t[ti] = dummiedLeafs[ti]->triangles[Min(i, dummiedLeafs[ti]->amountOfTriangles - 1)];
			}

			v3Wide taP = LoadFromStruct(t, pos);
			v3Wide taD1 = LoadFromStruct(t, d1);
			v3Wide taD2 = LoadFromStruct(t, d2);
			v3Wide taN = LoadFromStruct(t, normal);

			//todo :should we precalc these?
			f32Wide lenD1Sq = Dot(taD1, taD1);
			f32Wide lenD2Sq = Dot(taD2, taD2);
			f32Wide d12 = Dot(taD1, taD2);
			f32Wide denomInv = (1.0f / (lenD1Sq * lenD2Sq - d12 * d12));

			f32Wide rayPTerm = Dot(taN, rayP);
			f32Wide taPTerm = Dot(taN, taP);
			f32Wide directionTerm = Dot(taN, rayD);

			f32Wide constantTerm = taPTerm - rayPTerm;

			f32Wide intersectionT = constantTerm / directionTerm;
			u32Wide intersectionGood = (directionTerm < Zerof32Wide()) & (intersectionT < min) & (intersectionT >= Zerof32Wide());

			v3Wide intersection = intersectionT * rayD + rayP;
			v3Wide relInt = intersection - taP;

			f32Wide dotV1 = Dot(relInt, taD1);
			f32Wide dotV2 = Dot(relInt, taD2);
			f32Wide bary1 = (lenD2Sq * dotV1 - d12 * dotV2) * denomInv; //barycentric coordinates
			f32Wide bary2 = (lenD1Sq * dotV2 - d12 * dotV1) * denomInv;

			u32Wide hitInTriangleBounds = (Zerof32Wide() <= bary1) & (Zerof32Wide() <= bary2) & (bary1 + bary2 <= Load(1.0f));

			//todo: fix this w/o eps?
			u32Wide hitInAABBBounds =
				(aabbMin.x <= intersection.x + epsilon) &
				(aabbMin.y <= intersection.y + epsilon) &
				(aabbMin.z <= intersection.z + epsilon) &
				(aabbMax.x >= intersection.x - epsilon) &
				(aabbMax.y >= intersection.y - epsilon) &
				(aabbMax.z >= intersection.z - epsilon);

			u32Wide assignMask = (intersectionGood & hitInTriangleBounds & hitInAABBBounds);
			finishedMask = finishedMask | assignMask;

			ConditionalAssign(&min, assignMask, intersectionT);

			i++;
			iMask = (Load(i) < triangleAmount);
		}

		u32Wide hit = (min < Load(F32MAX));

		ConditionalAssign(&returnTime, hit, min);

		//we are complaring dot products to 0, so we only get the _second_ intersection, so we only need to take the minimal second intersection
		f32Wide curIntersectionMin = Load(MAXF32);
		u32Wide nextNodeIndex = ZeroU32Wide();

		f32Wide x = rayP.x;
		f32Wide dx = rayD.x;
		f32Wide y = rayP.y;
		f32Wide dy = rayD.y;
		f32Wide z = rayP.z;
		f32Wide dz = rayD.z;

		f32Wide aabbMinX = aabbMin.x;
		f32Wide aabbMaxX = aabbMax.x;
		f32Wide aabbMinY = aabbMin.y;
		f32Wide aabbMaxY = aabbMax.y;
		f32Wide aabbMinZ = aabbMin.z;
		f32Wide aabbMaxZ = aabbMax.z;

		f32Wide t1x = (aabbMaxX - x) / dx;
		{
			u32Wide hitAABB = dx > Zerof32Wide() & t1x <= curIntersectionMin;
			ConditionalAssign(&nextNodeIndex, hitAABB, Load(0u));
			ConditionalAssign(&curIntersectionMin, hitAABB, t1x);
		}

		f32Wide t2x = (aabbMinX - x) / dx;
		{
			u32Wide hitAABB = (dx < Zerof32Wide() & t2x <= curIntersectionMin);
			ConditionalAssign(&nextNodeIndex, hitAABB, Load(1u));
			ConditionalAssign(&curIntersectionMin, hitAABB, t2x);
		}

		f32Wide t1y = (aabbMaxY - y) / dy;
		{
			u32Wide hitAABB = (dy > Zerof32Wide() & t1y <= curIntersectionMin);
			ConditionalAssign(&nextNodeIndex, hitAABB, Load(2u));
			ConditionalAssign(&curIntersectionMin, hitAABB, t1y);
		}

		f32Wide t2y = (aabbMinY - y) / dy;
		{
			u32Wide hitAABB = (dy < Zerof32Wide() & t2y <= curIntersectionMin);
			ConditionalAssign(&nextNodeIndex, hitAABB, Load(3u));
			ConditionalAssign(&curIntersectionMin, hitAABB, t2y);
		}

		f32Wide t1z = (aabbMaxZ - z) / dz;
		{
			u32Wide hitAABB = (dz > Zerof32Wide() & t1z <= curIntersectionMin);
			ConditionalAssign(&nextNodeIndex, hitAABB, Load(4u));
			ConditionalAssign(&curIntersectionMin, hitAABB, t1z);
		}

		f32Wide t2z = (aabbMinZ - z) / dz;
		{
			u32Wide hitAABB = (dz < Zerof32Wide() & t2z <= curIntersectionMin);
			ConditionalAssign(&nextNodeIndex, hitAABB, Load(5u));
			ConditionalAssign(&curIntersectionMin, hitAABB, t2z);
		}
		v3Wide curExit = rayD * curIntersectionMin + rayP;

		for (u32 laneIndex = 0; laneIndex < 4; laneIndex++)
		{
			if (!Lane(finishedMask, laneIndex))
			{
				currentLeaf[laneIndex] = TraverseDown(currentLeaf[laneIndex]->neighboors[Lane(nextNodeIndex, laneIndex)], Lane(curExit, laneIndex));
			}
		}
	}

	u32Wide AssignMask = (returnTime * returnTime >= lightSourceNormSq - epsilon);

	v3Wide ret = ZeroV3Wide();

	ConditionalAssign(&ret, AssignMask, (bounceWeight * initialAttenuation));

	return ret;
}

static IrradianceSample SampleIrradiance(v3 initialP, LightingTriangle *initialT, KdNode *initialLeaf, v3 lightSource, v3 initialAttenuation, u32 rayAmountPerBounceAmount, u32 maxBounceCount, RandomSeries *entropy, AABB sceneAABB)
{
	f32 epsilon = 0.001f;

	v3 indirectColor = V3();

	v3 initialN = initialT->normal;

	f32 minHitDist = F32MAX;
	f32 rayWeight = 1.0f / (f32)rayAmountPerBounceAmount;
	f32 bounceWeight = 1.0f / (f32)maxBounceCount;

	v3 initialTb1 = FastNormalize(initialT->d1);
	v3 initialTb2 = CrossProduct(initialN, initialTb1);

	for (u32 rayIndex = 0; rayIndex < rayAmountPerBounceAmount; rayIndex++)
	{
		v3 rayColor = V3();

		for (u32 maxBounceIndex = 1; maxBounceIndex < maxBounceCount; maxBounceIndex++)
		{
			v3 rayP = initialP;
			v3 entropyForD = FastNormalize(V3(RandomGaussian(entropy), RandomGaussian(entropy), RandomPositiveGaussian(entropy)));
			v3 rayD = ((entropyForD.x * initialTb1) + (entropyForD.y * initialTb2) + (entropyForD.z * initialN));

			KdNode *currentLeaf = initialLeaf;
			v3 attenuation = initialAttenuation;

			for (u32 bounceCount = 0; bounceCount < maxBounceIndex; )
			{
				LightingTriangle **triangles = currentLeaf->triangles;
				AABB aabb = currentLeaf->aabb;

				//todo: save these or just i?
				v3 normal;
				v3 b1;
				v3 color;

				u64 triangleTimer = __rdtsc();
				f32 min = (MAXFLOAT);
				for (u32 i = 0; i < currentLeaf->amountOfTriangles; i++)
				{
					LightingTriangle* t = triangles[i];

					v3 taP = (t->pos);
					v3 taD1 = (t->d1);
					v3 taD2 = (t->d2);
					v3 taN = (t->normal);
					
					//todo :should we precalc these?
					f32 lenD1Sq = Dot(taD1, taD1);
					f32 lenD2Sq = Dot(taD2, taD2);
					f32 d12 = Dot(taD1, taD2);
					f32 denomInv = (1.0f / (lenD1Sq * lenD2Sq - d12 * d12));

					f32 rayPTerm = Dot(taN, rayP);
					f32 taPTerm = Dot(taN, taP);
					f32 directionTerm = Dot(taN, rayD);
					f32 constantTerm = taPTerm - rayPTerm;

					f32 intersectionT = constantTerm / directionTerm;

					bool intersectionGood = (directionTerm < 0.0f) & (intersectionT < min) & (intersectionT >= -0);

					v3 intersection = intersectionT * rayD + rayP;
					v3 relInt = intersection - taP;

					f32 dotV1 = Dot(relInt, taD1);
					f32 dotV2 = Dot(relInt, taD2);
					f32 bary1 = (lenD2Sq * dotV1 - d12 * dotV2) * denomInv; //barycentric coordinates
					f32 bary2 = (lenD1Sq * dotV2 - d12 * dotV1) * denomInv;

					bool hitInTriangleBounds = (0.0f <= bary1) & (0.0f <= bary2) & (bary1 + bary2 <= 1.0f);

					//todo: fix this w/o eps? or precalc?
					bool hitInAABBBounds =
						(aabb.minDim.x <= intersection.x + epsilon) &
						(aabb.minDim.y <= intersection.y + epsilon) &
						(aabb.minDim.z <= intersection.z + epsilon) &
						(aabb.maxDim.x >= intersection.x - epsilon) &
						(aabb.maxDim.y >= intersection.y - epsilon) &
						(aabb.maxDim.z >= intersection.z - epsilon);


					if (intersectionGood && hitInTriangleBounds && hitInAABBBounds)
					{
						v3 taColor = (triangles[i]->color);
						b1 = taD1;
						min = intersectionT;
						normal = taN;
						color = taColor;
					}

				}

				bool hit = (min < MAXF32);
				if (!hit)
				{
					//we are complaring dot products to 0, so we only get the _second_ intersection, so we only need to take the minimal second intersection?
					f32 curIntersectionMin = MAXF32;
					KdNode *nextNode = NULL;

					f32 x = rayP.x;
					f32 dx = rayD.x;
					f32 y = rayP.y;
					f32 dy = rayD.y;
					f32 z = rayP.z;
					f32 dz = rayD.z;

					f32 aabbMinX = aabb.minDim.x;
					f32 aabbMaxX = aabb.maxDim.x;
					f32 aabbMinY = aabb.minDim.y;
					f32 aabbMaxY = aabb.maxDim.y;
					f32 aabbMinZ = aabb.minDim.z;
					f32 aabbMaxZ = aabb.maxDim.z;

					f32 t1x = (aabbMaxX - x) / dx;
					if (dx > 0 && t1x <= curIntersectionMin)
					{
						nextNode = currentLeaf->posXNeighboor;
						curIntersectionMin = t1x;
					}

					f32 t2x = (aabbMinX - x) / dx;
					if (dx < 0 && t2x <= curIntersectionMin)
					{
						nextNode = currentLeaf->negXNeighboor;
						curIntersectionMin = t2x;
					}

					f32 t1y = (aabbMaxY - y) / dy;
					if (dy > 0 && t1y <= curIntersectionMin)
					{
						nextNode = currentLeaf->posYNeighboor;
						curIntersectionMin = t1y;
					}

					f32 t2y = (aabbMinY - y) / dy;
					if (dy < 0 && t2y <= curIntersectionMin)
					{
						nextNode = currentLeaf->negYNeighboor;
						curIntersectionMin = t2y;
					}

					f32 t1z = (aabbMaxZ - z) / dz;
					if (dz > 0 && t1z <= curIntersectionMin)
					{
						nextNode = currentLeaf->posZNeighboor;
						curIntersectionMin = t1z;
					}

					f32 t2z = (aabbMinZ - z) / dz;
					if (dz < 0 && t2z <= curIntersectionMin)
					{
						nextNode = currentLeaf->negZNeighboor;
						curIntersectionMin = t2z;
					}
					v3 curExit = rayD * curIntersectionMin + rayP;

					while (!nextNode->isLeaf)
					{
						v3 planePos = nextNode->planePos;

						if (Dot(nextNode->normal, curExit - planePos) > 0)
						{
							nextNode = nextNode->positive;
						}
						else
						{
							nextNode = nextNode->negative;
						}
					}

					currentLeaf = nextNode;
					continue;

				}
				else
				{
					f32 cosAtten = Dot(-rayD, normal);
					cosAtten = (cosAtten < 0) ? 0 : cosAtten;
					attenuation = cosAtten * attenuation * color;

					rayP = FitV3ToAABB(rayD * min + rayP, sceneAABB);

					v3 entropyForD = FastNormalize(V3(RandomGaussian(entropy), RandomGaussian(entropy), RandomPositiveGaussian(entropy)));

					b1 = FastNormalize(b1);
					v3 b2 = CrossProduct(normal, b1);
					v3 randomBounce = ((entropyForD.x * b1) + (entropyForD.y * b2) + (entropyForD.z * normal));

					rayD = randomBounce;
					if (!bounceCount)
					{
						minHitDist = Min(min, minHitDist);
					}

					bounceCount++;
				}

			}//for bounceCount

			v3 relLightSource = lightSource - rayP;
			float lightSourceNormSq = QuadNorm(relLightSource);
			v3 lightD = FastNormalize(relLightSource);

			//calculating if ray could have come from light source
			while (true)
			{
				LightingTriangle **triangles = currentLeaf->triangles;

				AABB aabb = currentLeaf->aabb;

				f32 min = (MAXFLOAT);

				for (u32 i = 0; i < currentLeaf->amountOfTriangles; i++)
				{
					LightingTriangle *t = triangles[i];

					v3 taP = (t->pos);
					v3 taD1 = (t->d1);
					v3 taD2 = (t->d2);
					v3 taN = (t->normal);
#if 0
					f32 lenD1Sq = (t->lenD1Sq);
					f32 lenD2Sq = (t->lenD2Sq);
					f32 d12 = (t->d12);
					f32 denomInv = (t->denomInv);
#endif
					//todo :should we precalc these?
					f32 lenD1Sq = Dot(taD1, taD1);
					f32 lenD2Sq = Dot(taD2, taD2);
					f32 d12 = Dot(taD1, taD2);
					f32 denomInv = (1.0f / (lenD1Sq * lenD2Sq - d12 * d12));

					f32 rayPTerm = Dot(taN, rayP);
					f32 taPTerm = Dot(taN, taP);
					f32 directionTerm = Dot(taN, lightD);
					f32 constantTerm = taPTerm - rayPTerm;

					f32 intersectionT = constantTerm / directionTerm;
					bool intersectionGood = (directionTerm < 0.0f) & (intersectionT < min) & (intersectionT >= -0);

					v3 intersection = intersectionT * lightD + rayP;
					v3 relInt = intersection - taP;

					f32 dotV1 = Dot(relInt, taD1);
					f32 dotV2 = Dot(relInt, taD2);
					f32 bary1 = (lenD2Sq * dotV1 - d12 * dotV2) * denomInv; //barycentric coordinates
					f32 bary2 = (lenD1Sq * dotV2 - d12 * dotV1) * denomInv;

					bool hitInTriangleBounds = (0.0f <= bary1) & (0.0f <= bary2) & (bary1 + bary2 <= 1.0f);

					//todo: fix this w/o eps?
					bool hitInAABBBounds =
						(aabb.minDim.x <= intersection.x + epsilon) &
						(aabb.minDim.y <= intersection.y + epsilon) &
						(aabb.minDim.z <= intersection.z + epsilon) &
						(aabb.maxDim.x >= intersection.x - epsilon) &
						(aabb.maxDim.y >= intersection.y - epsilon) &
						(aabb.maxDim.z >= intersection.z - epsilon);

					if (intersectionGood & hitInTriangleBounds & hitInAABBBounds)
					{
						min = intersectionT;
					}
				}

				bool hit = (min < F32MAX);

				if (!hit)
				{
					//we are complaring dot products to 0, so we only get the _second_ intersection, so we only need to take the minimal second intersection
					f32 curIntersectionMin = MAXF32;
					KdNode *nextNode = NULL;

					f32 x = rayP.x;
					f32 dx = lightD.x;
					f32 y = rayP.y;
					f32 dy = lightD.y;
					f32 z = rayP.z;
					f32 dz = lightD.z;

					f32 aabbMinX = aabb.minDim.x;
					f32 aabbMaxX = aabb.maxDim.x;
					f32 aabbMinY = aabb.minDim.y;
					f32 aabbMaxY = aabb.maxDim.y;
					f32 aabbMinZ = aabb.minDim.z;
					f32 aabbMaxZ = aabb.maxDim.z;

					f32 t1x = (aabbMaxX - x) / dx;
					if (dx > 0 && t1x <= curIntersectionMin)
					{
						nextNode = currentLeaf->posXNeighboor;
						curIntersectionMin = t1x;
					}

					f32 t2x = (aabbMinX - x) / dx;
					if (dx < 0 && t2x <= curIntersectionMin)
					{
						nextNode = currentLeaf->negXNeighboor;
						curIntersectionMin = t2x;
					}

					f32 t1y = (aabbMaxY - y) / dy;
					if (dy > 0 && t1y <= curIntersectionMin)
					{
						nextNode = currentLeaf->posYNeighboor;
						curIntersectionMin = t1y;
					}

					f32 t2y = (aabbMinY - y) / dy;
					if (dy < 0 && t2y <= curIntersectionMin)
					{
						nextNode = currentLeaf->negYNeighboor;
						curIntersectionMin = t2y;
					}

					f32 t1z = (aabbMaxZ - z) / dz;
					if (dz > 0 && t1z <= curIntersectionMin)
					{
						nextNode = currentLeaf->posZNeighboor;
						curIntersectionMin = t1z;
					}

					f32 t2z = (aabbMinZ - z) / dz;
					if (dz < 0 && t2z <= curIntersectionMin)
					{
						nextNode = currentLeaf->negZNeighboor;
						curIntersectionMin = t2z;
					}
					v3 curExit = lightD * curIntersectionMin + rayP;

					while (!nextNode->isLeaf)
					{
						v3 planePos = nextNode->planePos;

						if (Dot(nextNode->normal, curExit - planePos) > 0)
						{
							nextNode = nextNode->positive;
						}
						else
						{
							nextNode = nextNode->negative;
						}
					}
					currentLeaf = nextNode;
				}
				else
				{
					if (min * min >= lightSourceNormSq - epsilon)
					{
						rayColor += bounceWeight * attenuation;
					}
					break;
				}
			}

		}//for maxBounceIndex

		indirectColor += rayWeight * rayColor;
	} //for rayIndex
	IrradianceSample ret;
	ret.color = indirectColor;
	ret.pos = initialP;
	ret.quadR = Max(0.2f, Min(Square(minHitDist), 5.0f));
	//Min(Square((f32)(((maxBounceCount * (maxBounceCount + 1)) / 2) * rayAmountPerBounceAmount) / minHitDist), 2.0f);

	return ret;
}

static RayCastResult CastRaysCache(Bitmap bitmap, ClipRect clipRect, World world)
{
	u32 rayAmountPerBounceAmount = 1u;
	u32 maxBounceCount = 2u;

	LightingTriangle *triangles = world.lightingTriangles;
	IrradianceCache *cache = world.cache;

	RayCastResult ret = {};

	Camera camera = world.camera;

	KdNode *kdTree = world.kdTree;
	RandomSeries entropy = { RandomSeed() };
	//RandomSeries entropy = { 123 };
	v3 lightSource = world.lightSource;

	f32 aspectRatio = ((float)globalLightingImageWidth / (float)globalLightingImageHeight);

	v3 camP = (camera.pos);
	v3 camB1 = (camera.basis.d1);
	v3 camB2 = (camera.basis.d2);
	v3 camB3 = (camera.basis.d3);

	m4x4 proj = Projection(((float)globalLightingImageWidth / (float)globalLightingImageHeight), 1.0f) * CameraTransform(camera.basis.d1, camera.basis.d2, camera.basis.d3, camera.pos);
	m4x4 inv = InvOrId(proj);
	m4x4 id = proj * inv;
	v3 p1 = inv * V3(-1, -1, -1);
	v3 p2 = inv * V3(1, -1, -1);
	v3 p3 = inv * V3(-1, 1, -1);

	v3 cameraPlaneUL = p1;
	v3 xVec = p2 - p1;
	v3 yVec = p3 - p1;

	f32 rayWeight = 1.0f / (f32)rayAmountPerBounceAmount;
	f32 bounceWeight = 1.0f / (f32)maxBounceCount;

	KdNode *entryLeaf = GetLeaf(kdTree, camP);

	for (u32 h = clipRect.yMin; h < clipRect.yMax; h++)
	{
		u32 *xIt = GetPixel(bitmap, clipRect.xMin, h);

		for (u32 w = clipRect.xMin; w < clipRect.xMax; w++)
		{
			f32 xAdj = (f32)w;// + RandomPercent(&entropy);
			f32 yAdj = (f32)h;// + RandomPercent(&entropy);

			v3 xOff = (xAdj / (f32)globalLightingImageWidth) * xVec;
			v3 yOff = (yAdj / (f32)globalLightingImageHeight) * yVec;

			v3 rayDUN = cameraPlaneUL + xOff + yOff - camP;
			v3 xyD = FastNormalize(rayDUN); //biased?

			u64 timer = __rdtsc();
			TriangleSearchResult firstTResult = GetInitialTriangle(camP, xyD, entryLeaf);
			ret.averageCyclesPerBounce += __rdtsc() - timer;


			LightingTriangle *initialT = firstTResult.initialT;
			v3 initialP = FitV3ToAABB(firstTResult.initialP, kdTree->aabb);
			KdNode *initialLeaf = firstTResult.initialLeaf;

			u32 initialTriangleIndex = GetTriangleIndex(triangles, initialT);
			v3 initialN = initialT->normal;
			v3 initialAttenuation = Max(0.0f, Dot(-xyD, initialN)) * initialT->color;

			v3 directColor = CastRayToLightSource(initialP, lightSource, initialLeaf, bounceWeight, initialAttenuation);
			v3 indirectColor = V3();

			IrradianceSampleArray currentSamples = cache->triangleSamples[initialTriangleIndex];

			u32 entriesFound = 0;
			for (u32 cacheIt = 0; cacheIt < currentSamples.amount; cacheIt++)
			{
				IrradianceSample entry = currentSamples.entries[cacheIt];

				if (QuadDist(entry.pos, initialP) < entry.quadR)
				{
					indirectColor += entry.color;
					entriesFound++;
				}
			}

			if (entriesFound)
			{
				indirectColor /= (f32)entriesFound;

				*xIt++ = Pack3x8(directColor + indirectColor);
				continue;
			}

			f32 minHitDist = F32MAX;

			//calculate Indirect Lighting
			IrradianceSample newEntry = {};
			newEntry = SampleIrradiance(initialP, initialT, initialLeaf, lightSource, initialAttenuation, rayAmountPerBounceAmount, maxBounceCount, &entropy, kdTree->aabb);
			u32 insetedSamples = InsertSample(kdTree, &newEntry, triangles, cache);

			*xIt++ = (Pack3x8(directColor + newEntry.color));
		}
	}

	u32 clipWidth = clipRect.xMax - clipRect.xMin;
	u32 clipHeight = clipRect.yMax - clipRect.yMin;

	ret.averageCyclesPerBounce /= clipWidth*clipHeight;

	return ret;
}

static RayCastResult CastRaysCacheWide(Bitmap bitmap, ClipRect clipRect, World world)
{
	u32 rayAmountPerBounceAmount = 1024u;
	u32 maxBounceCount = 2u;

	IrradianceCache *cache = world.cache;
	LightingTriangle *triangles = world.lightingTriangles;

	RayCastResult ret = {};

	Camera camera = world.camera;

	KdNode *kdTree = world.kdTree;

	//RandomSeries entropy = { 123 };

	u32Wide entropy = Load(RandomSeed(), RandomSeed(), RandomSeed(), RandomSeed());

	v3Wide lightSource = Load(world.lightSource);

	f32Wide aspectRatio = Load((float)globalLightingImageWidth / (float)globalLightingImageHeight);

	v3Wide camP = Load(camera.pos);
	v3Wide camB1 = Load(camera.basis.d1);
	v3Wide camB2 = Load(camera.basis.d2);
	v3Wide camB3 = Load(camera.basis.d3);

	m4x4 proj = Projection(((float)globalLightingImageWidth / (float)globalLightingImageHeight), 1.0f) * CameraTransform(camera.basis.d1, camera.basis.d2, camera.basis.d3, camera.pos);
	m4x4 inv = InvOrId(proj);
	m4x4 id = proj * inv;
	v3 p1 = inv * V3(-1, -1, -1);
	v3 p2 = inv * V3(1, -1, -1);
	v3 p3 = inv * V3(-1, 1, -1);

	v3Wide cameraPlaneUL = Load(p1);
	v3Wide xVec = Load(p2 - p1);
	v3Wide yVec = Load(p3 - p1);

	f32Wide rayWeight = Load(1.0f / (f32)rayAmountPerBounceAmount);
	f32Wide bounceWeight = Load(1.0f / (f32)maxBounceCount);

	KdNode *entryLeaf = GetLeaf(kdTree, camera.pos);
	//todo: what happens if amount t = max t, maybe actually put new entry in every time?
	for (u32 h = clipRect.yMin; h < clipRect.yMax; h += 2)
	{
		for (u32 w = clipRect.xMin; w < clipRect.xMax; w += 2)
		{
			f32Wide wWide = CastToF32(Load(w, (w + 1), w, (w + 1)));
			f32Wide hWide = CastToF32(Load(h, h, (h + 1), (h + 1)));

			f32Wide xAdj = wWide + RandomPercentWide(&entropy);
			f32Wide yAdj = hWide + RandomPercentWide(&entropy);

			v3Wide xOff = (xAdj / (f32)globalLightingImageWidth) * xVec;
			v3Wide yOff = (yAdj / (f32)globalLightingImageHeight) * yVec;

			v3Wide rayDUN = cameraPlaneUL + xOff + yOff - camP;
			v3Wide xyD = FastNormalize(rayDUN); //biased?

			u64 timer = __rdtsc();
			TriangleSearchResultWide firstTResult = GetInitialTriangleWide(camP, xyD, entryLeaf);
			ret.averageCyclesPerBounce += __rdtsc() - timer;

#if 0
			* GetPixel(bitmap, w, h) = Pack3x8(firstTResult.initialT[0]->color);
			*GetPixel(bitmap, w + 1, h) = Pack3x8(firstTResult.initialT[1]->color);
			*GetPixel(bitmap, w, h + 1) = Pack3x8(firstTResult.initialT[2]->color);
			*GetPixel(bitmap, w + 1, h + 1) = Pack3x8(firstTResult.initialT[3]->color);
			continue;
#endif

			LightingTriangle **initialT = firstTResult.initialT;
			v3Wide initialP = FitV3ToAABB(firstTResult.initialP, kdTree->aabb);
			KdNode **initialLeaf = firstTResult.initialLeaf;

			v3Wide initialN = LoadFromStruct(initialT, normal);
			v3Wide initialAttenuation = Max(0.0f, Dot(-xyD, initialN)) * LoadFromStruct(initialT, color);

			v3Wide directColor = CastRayToLightSourceWide(initialP, lightSource, initialLeaf, bounceWeight, initialAttenuation);

#if 0

			u32Wide amountEntries = Load(initialT[0]->amount, initialT[1]->amount, initialT[2]->amount, initialT[3]->amount);

			u32Wide hasEntries = amountEntries > ZeroU32Wide();

			u32 firstZeroLane;

			{
				u32Wide entriesFound = ZeroU32Wide();
				v3Wide sampledColor = ZeroV3Wide();

				u32 cacheIt = 0;
				u32Wide maskCacheIt = Load(cacheIt) < amountEntries;
				while (AnyTrue(maskCacheIt))
				{
					IrradianceSample *entry[4];
					for (u32 laneIndex = 0; laneIndex < 4; laneIndex++)
					{
						entry[laneIndex] = initialT[laneIndex]->entries[Min(cacheIt, initialT[laneIndex]->amount - 1)];
					}

					v3Wide entryPos = LoadFromStruct(entry, pos);
					f32Wide entryR = LoadFromStruct(entry, quadR);
					v3Wide entryColor = LoadFromStruct(entry, color);

					u32Wide assignMask = QuadDist(entryPos, initialP) < entryR;
					{
						ConditionalAssign(&sampledColor, assignMask, sampledColor + entryColor);
						ConditionalAssign(&entriesFound, assignMask, entriesFound + Load(1u));
					}
					cacheIt++;
					maskCacheIt = Load(cacheIt) < amountEntries;
				}


				v3Wide indirectColor = sampledColor;

				u32Wide entriesFoundMask = entriesFound > ZeroU32Wide();
				ConditionalAssign(&indirectColor, entriesFoundMask, indirectColor / CastToF32(entriesFound));

				u32Wide entryMask = entriesFound > ZeroU32Wide();

				if (AllTrue(entryMask))
				{
					*GetPixel(bitmap, w, h) = Pack3x8(Lane(directColor + indirectColor, 0));
					*GetPixel(bitmap, w + 1, h) = Pack3x8(Lane(directColor + indirectColor, 1));
					*GetPixel(bitmap, w, h + 1) = Pack3x8(Lane(directColor + indirectColor, 2));
					*GetPixel(bitmap, w + 1, h + 1) = Pack3x8(Lane(directColor + indirectColor, 3));
					continue;
				}

				firstZeroLane = FirstZeroLane(entryMask);
			}
			else
			{
				firstZeroLane = FirstZeroLane(hasEntries);
			}
#endif

			u32 entriesFoundLane[4] = { 0, 0, 0, 0 };
			v3 sampledColor[4] = { V3(), V3(), V3(), V3() };
			u32 triangleSampleAmountLane[4];
			for (u32 laneIndex = 0; laneIndex < 4; laneIndex++)
			{
				u32 initialTriangleIndex = GetTriangleIndex(triangles, initialT[laneIndex]);
				IrradianceSampleArray currentSamples = cache->triangleSamples[initialTriangleIndex];
				v3 laneP = Lane(initialP, laneIndex);
				triangleSampleAmountLane[laneIndex] = currentSamples.amount;

				for (u32 cacheIt = 0; cacheIt < currentSamples.amount; cacheIt++)
				{
					IrradianceSample entry = currentSamples.entries[cacheIt];

					if (QuadDist(entry.pos, laneP) < entry.quadR)
					{
						sampledColor[laneIndex] += entry.color;
						entriesFoundLane[laneIndex]++;
					}
				}
			}

			v3Wide indirectColor = LoadFromArray(sampledColor);
			u32Wide entriesFound = LoadFromArray(entriesFoundLane);

			u32Wide laneAmount = LoadFromArray(triangleSampleAmountLane);
			u32Wide maxAmount = Load(cache->maxEntriesPerTriangle);

			u32Wide atMaxMask = (laneAmount == maxAmount);

			u32Wide entriesFoundMask = entriesFound > ZeroU32Wide();

			if (AllTrue(entriesFoundMask | atMaxMask))
			{
				ConditionalAssign(&indirectColor, entriesFoundMask, indirectColor / CastToF32(entriesFound));
				*GetPixel(bitmap, w, h) = Pack3x8(Lane(directColor + indirectColor, 0));
				*GetPixel(bitmap, w + 1, h) = Pack3x8(Lane(directColor + indirectColor, 1));
				*GetPixel(bitmap, w, h + 1) = Pack3x8(Lane(directColor + indirectColor, 2));
				*GetPixel(bitmap, w + 1, h + 1) = Pack3x8(Lane(directColor + indirectColor, 3));
				continue;
			}

			u32 firstZeroLane = FirstZeroLane(entriesFoundMask);

			f32 minHitDist = F32MAX;
			v3 newP = Lane(initialP, firstZeroLane);
			LightingTriangle *newT = initialT[firstZeroLane];
			KdNode *newLeaf = initialLeaf[firstZeroLane];

			IrradianceSample newEntry = {};
			newEntry = SampleIrradiance(newP, newT, newLeaf, world.lightSource, Lane(initialAttenuation, firstZeroLane), rayAmountPerBounceAmount, maxBounceCount, (RandomSeries *)&entropy, kdTree->aabb);
			InsertSample(kdTree, &newEntry, triangles, cache);


			*GetPixel(bitmap, w, h) = Pack3x8(Lane(directColor, 0) + newEntry.color);
			*GetPixel(bitmap, w + 1, h) = Pack3x8(Lane(directColor, 1) + newEntry.color);
			*GetPixel(bitmap, w, h + 1) = Pack3x8(Lane(directColor, 2) + newEntry.color);
			*GetPixel(bitmap, w + 1, h + 1) = Pack3x8(Lane(directColor, 3) + newEntry.color);

		}
	}

	u32 clipWidth = clipRect.xMax - clipRect.xMin;
	u32 clipHeight = clipRect.yMax - clipRect.yMin;

	ret.averageCyclesPerBounce /= ((clipWidth*clipHeight) / 4);

	return ret;
}

static void RayCastWorker(void *data)
{
	RayCastWork *work = (RayCastWork *)data;
	CastRaysCache(work->bitmap, work->clipRect, work->world);
}

static void RenderAABBOutline(AABB aabb, RenderGroup *rg)
{
	PushAABBOutLine(rg, aabb.minDim, aabb.maxDim);
}

static void RenderKdTree(KdNode *node, RenderGroup *rg)
{
	if (node->isLeaf)
	{
		RenderAABBOutline(node->aabb, rg);
		return;
	}

	RenderAABBOutline(node->aabb, rg);
	RenderKdTree(node->positive, rg);
	RenderKdTree(node->negative, rg);
}

static v3 zPlusEps(v3 v)
{
	v.z += 0.001f;
	return v;
}


static void InitLighting(World *world)
{
	Assert(!world->lightingTriangles);
	dummyLeaf = PushStruct(constantArena, KdNode);
	dummyLeaf->amountOfTriangles = 1;
	dummyLeaf->triangles = PushStruct(constantArena, LightingTriangle*);
	*dummyLeaf->triangles = PushZeroStruct(constantArena, LightingTriangle);

	//bitmap.height = imageHeight;
	//bitmap.width = imageWidth;
	//bitmap.pixels = image[0];
	globalLightingBitmap = CreateBitmap(image[0], globalLightingImageWidth, globalLightingImageHeight);

	TriangleArray t = world->triangles;
	u32 amountOfTriangles = t.amount;
	world->amountOfTriangles = amountOfTriangles;
	LightingTriangle *trs = PushArray(constantArena, LightingTriangle, amountOfTriangles);
	for (u32 i = 0; i < amountOfTriangles; i++)
	{
		trs[i] = CreateLightingTriangleFromThreePoints(t[i].p1, t[i].p2, t[i].p3, Unpack3x8(t[i].c2), constantArena);
	}
	world->lightingTriangles = trs;
	world->kdTree = BuildKdTree(trs, t.amount, constantArena, workingArena);

	world->cache = PushStruct(constantArena, IrradianceCache);
	world->cache->maxEntriesPerTriangle = 200;
	world->cache->triangleSamples = PushArray(constantArena, IrradianceSampleArray, amountOfTriangles);
	for (u32 i = 0; i < amountOfTriangles; i++)
	{
		world->cache->triangleSamples[i].entries = PushArray(constantArena, IrradianceSample, world->cache->maxEntriesPerTriangle);
		world->cache->triangleSamples[i].amount = 0;
	}
}

static bool lightingInitialized = false;

static void PushLightingImage(RenderGroup *rg)
{
	PushRenderSetUp(rg, {}, V3(), Setup_Orthogonal);
	UpdateGPUTexture(globalLightingBitmap);
	PushBitmap(rg, V2(), globalLightingBitmap);
}


static void PushLightingSolution(RenderGroup *rg, World *world)
{
	if (!lightingInitialized)
	{
		InitLighting(world);
		lightingInitialized = true;
	}
	ClipRect wholeScreen;
	wholeScreen.xMin = 0;
	wholeScreen.yMin = 0;
	wholeScreen.xMax = globalLightingImageWidth;
	wholeScreen.yMax = globalLightingImageHeight;

	CastRaysCache(globalLightingBitmap, wholeScreen, *world);
	//CastRaysCacheWide(bitmap, wholeScreen, *world);

	PushRenderSetUp(rg, {}, V3(), Setup_Orthogonal);
	UpdateGPUTexture(globalLightingBitmap);
	PushBitmap(rg, V2(), globalLightingBitmap);
}

#if 0
void LightingMain(RenderCommands *renderComands, WorkHandler *workHandler, Input *input)
{
#if 1
	LightingTriangle t1 = Create3PointTriangle(V3(1, 0, 0), V3(0, 1, 0), V3(1, 1, 0));
	LightingTriangle t2 = Create3PointTriangle(V3(1, 0, 0), V3(0, 0, 1), V3(1.5f, 0, 0));
	LightingTriangle t3 = Create3PointTriangle(V3(100, 0, 0), V3(0, 100, 0), V3(100, 100, 0));

	IrradianceSample s1 = CreateIrradianceSample(V3(), V3(), 1.0f);
	IrradianceSample s2 = CreateIrradianceSample(V3(), V3(), 0.1f);
	IrradianceSample s3 = CreateIrradianceSample(V3(10, 10, 10), V3(), 5.0f);

	Assert(TriangleIntersectsSample(&t1, &s1));
	Assert(!TriangleIntersectsSample(&t1, &s2));
	Assert(!TriangleIntersectsSample(&t1, &s3));
	Assert(TriangleIntersectsSample(&t2, &s1));
	Assert(!TriangleIntersectsSample(&t2, &s2));
	Assert(!TriangleIntersectsSample(&t2, &s3));
	Assert(!TriangleIntersectsSample(&t3, &s1));
	Assert(!TriangleIntersectsSample(&t3, &s2));
	Assert(!TriangleIntersectsSample(&t3, &s3));

	AABB a1 = { V3(0.5f, 0, 0), V3(1, 1, 1) };
	AABB a2 = { V3(-1, 0, 0), V3(8.5f, 8.5f, 8.5f) };
	AABB a3 = { V3(-1, -1, -1), V3(1, 1, 1) };

	Assert(SampleIntersectsAABB(&s1, a1));
	Assert(SampleIntersectsAABB(&s1, a2));
	Assert(SampleIntersectsAABB(&s1, a3));
	Assert(!SampleIntersectsAABB(&s2, a1));
	Assert(SampleIntersectsAABB(&s2, a2));
	Assert(SampleIntersectsAABB(&s2, a3));
	Assert(!SampleIntersectsAABB(&s3, a1));
	Assert(SampleIntersectsAABB(&s3, a2));
	Assert(!SampleIntersectsAABB(&s3, a3));
#endif

	u32Wide u = Load(1u) > Load(0u);
	int sada = _mm_movemask_epi8(u);

	u32 sadasd = FirstZeroLane(u);


	static bool initialized = false;

	v3 focusPoint = V3(0, 5, 5);

	if (!initialized)
	{
		assetHandl = new AssetHandler();
		assetHandl->LoadAsset({ Asset_Font });
		globalWorld.camera.pos = V3(0.0f, 0.0f, -12.0f);
		//camera.pos += 3 * camera.basis.d3;
		globalWorld.camera.basis = { V3(1, 0, 0), V3(0, 1, 0.0f), V3(0, 0.0f, 1) };

		float aspectRatio = (float)imageWidth / (float)imageHeight;
		Vector3Basis bottomBasis = { V3(21, 0, 0), V3(0, 21, 0), V3(0, 0, 1) };

		v3 red = V3(1.0f, 0.0f, 0.0f);
		v3 green = V3(0.0f, 1.0f, 0.0f);
		v3 blue = V3(0.0f, 0.0f, 1.0f);
		v3 white = V3(1.0f, 1.0f, 1.0f);
		v3 mellowYellow = V3(1.0f, 1.0f, 0.2f);

		const u32 meshSize = 20;

		RandomSeries series = { RandomSeed() };

		v3 groundMesh[meshSize][meshSize];

		for (u32 x = 0; x < meshSize; x++)
		{
			for (u32 y = 0; y < meshSize; y++)
			{
				f32 zlevel = (x > 0) ? (groundMesh[x - 1][y].z) : 0.0f;

				if (y > 0)
				{
					zlevel += groundMesh[x][y - 1].z;
					zlevel /= 2.0f;
				}

				f32 xEntropy = 0.15f * RandomSignedPercent(&series);
				f32 yEntropy = 0.15f * RandomSignedPercent(&series);
				f32 zEntropy = -0.3f * RandomPercent(&series);

				groundMesh[x][y] = 0.5f * V3(xEntropy + (f32)x - 10.0f, yEntropy + (f32)y - 10.0f, zlevel + zEntropy);
			}
		}

		u32 amountOfTriangles = 1;
		amountOfTriangles += 12;
		amountOfTriangles += 4 * (meshSize - 2) * (meshSize - 2);

		LightingTriangle *trianglesIt = PushArray(constantArena, LightingTriangle, amountOfTriangles);
		LightingTriangle *triangles = trianglesIt;

		for (u32 y = 1; y < (meshSize - 1); y++)
		{
			for (u32 x = 1; x < (meshSize - 1); x++)
			{
				*trianglesIt++ = CreateLightingTriangleFromThreePoints(groundMesh[x][y], groundMesh[x][y + 1], groundMesh[x + 1][y], red, constantArena);
				*trianglesIt++ = CreateLightingTriangleFromThreePoints(groundMesh[x][y], groundMesh[x + 1][y], groundMesh[x + 1][y - 1], red, constantArena);

				*trianglesIt++ = CreateLightingTriangleFromThreePoints(zPlusEps(groundMesh[x][y + 1]), zPlusEps(groundMesh[x][y]), zPlusEps(groundMesh[x + 1][y]), red, constantArena);
				*trianglesIt++ = CreateLightingTriangleFromThreePoints(zPlusEps(groundMesh[x + 1][y]), zPlusEps(groundMesh[x][y]), zPlusEps(groundMesh[x + 1][y - 1]), red, constantArena);

				//*trianglesIt++ = Create3PointTriangleWithCache(Zinversion(groundMesh[x][y + 1]), Zinversion(groundMesh[x][y]), Zinversion(groundMesh[x + 1][y]), red,  globalArena);
				//*trianglesIt++ = Create3PointTriangleWithCache(Zinversion(groundMesh[x + 1][y]), Zinversion(groundMesh[x][y]),  Zinversion(groundMesh[x + 1][y - 1]), red, globalArena);
			}
		}

		*trianglesIt++ = CreateLightingTriangleFromThreePoints(V3(-19, -19, -19), V3(-20, -19, -19), V3(-20, -20, -20), white, constantArena);
		v3 lightBlue = Unpack3x8(0x87CEEB);

		f32 boxRadius = 40.0f;

		v3 sb1 = V3(-boxRadius, -boxRadius, -boxRadius);
		v3 sb2 = V3(-boxRadius, boxRadius, boxRadius);
		v3 sb3 = V3(-boxRadius, boxRadius, -boxRadius);
		v3 sb4 = V3(-boxRadius, -boxRadius, boxRadius);
		v3 sb5 = V3(boxRadius, -boxRadius, -boxRadius);
		v3 sb6 = V3(boxRadius, boxRadius, boxRadius);
		v3 sb7 = V3(boxRadius, boxRadius, -boxRadius);
		v3 sb8 = V3(boxRadius, -boxRadius, boxRadius);

		*trianglesIt++ = CreateLightingTriangleFromThreePoints(sb1, sb2, sb3, V3(1, 0, 0), lightBlue, constantArena);
		*trianglesIt++ = CreateLightingTriangleFromThreePoints(sb1, sb2, sb4, V3(1, 0, 0), lightBlue, constantArena);

		*trianglesIt++ = CreateLightingTriangleFromThreePoints(sb5, sb6, sb7, V3(-1, 0, 0), lightBlue, constantArena);
		*trianglesIt++ = CreateLightingTriangleFromThreePoints(sb5, sb6, sb8, V3(-1, 0, 0), lightBlue, constantArena);

		*trianglesIt++ = CreateLightingTriangleFromThreePoints(sb1, sb8, sb5, V3(0, 1, 0), lightBlue, constantArena);
		*trianglesIt++ = CreateLightingTriangleFromThreePoints(sb1, sb8, sb4, V3(0, 1, 0), lightBlue, constantArena);

		*trianglesIt++ = CreateLightingTriangleFromThreePoints(sb3, sb6, sb7, V3(0, -1, 0), lightBlue, constantArena);
		*trianglesIt++ = CreateLightingTriangleFromThreePoints(sb3, sb6, sb2, V3(0, -1, 0), lightBlue, constantArena);

		*trianglesIt++ = CreateLightingTriangleFromThreePoints(sb1, sb7, sb3, V3(0, 0, 1), lightBlue, constantArena);
		*trianglesIt++ = CreateLightingTriangleFromThreePoints(sb1, sb7, sb5, V3(0, 0, 1), lightBlue, constantArena);

		*trianglesIt++ = CreateLightingTriangleFromThreePoints(sb4, sb6, sb2, V3(0, 0, -1), lightBlue, constantArena);
		*trianglesIt++ = CreateLightingTriangleFromThreePoints(sb4, sb6, sb8, V3(0, 0, -1), lightBlue, constantArena);

		globalWorld.kdTree = BuildKdTree(triangles, amountOfTriangles, constantArena, frameArena);
		globalWorld.lightingTriangles = triangles;
		globalWorld.amountOfTriangles = amountOfTriangles;
		globalWorld.lightSource = V3(-0, 0, -10);

		u32 irrandianceCacheSize = 20000;
		globalWorld.cache = PushStruct(constantArena, IrradianceCache);
		IrradianceSample *mem = PushArray(constantArena, IrradianceSample, irrandianceCacheSize);
		globalWorld.cache->irradianceArena = InitArena(mem, sizeof(IrradianceSample) * irrandianceCacheSize);
		globalWorld.cache->entries = (IrradianceSample *)globalWorld.cache->irradianceArena->base;



	}
	//*globalWorld.triangles = Create3PointTriangle(V3(), V3(1.0f, 1.0f, 0.0f), V3(1.0f, 0.0f, 0.0f), CreateMaterial(V3(1.0f, 0.5f, 0.2f), 0.0f, 0.8f));

	RenderGroup rg = InitRenderGroup(assetHandl, renderComands);

#if 0
	UpdateCamFocus(input, V3(), &globalWorld.camera);
#else
	UpdateCamGodMode(input, &globalWorld.camera);
#endif

	if (!bitmap.pixels)
	{
		bitmap = CreateBitmap(&(image[0][0]), imageWidth, imageHeight);
	}

	u32 coreCount = 8;
	u32 clipWidth = imageWidth / coreCount;
	u32 clipHeight = clipWidth;
	clipWidth = clipHeight = 64;
	u32 clipAmountX = (imageWidth + clipWidth - 1) / clipWidth;
	u32 clipAmountY = (imageHeight + clipHeight - 1) / clipHeight;

	u32 clipAmount = clipAmountX * clipAmountY;
	Assert(clipAmount <= WorkQueueSize);
	u32 neededArenaCapacity = clipAmount * sizeof(RayCastWork);

	v4 clearColor = V4(1.0f, 0.1f, 0.1f, 0.1f);

	ClearPushBuffer(&rg);
	PushClear(&rg, clearColor);

	static bool debugViewSwitch = false;

	if (input->keybord->y.pressedThisFrame)
	{
		if (!debugViewSwitch)
		{
			iteration = 0;
		}
		debugViewSwitch = !debugViewSwitch;
	}

	if (input->keybord->h.pressedThisFrame)
	{
		RandomSeries series = { RandomSeed() };
		LightingTriangle *initialTriangle = globalWorld.lightingTriangles + 20;
		v3 initialN = initialTriangle->normal;
		v3 initialTb1 = Normalize(initialTriangle->d1);
		v3 initialTb2 = CrossProduct(initialN, initialTb1);

#if 0
		const u32 histogramWidth = imageWidth / 2;
		u32 sampleCounted[histogramWidth] = { 0 };

		u32 histogramX = imageWidth / 4;

		for (u32 i = 0; i < 40000; i++)
		{
			f32 sample = RandomGaussian(&series);
			if (-2.0f <= sample && sample <= 2.0f)
			{
				u32 samplePos = (u32)((f32)histogramWidth * (sample + 3.0f) / 5.0f);
				*GetPixel(bitmap, histogramX + samplePos, sampleCounted[samplePos]++) = 0xFFFFFFFF;
			}
		}
		f32 testsdasd[100];
		for (u32 i = 0; i < ArrayCount(testsdasd); i++)
		{
			testsdasd[i] = RandomGaussian(&series, 1.0f);
		}
#endif
	}


	RayCastResult result = {};
	if (debugViewSwitch)
	{
#if 0
		for (u32 x = 0; x < clipAmountX; x++)
		{
			for (u32 y = 0; y < clipAmountY; y++)
			{
				RayCastWork *rayWork = PushStruct(transientArena, RayCastWork);

				ClipRect rect;
				rect.xMin = x * clipWidth;
				rect.yMin = y * clipHeight;

				rect.xMax = Min((x + 1)*clipWidth, imageWidth);
				rect.yMax = Min((y + 1)*clipHeight, imageHeight);

				rayWork->clipRect = rect;
				rayWork->bitmap = bitmap;
				rayWork->world = globalWorld;

				Work work;
				work.callback = RayCastWorker;
				work.data = rayWork;

				workHandler->PushBack(work);
			}
		}
		workHandler->DoAllWork();

		Clear(transientArena);
#else
		ClipRect wholeScreen;
		wholeScreen.xMin = 0;
		wholeScreen.yMin = 0;
		wholeScreen.xMax = imageWidth;
		wholeScreen.yMax = imageHeight;

		RandomSeries series = { RandomSeed() };

		//result = CastRaysCache(bitmap, wholeScreen, globalWorld);		
		result = CastRaysCacheWide(bitmap, wholeScreen, globalWorld);
#endif

		PushOrthogonalTransform(&rg);
		PushClear(&rg, clearColor);
		PushUpdateBitmap(&rg, bitmap);
		PushBitmap(&rg, &bitmap, V3());
		iteration++;
	}
	else
	{
		PushProjectivTransform(&rg, globalWorld.camera.pos, globalWorld.camera.basis);

		LightingTriangle *wTri = globalWorld.lightingTriangles;
		u32 wTriAmount = globalWorld.amountOfTriangles;

		PushCuboid(&rg, globalWorld.lightSource, 0.7f * v3StdBasis, V4(1, 1, 1, 1));

		if (!initialized)
		{
			for (u32 triIndex = 0; triIndex < wTriAmount; triIndex++)
			{
				PushTriangle(&rg, wTri[triIndex].pos, wTri[triIndex].pos + wTri[triIndex].d1, wTri[triIndex].pos + wTri[triIndex].d2, V4(1.0f, wTri[triIndex].color));
				v3 midPoint = wTri[triIndex].pos + (wTri[triIndex].d1 + wTri[triIndex].d2) / 3.0f;
				//rg.PushLine(midPoint, midPoint + wTri[triIndex].normal);
				//rg.PushLine(midPoint, globalWorld.lightSource);			
			}

		}
		else
		{
			//rg.currentTriangles->vertexCount = 3 * wTriAmount + 3* 12;
		}
		//RenderKdTree(globalWorld.kdTree, &rg);

		for (u32 irrI = 0; irrI < (globalWorld.cache->irradianceArena->current - globalWorld.cache->irradianceArena->base) / sizeof(IrradianceSample); irrI++)
		{
			PushCuboid(&rg, globalWorld.cache->entries[irrI].pos - V3(0.05f, 0.05f, 0.05f), 0.1f * v3StdBasis, V4(1.0f, 1.0f, 1.0f, 1.0f));
		}

	}

	PushOrthogonalTransform(&rg);
	PushRectangle(&rg, V2(), (f32)imageWidth, imageHeight / 20, V4(1.0f, 0.0f, 0.0f, 0.0f));
	PushDebugString(&rg, V3(imageWidth - 3 * (imageHeight / 20), 0, 0), CreateString(frameArena, iteration), imageHeight / 20);
	//rg.PushDebugString(V3(imageWidth - 10* (imageHeight / 20), 0, 0), String("Ttime: ") + String((f32)result.averageCyclesPerTriangleIntersection), imageHeight / 60);
	PushDebugString(&rg, V3(imageWidth - 15 * (imageHeight / 20), 0, 0), Append(frameArena, CreateString("Btime: "), CreateString(frameArena, (f32)result.averageCyclesPerBounce)), imageHeight / 60);
	//rg.PushDebugString(V3(imageWidth - 20 * (imageHeight / 20), 0, 0), String("Rtime: ") + String((f32)result.averageCyclesPerRayCast), imageHeight / 60);
	initialized = true;
}
#endif

#endif // !RR_Lighting

