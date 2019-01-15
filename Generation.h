#ifndef RR_GENERATION
#define RR_GENERATION


#if 0

float dotGridGradient(int ix, int iy, float x, float y) {
   
   extern float Gradient[IYMAX][IXMAX][2];
   
   // Compute the distance vector
   float dx = x - (float)ix;
   float dy = y - (float)iy;
   
   // Compute the dot-product
   return (dx*Gradient[iy][ix][0] + dy*Gradient[iy][ix][1]);
}

static TriangleArray Perlin(i32 min, i32 max) 
{
   TriangleMesh ret;
	Material mat;
	mat.bitmapID = RegisterAsset(assetHandler, Asset_Texture, "stone.texture");
	mat.spectularExponent = 96.078431f;
	mat.ambientColor = V3(1.000000f, 1.000000f, 1.000000f);
	mat.diffuseColor = V3(0.640000f, 0.640000f, 0.640000f);
	mat.specularColor = V3(0.500000f, 0.500000f, 0.500000f);
	mat.ke = V3(0.000000f, 0.000000f, 0.000000f);
	mat.indexOfReflection = 1;
	mat.dissolved = 1.000000;
	mat.illuminationModel = 2;
	mat.name = CreateString("Perlin");
	mat.texturePath = CreateString("textures/white.texture");
   
   // Determine grid cell coordinates
   i32 x0 = (i32)x;
   i32 x1 = x0 + 1;
   i32 y0 = (i32)y;
   i32 y1 = y0 + 1;
   
   // Determine interpolation weights
   // Could also use higher order polynomial/s-curve here
   float sx = x - (float)x0;
   float sy = y - (float)y0;
   
   // Interpolate between grid point gradients
   f32 n0    = dotGridGradient(x0, y0, x, y);
   f32 n1    = dotGridGradient(x1, y0, x, y);
   f32 ix0   = lerp(n0, sx, n1);
   
   f32 n0    = dotGridGradient(x0, y1, x, y);
   f32 n1    = dotGridGradient(x1, y1, x, y);
   f32 ix1   = lerp(n0, n1, sx);
   
   f32 value = lerp(ix0, sy, ix1);
   
   return value;
}


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

static RockCorner CreateRockCorner(v3 p, u32 colorSeed)
{
	RockCorner ret;
	ret.p = p;
	ret.color = GrayFromU32(colorSeed);
   
	//ret.prev = NULL;
	//ret.next = NULL;
	return ret;
}

