#ifndef RR_WORLD
#define RR_WORLD



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

DefineArray(Triangle);

struct VertexFormat
{
	v3 p;
	v2 uv;
	v3 n;
	u32 c;
};

enum TriangleMeshType
{
	TriangleMeshType_List,
	TrianlgeMeshType_Strip,
};

struct Material
{
	f32 spectularExponent;		// Ns
	v3 ambientColor;			// Ka
	v3 diffuseColor;			// Kd
	v3 specularColor;			// Ks
	v3 ke;						// not in the spec and always (0,0,0)

	f32 indexOfReflection;		// Ni ("optical density")	0.001 - 10
	f32 dissolved;				// d						0 - 1	
								//v3 transmissionFilter;		// Tf						
	u32 illuminationModel;		// illum					0 - 10
	u32 bitmapID;
	String name;
};

DefineDynamicArray(Material);

struct IndexSet
{
	u32 amount;
	u32 offset;

	Material mat; 
};

DefineArray(IndexSet);
DefineArray(VertexFormat);
DefineDynamicArray(IndexSet);

struct TriangleMesh
{
	u32 type;
	VertexFormatArray vertices;
	u16Array indices;
	IndexSetArray indexSets;
	
	u32 vertexVBO; // to init this call glBufferData and glBindData
	u32 indexVBO;
};

DefineArray(TriangleMesh);
DefineDynamicArray(TriangleMesh);

void RegisterTriangleMesh(TriangleMesh *mesh);

static void WriteTriangleMesh(TriangleMesh mesh, char *fileName) // todo : when we feel bored, we can make this a bit mor efficient
{
	File file;
	// begin of daisy chain
	file.memory = PushData(frameArena, u8, 0);
	*PushStruct(frameArena, u32) = mesh.type;
	*PushStruct(frameArena, u32) = mesh.vertices.amount;
	For(mesh.vertices)
	{
		/*
		*PushStruct(frameArena, v3) = it->p;
		*PushStruct(frameArena, v2) = it->uv;
		*PushStruct(frameArena, v3) = it->n;
		*PushStruct(frameArena, u32) = it->c;
		*/
		*PushStruct(frameArena, VertexFormat) = *it;
	}

	*PushStruct(frameArena, u32) = mesh.indices.amount;
	For(mesh.indices)
	{
		*PushStruct(frameArena, u16) = *it;
	}

	*PushStruct(frameArena, u32) = mesh.indexSets.amount;
	For(mesh.indexSets)
	{
		*PushStruct(frameArena, IndexSet) = *it;
#if 0
		*PushStruct(frameArena, u32) = it->offset;
		Material mat = it->mat;
		*PushStruct(frameArena, f32) = mat.spectularExponent;

		*PushStruct(frameArena, v3) = mat.ambientColor;
		*PushStruct(frameArena, v3) = mat.diffuseColor;
		*PushStruct(frameArena, v3) = mat.specularColor;
		*PushStruct(frameArena, v3) = mat.ke;

		*PushStruct(frameArena, f32) = mat.indexOfReflection;
		*PushStruct(frameArena, f32) = mat.dissolved;

		*PushStruct(frameArena, u32) = mat.illuminationModel;

		*PushStruct(frameArena, String);
#endif
	}

	For(mesh.indexSets)
	{
		Material mat = it->mat;
		*PushStruct(frameArena, u32) = mat.name.length;
		Char *dest = PushData(frameArena, Char, mat.name.length);
		memcpy(dest, mat.name.data, mat.name.length * sizeof(Char));
		PushZeroStruct(frameArena, Char); // to get it zero Terminated
		
	}

	file.fileSize = (u32)(frameArena->current - (u8 *)file.memory);
	WriteEntireFile(fileName, file);
}

static TriangleMesh LoadMesh(AssetHandler *assetHandler, char *fileName)
{
	TriangleMesh ret;

	File file = LoadFile(fileName);
	if (!file.fileSize) return {};
	u8 *cur = (u8 *)file.memory;
	ret.type = *(u32 *)cur;
	cur += sizeof(u32);
	ret.vertices.amount = *(u32 *)cur;
	cur += sizeof(u32);
	ret.vertices.data = (VertexFormat *)cur;
	cur += ret.vertices.amount * sizeof(VertexFormat);

	ret.indices.amount = *(u32 *)cur;
	cur += sizeof(u32);
	ret.indices.data = (u16 *)cur;
	cur += ret.indices.amount * sizeof(u16);

	ret.indexSets.amount = *(u32 *)cur;
	cur += sizeof(u32);
	ret.indexSets.data = (IndexSet *)cur;
	cur += ret.indexSets.amount * sizeof(IndexSet);

	For(ret.indexSets)
	{
		it->mat.name.length = *(u32 *)cur;
		cur += sizeof(u32);
		it->mat.name.data = (Char *)cur;
		cur += it->mat.name.length * sizeof(Char);
		cur++; // to get it zero Terminated
		String name = FormatString("obj/maja/%s.texture%c1", it->mat.name, '\0'); // todo hard coded. this should probably come out of the "name"

		it->mat.bitmapID = RegisterAsset(assetHandler, Asset_Texture, (char *)name.data);
	}

	RegisterTriangleMesh(&ret);


	return ret;
}

