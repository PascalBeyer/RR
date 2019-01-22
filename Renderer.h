#ifndef RR_RENDERER
#define RR_RENDERER

// this is kind of a Platform thing, thats why it was in buffers.h...
struct RenderCommands
{
	u32 maxBufferSize;
	u32 pushBufferSize;
	u8 *pushBufferBase;
   
	u32 width;
	u32 height;
	f32 aspectRatio;
   
};

enum RenderGroupEntryType
{
	RenderGroup_EntryChangeRenderSetup,
   RenderGroup_EntryClear,
	RenderGroup_EntryTexturedQuads,
	RenderGroup_EntryLines,
	RenderGroup_EntryTriangles,
	RenderGroup_EntryTriangleMesh,
};

enum OpenGLShaderFlags : u32
{
   ShaderFlags_None           = 0x0,
   ShaderFlags_Textured       = 0x1,
   ShaderFlags_ShadowMapping  = 0x2,
   ShaderFlags_Phong          = 0x4,
   ShaderFlags_Animated       = 0x8,
   ShaderFlags_ZBias          = 0x10,
   ShaderFlags_MultiTextured  = 0x20,
   
   ShaderArray_Size           = 0x40, // right now this does not have to be a hash map I guess..
};

// not sure if we should try to make this as small as possible and calculate more later
struct RenderSetup 
{
	u32 flags;
   b32 isProjective; // todo we could actually join this with the flags, if we just make it real big, bigger then size.
   m4x4 projection;
	m4x4 cameraTransform;
   m4x4 shadowMat;
   v3 lightP;
   v3 cameraP;
};

struct RenderGroupEntryHeader
{
	RenderGroupEntryType type;
};

struct EntryChangeRenderSetup
{
   RenderGroupEntryHeader header;
   RenderSetup setup;
};

struct EntryColoredVertices
{
	RenderGroupEntryHeader header;
   VertexFormatPC *data;
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
   u32 vertexFormatSize;
   
   u32 meshType;
   m4x4 objectTransform;
   v4 scaleColor;
   m4x4Array boneStates;
   
   IndexSetArray indexSets;
   u32Array textureIDs;
   
   u32 vertexVBO;
	u32 indexVBO;
};

struct EntryTexturedQuads
{
	RenderGroupEntryHeader header;
   VertexFormatPCUI *vertexBuffer;
   u16 *indexBuffer;
   
