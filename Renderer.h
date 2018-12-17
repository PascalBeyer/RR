#ifndef RR_RENDERER
#define RR_RENDERER

enum RenderGroupEntryType
{
	RenderGroup_EntryTexturedQuads,
	RenderGroup_EntryLines,
	RenderGroup_EntryTriangles,
	RenderGroup_EntryClear,
	RenderGroup_EntryTriangleMesh,
};

enum RenderSetUpFlags
{
	Setup_Ivalid = 0x0,
	Setup_Projective = 0x1,
	Setup_Orthogonal = 0x2,
	Setup_ZeroToOne = 0x3,
	Setup_ShadowMapping = 0x4,
};

struct RenderSetup
{
	m4x4 projection;
	m4x4 cameraTransform;
	v3 lightPos;
	v3 cameraPos; 
	u32 flag;
};


//todo: remove this out of the structs, by just pushing it independently?
struct RenderGroupEntryHeader
{
	RenderGroupEntryType type;
	RenderSetup setup;
};


struct EntryColoredVertices
{
	RenderGroupEntryHeader header;
	ColoredVertex *data;
	u32 vertexCount;
	u32 maxAmount;
};

struct EntryClear
{
	RenderGroupEntryHeader header;
	v4 color;
};

struct EntryTriangleMesh
{
	RenderGroupEntryHeader header;
	TriangleMesh mesh;
	u32Array textureIDs;
	Quaternion orientation;
	v3 pos;
	f32 scale;
	v4 scaleColor;
};

struct EntryTexturedQuads
{
	RenderGroupEntryHeader header;
	TexturedVertex *data;
	u32 vertexCount;
	u32 maxAmount;
	Bitmap *quadBitmaps;
};

struct EntryUpdateTexture
{
	RenderGroupEntryHeader header;
	Bitmap bitmap;
};

struct RenderGroup
{	
	EntryColoredVertices *currentTriangles;
	EntryColoredVertices *currentLines;
	EntryTexturedQuads *currentQuads;
	
	RenderSetup setup;

	RenderCommands *commands;
	AssetHandler *assetHandler;
};

static RenderGroup InitRenderGroup(AssetHandler *assetHandler, RenderCommands *comands)
{
	Assert(comands);
	Assert(assetHandler);
	RenderGroup ret;
	ret.assetHandler = assetHandler;
	
	ret.currentLines = NULL;
	ret.currentQuads = NULL;
	ret.currentTriangles = NULL;

	ret.commands = comands;
	return ret;
}

#define PushRenderEntryType(type, entryId) (type *)PushRenderEntry_(rg, sizeof(type), RenderGroup_##entryId)
#define PushRenderEntry(type) (type *)PushRenderEntry_(rg, sizeof(type), RenderGroup_##type)

static RenderGroupEntryHeader *PushRenderEntry_(RenderGroup *rg, u32 size, RenderGroupEntryType type)
{
	RenderGroupEntryHeader *result = 0;
	if (rg->commands)
	{
		RenderCommands *commands = rg->commands;
		if (commands->pushBufferSize + size < commands->maxBufferSize)
		{
			result = (RenderGroupEntryHeader *)(commands->pushBufferBase + commands->pushBufferSize);
			result->type = type;
			result->setup = rg->setup;
			commands->pushBufferSize += size;
		}
		else
		{
			Die;
		}
	}
	else
	{
		Die;
	}

	return result;
}

static void ClearPushBuffer(RenderGroup *rg)
{
	rg->commands->pushBufferSize = 0;
}

