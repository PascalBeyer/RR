#ifndef RR_RENDERER
#define RR_RENDERER

#include "Vector3.h"
#include "String.h"
#include "AssetHandler.h"
#include "Math.h"
#include "LinearAlgebra.h"
#include "WorkHandler.h"

#include "Math.h"
#include "Intrinsics.h"
#include "Fonts.h"
#include "Debug.h"

struct ColoredVertex
{
	v3 pos;
	u32 color;
};

enum RenderGroupEntryType
{
	RenderGroup_EntryBitmap,
	RenderGroup_EntryTexturedQuadrilateral,
	RenderGroup_EntryLine,
	RenderGroup_EntryClear,
	RenderGroup_EntryUpdateBitmap,
	RenderGroup_EntryTriangle,
	RenderGroup_EntryPoint,
	RenderGroup_EntryTriangles,

	RenderGroup_EntryTransform
};

struct RenderGroupEntryHeader
{
	RenderGroupEntryType type;
};

struct EntryTransform
{
	RenderGroupEntryHeader header;
	m4x4 m;
};


struct EntryLine
{
	RenderGroupEntryHeader header;
	v3 p1;
	v3 p2;
	v4 color;
};

struct EntryTriangle
{
	RenderGroupEntryHeader header;
	v3 pos;
	v3 d1;
	v3 d2;
	v4 color;
};

struct EntryTriangles
{
	RenderGroupEntryHeader header;
	u32 vertexCount;
	u32 offset;
};

struct EntryClear
{
	RenderGroupEntryHeader header;
	v4 color;
};

struct EntryBitmap
{
	RenderGroupEntryHeader header;
	v3 pos;
	Vector3Basis basis;
	Bitmap *bitmap;
};
struct EntryTexturedQuadrilateral
{
	RenderGroupEntryHeader header;
	union
	{
		struct
		{
			v3 p1;
			v3 p2;
			v3 p3;
			v3 p4;
		};
		v3 p[4];
	};
	Bitmap *bitmap;

	v4 color;
};

struct EntryUpdateBitmap
{
	RenderGroupEntryHeader header;
	Bitmap bitmap;
};


struct RenderGroup
{	
	EntryTriangles* currentTriangles;
	RenderCommands *commands;
	AssetHandler *assetHandler;
};

static RenderGroup InitRenderGroup(AssetHandler *assetHandler, RenderCommands *comands)
{
	Assert(comands);
	Assert(assetHandler);
	RenderGroup ret;
	ret.assetHandler = assetHandler;
	ret.currentTriangles = NULL;
	ret.commands = comands;
	return ret;
}

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
	rg->currentTriangles = NULL;
	return result;
}


static void ClearPushBuffer(RenderGroup *rg)
{
	rg->commands->pushBufferSize = 0;
}

static v4 SRGB255ToLinear1(v4 input)
{
	v4 result = input * (1.0f / 255.0f);
	result = result * result;
	return result;
}
static v4 Linear1ToSRGB255(v4 input)
{
	v4 result = V4(Sqrt(input.a), Sqrt(input.r), Sqrt(input.g), Sqrt(input.b));
	result = result;

	return result;
}

static void PushProjectivTransform(RenderGroup *rg, v3 cameraPos, Vector3Basis cameraBasis, f32 aspectRatio, float focalLength)
{
	EntryTransform *transform = PushRenderEntry(EntryTransform);

	if (transform)
	{
		
		m4x4 proj = Projection(aspectRatio, focalLength);
		m4x4 cam = CameraTransform(cameraBasis.d1, cameraBasis.d2, cameraBasis.d3, cameraPos);
		transform->m = proj * cam;
	}
}

static void PushOrthogonalTransform(RenderGroup *rg, float width, float height)
{
	EntryTransform *transform = PushRenderEntry(EntryTransform);
	if (transform)
	{
		m4x4 ortho = Orthogonal(width, height);

		transform->m = Translate(ortho, V3(-1.0f, 1.0f, 0.0f));
	}
}

static void PushLine(RenderGroup *rg, v2 p1, v2 p2)
{
	EntryLine *result = PushRenderEntry(EntryLine);
	if (result)
	{
		result->p1 = i12(p1);
		result->p2 = i12(p2);
		result->color = V4(1, 1, 1, 1);
	}
}
static void PushLine(RenderGroup *rg, v3 p1, v3 p2)
{
	EntryLine *result = PushRenderEntry(EntryLine);
	if (result)
	{
		result->p1 = p1;
		result->p2 = p2;
		result->color = V4(1, 1, 1, 1);
	}
}

