#ifndef RR_KDTREE
#define RR_KDTREE

#define MAX_KD_TREE_DEPTH 20

struct IrradianceSample
{
	v3 pos;
	f32 quadR;
	v3 color;
	//u32 triangleIndex;
};

struct IrradianceSampleArray
{
	IrradianceSample *entries; //pointer type of hardcopy every time?
	u32 amount;
};

struct IrradianceCache
{
	IrradianceSampleArray *triangleSamples; 
	u32 maxEntriesPerTriangle;
};

static IrradianceSample CreateIrradianceSample(v3 pos, v3 color, f32 r)
{
	IrradianceSample ret;
	ret.color = color;
	ret.pos = pos;
	ret.quadR = r*r;
	return ret;
}

struct LightingTriangle
{
	v3 pos;
	v3 d1;
	v3 d2;
	v3 normal;
	v3 color;
	u32 textureId;
};

DefineArray(LightingTriangle);

//save planePos?
struct KdNode
{
	bool isLeaf;
	AABB aabb;
   
	union
	{
		struct //8 32-bit
		{
			LightingTriangle** triangles; 
			u32 amountOfTriangles;
         
			union {
            
				KdNode *neighboors[6];
            
				struct
				{
					KdNode *posXNeighboor; 
					KdNode *negXNeighboor;
					KdNode *posYNeighboor;
					KdNode *negYNeighboor;
					KdNode *posZNeighboor;
					KdNode *negZNeighboor;
				};
			};
		};
		struct // 8 32-bit
		{
			v3 planePos;
			v3 normal;
         
			KdNode *positive;
			KdNode *negative;
		};
	};
};

static u32 debugTriangleIndex = 0;
static LightingTriangle CreateLightingTriangleFromThreePoints(v3 p1, v3 p2, v3 p3, v3 color)
{
	LightingTriangle ret;
	ret.pos = p1;
	ret.d1 = (p2 - p1);
	ret.d2 = (p3 - p1);
	ret.normal = Normalize(CrossProduct(ret.d1, ret.d2));
	//ret.lenD1Sq = Dot(ret.d1, ret.d1);
	//ret.lenD2Sq = Dot(ret.d2, ret.d2);
	//ret.d12 = Dot(ret.d1, ret.d2);
	//ret.denomInv = 1.0f / (ret.lenD1Sq * ret.lenD2Sq - ret.d12 * ret.d12);	
	//ret.index = debugTriangleIndex++;
   
	ret.color = color;
	
	return ret;
}

static LightingTriangle Create3PointTriangle(v3 p1, v3 p2, v3 p3)
{
	LightingTriangle ret;
	ret.pos = p1;
	ret.d1 = (p2 - p1);
	ret.d2 = (p3 - p1);
	ret.normal = Normalize(CrossProduct(ret.d1, ret.d2));
	//ret.lenD1Sq = Dot(ret.d1, ret.d1);
	//ret.lenD2Sq = Dot(ret.d2, ret.d2);
	//ret.d12 = Dot(ret.d1, ret.d2);
	//ret.denomInv = 1.0f / (ret.lenD1Sq * ret.lenD2Sq - ret.d12 * ret.d12);
	return ret;
}

static bool operator==(Interval a, Interval b)
{
	return (a.min == b.min) && (a.max == b.max);
}

inline KdNode *GetNextNode(KdNode *node, v3 pos)
{
	KdNode *ret = node;
   
	v3 planePos = ret->planePos;
	if (Dot(ret->normal, pos - planePos) > 0)
	{
		ret = ret->positive;
	}
	else
	{
		ret = ret->negative;
	}
	return ret;
}


static KdNode *GetLeaf(KdNode *tree, v3 pos)
{
   
	KdNode *ret = tree;
	while (!ret->isLeaf)
	{
		v3 planePos = ret->planePos;
      
		if (Dot(ret->normal, pos - planePos) > 0)
		{
			ret = ret->positive;
		}
		else
		{
			ret = ret->negative;
		}
	}
	return ret;
}

static Interval ProjectTriangle(v3 normal, LightingTriangle *triangle)
{
	Interval ret = InvertedInfinityInterval();
	v3 trianglePoints[3] =
	{
		triangle->pos,
		triangle->pos + triangle->d1,
		triangle->pos + triangle->d2,
	};
   
	for (u32 i = 0; i < 3; i++)
	{
		f32 val = Dot(normal, trianglePoints[i]);
		ret.min = Min(ret.min, val);
		ret.max = Max(ret.max, val);
	}
	return ret;
}
static Interval ProjectAABB(v3 normal, AABB aabb)
{
	Interval ret = InvertedInfinityInterval();
   
	v3 d1 = V3(aabb.maxDim.x - aabb.minDim.x, 0, 0);
	v3 d2 = V3(0, aabb.maxDim.y - aabb.minDim.y, 0);
	v3 d3 = V3(0, 0, aabb.maxDim.z - aabb.minDim.z);
   
	v3 boxPoints[8] =
	{
		aabb.minDim,
      
		aabb.minDim + d1,
		aabb.minDim + d2,
		aabb.minDim + d3,
      
		aabb.minDim + d1 + d2,
		aabb.minDim + d2 + d3,
		aabb.minDim + d3 + d1,
      
		aabb.maxDim,
      
	};
   
	for (u32 i = 0; i < 8; i++)
	{
		f32 val = Dot(normal, boxPoints[i]);
		ret.min = Min(ret.min, val);
		ret.max = Max(ret.max, val);
	}
	return ret;
}