static void PushRenderSetup(RenderGroup *rg, Camera camera, v3 lightPos, u32 flag)
{
	TimedBlock;

	RenderCommands *commands = rg->commands;

	RenderSetup ret;
	
	switch (flag & 0x3)
	{
	case Setup_Projective:
	{
		m4x4 proj = Projection(commands->aspectRatio, camera.focalLength);
		m4x4 cameraTransform = CameraTransform(camera.basis.d1, camera.basis.d2, camera.basis.d3, camera.pos);
		ret.projection = proj;
		ret.cameraTransform = cameraTransform;

	}break;
	case Setup_Orthogonal:
	{
		m4x4 ortho = Orthogonal((f32)commands->width, (f32)commands->height);

		ret.projection = Translate(ortho, V3(-1.0f, 1.0f, 0.0f));
		ret.cameraTransform = Identity();

	}break;
	case Setup_ZeroToOne:
	{
		m4x4 ortho = Orthogonal(1.0f, 1.0f);

		ret.projection = Translate(ortho, V3(-1.0f, 1.0f, 0.0f));
		ret.cameraTransform = Identity();

	}break;
	default:
		Assert(!"Unhandled");
		break;
	}

	rg->currentLines = NULL;
	rg->currentTriangles = NULL;
	rg->currentQuads = NULL;
	
	ret.cameraPos = camera.pos;
	ret.lightPos = lightPos;
	ret.flag = flag;

	rg->setup = ret;
}

struct TexturedQuad
{
	TexturedVertex *verts;
	Bitmap *bitmap;
};

static TexturedQuad GetTexturedQuadMemory(RenderGroup *rg)
{
	if (rg->currentQuads->vertexCount + 4 <= rg->currentQuads->maxAmount)
	{
		Bitmap *bitmap = rg->currentQuads->quadBitmaps + (rg->currentQuads->vertexCount >> 2);
		TexturedVertex *verts = rg->currentQuads->data + rg->currentQuads->vertexCount;
		rg->currentQuads->vertexCount += 4;
		TexturedQuad ret;
		ret.bitmap = bitmap;
		ret.verts = verts;
		return ret;
	}
	else
	{
		EntryTexturedQuads *quads = PushRenderEntry(EntryTexturedQuads);
		quads->maxAmount = 1000;
		quads->data = PushData(frameArena, TexturedVertex, quads->maxAmount);
		quads->vertexCount = 0;

		rg->currentQuads = quads;

		u32 bitmapAmount = quads->maxAmount / 4;
		rg->currentQuads->quadBitmaps = PushData(frameArena, Bitmap, bitmapAmount);

		return GetTexturedQuadMemory(rg);
	}
}

static ColoredVertex *GetVertexMemory(RenderGroup *rg, RenderGroupEntryType type)
{ 
	switch (type)
	{
	case RenderGroup_EntryLines:
	{
		if (!rg->currentLines)
		{
			EntryColoredVertices *lines = PushRenderEntryType(EntryColoredVertices, EntryLines);
			lines->maxAmount = 65536;
			lines->data = PushData(frameArena, ColoredVertex, lines->maxAmount);
			lines->vertexCount = 0;

			rg->currentLines = lines;
		}

		u32 amount = 2;
		auto currentVertices = rg->currentLines;
		if (currentVertices->vertexCount + amount < currentVertices->maxAmount)
		{
			ColoredVertex *ret = currentVertices->data + currentVertices->vertexCount;
			currentVertices->vertexCount += amount;
			return ret;
		}
		else
		{
			rg->currentLines = NULL;
			return GetVertexMemory(rg, type);
		}

	}break;
	case RenderGroup_EntryTriangles:
	{
		if (!rg->currentTriangles)
		{
			EntryColoredVertices *triangles = PushRenderEntryType(EntryColoredVertices, EntryTriangles);
			triangles->maxAmount = 65536;
			triangles->data = PushData(frameArena, ColoredVertex, triangles->maxAmount);
			triangles->vertexCount = 0;

			rg->currentTriangles = triangles;
		}

		u32 amount = 3;
		auto currentVertices = rg->currentTriangles;
		if (currentVertices->vertexCount + amount < currentVertices->maxAmount)
		{
			ColoredVertex *ret = currentVertices->data + currentVertices->vertexCount;
			currentVertices->vertexCount += amount;
			return ret;
		}
		else
		{
			rg->currentTriangles = NULL;
			return GetVertexMemory(rg, type);
		}

	}break;
	default:
	{
		Die;
	}break;
	}

	return NULL;
}
static void PushTriangle(RenderGroup *rg, v3 p1, v3 p2, v3 p3, u32 c1, u32 c2, u32 c3)
{
	
	ColoredVertex *vert = GetVertexMemory(rg, RenderGroup_EntryTriangles);
	vert[0].pos = p1;
	vert[1].pos = p2;
	vert[2].pos = p3;

	vert[0].color = c1;
	vert[1].color = c2;
	vert[2].color = c3;
	
}
static void PushTriangle(RenderGroup *rg, ColoredVertex cv1, ColoredVertex cv2, ColoredVertex cv3)
{
	PushTriangle(rg, cv1.pos, cv2.pos, cv3.pos, cv1.color, cv2.color, cv3.color);
}

