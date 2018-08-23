#ifndef RR_RENDERER
#define RR_RENDERER

#include "String.h"
#include "AssetHandler.h"
#include "Math.h"
#include "LinearAlgebra.h"
#include "WorkHandler.h"

#include "Math.h"
#include "Intrinsics.h"
#include "Fonts.h"
#include "Debug.h"

enum RenderGroupEntryType
{
	RenderGroup_EntryTexturedQuads,
	RenderGroup_EntryLines,
	RenderGroup_EntryTriangles,
	RenderGroup_EntryClear,

	RenderGroup_EntryUpdateTexture,
	RenderGroup_EntryTransform
};


//todo: remove this out of the structs, by just pushing it independently?
struct RenderGroupEntryHeader
{
	RenderGroupEntryType type;
};

struct EntryTransform
{
	RenderGroupEntryHeader header;
	m4x4 m;
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
			commands->pushBufferSize += size;
		}
	}

	if (type == RenderGroup_EntryTransform)
	{
		rg->currentLines = NULL;
		rg->currentTriangles = NULL;
		rg->currentQuads = NULL;
	}

	if (type == RenderGroup_EntryUpdateTexture)
	{
		rg->currentQuads = NULL;
	}

	return result;
}

static void ClearPushBuffer(RenderGroup *rg)
{
	rg->commands->pushBufferSize = 0;
}

static void PushProjectivTransform(RenderGroup *rg, v3 cameraPos, Vector3Basis cameraBasis)
{
	EntryTransform *transform = PushRenderEntry(EntryTransform);

	if (transform)
	{
		RenderCommands *commands = rg->commands;
		m4x4 proj = Projection(commands->aspectRatio, commands->focalLength);
		m4x4 cam = CameraTransform(cameraBasis.d1, cameraBasis.d2, cameraBasis.d3, cameraPos);
		transform->m = proj * cam;
	}
}

static void PushOrthogonalTransform(RenderGroup *rg)
{
	EntryTransform *transform = PushRenderEntry(EntryTransform);
	if (transform)
	{
		RenderCommands *commands = rg->commands;
		m4x4 ortho = Orthogonal((f32)commands->width, (f32)commands->height);

		transform->m = Translate(ortho, V3(-1.0f, 1.0f, 0.0f));
	}
}

struct TexturedQuad
{
	TexturedVertex *verts;
	Bitmap *bitmap;
};

static TexturedQuad GetTexturedQuadMemory(RenderGroup *rg)
{
	if (rg->currentQuads->vertexCount + 4 < rg->currentQuads->maxAmount)
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
		//todo: allocate new array, send the old one off
		Die;
		return {};
	}
}

