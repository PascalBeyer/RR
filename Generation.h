#ifndef RR_GENERATION
#define RR_GENERATION


static TriangleMesh GenerateMeshForFlatTile(AssetHandler *assetHandler, AABB aabb, Arena* arena)
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

	u32 meshSize = 9; // @hardcoded

	ret.type = TrianlgeMeshType_Strip;
	u32 amountOfVerticies = meshSize * meshSize;
	Assert(amountOfVerticies < 65536);
	ret.vertices = PushArray(arena, VertexFormat, amountOfVerticies);

	RandomSeries series = { RandomSeed() };

	f32 xFac = (aabb.maxDim.x - aabb.minDim.x) / (meshSize - 1);
	f32 yFac = (aabb.maxDim.y - aabb.minDim.y) / (meshSize - 1);

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


	for (u32 x = 1; x < meshSize - 1; x++)
	{
		f32 zVariance = aabb.maxDim.z - aabb.minDim.z;
		for (u32 y = 1; y < meshSize - 1; y++)
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

	for (u32 x = 0; x < meshSize - 1; x++)
	{
		ret.indices[index++] = 0xFFFF; // reset Index

		ret.indices[index++] = x * meshSize + 0; // (x, 0)

												 //move up
		for (u32 y = 0; y < meshSize - 1; y++) // 2* (meshSize - 1)
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


	for (u32 i = 1; i < meshSize - 1; i++)
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

	for (u32 x = 1; x < meshSize - 1; x++)
	{
		for (u32 y = 1; y < meshSize - 1; y++)
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



static TileMap InitTileMap(AssetHandler *assetHandler, u32 width, u32 height, Arena *arena)
{
	TileMap ret;
	ret.width = width;
	ret.height = height;

	ret.tiles = PushData(arena, Tile, width * height);

	RandomSeries series = GetRandomSeries();

	for (u32 x = 0; x < width; x++)
	{
		for (u32 y = 0; y < height; y++)
		{
			Tile *tempTile = ret.tiles + x + width * y;
			tempTile->type = (RandomU32(&series) % 2) ? Tile_Empty : Tile_Blocked;
		}
	}

	return ret;
}


#endif // !RR_GENERATION