static void PushTriangle(RenderGroup *rg, v3 p1, v3 p2, v3 p3, u32 c)
{
	PushTriangle(rg, p1, p2, p3, c, c, c);
}
static void PushTriangle(RenderGroup *rg, v3 p1, v3 p2, v3 p3, v4 color = V4(1, 1, 1, 1))
{
	PushTriangle(rg, p1, p2, p3, Pack4x8(color));
}
static void PushTriangle(RenderGroup *rg, v2 p1, v2 p2, v2 p3, v4 color = V4(1, 1, 1, 1))
{
	PushTriangle(rg, i12(p1), i12(p2), i12(p3), Pack4x8(color));
}
static void PushTriangle(RenderGroup *rg, v3 p1, v3 p2, v3 p3, v3 color)
{
	PushTriangle(rg, p1, p2, p3, V4(1.0f, color));
}

static void PushTriangleMesh(RenderGroup *rg, u32 meshId, Quaternion orientation, v3 pos, f32 scale, v4 scaleColor)
{
	TriangleMesh *mesh = GetMesh(rg->assetHandler, meshId);

	if (!mesh) return; // todo think about this, we want this if we redo our mesh write

	EntryTriangleMesh *meshHeader = PushRenderEntry(EntryTriangleMesh);
	meshHeader->mesh = *mesh;
	meshHeader->orientation = orientation;
	u32Array arr = PushArray(frameArena, u32, mesh->indexSets.amount);
	for (u32 i = 0; i < arr.amount; i++)
	{
		arr[i] = GetTexture(rg->assetHandler, mesh->indexSets[i].mat.bitmapID)->textureHandle;
	}
	meshHeader->textureIDs = arr;
	meshHeader->pos = pos;
	meshHeader->scale = scale;
	meshHeader->scaleColor = scaleColor;
}

static void PushLine(RenderGroup *rg, v3 p1, v3 p2, u32 color = 0xFFFFFFFF)
{
	ColoredVertex *vert = GetVertexMemory(rg, RenderGroup_EntryLines);
	if (vert)
	{
		vert[0].pos = p1;
		vert[1].pos = p2;

		vert[0].color = color;
		vert[1].color = color;
	}
}
static void PushLine(RenderGroup *rg, v2 p1, v2 p2)
{
	PushLine(rg, i12(p1), i12(p2));
}


static void PushOutlinedTriangle(RenderGroup *rg, v3 p1, v3 p2, v3 p3, v4 color)
{
	PushTriangle(rg, p1, p2, p3, color);
	PushLine(rg, p1, p2);
	PushLine(rg, p1, p3);
	PushLine(rg, p2, p3);
}

static void PushOutlinedTriangle(RenderGroup *rg, v3 p1, v3 p2, v3 p3, u32 c1, u32 c2, u32 c3)
{
	PushTriangle(rg, p1, p2, p3, c1, c2, c3);
	PushLine(rg, p1, p2);
	PushLine(rg, p1, p3);
	PushLine(rg, p2, p3);
}