static ColoredVertex *GetVertexMemory(RenderGroup *rg, u32 amount)
{ 
	Assert(rg->currentTriangles);
	if (rg->commands && rg->commands->vertexCount + amount < rg->commands->MaxVertexCount)
	{
		ColoredVertex *ret = rg->commands->vertexArray + rg->commands->vertexCount;
		rg->currentTriangles->vertexCount += amount;
		rg->commands->vertexCount += amount;
		return ret;
	}
	return NULL;
}

static void PushTriangle(RenderGroup *rg, v3 p1, v3 p2, v3 p3, v4 color = V4(1, 1, 1, 1))
{
	if (!rg->currentTriangles)
	{
		 EntryTriangles *triangles = PushRenderEntry(EntryTriangles);

		 triangles->offset = rg->commands->vertexCount;
		 triangles->vertexCount = 0;
		 rg->currentTriangles = triangles;
	}

	ColoredVertex *vert = GetVertexMemory(rg, 3);
	if (vert)
	{
		vert[0].pos = p1;
		vert[1].pos = p2;
		vert[2].pos = p3;

		vert[0].color = Pack4x8(color);
		vert[1].color = Pack4x8(color);
		vert[2].color = Pack4x8(color);
	}	
}

static void PushTriangle(RenderGroup *rg, v3 p1, v3 p2, v3 p3, v3 color)
{
	PushTriangle(rg, p1, p2, p3, V4(1.0f, color));
}


static void PushOutlinedTriangle(RenderGroup *rg, v3 pos, v3 d1, v3 d2, v4 color)
{
	PushTriangle(rg, pos, d1, d2, color);
	PushLine(rg, pos, pos + d1);
	PushLine(rg, pos, pos + d2);
	PushLine(rg, pos + d1, pos + d2);
}

