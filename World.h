#ifndef RR_WORLD
#define RR_WORLD

struct AABB
{
	v3 minDim;
	v3 maxDim;
};


static AABB CreateAABB(v3 minDim, v3 maxDim)
{
	// todo assert if we have volume?

	return { minDim, maxDim };
}

static AABB InvertedInfinityAABB()
{
	AABB ret;
	ret.maxDim = V3(MINF32, MINF32, MINF32);
	ret.minDim = V3(MAXF32, MAXF32, MAXF32);
	return ret;
}

static b32 PointInAABB(AABB target, v3 point)
{
	return 
		(
			target.minDim.x <= point.x + 0.001f &&
			target.minDim.y <= point.y + 0.001f &&
			target.minDim.z <= point.z + 0.001f &&
			target.maxDim.x + 0.001f >= point.x &&
			target.maxDim.y + 0.001f >= point.y &&
			target.maxDim.z + 0.001f >= point.z
		);
}
//Righthanded coordinates, +Z going into the screen, +Y is "down" ? , +X is to the right
//todo just store the projection matrix instead of aspectratio and focal length?
struct Camera
{
	v3 pos;
	f32 aspectRatio;
	f32 focalLength;

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


static v2 ScreenZeroToOneToInGame(Camera cam, v2 point)
{

	m4x4 proj = Projection(cam.aspectRatio, cam.focalLength) * CameraTransform(cam.basis.d1, cam.basis.d2, cam.basis.d3, cam.pos);
	m4x4 inv = InvOrId(proj);

	v3 p1 = inv * V3(-1, -1, -1);
	v3 p2 = inv * V3(1, -1, -1);
	v3 p3 = inv * V3(-1, 1, -1);
	//v3 p4 = inv * V3(1, 1, -1);

	f32 posXinGame = point.x;
	f32 posYinGame =  1.0f - point.y;

	// this works as the screen in game is again a Rectangle
	v3 inGameP = p1 + posXinGame * (p2 - p1) + posYinGame * (p3 - p1);

	float ctPos = -cam.pos.z / (inGameP.z - cam.pos.z);
	return p12(ctPos * (inGameP - cam.pos) + cam.pos);
}

static v3 ScreenZeroToOneToZ(Camera cam, v2 point, i32 z)
{

	m4x4 proj = Projection(cam.aspectRatio, cam.focalLength) * CameraTransform(cam.basis.d1, cam.basis.d2, cam.basis.d3, cam.pos);
	m4x4 inv = InvOrId(proj);

	v3 p1 = inv * V3(-1, -1, -1);
	v3 p2 = inv * V3(1, -1, -1);
	v3 p3 = inv * V3(-1, 1, -1);
	//v3 p4 = inv * V3(1, 1, -1);

	f32 posXinGame = point.x;
	f32 posYinGame = 1.0f - point.y;

	// this works as the screen in game is again a Rectangle
	v3 inGameP = p1 + posXinGame * (p2 - p1) + posYinGame * (p3 - p1);

	float ctPos = ((f32)z - cam.pos.z) / (inGameP.z - cam.pos.z);
	return (ctPos * (inGameP - cam.pos) + cam.pos);
}

static v3 ScreenZeroToOneToScreenInGame(Camera cam, v2 point)
{

	m4x4 proj = Projection(cam.aspectRatio, cam.focalLength) * CameraTransform(cam.basis.d1, cam.basis.d2, cam.basis.d3, cam.pos);
	m4x4 inv = InvOrId(proj);

	v3 p1 = inv * V3(-1, -1, -1);
	v3 p2 = inv * V3(1, -1, -1);
	v3 p3 = inv * V3(-1, 1, -1);
	//v3 p4 = inv * V3(1, 1, -1);

	f32 posXinGame = point.x;
	f32 posYinGame = 1.0f - point.y;

	// this works as the screen in game is again a Rectangle
	v3 inGameP = p1 + posXinGame * (p2 - p1) + posYinGame * (p3 - p1);

	return inGameP;
}

static v3 ScreenZeroToOneToDirecion(Camera cam, v2 point)
{
	m4x4 proj = Projection(cam.aspectRatio, cam.focalLength) * CameraTransform(cam.basis.d1, cam.basis.d2, cam.basis.d3, cam.pos);
	m4x4 inv = InvOrId(proj);

	v3 p1 = inv * V3(-1, -1, -1);
	v3 p2 = inv * V3(1, -1, -1);
	v3 p3 = inv * V3(-1, 1, -1);
	v3 p4 = inv * V3(1, 1, -1);

	f32 posXinGame = point.x;
	f32 posYinGame = 1.0f - point.y;

	v3 inGameP = p1 + posXinGame * (p2 - p1) + posYinGame * (p3 - p1);

	return (inGameP - cam.pos);
}


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
	String texturePath;
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

	AABB aabb;
	
	u32 vertexVBO; // to init this call glBufferData and glBindData
	u32 indexVBO;
};

DefineArray(TriangleMesh);
DefineDynamicArray(TriangleMesh);