static void PushCuboid(RenderGroup *rg, v3 pos, Vector3Basis basis, v4 color = V4(1, 1, 1, 1))
{	

	v3 d1 = basis.d1;
	v3 d2 = basis.d2;
	v3 d3 = basis.d3;

	v3 p =		pos;

	v3 p1 =		pos + d1;
	v3 p2 =		pos + d2;
	v3 p3 =		pos + d3;

	v3 p12 =	pos + d1 + d2;
	v3 p13 =	pos + d1 + d3;
	v3 p23 =	pos + d2 + d3;

	v3 p123 =	pos + d1 + d2 + d3;


	PushTriangle(rg, p, p1, p2, color);	
	PushTriangle(rg, p12, p1, p2, color);

	PushTriangle(rg, p, p1, p3, color);
	PushTriangle(rg, p13, p1, p3, color);
	
	PushTriangle(rg, p, p2, p3, color);
	PushTriangle(rg, p23, p2, p3, color);
	
	
	PushTriangle(rg, p3, p13, p23, color);
	PushTriangle(rg, p123, p13, p23, color);
	
	PushTriangle(rg, p2, p12, p23, color);
	PushTriangle(rg, p123, p12, p23, color);
	
	PushTriangle(rg, p1, p12, p13, color);
	PushTriangle(rg, p123, p12, p13, color);
	
}
static void PushCenteredCuboid(RenderGroup *rg, v3 pos, Vector3Basis basis, v4 color = V4(1, 1, 1, 1))
{
	v3 posAd = pos - 0.5f * basis.d1 - 0.5f * basis.d2 - 0.5f * basis.d3;
	PushCuboid(rg, posAd, basis, color);
}

static void PushDebugPointCuboid(RenderGroup *rg, v3 pos, v4 color = V4(1, 1, 1, 1))
{
	f32 scale = 0.1f;
	PushCenteredCuboid(rg, pos, scale * v3StdBasis, color);
}


static void PushAABBOutLine(RenderGroup *rg, v3 minDim, v3 maxDim)
{
	v3 d1 = V3(maxDim.x - minDim.x, 0, 0);
	v3 d2 = V3(0, maxDim.y - minDim.y, 0);
	v3 d3 = V3(0, 0, maxDim.z - minDim.z);

	v3 p[8] =
	{
		minDim,			//0

		minDim + d1,		//1
		minDim + d2,		//2
		minDim + d3,		//3

		minDim + d1 + d2,	//4
		minDim + d2 + d3,	//5
		minDim + d3 + d1,	//6

		maxDim,			//7
		
	};

	//_upper_ square
	PushLine(rg, p[0], p[1]);
	PushLine(rg, p[1], p[4]);
	PushLine(rg, p[4], p[2]);
	PushLine(rg, p[2], p[0]);

	//_lower_ square
	PushLine(rg, p[7], p[5]);
	PushLine(rg, p[5], p[3]);
	PushLine(rg, p[3], p[6]);
	PushLine(rg, p[6], p[7]);

	//_connecting_ lines
	PushLine(rg, p[0], p[3]);
	PushLine(rg, p[2], p[5]);
	PushLine(rg, p[1], p[6]);
	PushLine(rg, p[4], p[7]);

}

static void PushClear(RenderGroup *rg, v4 color)
{
	EntryClear *result = PushRenderEntry(EntryClear);
	if (result)
	{
		result->color = color;
	}
}

static void PushQuadrilateral(RenderGroup *rg, v3 p1, v3 p2, v3 p3, v3 p4, u32 c1, u32 c2, u32 c3, u32 c4)
{
	PushTriangle(rg, p1, p2, p3, c1, c2, c3);
	PushTriangle(rg, p4, p2, p3, c4, c2, c3);
}

static void PushQuadrilateral(RenderGroup *rg, v3 p1, v3 p2, v3 p3, v3 p4, v4 color)
{
	PushTriangle(rg, p1, p2, p3, color);
	PushTriangle(rg, p4, p2, p3, color);
}