struct LightingSolution
{
	struct IrradianceCache *cache;
	struct KdNode *kdTree;
	struct LightingTriangle *lightingTriangles;
	u32 amountOfTriangles;

};

struct PlacedMesh
{
	TriangleMesh mesh;
	f32 scale;
	Quaternion orientation;
	v3 pos;
};

DefineDynamicArray(PlacedMesh);

static PlacedMesh CreatePlacedMesh(TriangleMesh mesh, f32 scale, Quaternion orientation, v3 pos)
{
	PlacedMesh ret;
	ret.mesh = mesh;
	ret.scale = scale;
	ret.orientation = orientation;
	ret.pos = pos;
	return ret;
};

struct World
{
	LightingSolution light;
	TriangleArray triangles;
	Camera camera;
	Camera debugCamera;
	v3 lightSource;
	PlacedMeshDynamicArray placedMeshes;
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
	Tweekable(f32, cameraMoveSpeed);
#if 0
	if (input->keybord[Key_shift].flag & KeyState_Down)
	{
		moveSpeed = 0.25f;
	}
#endif

	if (tracker.wDown)
	{
		cam->pos += cam->basis.d3 * cameraMoveSpeed;
	}
	if (tracker.sDown)
	{
		cam->pos -= cam->basis.d3 * cameraMoveSpeed;
	}
	if (tracker.dDown)
	{
		cam->pos += cam->basis.d1 * cameraMoveSpeed;
	}
	if (tracker.aDown)
	{
		cam->pos -= cam->basis.d1 * cameraMoveSpeed;
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
	ret.data = PushData(arena, Triangle, 0);
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

	TriangleArray ret = PushArray(arena, Triangle, 0);

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


#if 0
static TriangleMesh GenerateAndPushTriangleFloorMesh(AABB aabb, Arena* arena, u32 meshSize = 255)
{
	TriangleMesh ret;

	ret.type = TrianlgeMeshType_Strip;
	u32 blackBrown = RGBAfromHEX(0x553A26);
	u32 grassGreen = RGBAfromHEX(0x4B6F44);

	ret.amountOfVerticies = meshSize * meshSize; // 1600
	Assert(ret.amountOfVerticies < 65536);
	ret.vertices = PushData(arena, v3, ret.amountOfVerticies);
	ret.colors = PushData(arena, u32, ret.amountOfVerticies);

	RandomSeries series = { RandomSeed() };

	for (u32 x = 0; x < meshSize; x++)
	{
		for (u32 y = 0; y < meshSize; y++)
		{
			f32 xEntropy = 0.15f * RandomSignedPercent(&series);
			f32 yEntropy = 0.15f * RandomSignedPercent(&series);
			f32 zEntropy = 0.3f * RandomPercent(&series);

			f32 xVal = (xEntropy + (f32)x) / (meshSize) * (aabb.maxDim.x - aabb.minDim.x) + (aabb.minDim.x);
			f32 yVal = (yEntropy + (f32)y) / (meshSize) * (aabb.maxDim.y - aabb.minDim.y) + (aabb.minDim.y);

			ret.vertices[x + meshSize * y] = 0.5f * V3(xVal, yVal, zEntropy + aabb.minDim.z);
			ret.colors  [x + meshSize * y] = Pack3x8(LerpVector3(Unpack3x8(blackBrown), Unpack3x8(grassGreen), zEntropy * 3.0f));
		}
	}

	ret.amountOfIndicies = (meshSize - 1) * (3 + 2 * (meshSize - 1));
	ret.indecies = PushData(constantArena, u16, ret.amountOfIndicies);

	u32 index = 0;

	for (u32 x = 0; x < meshSize - 1; x++) 
	{
		ret.indecies[index++] = 0xFFFF; // reset Index

		ret.indecies[index++] = x * meshSize + 0; // (x, 0)

		//move up
		for (u32 y = 0; y < meshSize - 1; y++) // 2* (meshSize - 1)
		{	
			ret.indecies[index++] = (x + 1) * meshSize + y;
			ret.indecies[index++] = x       * meshSize + (y + 1);
		}

		ret.indecies[index++] = (x + 1) * meshSize + (meshSize - 1);
	}
	
	Assert(ret.amountOfIndicies == index);

	RegisterTriangleMesh(&ret);

	return ret;
}

#endif

#endif 