static bool Intersect(AABB aabb, LightingTriangle *triangle)
{
	v3 boxNormals[3] =
	{
		V3(1, 0, 0),
		V3(0, 1, 0),
		V3(0, 0, 1),
	};
   
	for (u32 i = 0; i < 3; i++)
	{
		Interval triangleProjection = ProjectTriangle(boxNormals[i], triangle);
		Interval aabbProjection = ProjectAABB(boxNormals[i], aabb);
      
		if (aabbProjection.max < triangleProjection.min || triangleProjection.max < aabbProjection.min)
		{
			return false;
		}
	}
   
	v3 triangleNormal = triangle->normal;
   
	{
		Interval triangleProjection = ProjectTriangle(triangleNormal, triangle);
		Interval aabbProjection = ProjectAABB(triangleNormal, aabb);
      
		if (aabbProjection.max < triangleProjection.min || triangleProjection.max < aabbProjection.min)
		{
			return false;
		}
	}
   
	v3 triangleEdges[3] =
	{
		triangle->d1,
		triangle->d2,
		triangle->pos + triangle->d1  - triangle->d2,
	};
	for (u32 i = 0; i < 3; i++)
	{
		for (u32 j = 0; j < 3; j++)
		{
			v3 toSearchPlaneNormal = CrossProduct(triangleEdges[i], boxNormals[j]);
			Interval triangleProjection = ProjectTriangle(triangleNormal, triangle);
			Interval aabbProjection = ProjectAABB(triangleNormal, aabb);
         
			if (aabbProjection.max < triangleProjection.min || triangleProjection.max < aabbProjection.min)
			{
				return false;
			}
		}
	}
   
	return true;
}