static void PushRectangle(RenderGroup *rg, v3 pos, v3 vec1, v3 vec2, v4 color)
{
	PushQuadrilateral(rg, pos, pos + vec1, pos + vec2, pos + vec1 + vec2, color);
}

#define Orthogonal_Rectangle_OFFSET 0.0000001f

// note : these are a bit "back" so they render behind the "text" / all bit maps
static void PushRectangle(RenderGroup *rg, v2 pos, v2 vec1, v2 vec2, v4 color)
{
	PushQuadrilateral(rg, V3(pos, Orthogonal_Rectangle_OFFSET), V3(pos + vec1, Orthogonal_Rectangle_OFFSET), V3(pos + vec2, Orthogonal_Rectangle_OFFSET), V3(pos + vec1 + vec2, Orthogonal_Rectangle_OFFSET), color);
}

static void PushRectangle(RenderGroup *rg, v2 min, v2 max, v4 color)
{
	v2 p1 = min;
	v2 p2 = V2(max.x, min.y);
	v2 p3 = V2(min.x, max.y);
	v2 p4 = max;

	PushQuadrilateral(rg, V3(p1, Orthogonal_Rectangle_OFFSET), V3(p2, Orthogonal_Rectangle_OFFSET), V3(p3, Orthogonal_Rectangle_OFFSET), V3(p4, Orthogonal_Rectangle_OFFSET), color);
}

static void PushRectangle(RenderGroup *rg, v2 pos, f32 width, f32 height, f32 zOffset, v4 color)
{
	v2 vec1 = V2(width, 0.0f);
	v2 vec2 = V2(0.0f, height);

	PushQuadrilateral(rg, V3(pos, zOffset), V3(pos + vec1, zOffset), V3(pos + vec2, zOffset), V3(pos + vec1 + vec2, zOffset), color);
}

static void PushRectangle(RenderGroup *rg, v2 pos, f32 width, f32 height, f32 zOffset, u32 c1, u32 c2, u32 c3, u32 c4)
{
	v2 vec1 = V2(width, 0.0f);
	v2 vec2 = V2(0.0f, height);

	PushQuadrilateral(rg, V3(pos, zOffset), V3(pos + vec1, zOffset), V3(pos + vec2, zOffset), V3(pos + vec1 + vec2, zOffset), c1, c2, c3, c4);
}

static void PushRectangle(RenderGroup *rg, v2 min, v2 max, u32 c1, u32 c2, u32 c3, u32 c4)
{
	v2 p1 = min;
	v2 p2 = V2(max.x, min.y);
	v2 p3 = V2(min.x, max.y);
	v2 p4 = max;

	PushQuadrilateral(rg, V3(p1, Orthogonal_Rectangle_OFFSET), V3(p2, Orthogonal_Rectangle_OFFSET), V3(p3, Orthogonal_Rectangle_OFFSET), V3(p4, Orthogonal_Rectangle_OFFSET), c1, c2, c3, c4);
}

static void PushRectangle(RenderGroup *rg, v2 min, v2 max, u32 c)
{
	v2 p1 = min;
	v2 p2 = V2(max.x, min.y);
	v2 p3 = V2(min.x, max.y);
	v2 p4 = max;

	PushQuadrilateral(rg, V3(p1, Orthogonal_Rectangle_OFFSET), V3(p2, Orthogonal_Rectangle_OFFSET), V3(p3, Orthogonal_Rectangle_OFFSET), V3(p4, Orthogonal_Rectangle_OFFSET), c, c, c, c);
}

static void PushRectangle(RenderGroup *rg, v2 pos, f32 width, f32 height, u32 c1, u32 c2, u32 c3, u32 c4)
{
	v2 vec1 = V2(width, 0.0f);
	v2 vec2 = V2(0.0f, height);

	PushQuadrilateral(rg, V3(pos, Orthogonal_Rectangle_OFFSET), V3(pos + vec1, Orthogonal_Rectangle_OFFSET), V3(pos + vec2, Orthogonal_Rectangle_OFFSET), V3(pos + vec1 + vec2, Orthogonal_Rectangle_OFFSET), c1, c2, c3, c4);
}