void RegisterTriangleMesh(TriangleMesh *mesh);


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

#if 0
static void ColorForTileMap(Level *world)
{
	v3 screenPos = world->camera.pos;

	v2 screenUL = ScreenZeroToOneToInGame(world->camera, V2(0.0f, 0.0f));
	v2 screenUR = ScreenZeroToOneToInGame(world->camera, V2(1.0f, 0.0f));
	v2 screenBL = ScreenZeroToOneToInGame(world->camera, V2(0.0f, 1.0f));
	v2 screenBR = ScreenZeroToOneToInGame(world->camera, V2(1.0f, 1.0f));

	float minScreenX = Min(Min(screenUL.x, screenUR.x), Min(screenBL.x, screenBR.x));
	float minScreenY = Min(Min(screenUL.y, screenUR.y), Min(screenBL.y, screenBR.y));

	float maxScreenX = Max(Max(screenUL.x, screenUR.x), Max(screenBL.x, screenBR.x));
	float maxScreenY = Max(Max(screenUL.y, screenUR.y), Max(screenBL.y, screenBR.y));

	u32 minScreenXi = (u32)Max((int)floorf(minScreenX) - 1.0f, 0.0f);
	u32 minScreenYi = (u32)Max((int)floorf(minScreenY) - 1.0f, 0.0f);

	u32 maxScreenXi = (u32)Min((int)ceilf(maxScreenX) + 1.0f, (float)world->tileMap.width);
	u32 maxScreenYi = (u32)Min((int)ceilf(maxScreenY) + 1.0f, (float)world->tileMap.height);

	Tweekable(b32, DrawWholeMap);
	if (DrawWholeMap)
	{
		minScreenXi = 0;
		minScreenYi = 0;
		maxScreenXi = world->tileMap.width;
		maxScreenYi = world->tileMap.height;
	}
	for (u32 x = minScreenXi; x < maxScreenXi; x++)
	{
		for (u32 y = minScreenYi; y < maxScreenYi; y++)
		{
			Tile *tile = GetTile(world->tileMap, V2(x, y));
			
			v4 slightlyRed = V4(1.0f, 1.0f, 0.55f, 0.55f);
			v4 slightlyGreen = V4(1.0f, 0.55f, 1.0f, 0.55f);
			v4 slightlyYellow = V4(1.0f, 1.0f, 1.0f, 0.55f);
			v4 slightlyBlue = V4(1.0f, 0.55f, 0.55f, 1.0f);

			if (tile)
			{
				switch (tile->type)
				{
				case Tile_Blocked:
				{
					world->entities[tile->meshIndex].frameColor *= slightlyRed;
				}break;
				case Tile_Empty:
				{
					world->entities[tile->meshIndex].frameColor *= slightlyGreen;
				}break;
				case Tile_Goal:
				{
					world->entities[tile->meshIndex].frameColor *= slightlyYellow;
				}break;
				case Tile_Spawner:
				{
					world->entities[tile->meshIndex].frameColor *= slightlyBlue;
				}break;

				}
			}
		}
	}
}
#endif

static void UpdateCamGame(Input *input, Camera *camera)
{
	Tweekable(f32, screenScrollBorder, 0.05f);
	Tweekable(f32, screenScrollSpeed, 1.0f);

	v2 asd = input->mouseZeroToOne - V2(0.5f, 0.5f);

	f32 boxNorm = BoxNorm(asd) + screenScrollBorder;

	if (boxNorm > 0.5f)
	{
		camera->pos += 2.0f * boxNorm * screenScrollSpeed * i12(asd);
	}

}

static void UpdateCamFocus(Input *input, Camera *camera, DEBUGKeyTracker *tracker)
{
	v3 focusPoint = V3(ScreenZeroToOneToInGame(*camera, V2(0.5f, 0.5f)), 0.0f);

	f32 factor = 1.0f + tracker->middleBack * 0.1f - tracker->middleForward * 0.1f;

	tracker->middleForward = false;
	tracker->middleBack = false;

	camera->pos = focusPoint + factor * (camera->pos - focusPoint);

	if (!tracker->middleDown)
	{
		return;
		
	}

	v2 mouseDelta = input->mouseDelta;
	
	float rotSpeed = 0.001f * 3.141592f;
	
	f32 mouseZRot = -mouseDelta.y * rotSpeed; // this should rotate around the z axis
	f32 mouseCXRot = mouseDelta.x * rotSpeed; // this should rotate around the camera x axis

	m3x3 cameraT = Rows3x3(camera->b1, camera->b2, camera->b3);
	m3x3 cameraTInv = Invert(cameraT);

	m3x3 id = cameraT * cameraTInv;
	m3x3 rotX = XRotation3x3(mouseZRot);
	m3x3 rotZ = ZRotation3x3(mouseCXRot);
	m3x3 rot = cameraTInv * rotX * cameraT * rotZ;

	v3 delta = camera->pos - focusPoint;

	camera->pos = focusPoint + rot * delta;

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

#endif 