	u32 count;
   u32 maxAmount;
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

static void PushOrthogonalSetup(RenderGroup *rg, u32 flags)
{
   TimedBlock;
   if(rg->setup.isProjective == false  && rg->setup.flags == flags) return;
   
   RenderCommands *commands = rg->commands;
   EntryChangeRenderSetup *entry = PushRenderEntry(EntryChangeRenderSetup);
   RenderSetup *setup = &entry->setup;
   
   {
      m4x4 ortho = Orthogonal(1.0f, 1.0f);
      
      setup->projection = Translate(ortho, V3(-1.0f, 1.0f, 0.0f));
      setup->cameraTransform = Identity();
      setup->isProjective = false;
   }
   
   // todo reuse the rest of the memory!
   rg->currentLines = NULL;
   rg->currentTriangles = NULL;
   rg->currentQuads = NULL;
   
   setup->flags = flags;
   rg->setup = *setup;
}


static void PushProjectiveSetup(RenderGroup *rg, Camera camera, LightSource lightSource, u32 flags)
{
   TimedBlock;
   
   // todo  this is fine for now, but if we doit like this we should have frame data, like camera pos and lightsource.... it does not really make sense to pass them every time.
   if(rg->setup.isProjective && rg->setup.flags == flags) return;
   
   RenderCommands *commands = rg->commands;
   EntryChangeRenderSetup *entry = PushRenderEntry(EntryChangeRenderSetup);
   RenderSetup *setup = &entry->setup;
   
   {//projective Codes
      setup->projection = Projection(commands->aspectRatio, camera.focalLength);
      setup->cameraTransform = CameraTransform(camera.orientation, camera.pos);;
      setup->lightP = lightSource.pos;
      setup->shadowMat = CameraTransform(lightSource.orientation, lightSource.pos);
      setup->cameraP = camera.pos;
      setup->isProjective = true;
   }
   
   rg->currentLines = NULL;
   rg->currentTriangles = NULL;
   rg->currentQuads = NULL;
   
   setup->flags = flags;
   rg->setup = *setup;
}

static VertexFormatPC *GetVertexMemory(RenderGroup *rg, RenderGroupEntryType type)
{ 
   switch (type)
   {
      case RenderGroup_EntryLines:
      {
         if (!rg->currentLines)
         {
            EntryColoredVertices *lines = PushRenderEntryType(EntryColoredVertices, EntryLines);
            lines->maxAmount = 5536;
            lines->data = PushData(frameArena, VertexFormatPC, lines->maxAmount);
            lines->vertexCount = 0;
            
            rg->currentLines = lines;
         }
         
         u32 amount = 2;
         auto currentVertices = rg->currentLines;
         if (currentVertices->vertexCount + amount < currentVertices->maxAmount)
         {
            VertexFormatPC *ret = currentVertices->data + currentVertices->vertexCount;
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
            triangles->maxAmount = 5536;
            triangles->data = PushData(frameArena, VertexFormatPC, triangles->maxAmount);
            triangles->vertexCount = 0;
            
            rg->currentTriangles = triangles;
         }
         
         u32 amount = 3;
         auto currentVertices = rg->currentTriangles;
         if (currentVertices->vertexCount + amount < currentVertices->maxAmount)
         {
            VertexFormatPC *ret = currentVertices->data + currentVertices->vertexCount;
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
   
   VertexFormatPC *vert = GetVertexMemory(rg, RenderGroup_EntryTriangles);
   vert[0].p = p1;
   vert[1].p = p2;
   vert[2].p = p3;
   
   vert[0].c = c1;
   vert[1].c = c2;
   vert[2].c = c3;
   
}
static void PushTriangle(RenderGroup *rg, VertexFormatPC cv1, VertexFormatPC cv2, VertexFormatPC cv3)
{
   PushTriangle(rg, cv1.p, cv2.p, cv3.p, cv1.c, cv2.c, cv3.c);
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

static void PushTriangleMesh(RenderGroup *rg, TriangleMesh *mesh, Quaternion orientation, v3 pos, f32 scale, v4 scaleColor)
{
   if (!mesh) return; // todo think about this, we want this if we redo our mesh write
   
   EntryTriangleMesh *meshHeader = PushRenderEntry(EntryTriangleMesh);
   u32Array arr = PushArray(frameArena, u32, mesh->indexSets.amount);
   
   // todo this still feels pretty stupid
   for (u32 i = 0; i < arr.amount; i++)
   {
      arr[i] = GetTexture(rg->assetHandler, mesh->indexSets[i].mat.bitmapID).index;
   }
   
   // todo most of these could be stored on the asset entry. But maybe this should just be on there for cache
   // this depeds whether I usually access these per id or per mesh, per id we might be able to only get the 
   // asset entry. We might be able to get rid of meshes that way.
   meshHeader->vertexFormatSize = mesh->vertexFormatSize;
   meshHeader->vertexVBO  = mesh->vertexVBO;
   meshHeader->indexVBO   = mesh->indexVBO;
   
   meshHeader->indexSets = mesh->indexSets;
   meshHeader->textureIDs = arr;
   
   meshHeader->objectTransform = InterpolationDataToMatrix(pos, orientation, scale);
   meshHeader->scaleColor = scaleColor;
   meshHeader->meshType   = mesh->type;
   meshHeader->boneStates = {};
}

static void PushTriangleMesh(RenderGroup *rg, u32 meshId, Quaternion orientation, v3 pos, f32 scale, v4 scaleColor)
{
   TriangleMesh *mesh = GetMesh(rg->assetHandler, meshId);
   
   PushTriangleMesh(rg, mesh, orientation, pos, scale, scaleColor);
}

// todo unify this with pushTriangle mesh.
static void PushAnimatedMesh(RenderGroup *rg, TriangleMesh *mesh, Quaternion orientation, v3 pos, f32 scale, v4 scaleColor, m4x4Array boneStates)
{
   if (!mesh) return; // todo think about this, we want this if we redo our mesh write
   
   EntryTriangleMesh *meshHeader = PushRenderEntry(EntryTriangleMesh);
   u32Array arr = PushArray(frameArena, u32, mesh->indexSets.amount);
   for (u32 i = 0; i < arr.amount; i++)
   {
      arr[i] = GetTexture(rg->assetHandler, mesh->indexSets[i].mat.bitmapID).index;
   }
   
   meshHeader->vertexFormatSize = mesh->vertexFormatSize;
   meshHeader->vertexVBO = mesh->vertexVBO;
   meshHeader->indexVBO = mesh->indexVBO;
   
   meshHeader->indexSets = mesh->indexSets;
   meshHeader->textureIDs = arr;
   
   meshHeader->objectTransform =InterpolationDataToMatrix(pos, orientation, scale);
   meshHeader->scaleColor = scaleColor;
   
   meshHeader->boneStates = boneStates;
   meshHeader->meshType   = mesh->type;
}

static void PushAnimatedMesh(RenderGroup *rg, u32 meshId, Quaternion orientation, v3 pos, f32 scale, v4 scaleColor, m4x4Array boneStates)
{
   PushAnimatedMesh(rg, GetMesh(rg->assetHandler, meshId), orientation, pos, scale, scaleColor, boneStates);
}

static void PushAnimatedMeshImmidiet(RenderGroup *rg, TriangleMesh *mesh, Quaternion orientation, v3 pos, f32 scale, v4 scaleColor, m4x4Array boneStates)
{
   Skeleton *skeleton = &mesh->skeleton;
   
#if 1
   v3Array out = PushArray(frameArena, v3, mesh->positions.amount);
   
   for (u32 i = 0; i < mesh->positions.amount; i++)
   {
      u32 unflattend = skeleton->vertexMap[i];
      WeightDataArray weights = skeleton->vertexToWeightsMap[unflattend];
      
      out[i] = V3();
      
      v4 v = V4(mesh->positions[i], 1.0f);
      
      For(weights)
      {
         out[i] += it->weight * (boneStates[it->boneIndex] * v).xyz;
      }
   }
   
   for(u32 i = 0; i < mesh->positions.amount; i++)
   {
      u32 i0 = mesh->indices[i + 0];
      u32 i1 = mesh->indices[i + 1];
      u32 i2 = mesh->indices[i + 2];
      
      PushTriangle(rg, out[i0], out[i1], out[i2]);
   }
   
   
   
#else
   
   for (u32 i = 0; i < mesh->indices.amount; i += 3)
   {
      u16 i1 = mesh->indices[i + 0];
      u16 i2 = mesh->indices[i + 1];
      u16 i3 = mesh->indices[i + 2];
      
      VertexFormatPCUNBD x1 = mesh->dumbDebugPointer[i1];
      VertexFormatPCUNBD x2 = mesh->dumbDebugPointer[i2];
      VertexFormatPCUNBD x3 = mesh->dumbDebugPointer[i3];
      
      v3 p1 = V3();
      {
         v4 boneWeights = x1.bw;
         v4i boneIndices = x1.bi;
         v4 inputVertex = V4(x1.p, 1.0f);
         
         p1 += boneWeights.x * (boneStates[boneIndices.x] * inputVertex).xyz;
         p1 += boneWeights.y * (boneStates[boneIndices.y] * inputVertex).xyz;
         p1 += boneWeights.z * (boneStates[boneIndices.z] * inputVertex).xyz;
         p1 += boneWeights.w * (boneStates[boneIndices.w] * inputVertex).xyz;
      }
      v3 p2 = V3();
      {
         v4 boneWeights = x2.bw;
         v4i boneIndices = x2.bi;
         v4 inputVertex = V4(x2.p, 1.0f);
         
         p2 += boneWeights.x * (boneStates[boneIndices.x] * inputVertex).xyz;
         p2 += boneWeights.y * (boneStates[boneIndices.y] * inputVertex).xyz;
         p2 += boneWeights.z * (boneStates[boneIndices.z] * inputVertex).xyz;
         p2 += boneWeights.w * (boneStates[boneIndices.w] * inputVertex).xyz;
      }
      v3 p3 = V3();
      {
         v4 boneWeights = x3.bw;
         v4i boneIndices = x3.bi;
         v4 inputVertex = V4(x3.p, 1.0f);
         
         p3 += boneWeights.x * (boneStates[boneIndices.x] * inputVertex).xyz;
         p3 += boneWeights.y * (boneStates[boneIndices.y] * inputVertex).xyz;
         p3 += boneWeights.z * (boneStates[boneIndices.z] * inputVertex).xyz;
         p3 += boneWeights.w * (boneStates[boneIndices.w] * inputVertex).xyz;
      }
      
      PushTriangle(rg, p1, p2, p3, scaleColor);
   }
#endif
   
}

static void PushLine(RenderGroup *rg, v3 p1, v3 p2, u32 color = 0xFFFFFFFF)
{
   VertexFormatPC *vert = GetVertexMemory(rg, RenderGroup_EntryLines);
   if (vert)
   {
      vert[0].p = p1;
      vert[1].p = p2;
      
      vert[0].c = color;
      vert[1].c = color;
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

static VertexFormatPCUI *GetTexturedQuadMemory(RenderGroup *rg)
{
   if (!rg->currentQuads)
   {
      EntryTexturedQuads *quads = PushRenderEntry(EntryTexturedQuads);
      if(quads)
      {
         quads->maxAmount    = 1000;
         quads->vertexBuffer = PushData(frameArena, VertexFormatPCUI, 4u * quads->maxAmount);
         quads->indexBuffer  = PushData(frameArena, u16, 6u * quads->maxAmount);
         quads->count  = 0;
         
         rg->currentQuads = quads;
      }
      else
      {
         Die;
         return NULL;
      }
   }
   
   
   if (rg->currentQuads->count < rg->currentQuads->maxAmount)
   {
      VertexFormatPCUI *verts = rg->currentQuads->vertexBuffer + 4u * rg->currentQuads->count;
      
      u16 vertsAt = (u16)(4u * rg->currentQuads->count);
      Assert(vertsAt == (4u * rg->currentQuads->count));
      u32 indexAt = 6u * rg->currentQuads->count;
      
      u16 *indices = rg->currentQuads->indexBuffer + indexAt;
      *indices++ = (vertsAt + 0);
      *indices++ = (vertsAt + 1);
      *indices++ = (vertsAt + 2);
      
      *indices++ = (vertsAt + 2);
      *indices++ = (vertsAt + 1);
      *indices++ = (vertsAt + 3);
      
      rg->currentQuads->count++;
      return verts;
   }
   else
   {
      rg->currentQuads = NULL;
      return GetTexturedQuadMemory(rg);
   }
}

static void PushTexturedQuad(RenderGroup *rg, v3 p1, v3 p2, v3 p3, v3 p4, TextureIndex textureIndex, bool inverted = false,  v2 minUV = {0, 0}, v2 maxUV = {1, 1}, u32 c = 0xFFFFFFFF)
{
   VertexFormatPCUI *verts = GetTexturedQuadMemory(rg);
   if(!verts) return;
   
   verts[0].p = p1;
   verts[1].p = p2;
   verts[2].p = p3;
   verts[3].p = p4;
   
   if (inverted)
   {
      verts[0].uv = V2(minUV.x, minUV.y); // 0 0 
      verts[1].uv = V2(maxUV.x, minUV.y); // 1 0
      verts[2].uv = V2(minUV.x, maxUV.y); // 0 1
      verts[3].uv = V2(maxUV.x, maxUV.y); // 1 1
   }
   else
   {
      verts[0].uv = V2(minUV.x, maxUV.y); // 0 1
      verts[1].uv = V2(maxUV.x, maxUV.y); // 1 1
      verts[2].uv = V2(minUV.x, minUV.y); // 0 0
      verts[3].uv = V2(maxUV.x, minUV.y); // 1 0
      
   }
   
   verts[0].c = c;
   verts[1].c = c;
   verts[2].c = c;
   verts[3].c = c;
   
   u16 castTextureIndex = (u16)textureIndex.index;
   Assert(castTextureIndex == textureIndex.index);
   
   verts[0].textureIndex = castTextureIndex;
   verts[1].textureIndex = castTextureIndex;
   verts[2].textureIndex = castTextureIndex;
   verts[3].textureIndex = castTextureIndex;
}

static void PushTexturedQuad(RenderGroup *rg, v3 p1, v3 p2, v3 p3, v3 p4, u32 textureId, bool inverted = false,  v2 minUV = {0, 0}, v2 maxUV = {1, 1}, v4 color = {1, 1, 1, 1})
{
   TextureIndex textureIndex = GetTexture(rg->assetHandler, textureId);
   PushTexturedQuad(rg, p1, p2, p3, p4, textureIndex, inverted, minUV, maxUV, Pack4x8(color));
}

static void PushTexturedRect(RenderGroup *rg, v2 p1, f32 width, f32 height, TextureIndex textureIndex,  bool inverted = false, v2 minUV = {0, 0}, v2 maxUV = {1, 1}, v4 color = {1, 1, 1, 1})
{
   v2 p2 = p1 + V2(width, 0);
   v2 p3 = p1 + V2(0, height);
   v2 p4 = p1 + V2(width, height);
   PushTexturedQuad(rg, i12(p1), i12(p2), i12(p3), i12(p4), textureIndex, inverted, minUV, maxUV, Pack4x8(color));
}

static void PushTexturedRect(RenderGroup *rg, v2 p1, f32 width, f32 height, u32 textureId,  bool inverted = false, v2 minUV = {0, 0}, v2 maxUV = {1, 1}, v4 color = {1, 1, 1, 1})
{
   TextureIndex textureIndex = GetTexture(rg->assetHandler, textureId);
   
   v2 p2 = p1 + V2(width, 0);
   v2 p3 = p1 + V2(0, height);
   v2 p4 = p1 + V2(width, height);
   PushTexturedQuad(rg, i12(p1), i12(p2), i12(p3), i12(p4), textureIndex, inverted, minUV, maxUV, Pack4x8(color));
}

static void PushQuadrilateral(RenderGroup *rg, v3 p1, v3 p2, v3 p3, v3 p4, u32 c)
{
   TextureIndex textureIndex = GetTexture(rg->assetHandler, rg->assetHandler->whiteTextureId);
   PushTexturedQuad(rg, p1, p2, p3, p4, textureIndex, false, {0, 0}, {1, 1}, c);
}

static void PushRectangle(RenderGroup *rg, v3 pos, v3 vec1, v3 vec2, v4 color)
{
   PushQuadrilateral(rg, pos, pos + vec1, pos + vec2, pos + vec1 + vec2, Pack4x8(color));
}

static void PushRectangle(RenderGroup *rg, v2 min, v2 max, i8 layer, u32 c)
{
   v2 p1 = min;
   v2 p2 = V2(max.x, min.y);
   v2 p3 = V2(min.x, max.y);
   v2 p4 = max;
   
   // only negative exponents, retains sign of layer looses first bit..
   i32 up = (layer << 24) | (1 << 31); 
   f32 Zlayer = *(f32 *)&up;
   
   PushQuadrilateral(rg, V3(p1, Zlayer), V3(p2, Zlayer), V3(p3, Zlayer), V3(p4, Zlayer), c);
}

static void PushRectangle(RenderGroup *rg, v2 min, v2 max, i8 layer, v4 color)
{
   PushRectangle(rg, min, max, layer, Pack4x8(color));
}

static void PushRectangle(RenderGroup *rg, v2 pos, f32 width, f32 height, i8 layer, u32 c)
{
   PushRectangle(rg, pos, pos + V2(width, height), layer, c);
}

static void PushRectangle(RenderGroup *rg, v2 pos, f32 width, f32 height, i8 layer, v4 color)
{
   PushRectangle(rg, pos, pos + V2(width, height), layer, Pack4x8(color));
}

static void PushCenteredRectangle(RenderGroup *rg, v2 pos, float width, float height, i8 layer, v4 color)
{
   v2 vec1 = V2(width, 0.0f);    
   v2 vec2 = V2(0.0f, height);
   
   v2 p1 = (pos - 0.5f * (vec1 + vec2));
   v2 p2 = (pos + vec1 - 0.5f * (vec1 + vec2));
   v2 p3 = (pos + vec2 - 0.5f * (vec1 + vec2));
   v2 p4 = (pos + vec1 + vec2 - 0.5f * (vec1 + vec2));
   
   PushQuadrilateral(rg, pos - 0.5f * V2(width, height), pos + 0.5f * V2(width, height), layer, Pack4x8(color));
}

// exactly used in colorpicker...
static void PushRectangle(RenderGroup *rg, v2 pos, f32 width, f32 height, u32 c1, u32 c2, u32 c3, u32 c4)
{
   v2 p1 = pos;
   v2 p2 = V2(pos.x + width, pos.y);
   v2 p3 = V2(pos.x, pos.y + height);
   v2 p4 = pos + V2(width, height);
   
   PushTriangle(rg, i12(p1), i12(p2), i12(p3), c1, c2, c3);
   PushTriangle(rg, i12(p2), i12(p3), i12(p4), c2, c3, c4);
}


static f32 PushString(RenderGroup *rg, v2 pos, unsigned char* string, u32 stringLength, f32 size, v4 color = V4(1, 1, 1, 1))
{
   TimedBlock;
   
   f32 x = pos.x;
   
   Font font = rg->assetHandler->currentlyLoadedFont;
   
   float fScale = size / (f32)font.charHeight;
   TextureIndex tex = GetTexture(rg->assetHandler, font.textureId);
   
   for(u32 i = 0; i < stringLength; i++)
   {
      if(string[i] >= font.charData.amount)
      {
         Assert(!"Not handled font symbol");
      }
      
      CharData data = font.charData[string[i]];
      
      f32 offSetX = fScale * data.xOff;
      f32 offSetY = fScale * data.yOff;
      
      f32 scaledWidth = fScale * (f32)data.width;
      f32 scaledHeight = fScale * (f32)data.height;
      
      f32 y = pos.y + (f32)size;
      
      v2 writePos = V2(x + offSetX, y + offSetY);
      
      PushTexturedRect(rg, writePos, scaledWidth, scaledHeight, tex, true, data.minUV, data.maxUV);
      float actualFloatWidth = data.xAdvance * fScale;
      x += actualFloatWidth;
      
   }
   
   return (x - pos.x);
}

static f32 PushString(RenderGroup *rg, v2 pos, const char* string, f32 size, v4 color = V4(1, 1, 1, 1))
{
   return PushString(rg, pos, (unsigned char *)string, NullTerminatedStringLength(string), size, color);
}

static f32 PushString(RenderGroup *rg, v2 pos, String string, f32 size, v4 color = V4(1, 1, 1, 1))
{
   return PushString(rg, pos, string.data, string.length, size, color);
}



#endif