static TriangleMesh GenerateMesh(AssetHandler *assetHandler, AABB aabb, Arena* arena)
{
	TriangleMesh ret;
	Material mat;
	mat.bitmapID = RegisterAsset(assetHandler, Asset_Texture, "stone.texture");
	mat.spectularExponent = 96.078431f;
	mat.ambientColor = V3(1.000000f, 1.000000f, 1.000000f);
	mat.diffuseColor = V3(0.640000f, 0.640000f, 0.640000f);
	mat.specularColor = V3(0.500000f, 0.500000f, 0.500000f);
	mat.ke = V3(0.000000f, 0.000000f, 0.000000f);
	mat.indexOfReflection = 1;
	mat.dissolved = 1.000000;
	mat.illuminationModel = 2;
	mat.name = CreateString("tileMapTile");
	mat.texturePath = CreateString("textures/stone.texture");
   
   
	u32 blackBrown = 0xFFFFFFFF; //RGBAfromHEX(0x553A26);
	u32 grassGreen = 0xFFFFFFFF; //RGBAfromHEX(0x4B6F44);
   
	u16 meshSize = 9; // @hardcoded
   
	ret.type = TrianlgeMeshType_Strip;
	u32 amountOfVerticies = meshSize * meshSize;
	Assert(amountOfVerticies < 65536);
	ret.vertices = PushArray(arena, VertexFormatPCUN, amountOfVerticies);
   
	RandomSeries series = { RandomSeed() };
   
	f32 xFac = (aabb.maxDim.x - aabb.minDim.x) / (meshSize - 1);
	f32 yFac = (aabb.maxDim.y - aabb.minDim.y) / (meshSize - 1);
   
   // generate the gradiants for perlin noise.
   v2Array gradiants = PushArray(frameArena, v2, amountOfVertices);
   
   
   
   // generate the vertices
	f32 meshInv = 1.0f / (f32)(meshSize - 1);
	for (u32 i = 0; i < meshSize; i++)
	{
		f32 percent = (f32)i * meshInv;
      
		ret.vertices[0 + meshSize * i].p = V3(0, yFac * i, 0) + aabb.minDim;
		ret.vertices[0 + meshSize * i].c = blackBrown;
		ret.vertices[0 + meshSize * i].uv = V2(0, percent);
      
		ret.vertices[(meshSize - 1) + meshSize * i].p = V3(xFac * (meshSize - 1), yFac * i, 0) + aabb.minDim;
		ret.vertices[(meshSize - 1) + meshSize * i].c = blackBrown;
		ret.vertices[(meshSize - 1) + meshSize * i].uv = V2(1.0f, percent);
      
		ret.vertices[i + meshSize * (meshSize - 1)].p = V3(xFac * i, yFac * (meshSize - 1), 0) + aabb.minDim;
		ret.vertices[i + meshSize * (meshSize - 1)].c = blackBrown;
		ret.vertices[i + meshSize * (meshSize - 1)].uv = V2(percent, 1.0f);
      
		ret.vertices[i + meshSize * 0].p = V3(xFac * i, 0, 0) + aabb.minDim;
		ret.vertices[i + meshSize * 0].c = blackBrown;
		ret.vertices[i + meshSize * 0].uv = V2(percent, 0.0f);
      
	}
   
   
	for (u32 x = 1; x < meshSize - 1u; x++)
	{
		f32 zVariance = aabb.maxDim.z - aabb.minDim.z;
		for (u32 y = 1; y < meshSize - 1u; y++)
		{
			f32 xEntropy = 0.5f * RandomSignedPercent(&series);
			f32 yEntropy = 0.5f * RandomSignedPercent(&series);
			f32 zEntropy = 0.5f * RandomPercent(&series);
         
			f32 xVal = (xEntropy + (f32)x) * xFac + (aabb.minDim.x);
			f32 yVal = (yEntropy + (f32)y) * yFac + (aabb.minDim.y);
			f32 zVal = zVariance * zEntropy + aabb.minDim.z;
         
			f32 lerpFactor = zEntropy;
         
			ret.vertices[x + meshSize * y].p = V3(xVal, yVal, zVal);
			ret.vertices[x + meshSize * y].c = Pack3x8(LerpVector3(Unpack3x8(blackBrown), Unpack3x8(grassGreen), lerpFactor));
			ret.vertices[x + meshSize * y].uv = V2((f32)x * meshInv, (f32)y * meshInv);
		}
	}
   
	u32 amountOfIndecies = (meshSize - 1) * (3 + 2 * (meshSize - 1));
	ret.indices = PushArray(arena, u16, amountOfIndecies);
	ret.indexSets = PushArray(arena, IndexSet, 1);
	ret.indexSets[0].mat = mat;
	ret.indexSets[0].amount = amountOfIndecies;
	ret.indexSets[0].offset = 0;
   
	u32 index = 0;
   
	for (u16 x = 0; x < meshSize - 1; x++)
	{
		ret.indices[index++] = 0xFFFF; // reset Index
      
		ret.indices[index++] = x * meshSize + 0; // (x, 0)
      
      //move up
		for (u16 y = 0; y < meshSize - 1; y++) // 2* (meshSize - 1)
		{
			ret.indices[index++] = (x + 1) * meshSize + y;
			ret.indices[index++] = x       * meshSize + (y + 1);
		}
      
		ret.indices[index++] = (x + 1) * meshSize + (meshSize - 1);
	}
   
	Assert(ret.indices.amount == index);
   
	// calculating vertices
	{
		v3 p = ret.vertices[0 + meshSize * 0].p;
      
		v3 p1 = ret.vertices[0 + meshSize * 1].p;
		v3 p2 = ret.vertices[1 + meshSize * 1].p;
		v3 p3 = ret.vertices[1 + meshSize * 0].p;
      
		v3 n1 = Normalize(CrossProduct(p1 - p, p2 - p));
		v3 n2 = Normalize(CrossProduct(p2 - p, p3 - p));
      
		ret.vertices[0 + meshSize * 0].n = Normalize((n1 + n2));
	}
   
   
	{
		v3 p = ret.vertices[(meshSize - 1) + meshSize * 0].p;
      
		v3 p3 = ret.vertices[(meshSize - 1) + meshSize * 1].p;
		v3 p2 = ret.vertices[(meshSize - 2) + meshSize * 1].p;
		v3 p1 = ret.vertices[(meshSize - 2) + meshSize * 0].p;
      
		v3 n1 = Normalize(CrossProduct(p1 - p, p2 - p));
		v3 n2 = Normalize(CrossProduct(p2 - p, p3 - p));
      
		ret.vertices[(meshSize - 1) + meshSize * 0].n = Normalize((n1 + n2));
	}
   
   
	{
		v3 p = ret.vertices[0 + meshSize * (meshSize - 1)].p;
      
		v3 p3 = ret.vertices[0 + meshSize * (meshSize - 2)].p;
		v3 p2 = ret.vertices[1 + meshSize * (meshSize - 2)].p;
		v3 p1 = ret.vertices[1 + meshSize * (meshSize - 1)].p;
      
		v3 n1 = Normalize(CrossProduct(p1 - p, p2 - p));
		v3 n2 = Normalize(CrossProduct(p2 - p, p3 - p));
      
		ret.vertices[0 + meshSize * (meshSize - 1)].n = Normalize((n1 + n2));
	}
   
   
	{
		v3 p = ret.vertices[(meshSize - 1) + meshSize * (meshSize - 1)].p;
      
		v3 p1 = ret.vertices[(meshSize - 1) + meshSize * (meshSize - 2)].p;
		v3 p2 = ret.vertices[(meshSize - 2) + meshSize * (meshSize - 2)].p;
		v3 p3 = ret.vertices[(meshSize - 2) + meshSize * (meshSize - 1)].p;
      
		v3 n1 = Normalize(CrossProduct(p1 - p, p2 - p));
		v3 n2 = Normalize(CrossProduct(p2 - p, p3 - p));
      
		ret.vertices[(meshSize - 1) + meshSize * (meshSize - 1)].n = Normalize((n1 + n2));
	}
   
   
	for (i32 i = 1; i < meshSize - 1; i++)
	{
		f32 percent = (f32)i * meshInv;
      
		{
			v3 p = ret.vertices[0 + meshSize * (i + 0)].p;
         
			//the 5 points top and clockwise
			v3 p1 = ret.vertices[0 + meshSize * (i + 1)].p;
			v3 p2 = ret.vertices[1 + meshSize * (i + 1)].p;
			v3 p3 = ret.vertices[1 + meshSize * (i + 0)].p;
			v3 p4 = ret.vertices[1 + meshSize * (i - 1)].p;
			v3 p5 = ret.vertices[0 + meshSize * (i - 1)].p;
         
			//the 4 triangle vertices
			v3 n1 = Normalize(CrossProduct(p1 - p, p2 - p));
			v3 n2 = Normalize(CrossProduct(p2 - p, p3 - p));
			v3 n3 = Normalize(CrossProduct(p3 - p, p4 - p));
			v3 n4 = Normalize(CrossProduct(p4 - p, p5 - p));
         
			ret.vertices[0 + meshSize * i].n = Normalize((n1 + n2 + n3 + n4));
         
		}
      
		{
			v3 p = ret.vertices[(meshSize - 1) + meshSize * (i + 0)].p;
         
			//the 5 points top and clockwise
			v3 p1 = ret.vertices[((meshSize - 1) + 0) + meshSize * (i + 1)].p;
			v3 p5 = ret.vertices[((meshSize - 1) + 0) + meshSize * (i - 1)].p;
			v3 p6 = ret.vertices[((meshSize - 1) - 1) + meshSize * (i - 1)].p;
			v3 p7 = ret.vertices[((meshSize - 1) - 1) + meshSize * (i + 0)].p;
			v3 p8 = ret.vertices[((meshSize - 1) - 1) + meshSize * (i + 1)].p;
         
         
			//the 4 triangle vertices
			v3 n5 = Normalize(CrossProduct(p5 - p, p6 - p));
			v3 n6 = Normalize(CrossProduct(p6 - p, p7 - p));
			v3 n7 = Normalize(CrossProduct(p7 - p, p8 - p));
			v3 n8 = Normalize(CrossProduct(p8 - p, p1 - p));
         
			ret.vertices[(meshSize - 1) + meshSize * i].n = Normalize((n5 + n6 + n7 + n8));
		}
      
      
      
		{
			v3 p = ret.vertices[(i + 0) + meshSize * (meshSize - 1)].p;
         
			//the 8 points top and clockwise
			v3 p3 = ret.vertices[(i + 1) + meshSize * (meshSize - 1 + 0)].p;
			v3 p4 = ret.vertices[(i + 1) + meshSize * (meshSize - 1 - 1)].p;
			v3 p5 = ret.vertices[(i + 0) + meshSize * (meshSize - 1 - 1)].p;
			v3 p6 = ret.vertices[(i - 1) + meshSize * (meshSize - 1 - 1)].p;
			v3 p7 = ret.vertices[(i - 1) + meshSize * (meshSize - 1 + 0)].p;
         
         
			//the 8 triangle normals
			v3 n3 = Normalize(CrossProduct(p3 - p, p4 - p));
			v3 n4 = Normalize(CrossProduct(p4 - p, p5 - p));
			v3 n5 = Normalize(CrossProduct(p5 - p, p6 - p));
			v3 n6 = Normalize(CrossProduct(p6 - p, p7 - p));
         
			ret.vertices[i + meshSize * (meshSize - 1)].n = Normalize((n3 + n4 + n5 + n6));
		}
      
      
      
		{
			v3 p = ret.vertices[(i + 0) + meshSize * 0].p;
         
			//the 5 points top and clockwise
			v3 p1 = ret.vertices[(i + 0) + meshSize * (0 + 1)].p;
			v3 p2 = ret.vertices[(i + 1) + meshSize * (0 + 1)].p;
			v3 p3 = ret.vertices[(i + 1) + meshSize * (0 + 0)].p;
			v3 p7 = ret.vertices[(i - 1) + meshSize * (0 + 0)].p;
			v3 p8 = ret.vertices[(i - 1) + meshSize * (0 + 1)].p;
         
         
			//the 4 triangle normals
			v3 n1 = Normalize(CrossProduct(p1 - p, p2 - p));
			v3 n2 = Normalize(CrossProduct(p2 - p, p3 - p));
			v3 n7 = Normalize(CrossProduct(p7 - p, p8 - p));
			v3 n8 = Normalize(CrossProduct(p8 - p, p1 - p));
         
			ret.vertices[i + meshSize * 0].n = Normalize((n1 + n2 + n7 + n8));
		}
	}
   
	for (i32 x = 1; x < meshSize - 1; x++)
	{
		for (i32 y = 1; y < meshSize - 1; y++)
		{
			v3 p = ret.vertices[(x + 0) + meshSize * (y + 0)].p;
         
			//the 8 points top and clockwise
			v3 p1 = ret.vertices[(x + 0) + meshSize * (y + 1)].p;
			v3 p2 = ret.vertices[(x + 1) + meshSize * (y + 1)].p;
			v3 p3 = ret.vertices[(x + 1) + meshSize * (y + 0)].p;
			v3 p4 = ret.vertices[(x + 1) + meshSize * (y - 1)].p;
			v3 p5 = ret.vertices[(x + 0) + meshSize * (y - 1)].p;
			v3 p6 = ret.vertices[(x - 1) + meshSize * (y - 1)].p;
			v3 p7 = ret.vertices[(x - 1) + meshSize * (y + 0)].p;
			v3 p8 = ret.vertices[(x - 1) + meshSize * (y + 1)].p;
         
         
			//the 8 triangle normals
			v3 n1 = Normalize(CrossProduct(p1 - p, p2 - p));
			v3 n2 = Normalize(CrossProduct(p2 - p, p3 - p));
			v3 n3 = Normalize(CrossProduct(p3 - p, p4 - p));
			v3 n4 = Normalize(CrossProduct(p4 - p, p5 - p));
			v3 n5 = Normalize(CrossProduct(p5 - p, p6 - p));
			v3 n6 = Normalize(CrossProduct(p6 - p, p7 - p));
			v3 n7 = Normalize(CrossProduct(p7 - p, p8 - p));
			v3 n8 = Normalize(CrossProduct(p8 - p, p1 - p));
         
         
			ret.vertices[x + meshSize * y].n = Normalize((n1 + n2 + n3 + n4 + n5 + n6 + n7 + n8));
		}
	}
   
	// just use the passed in one?
	For(ret.vertices)
	{
		aabb.maxDim.x = Max(it->p.x, aabb.maxDim.x);
		aabb.maxDim.y = Max(it->p.y, aabb.maxDim.y);
		aabb.maxDim.z = Max(it->p.z, aabb.maxDim.z);
      
		aabb.minDim.x = Min(it->p.x, aabb.minDim.x);
		aabb.minDim.y = Min(it->p.y, aabb.minDim.y);
		aabb.minDim.z = Min(it->p.z, aabb.minDim.z);
	}
   
	ret.aabb = aabb;
   
	RegisterTriangleMesh(&ret);
   
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
	RandomSeries series = { RandomSeed() };
   
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

static TriangleArray CreateStoneAndPush(AABB aabb, f32 desiredVolume, Arena *arena, u32 iterations, Arena *workingArena)
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
			ret.colors[x + meshSize * y] = Pack3x8(LerpVector3(Unpack3x8(blackBrown), Unpack3x8(grassGreen), zEntropy * 3.0f));
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


#endif // !RR_GENERATION