static void PushRectangle(RenderGroup *rg, v2 pos, f32 width, f32 height, u32 c)
{
	v2 vec1 = V2(width, 0.0f);
	v2 vec2 = V2(0.0f, height);

	PushQuadrilateral(rg, V3(pos, Orthogonal_Rectangle_OFFSET), V3(pos + vec1, Orthogonal_Rectangle_OFFSET), V3(pos + vec2, Orthogonal_Rectangle_OFFSET), V3(pos + vec1 + vec2, Orthogonal_Rectangle_OFFSET), c, c, c, c);
}

static void PushRectangle(RenderGroup *rg, v2 pos, f32 width, f32 height, v4 color)
{
	v2 vec1 = V2(width, 0.0f);
	v2 vec2 = V2(0.0f, height);

	PushQuadrilateral(rg, V3(pos, Orthogonal_Rectangle_OFFSET), V3(pos + vec1, Orthogonal_Rectangle_OFFSET), V3(pos + vec2, Orthogonal_Rectangle_OFFSET), V3(pos + vec1 + vec2, Orthogonal_Rectangle_OFFSET), color);
}

static void PushCenteredRectangle(RenderGroup *rg, v2 pos, float width, float height, v4 color)
{
	v2 vec1 = V2(width, 0.0f);    
	v2 vec2 = V2(0.0f, height);

	v2 p1 = (pos - 0.5f * (vec1 + vec2));
	v2 p2 = (pos + vec1 - 0.5f * (vec1 + vec2));
	v2 p3 = (pos + vec2 - 0.5f * (vec1 + vec2));
	v2 p4 = (pos + vec1 + vec2 - 0.5f * (vec1 + vec2));

	PushQuadrilateral(rg, V3(p1, Orthogonal_Rectangle_OFFSET), V3(p2, Orthogonal_Rectangle_OFFSET), V3(p3, Orthogonal_Rectangle_OFFSET), V3(p4, Orthogonal_Rectangle_OFFSET), color);
}
 
#undef Orthogonal_Rectangle_OFFSET

static void PushTexturedQuad(RenderGroup *rg, v3 p1, v3 p2, v3 p3, v3 p4, Bitmap bitmap, v4 color, bool inverted, v2 minUV, v2 maxUV)
{
	if (!rg->currentQuads)
	{
		EntryTexturedQuads *quads = PushRenderEntry(EntryTexturedQuads);
		quads->maxAmount = 1000;
		quads->data = PushData(frameArena, TexturedVertex, quads->maxAmount);
		quads->vertexCount = 0;

		rg->currentQuads = quads;

		u32 bitmapAmount = quads->maxAmount / 4;
		rg->currentQuads->quadBitmaps = PushData(frameArena, Bitmap, bitmapAmount);
	}

	TexturedQuad mem = GetTexturedQuadMemory(rg);
	
	mem.verts[0].pos = p1;
	mem.verts[1].pos = p2;
	mem.verts[2].pos = p3;
	mem.verts[3].pos = p4;

	if (inverted)
	{
		mem.verts[0].uv = V2(minUV.x, minUV.y); // 0 0 
		mem.verts[1].uv = V2(maxUV.x, minUV.y); // 1 0
		mem.verts[2].uv = V2(minUV.x, maxUV.y); // 0 1
		mem.verts[3].uv = V2(maxUV.x, maxUV.y); // 1 1
	}
	else
	{
		mem.verts[0].uv = V2(minUV.x, maxUV.y); // 0 1
		mem.verts[1].uv = V2(maxUV.x, maxUV.y); // 1 1
		mem.verts[2].uv = V2(minUV.x, minUV.y); // 0 0
		mem.verts[3].uv = V2(maxUV.x, minUV.y); // 1 0

	}
	
	mem.verts[0].color = Pack4x8(color);
	mem.verts[1].color = Pack4x8(color);
	mem.verts[2].color = Pack4x8(color);
	mem.verts[3].color = Pack4x8(color);

	*mem.bitmap = bitmap;
}