static void PushCuboid(RenderGroup *rg, v3 pos, Vector3Basis basis, v4 color = V4(1, 1, 1, 1))
{	

	v3 d1 = basis.d1;
	v3 d2 = basis.d2;
	v3 d3 = basis.d3;

	v3 OpPos = pos + d1 + d2;

	v4 black = color;//V4(1.0f, 0.0f, 0.0f, 0.0f);

	v3 p = pos;
	v3 p1 = pos + d1;
	v3 p2 = pos + d2;
	v3 p3 = pos + d3;
	v3 p12 = pos + d1 + d2;
	v3 p13 = pos + d1 + d3;
	v3 p23 = pos + d2 + d3;
	v3 p123 = pos + d1 + d2 + d3;


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


static void PushBitmap(RenderGroup *rg, Bitmap *bitmap, v3 pos)
{
	EntryBitmap *result = PushRenderEntry(EntryBitmap);
	if (result)
	{
		result->bitmap = bitmap;
		result->pos = pos;
		result->basis.d1 = V3((float)bitmap->width, 0, 0);
		result->basis.d2 = V3(0, (float)bitmap->height, 0);
		result->basis.d3 = V3(0, 0, 0);
	}
}
static void PushBitmap(RenderGroup *rg, Bitmap *bitmap, v3 pos, Vector3Basis basis)
{
	EntryBitmap *result = PushRenderEntry(EntryBitmap);
	if (result)
	{
		result->pos = pos;
		result->bitmap = bitmap;
		result->basis = { basis.d1, basis.d2, basis.d3 };
	}
}
static void PushUpdateBitmap(RenderGroup *rg, Bitmap bitmap)
{
	EntryUpdateBitmap *result = PushRenderEntry(EntryUpdateBitmap);
	if (result)
	{
		result->bitmap = bitmap;
	}
}
static void PushAsset(RenderGroup *rg, v3 pos, AssetId assetId, u32 assetIndex)
{
	Asset *tileAsset = rg->assetHandler->GetAsset(assetId);
	if (tileAsset)
	{
		PushBitmap(rg, tileAsset->sprites[assetIndex], pos);
	}
}
static void PushAsset(RenderGroup *rg, v3 pos, AssetId assetId, u32 assetIndex, Vector3Basis basis)
{
	Asset *tileAsset = rg->assetHandler->GetAsset(assetId);
	if (tileAsset)
	{
		PushBitmap(rg, tileAsset->sprites[assetIndex], pos, basis);
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

static void PushRectangle(RenderGroup *rg, v3 pos, v3 vec1, v3 vec2, v4 color)
{
	EntryTexturedQuadrilateral *result = PushRenderEntry(EntryTexturedQuadrilateral);
	if (result)
	{
		result->p1 = pos;
		result->p2 = pos + vec1;
		result->p3 = pos + vec2;
		result->p4 = pos + vec1 + vec2;
		result->color = color;
		result->bitmap = rg->assetHandler->GetWhite();
	}
}
static void PushRectangle(RenderGroup *rg, v2 pos, v2 vec1, v2 vec2, v4 color)
{
	EntryTexturedQuadrilateral *result = PushRenderEntry(EntryTexturedQuadrilateral);
	if (result)
	{
		result->p1 = i12(pos);
		result->p2 = i12(pos + vec1);
		result->p3 = i12(pos + vec2);
		result->p4 = i12(pos + vec1 + vec2);
		result->color = color;
		result->bitmap = rg->assetHandler->GetWhite();
	}
}
static void PushRectangle(RenderGroup *rg, v2 pos, float width, float height, v4 color)
{
	EntryTexturedQuadrilateral *result = PushRenderEntry(EntryTexturedQuadrilateral);
	if (result)
	{
		v2 vec1 = V2(width, 0.0f);
		v2 vec2 = V2(0.0f, height);

		result->p1 = i12(pos);
		result->p2 = i12(pos + vec1);
		result->p3 = i12(pos + vec2);
		result->p4 = i12(pos + vec1 + vec2);
		result->color = color;
		result->bitmap = rg->assetHandler->GetWhite();
	}
}
static void PushCenteredRectangle(RenderGroup *rg, v2 pos, float width, float height, v4 color)
{
	EntryTexturedQuadrilateral *result = PushRenderEntry(EntryTexturedQuadrilateral);
	if (result)
	{
		v2 vec1 = V2(width, 0.0f);    
		v2 vec2 = V2(0.0f, height);

		result->p1 = i12(pos - 0.5f * (vec1 + vec2));
		result->p2 = i12(pos + vec1 - 0.5f * (vec1 + vec2));
		result->p3 = i12(pos + vec2 - 0.5f * (vec1 + vec2));
		result->p4 = i12(pos + vec1 + vec2 - 0.5f * (vec1 + vec2));
		result->color = color;
		result->bitmap = rg->assetHandler->GetWhite();
	}
}
static void PushQuadrilateral(RenderGroup *rg, v3 p1, v3 p2, v3 p3, v3 p4, v4 color)
{
	EntryTexturedQuadrilateral *result = PushRenderEntry(EntryTexturedQuadrilateral);
	if (result)
	{
		result->p1 = p1;
		result->p2 = p2;
		result->p3 = p3;
		result->p4 = p4;
		result->color = color;
		result->bitmap = rg->assetHandler->GetWhite();
	}
}
static void PushTexturedQuadrilateral(RenderGroup *rg, v3 p1, v3 p2, v3 p3, v3 p4, v4 color, AssetId assetId, u32 assetIndex)
{
	EntryTexturedQuadrilateral *result = PushRenderEntry(EntryTexturedQuadrilateral);
	if (result)
	{
		result->p1 = p1;
		result->p2 = p2;
		result->p3 = p3;
		result->p4 = p4;
		result->color = color;
		result->bitmap = rg->assetHandler->GetAsset(assetId)->sprites[assetIndex];
	}
}
static void PushDebugString(RenderGroup *rg, v3 pos, char* string, u32 stringSize, u32 size)
{
	v3 writePos = pos;

	float fSize = (float)size;

	for (u32 i = 0; i < stringSize; i++)
	{
		FontEnum charId = GetCharIndex(string[i]);
		if (charId < Font_InvalideID)
		{
			Vector3Basis basis;
			basis.d1 = V3(fSize, 0, 0);
			basis.d2 = V3(0, fSize, 0);
			basis.d3 = V3(0, 0, 0);
			PushAsset(rg, writePos, { Asset_Font }, charId, basis);
			float actualFloatWidth = FontGetActualFloatWidth(charId, (float)size);
			writePos += V3(actualFloatWidth, 0, 0);
		}
		else
		{
			Assert(!"Not handled font symbol");
		}
	}
}
static void PushDebugString(RenderGroup *rg, v3 pos, String string, u32 size)
{
	PushDebugString(rg, pos, string.string, string.size, size);
}

static void PushButton(RenderGroup *rg, v2 pos, float width, float height, String text, v4 color)
{
	PushRectangle(rg, i12(pos), V3(width, 0, 0), V3(0, height, 0), color);
	PushDebugString(rg, V3(pos, 1.0f), text, 20);
}
static void PushUnit(RenderGroup *rg, v3 pos, u32 facingDirection, u32 assetId, float radius)
{
	AssetId id;
	id.id = assetId;
	Asset *unitAsset = rg->assetHandler->GetAsset(id);
	if (unitAsset)
	{
		Vector3Basis basis = {};
		basis.d1 = V3(radius, 0, -0.2f);
		basis.d2 = V3(0, radius, -0.2f);
		basis.d3 = V3(0, 0, 1);
		basis = Scale(basis, 2.0f);

		PushBitmap(rg, unitAsset->sprites[facingDirection], pos - V3(radius, radius, 0), basis);
	}
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

		PushBitmap(rg, unitAsset->sprites[0], pos - V3(radius, radius, 0), basis);
	}

}

#endif