static ColoredVertex *GetVertexMemory(RenderGroup *rg, RenderGroupEntryType type)
{ 
	u32 amount = 0;
	EntryColoredVertices *currentVertices = NULL;
	switch (type)
	{
	case RenderGroup_EntryLines:
	{
		amount = 2;
		currentVertices = rg->currentLines;
	}break;
	case RenderGroup_EntryTriangles:
	{
		amount = 3;
		currentVertices = rg->currentTriangles;
	}break;
	default:
	{
		Die;
	}break;
	}

	if (currentVertices->vertexCount + amount < currentVertices->maxAmount)
	{
		ColoredVertex *ret = currentVertices->data + currentVertices->vertexCount;
		currentVertices->vertexCount += amount;
		return ret;
	}
	else
	{
		//todo: allocate new array, send the old one off
		Die;
	}

	return NULL;
}
static void PushTriangle(RenderGroup *rg, v3 p1, v3 p2, v3 p3, u32 c1, u32 c2, u32 c3)
{
	if (!rg->currentTriangles)
	{
		EntryColoredVertices *triangles = PushRenderEntryType(EntryColoredVertices, EntryTriangles);
		triangles->maxAmount = 10000;
		triangles->data = PushArray(frameArena, ColoredVertex, triangles->maxAmount);
		triangles->vertexCount = 0;
		
		rg->currentTriangles = triangles;
	}

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
static void PushTriangle(RenderGroup *rg, v3 p1, v3 p2, v3 p3, v3 color)
{
	PushTriangle(rg, p1, p2, p3, V4(1.0f, color));
}

static void PushLine(RenderGroup *rg, v3 p1, v3 p2)
{
	if (!rg->currentLines)
	{
		EntryColoredVertices *lines = PushRenderEntryType(EntryColoredVertices, EntryLines);
		lines->maxAmount = 1000;
		lines->data = PushArray(frameArena, ColoredVertex, lines->maxAmount);
		lines->vertexCount = 0;

		rg->currentLines = lines;
	}

	ColoredVertex *vert = GetVertexMemory(rg, RenderGroup_EntryLines);
	if (vert)
	{
		vert[0].pos = p1;
		vert[1].pos = p2;

		vert[0].color = 0xFFFFFFFF;
		vert[1].color = 0xFFFFFFFF;
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

static void PushDebugPointCuboid(RenderGroup *rg, v3 pos)
{
	f32 scale = 0.1f;
	PushCenteredCuboid(rg, pos, scale * v3StdBasis);
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
static void PushUpdateTexture(RenderGroup *rg, Bitmap bitmap)
{
	EntryUpdateTexture *result = PushRenderEntry(EntryUpdateTexture);
	if (result)
	{
		result->bitmap = bitmap;
	}
}
static void PushClear(RenderGroup *rg, v4 color)
{
	EntryClear *result = PushRenderEntry(EntryClear);
	if (result)
	{
		result->color = color;
	}
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

static void PushRectangle(RenderGroup *rg, v2 pos, v2 vec1, v2 vec2, v4 color)
{
	PushQuadrilateral(rg, i12(pos), i12(pos + vec1), i12(pos + vec2), i12(pos + vec1 + vec2), color);
}

static void PushRectangle(RenderGroup *rg, v2 pos, f32 width, f32 height, v4 color)
{
	v2 vec1 = V2(width, 0.0f);
	v2 vec2 = V2(0.0f, height);

	PushQuadrilateral(rg, i12(pos), i12(pos + vec1), i12(pos + vec2), i12(pos + vec1 + vec2), color);
}

static void PushCenteredRectangle(RenderGroup *rg, v2 pos, float width, float height, v4 color)
{
	v2 vec1 = V2(width, 0.0f);    
	v2 vec2 = V2(0.0f, height);

	v3 p1 = i12(pos - 0.5f * (vec1 + vec2));
	v3 p2 = i12(pos + vec1 - 0.5f * (vec1 + vec2));
	v3 p3 = i12(pos + vec2 - 0.5f * (vec1 + vec2));
	v3 p4 = i12(pos + vec1 + vec2 - 0.5f * (vec1 + vec2));

	PushQuadrilateral(rg, p1, p2, p3, p4, color);
}

static void PushTexturedQuad(RenderGroup *rg, v3 p1, v3 p2, v3 p3, v3 p4, Bitmap bitmap, v4 color = V4(1, 1, 1, 1))
{
	if (!rg->currentQuads)
	{
		EntryTexturedQuads *quads = PushRenderEntry(EntryTexturedQuads);
		quads->maxAmount = 1000;
		quads->data = PushArray(frameArena, TexturedVertex, quads->maxAmount);
		quads->vertexCount = 0;

		rg->currentQuads = quads;

		u32 bitmapAmount = quads->maxAmount / 4;
		rg->currentQuads->quadBitmaps = PushArray(frameArena, Bitmap, bitmapAmount);
	}

	TexturedQuad mem = GetTexturedQuadMemory(rg);
	
	mem.verts[0].pos = p1;
	mem.verts[1].pos = p2;
	mem.verts[2].pos = p3;
	mem.verts[3].pos = p4;

	mem.verts[0].uv = V2(0, 1);
	mem.verts[1].uv = V2(1, 1);
	mem.verts[2].uv = V2(0, 0);
	mem.verts[3].uv = V2(1, 0);
	
	mem.verts[0].color = Pack4x8(color);
	mem.verts[1].color = Pack4x8(color);
	mem.verts[2].color = Pack4x8(color);
	mem.verts[3].color = Pack4x8(color);

	*mem.bitmap = bitmap;
}

static void PushTexturedQuad(RenderGroup *rg, v3 p1, v3 p2, v3 p3, v3 p4, AssetId assetId, u32 assetIndex, v4 color = V4(1, 1, 1, 1))
{
	Bitmap bitmap = *rg->assetHandler->GetAsset(assetId)->sprites[assetIndex];
	PushTexturedQuad(rg, p1, p2, p3, p4, bitmap, color);
}

static void PushTexturedRect(RenderGroup *rg, v2 p1, f32 width, f32 height, Bitmap bitmap, v4 color = V4(1, 1, 1, 1))
{
	v2 p2 = p1 + V2(width, 0);
	v2 p3 = p1 + V2(0, height);
	v2 p4 = p1 + V2(width, height);
	PushTexturedQuad(rg, i12(p1), i12(p2), i12(p3), i12(p4), bitmap, color);
}

static void PushTexturedRect(RenderGroup *rg, v2 p1_, f32 width, f32 height, AssetId id, u32 assetIndex = 0, v4 color = V4(1, 1, 1, 1))
{
	v3 p1 = i12(p1_);
	v3 p2 = p1 + V3(width, 0, 0);
	v3 p3 = p1 + V3(0, height, 0);
	v3 p4 = p1 + V3(width, height, 0);
	Bitmap bitmap = *rg->assetHandler->GetAsset(id)->sprites[assetIndex];
	PushTexturedQuad(rg, p1, p2, p3, p4, bitmap, color);
}
static void PushBitmap(RenderGroup *rg, v2 pos, Bitmap bitmap, v4 color = V4(1, 1, 1, 1))
{
	PushTexturedRect(rg, pos, (f32)bitmap.width, (f32)bitmap.height, bitmap, color);
}

static void PushDebugString(RenderGroup *rg, v2 pos, char* string, u32 stringLength, u32 size)
{
	v2 writePos = pos;
	float fSize = (float)size;

	for (u32 i = 0; i < stringLength; i++)
	{
		FontEnum charId = GetCharIndex(string[i]);
		if (charId < Font_InvalideID)
		{
			PushTexturedRect(rg, writePos, (float)size, (float)size, { Asset_Font }, charId);
			float actualFloatWidth = FontGetActualFloatWidth(charId, (float)size);
			writePos += V2(actualFloatWidth, 0);
		}
		else
		{
			Assert(!"Not handled font symbol");
		}
	}
}

static void PushDebugString(RenderGroup *rg, v2 pos, String string, u32 size)
{
	PushDebugString(rg, pos, string.string, string.length, size);
}
static void PushUnit(RenderGroup *rg, v3 pos, u32 facingDirection, u32 assetId, float radius)
{
	AssetId id;
	id.id = assetId;
	Asset *unitAsset = rg->assetHandler->GetAsset(id);
	if (unitAsset)
	{

		Bitmap bitmap = *unitAsset->sprites[facingDirection];
		Die;
		//PushTexturedRect(rg,  pos - V3(radius, radius, 0),  2.0f * radius, 2.0f * radius, bitmap);
	}
	Die;
}
static void PushBuilding(RenderGroup *rg, v3 pos, u32 assetId, float radius)
{
	AssetId id;
	id.id = assetId;
	Asset *unitAsset = rg->assetHandler->GetAsset(id);
	if (unitAsset)
	{
		Vector3Basis basis = {};
		basis.d1 = V3(radius, 0, 0.2f);
		basis.d2 = V3(0, radius, 0.2f);
		basis.d3 = V3(0, 0, 1);
		Die;
		//PushBitmap(rg, unitAsset->sprites[0], pos - V3(radius, radius, 0), basis);
	}

}



#endif