static u32 GetAllLeafs(KdNode *root, Arena *arena)
{
	if (root->isLeaf)
	{
		KdNode **leafPtr = PushStruct(arena, KdNode*);
		*leafPtr = root;
		return 1u;
	}
   
	return GetAllLeafs(root->positive, arena) + GetAllLeafs(root->negative, arena);
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

static void BuildNeighboorLinksForLeaf(KdNode *root, KdNode *leaf)
{
	Assert(leaf->isLeaf);
	if (root->isLeaf)
	{
		for (u32 i = 0; i < 6; i++)
		{
			root->neighboors[i] = NULL;
		}
		return;
	}
   
	KdNode *neigboors[6] =
	{
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
	};	
   
   v3 leafP = 0.5f * (leaf->aabb.maxDim + leaf->aabb.minDim);
   
   { // move down
      KdNode *it = root;
      while (!it->isLeaf)
      {
         KdNode *next = GetNextNode(it, leafP);
         
         if (next == it->positive)
         {
            u32 normalIndex = GetCubeNormalIndex(-it->normal);
            neigboors[normalIndex] = it->negative;
         }
         else if (next == it->negative)
         {
            u32 normalIndex = GetCubeNormalIndex(it->normal);
            neigboors[normalIndex] = it->positive;
         }
         else
         {
            Assert(!"invalideCodePath");
         }
         it = next;
      }
      
      Assert(it == leaf);
   }
   
	for (u32 i = 0; i < 6; i++)
	{
		KdNode *toShrink = neigboors[i];
      
		if (toShrink)
		{
         
			v3 normal = GetCubeNormalFromIndex(i);
         
			if (normal.x)
			{
				KdNode *it = toShrink;
				KdNode *save = toShrink;
            
            
				while (!it->isLeaf &&
                   it->aabb.maxDim.y >= leaf->aabb.maxDim.y &&
                   it->aabb.maxDim.z >= leaf->aabb.maxDim.z &&
                   it->aabb.minDim.y <= leaf->aabb.minDim.y &&
                   it->aabb.minDim.z <= leaf->aabb.minDim.z
                   )
				{
					save = it;
					it = GetNextNode(it, leafP);
				}
				neigboors[i] = save;
			}
			if (normal.y)
			{
				KdNode *it = toShrink;
				KdNode *save = toShrink;
            
            
				while (!it->isLeaf &&
                   it->aabb.maxDim.x >= leaf->aabb.maxDim.x &&
                   it->aabb.maxDim.z >= leaf->aabb.maxDim.z &&
                   it->aabb.minDim.x <= leaf->aabb.minDim.x &&
                   it->aabb.minDim.z <= leaf->aabb.minDim.z
                   )
				{
					save = it;
					it = GetNextNode(it, leafP);
				}
				neigboors[i] = save;
            
			}
			if (normal.z)
			{
				KdNode *it = toShrink;
				KdNode *save = toShrink;
            
            
				while (!it->isLeaf &&
                   it->aabb.maxDim.x >= leaf->aabb.maxDim.x &&
                   it->aabb.maxDim.y >= leaf->aabb.maxDim.y &&
                   it->aabb.minDim.x <= leaf->aabb.minDim.x &&
                   it->aabb.minDim.y <= leaf->aabb.minDim.y
                   )
				{
					save = it;
					it = GetNextNode(it, leafP);
				}
				neigboors[i] = save;
			}
		}
	}
	for (u32 i = 0; i < 6; i++)
	{
		leaf->neighboors[i] = neigboors[i];
	}
}

static void BuildNeighboorLinks(KdNode *root, Arena *tempArena)
{
	Clear(tempArena);
	KdNode **leafIt = (KdNode **)tempArena->current;
   
	u32 amountOfLeafs = GetAllLeafs(root, tempArena);
   
	for (u32 i = 0; i< amountOfLeafs; i++)
	{
		BuildNeighboorLinksForLeaf(root, leafIt[i]);
	}
	Clear(tempArena);
}

static KdNode *BuildKdNode(LightingTriangle **triangles, u32 triangleAmount, Arena *arena, Arena *transientArena, u32 depth, KdNode *parent, AABB aabb)
{
	KdNode *node = PushStruct(arena, KdNode);
   
   // parent is unreferanced ????
   
	if (triangleAmount <= 8 || depth > MAX_KD_TREE_DEPTH)
	{
		node->aabb = aabb;
		node->isLeaf = true;
		if (triangleAmount)
		{
			node->amountOfTriangles = triangleAmount;
			node->triangles = PushZeroData(arena, LightingTriangle *, triangleAmount);
			for (u32 i = 0; i< triangleAmount; i++)
			{
				node->triangles[i] = triangles[i];
			}
			return node;
		}
		else
		{
			node->amountOfTriangles = 0;
			node->triangles = NULL;
			return node;
		}		
	}
   
	node->normal = ((v3 *)(&v3StdBasis))[depth % 3];	
	node->aabb = aabb;
	node->isLeaf = false;
   
	//todo here sah construction
	node->planePos = node->aabb.minDim + 0.5f * ((node->aabb.maxDim - node->aabb.minDim) * (node->normal * node->normal)); 
   
	AABB negativeBound = { aabb.minDim, aabb.maxDim - 0.5f * Dot(aabb.maxDim - aabb.minDim, node->normal) * node->normal };
	AABB positiveBound = { aabb.minDim + 0.5f * Dot(aabb.maxDim - aabb.minDim, node->normal) * node->normal, aabb.maxDim };
   
	u32 negativeAmount = 0;
	u32 positiveAmount = 0;
   
	u8 *saveArena = transientArena->current;
   
	LightingTriangle** negativeTriangle = PushData(transientArena, LightingTriangle*, triangleAmount);
	LightingTriangle** positiveTriangle = PushData(transientArena, LightingTriangle*, triangleAmount);
	for (u32 i = 0; i < triangleAmount; i++)
	{
		if (Intersect(negativeBound, triangles[i]))
		{
			negativeTriangle[negativeAmount++] = triangles[i];
		}
		if (Intersect(positiveBound, triangles[i]))
		{
			positiveTriangle[positiveAmount++] = triangles[i];
		}
	}
   
	node->positive = BuildKdNode(positiveTriangle, positiveAmount, arena, transientArena, depth + 1, node, positiveBound);
	node->negative = BuildKdNode(negativeTriangle, negativeAmount, arena, transientArena, depth + 1, node, negativeBound);
	
	transientArena->current = saveArena;
   
	return node;
}

static AABB GetEnclosingAABB(LightingTriangle *triangles, u32 triangleAmount)
{
	AABB ret = InvertedInfinityAABB();
   
	for (u32 i = 0; i < triangleAmount; i++)
	{
		LightingTriangle t = triangles[i];
		v3 points[3] =
		{
			t.pos,
			t.pos + t.d1,
			t.pos + t.d2,
		};
		for (u32 j = 0; j < 3; j++)
		{
			for (u32 k = 0; k < 3; k++)
			{
				ret.maxDim.v[k] = Max(ret.maxDim.v[k], points[j].v[k]);
				ret.minDim.v[k] = Min(ret.minDim.v[k], points[j].v[k]);
			}
		}
	}
	return ret;
}

static KdNode *BuildKdTree(LightingTriangle *triangles, u32 triangleAmount, Arena *arena, Arena *tempArena)
{
	KdNode *ret;
	Clear(tempArena);
	AABB tAABB = GetEnclosingAABB(triangles, triangleAmount);
	LightingTriangle **trianglePointers = PushData(tempArena, LightingTriangle *, triangleAmount);
	for (u32 i = 0; i< triangleAmount; i++)
	{
		trianglePointers[i] = triangles + i;
	}
	ret = BuildKdNode(trianglePointers, triangleAmount, arena, tempArena, 0, NULL, tAABB);
	Clear(tempArena);
   
	BuildNeighboorLinks(ret, tempArena);
   
	return ret;
}

#endif // !RR_KdTree