static void PushTexturedQuad(RenderGroup *rg, v3 p1, v3 p2, v3 p3, v3 p4, Bitmap bitmap, v4 color)
{
	PushTexturedQuad(rg, p1, p2, p3, p4, bitmap, color, false, V2(), V2(1, 1));
}

//Setup_BitmapsOnTop = 0x8, // for orthogonal UI stuff,	this is a general rule for now
static void PushTexturedRect(RenderGroup *rg, v2 p1, f32 width, f32 height, Bitmap bitmap, v4 color, bool inverted, v2 minUV, v2 maxUV)
{
	v2 p2 = p1 + V2(width, 0);
	v2 p3 = p1 + V2(0, height);
	v2 p4 = p1 + V2(width, height);
	PushTexturedQuad(rg, i12(p1), i12(p2), i12(p3), i12(p4), bitmap, color, inverted, minUV, maxUV);
}

static void PushTexturedRect(RenderGroup *rg, v2 p1, f32 width, f32 height, Bitmap bitmap)
{
	v2 p2 = p1 + V2(width, 0);
	v2 p3 = p1 + V2(0, height);
	v2 p4 = p1 + V2(width, height);
	PushTexturedQuad(rg, i12(p1), i12(p2), i12(p3), i12(p4), bitmap, V4(1, 1, 1, 1), false, V2(), V2(1, 1));
}

static void PushTexturedRect(RenderGroup *rg, v2 p1, f32 width, f32 height, Bitmap bitmap, bool inverted)
{
	v2 p2 = p1 + V2(width, 0);
	v2 p3 = p1 + V2(0, height);
	v2 p4 = p1 + V2(width, height);
	PushTexturedQuad(rg, i12(p1), i12(p2), i12(p3), i12(p4), bitmap, V4(1, 1, 1, 1), inverted, V2(), V2(1, 1));
}

static void PushBitmap(RenderGroup *rg, v2 pos, Bitmap bitmap, v4 color = V4(1, 1, 1, 1))
{
	PushTexturedRect(rg, pos, (f32)bitmap.width, (f32)bitmap.height, bitmap, color, false, V2(0, 0), V2(1, 1));
}

// returns actual string width
static f32 PushString(RenderGroup *rg, v2 pos, unsigned char* string, u32 stringLength, f32 size, Font font, v4 color = V4(1, 1, 1, 1))
{
	TimedBlock;

	f32 x = pos.x;
	float fScale = size / (f32)font.charHeight;

	Assert(font.amountOfChars);

	for (u32 i = 0; i < stringLength; i++)
	{
		if (string[i] < font.amountOfChars)
		{
			CharData data = font.charData[string[i]];

			f32 offSetX = fScale * data.xOff;
			f32 offSetY = fScale * data.yOff;

			f32 scaledWidth = fScale * (f32)data.width;
			f32 scaledHeight = fScale * (f32)data.height;

			f32 y = pos.y + (f32)size;

			v2 writePos = V2(x + offSetX, y + offSetY);
			
			PushTexturedRect(rg, writePos, scaledWidth, scaledHeight, font.bitmap, color, true, data.minUV, data.maxUV);
			float actualFloatWidth = data.xAdvance * fScale;
			x += actualFloatWidth;

		}
		else
		{
			Assert(!"Not handled font symbol");
		}
	}

	return (x - pos.x);
}
static f32 PushString(RenderGroup *rg, v2 pos, const char* string, f32 size, Font font, v4 color = V4(1, 1, 1, 1))
{
	return PushString(rg, pos, (unsigned char *)string, NullTerminatedStringLength(string), size, font, color);
}

static f32 PushString(RenderGroup *rg, v2 pos, String string, f32 size, Font font, v4 color = V4(1, 1, 1, 1))
{
	return PushString(rg, pos, string.data, string.length, size, font, color);
}



#endif
