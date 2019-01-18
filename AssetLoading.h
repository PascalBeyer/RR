


struct AssetHandler;
static AssetInfo GetAssetInfo(AssetHandler *handler, u32 id);


struct UVList
{
	u16 uvIndex;
	u16 flattendIndex;
	u16 normalIndex;
	UVList *next;
};
typedef UVList * UVListPtr;
DefineArray(UVListPtr);

static MaterialDynamicArray LoadMTL(String path, String fileName, Arena *arena)
{
	MaterialDynamicArray ret = MaterialCreateDynamicArray(globalAlloc);
	DeferRestore(frameArena);
   
	File file = LoadFile(FormatCString("%s%s\0", path, fileName), frameArena);
	Assert(file.fileSize);
	String string = CreateString((Char *)file.memory, file.fileSize);
   
	b32 success = true;
   
	while (string.length)
	{
		Material cur = {};
		String line = ConsumeNextLineSanitize(&string);
		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }
      
		// newmtl
		{
			String ident = EatToNextSpaceReturnHead(&line);
			Assert(ident == "newmtl");
			EatSpaces(&line);
         
			cur.name = CopyString(line, arena);
		}
      
		line = ConsumeNextLineSanitize(&string);
		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }
      
		// Ns
		{
			String ident = EatToNextSpaceReturnHead(&line);
			Assert(ident == "Ns");
			EatSpaces(&line);
			cur.specularExponent = StoF(line, &success);
		}
      
		line = ConsumeNextLineSanitize(&string);
		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }
      
		// Ka
		{
			String ident = EatToNextSpaceReturnHead(&line);
			Assert(ident == "Ka");
			EatSpaces(&line);
			String head = EatToNextSpaceReturnHead(&line);
			f32 a1 = StoF(head, &success);
         
			EatSpaces(&line);
			head = EatToNextSpaceReturnHead(&line);
			f32 a2 = StoF(head, &success);
         
			EatSpaces(&line);
			head = EatToNextSpaceReturnHead(&line);
			f32 a3 = StoF(head, &success);
         
			cur.ka = V3(a1, a2, a3);
		}
      
		line = ConsumeNextLineSanitize(&string);
		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }
      
		// Kd
		{
			String ident = EatToNextSpaceReturnHead(&line);
			Assert(ident == "Kd");
			EatSpaces(&line);
			String head = EatToNextSpaceReturnHead(&line);
			f32 a1 = StoF(head, &success);
         
			EatSpaces(&line);
			head = EatToNextSpaceReturnHead(&line);
			f32 a2 = StoF(head, &success);
         
			EatSpaces(&line);
			head = EatToNextSpaceReturnHead(&line);
			f32 a3 = StoF(head, &success);
         
			cur.kd = V3(a1, a2, a3);
		}
      
		line = ConsumeNextLineSanitize(&string);
		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }
      
		// Ks
		{
			String ident = EatToNextSpaceReturnHead(&line);
			Assert(ident == "Ks");
			EatSpaces(&line);
			String head = EatToNextSpaceReturnHead(&line);
			f32 a1 = StoF(head, &success);
         
			EatSpaces(&line);
			head = EatToNextSpaceReturnHead(&line);
			f32 a2 = StoF(head, &success);
         
			EatSpaces(&line);
			head = EatToNextSpaceReturnHead(&line);
			f32 a3 = StoF(head, &success);
         
			cur.ks = V3(a1, a2, a3);
		}
      
		line = ConsumeNextLineSanitize(&string);
		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }
      
		// Ke
		{
			String ident = EatToNextSpaceReturnHead(&line);
			Assert(ident == "Ke");
			EatSpaces(&line);
			String head = EatToNextSpaceReturnHead(&line);
			f32 a1 = StoF(head, &success);
         
			EatSpaces(&line);
			head = EatToNextSpaceReturnHead(&line);
			f32 a2 = StoF(head, &success);
         
			EatSpaces(&line);
			head = EatToNextSpaceReturnHead(&line);
			f32 a3 = StoF(head, &success);
         
			cur.ke = V3(a1, a2, a3);
		}
      
		line = ConsumeNextLineSanitize(&string);
		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }
      
		// Ni
		{
			String ident = EatToNextSpaceReturnHead(&line);
			Assert(ident == "Ni");
			EatSpaces(&line);
			String head = EatToNextSpaceReturnHead(&line);
			f32 a1 = StoF(head, &success);
         
			cur.indexOfRefraction = a1;
		}
      
		line = ConsumeNextLineSanitize(&string);
		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }
      
		// d
		{
			String ident = EatToNextSpaceReturnHead(&line);
			Assert(ident == "d");
			EatSpaces(&line);
			String head = EatToNextSpaceReturnHead(&line);
			f32 a1 = StoF(head, &success);
         
			cur.dissolved = a1;
		}
      
		line = ConsumeNextLineSanitize(&string);
		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }
      
		// illum
		{
			String ident = EatToNextSpaceReturnHead(&line);
			Assert(ident == "illum");
			EatSpaces(&line);
			String head = EatToNextSpaceReturnHead(&line);
			u32 a1 = StoU(head, &success);
         
			cur.illuminationModel = a1;
		}
      
		line = ConsumeNextLineSanitize(&string);
		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }
      
		// map_Kd
		{
			String ident = EatToNextSpaceReturnHead(&line);
			Assert(ident == "map_Kd");
			EatSpaces(&line);
			String tail = EatToCharFromBackReturnTail(&line, '\\');
			cur.texturePath = CopyString(CreateString(FormatCString("%s.texture", GetToChar(tail, '.'))), arena);
         
			//if(assetHandler) cur.bitmapID = RegisterAsset(assetHandler, Asset_Texture, (char *)cur.texturePath.data); ????
		}
      
		ArrayAdd(&ret, cur);
	}
   
	Assert(success);
   
	return ret;
}

static String GetDirectioryFromFilePath(String fileName)
{
	u32 positionOfLastSlash = 0;
	for (u32 i = fileName.length - 1; i < MAXU32; i--)
	{
		if (fileName[i] == '/')
		{
			positionOfLastSlash = i;
			break;
		}
	}
	if (!positionOfLastSlash)
	{
		return {};
	}
   
	String ret;
	ret.data = fileName.data;
	ret.length = positionOfLastSlash + 1;
	return ret;
}

struct ReadOBJIndex
{
   u16 posIndex;
   u16 uvIndex;
   u16 normalIndex;
};

DefineArray(ReadOBJIndex)

struct ReadOBJIterator
{
   Material mat;
   v3Array p;
   v2Array uv;
   v3Array n;
   ReadOBJIndexArray indices;
};

DefineDFArray(ReadOBJIterator);

static TriangleMesh ReadObj(char *fileName, Arena *arena)
{
	String filename = CreateString(fileName);
	String path = GetDirectioryFromFilePath(filename);
   
   
   //DeferReset(frameArena); caller should do that if he wants, so he could also call with arena == frameArena.
   
	File file = LoadFile(fileName, frameArena);
	if (!file.fileSize) return {};
	String string = CreateString((Char *)file.memory, file.fileSize);
   
	// here mtllib "bla.mtl"
	String mtllib = ConsumeNextLineSanitize(&string);
	while (string.length && mtllib.length == 0 || mtllib[0] == '#') { mtllib = ConsumeNextLineSanitize(&string); }
	Assert(string.length);
	String ident = EatToNextSpaceReturnHead(&mtllib);
	Assert(ident == "mtllib");
	EatSpaces(&mtllib);
   
	MaterialDynamicArray materials = LoadMTL(path, mtllib, arena);
	// todo make sure that multiples get handled, i.e make this assets.
   
	b32 success = true;
   
	String line = ConsumeNextLineSanitize(&string);
   
   ReadOBJIteratorDFArray stuff = ReadOBJIteratorCreateDFArray();
   
	//u16PtrDynamicArray indexPointerArray = u16PtrCreateDynamicArray();
   
	while (string.length)
	{
		ReadOBJIterator cur;
      
		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }
      
		// o
		{
			String head = EatToNextSpaceReturnHead(&line);
			Assert(head == "o");
			EatSpaces(&line);
			String o = line; // ignored for now
		}
      
		line = ConsumeNextLineSanitize(&string);
		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }
      
      // todo how broken of a file do we not crash on? We could just crash if the file ends in something we do not expect.
		BeginArray(frameArena, v3, tempVertexArray);
		while (string.length && line[0] == 'v' && line[1] == ' ')
		{
			Eat1(&line);
			EatSpaces(&line);
         f32 f1 = Eatf32(&line, &success);
			EatSpaces(&line);
         f32 f2 = Eatf32(&line, &success);
			EatSpaces(&line);
         f32 f3 = Eatf32(&line, &success);
         
			*PushStruct(frameArena, v3) = V3(f1, f2, f3);
			
			line = ConsumeNextLineSanitize(&string);
		}
		EndArray(frameArena, v3, tempVertexArray);
      
		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }
		
		BeginArray(frameArena, v2, textureCoordinates);
		while (string.length && line[0] == 'v' && line[1] == 't')
		{
			Eat1(&line);
			Eat1(&line);
			EatSpaces(&line);
         f32 f1 = Eatf32(&line, &success);
			EatSpaces(&line);
         f32 f2 = Eatf32(&line, &success);
         
			*PushStruct(frameArena, v2) = V2(f1, f2);
         
			line = ConsumeNextLineSanitize(&string);
		}
		EndArray(frameArena, v2, textureCoordinates);
      
		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }
		
		BeginArray(frameArena, v3, normals);
		while (string.length && line[0] == 'v' && line[1] == 'n')
		{
			Eat1(&line);
			Eat1(&line);
			EatSpaces(&line);
         f32 f1 = Eatf32(&line, &success);
			EatSpaces(&line);
         f32 f2 = Eatf32(&line, &success);
			EatSpaces(&line);
         f32 f3 = Eatf32(&line, &success);
         
			*PushStruct(frameArena, v3) = V3(f1, f2, f3);;
			
			line = ConsumeNextLineSanitize(&string);
		}
		EndArray(frameArena, v3, normals);
      
		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }
      
		// usemtl
		{
			String head = EatToNextSpaceReturnHead(&line);
			Assert(head == "usemtl");
			EatSpaces(&line);
			String name = line;
			For(materials)
			{
				if (it->name == name)
				{
					cur.mat = *it;
					break;
				}
			}
		}
      
		line = ConsumeNextLineSanitize(&string);
		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }
      
		// s -smoothening, what ever that means
		{
			String head = EatToNextSpaceReturnHead(&line);
			Assert(head == "s");
			EatSpaces(&line);
         
			//u32 s = StoU(line, &success); // ignored for now
		}
      
		line = ConsumeNextLineSanitize(&string);
		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }
      
      u32 vertexArrayLength = 0;
      
		BeginArray(frameArena, ReadOBJIndex, indices);
		while (line.length && line[0] == 'f')
		{
			Eat1(&line);
         
			// we assume they are all here
         
			for (u32 parseIndex = 0; parseIndex < 3; parseIndex++)
			{
				EatSpaces(&line);
				String s1 = EatToCharReturnHead(&line, '/');
				Eat1(&line);
				String s2 = EatToCharReturnHead(&line, '/');
				Eat1(&line);
				String s3 = EatToCharReturnHead(&line, ' ');
            
#if 1
            // these come in "absolute".
				u32 pIndex  = StoU(s1, &success);
				u32 uvIndex = StoU(s2, &success);
				u32 nIndex  = StoU(s3, &success);
#else
            
				u32 u1 = StoU(s1, &success) - amountOfVertsBefore;	// this has to be relative to be an index, but also saved absolute wrt the flattend array
				u32 u2 = StoU(s2, &success) - amountOfUVsBefore;		// this is relative as we just need it to build our array
				u32 u3 = StoU(s3, &success) - amountOfNormalsBefore;	// this is also relative 
#endif
            ReadOBJIndex *fill =  PushStruct(frameArena, ReadOBJIndex);
            fill->normalIndex = (u16)nIndex;
            fill->uvIndex     = (u16)uvIndex;
            fill->posIndex    = (u16)pIndex;
			}
         
			line = ConsumeNextLineSanitize(&string);
		}
      
		EndArray(frameArena, ReadOBJIndex, indices);
      
      cur.p = tempVertexArray;
      cur.n = normals;
      cur.uv = textureCoordinates;
      cur.indices = indices;
      
      ArrayAdd(&stuff, cur);
      
#if 0
		amountOfIndeciesBefore += indecies.amount;
		amountOfNormalsBefore += normals.amount;
		amountOfUVsBefore += textrueCoordinates.amount;
      amountOfVertsBefore += tempVertexArray.amount;
#endif
      
	}
   
   // we cant do this in the loop, as arena might be frameArena
   
   BeginArray(arena, IndexSet, indexSets);
   u32 amountOfVerts = 0;
   u32 amountOfIndices = 0;
   For(stuff)
   {
      IndexSet *indexSet = PushStruct(arena, IndexSet);
      indexSet->offset = amountOfIndices;
      indexSet->amount = it->indices.amount;
      indexSet->mat = it->mat;
      amountOfVerts   += it->p.amount;
      amountOfIndices += it->indices.amount;
   }
   EndArray(arena, IndexSet, indexSets);
   
   
   u16Array    indices = PushArray(arena, u16, amountOfIndices);
   UVListPtrArray flatten = PushArray(frameArena, UVListPtr, amountOfVerts);
   u32 flattener = 0;
   u32 iterator = 0;
   
   For(s, stuff)
   {
      For(base, s->indices)
      {
         u16 pI  = base->posIndex - 1;
         u16 nI  = base->normalIndex - 1;
         u16 uvI = base->uvIndex - 1;
         
         b32 found = false;
         u16 index = 0;
         
         for (UVList *it = flatten[pI]; it; it = it->next)
         {
            if (it->normalIndex == nI && it->uvIndex == uvI)
            {
               found = true;
               index = it->flattendIndex;
               break;
            }
         }
         
         if (!found)
         {
            UVList *append = PushStruct(frameArena, UVList);
            append->flattendIndex = (u16)flattener++;
            append->next = flatten[pI];
            append->normalIndex = (u16)nI;
            append->uvIndex = (u16)uvI;
            
            flatten[pI] = append;
            
            index = append->flattendIndex;
         }
         
         indices[iterator++] = index;
      }
   }
   
   Assert(flattener < 0xFFFF);
   u32 amountOfFlattendVertices = flattener;
   //Assert(flattener == (u32)((UVList *)frameArena->current - (UVList *)flatten.data));
   
   v3Array positions = PushArray(arena, v3,  amountOfFlattendVertices);
   v3Array normals   = PushArray(arena, v3,  amountOfFlattendVertices);
   v2Array uvs       = PushArray(arena, v2,  amountOfFlattendVertices);
   u32Array colors   = PushArray(arena, u32, amountOfFlattendVertices);
   
   For(base, flatten)
   {
      u32 base_it_index = (u32)(base - flatten.data);
      
      for(UVList *list = *base; list; list = list->next)
      {
         u32 i = list->flattendIndex;
         
         ReadOBJIterator *s = NULL;
         
         u32 pSub = 0;
         u32 nSub = 0;
         u32 uvSub = 0;
         
         For(stuff)
         {
            if(base_it_index - pSub < it->p.amount)
            {
               s = it;
               break;
            }
            else
            {
               pSub += it->p.amount;
               nSub += it->n.amount;
               uvSub += it->uv.amount;
            }
         }
         
         positions[i] = s->p[base_it_index - pSub];
         normals  [i] = s->n[list->normalIndex - nSub];
         uvs      [i] = s->uv[list->uvIndex - uvSub];
         colors   [i] = 0xFFFFFFFF;
      }
      
   }
   
	AABB aabb = InvertedInfinityAABB();
   
	For(positions)
	{
		aabb.maxDim.x = Max(it->x, aabb.maxDim.x);
		aabb.maxDim.y = Max(it->y, aabb.maxDim.y);
		aabb.maxDim.z = Max(it->z, aabb.maxDim.z);
      
		aabb.minDim.x = Min(it->x, aabb.minDim.x);
		aabb.minDim.y = Min(it->y, aabb.minDim.y);
		aabb.minDim.z = Min(it->z, aabb.minDim.z);
	}
   
   TriangleMesh ret;
	ret.skeleton = {}; //not neccesary, as ret has {}
   ret.indexSets = indexSets;
   ret.positions = positions;
   ret.normals = normals;
   ret.uvs = uvs;
   ret.colors = colors;
   ret.indices = indices;
	ret.type = TriangleMeshType_List;
	ret.aabb = aabb;
   
   {
      String name = filename;
      name = EatToCharFromBackReturnTail(&name, '/');
      name = EatToCharReturnHead(&name, '.');
      ret.name = name;
   }
   
	Assert(success);
   
	return ret;
}


struct DAEReturn
{
	TriangleMesh mesh;
	KeyFramedAnimation animation;
	b32 success;
};


struct DAEParam
{
	String name;
	String type;
};
DefineDFArray(DAEParam);

struct DAETechnique
{
	u32 count;
	u32 stride;
	DAEParamDFArray params;
};

enum DAEArrayType
{
	DAE_None,
   
	DAE_Float,
	DAE_Int,
	DAE_Name,
   
	DAE_Amount,
};


struct DAESource
{
	String id;
	//String name; // optional so no megusta
	String arr; // string, we will work on it later
	u32 elementCount;
	DAEArrayType type;
   
	DAETechnique technique;
};

DefineDFArray(DAESource);

static bool CStringsAreEqual(char *a, char *b)
{
	while (*a)
	{
		if (*a++ != *b++) return false;
	}
   
	return *a == *b;
}

//also eats spaces at the beginning
static String DAEEatAttAndReturnIt(String *line, char *att)
{
	EatSpaces(line);
   
	u32 attlength = NullTerminatedStringLength(att);
	if (!BeginsWithEat(line, att)) { Die; return {}; };
   
	if (!BeginsWithEat(line, "=\"")) { Die; return {}; };
   
	String ret = EatToCharReturnHead(line, '\"');
	Eat1(line);
   
	if (CStringsAreEqual(att, "source"))
	{
		Eat1(&ret);
	}
	return ret;
}

#if 0
enum DAEGeometryType
{
	DAE_Mesh,
	DAE_ConvexMesh,
	DAE_Spline,
};
#endif

struct DAEImage
{
	String id;
	String name;
	String file;
};
DefineArray(DAEImage);

struct DAEInputShared
{
	u32 offset;
	String semantic;
	String source;
	//u32 set; // optional
};

struct DAEInputUnshared
{
	String semantic;
	String source;
};


DefineDFArray(DAEInputShared);
DefineDFArray(DAEInputUnshared);

struct DAEVertices
{
	String id;
	DAEInputUnsharedDFArray inputs;
};

struct DAETriangles
{
	//String name; //optional
	u32 count;
	String material;
	DAEInputSharedDFArray inputs;
	String indexArray;
};

struct DAEMesh
{
	String id;
	String name;
	DAESourceDFArray sources;
	DAEVertices verts;
	DAETriangles triangles;
};

DefineDFArray(DAEMesh);

struct DAESampler
{
	String id;
	DAEInputUnsharedDFArray inputs;
};

struct DAEChannel
{
	String source;
	String target;
};

struct DAEAnimation
{
	String id;
	DAESourceDFArray sources;
	DAESampler sampler;
	DAEChannel channel;
};

DefineDFArray(DAEAnimation);

struct DAEJoints
{
	DAEInputUnsharedDFArray inputs;
};

struct DAEVertexWeights
{
	u32 count;
	DAEInputSharedDFArray inputs;
	String vcount;
	String v;
};

struct DAESkin
{
	String skinSource;
	String bind_shape_matrix;
	DAESourceDFArray sources;
	DAEJoints joints;
	DAEVertexWeights vertex_weights;
};

struct DAEController
{
	String id;
	String name;
	DAESkin skin;
};

DefineDFArray(DAEController);

struct DAENode;

typedef DAENode* DAENodePtr;

DefineDFArray(DAENodePtr);

enum DAETransformationType
{
	DAE_Scale,
	DAE_Rotate,
	DAE_Translate,
	DAE_Matrix,
   
	DAE_Skew,
	DAE_lookAt,
};

struct DAETransformation
{
	DAETransformationType type;
	String sid;
	String val;
};

DefineDFArray(DAETransformation);

struct DAENode
{
	String id;
	String name;
	String sid; // optional
	String type;
	//String layer; not in my file
	DAETransformationDFArray transformations;
   
	DAENodePtrDFArray children;
};

struct DAEVisualScene
{
	String id;
	String name;
	DAENodePtrDFArray nodes;
};

DefineDFArray(DAEVisualScene);

struct DAEdata
{
	DAEImageArray images;
	DAEAnimationDFArray animations;
	DAEVisualSceneDFArray scenes;
	DAEControllerDFArray controllers;
	DAEMeshDFArray meshes;
   MaterialDFArray materials;
};

static DAENode *DAEParseNode(String *line, String *remaining)
{
	DAENode *ret = PushStruct(frameArena, DAENode);
   
	if (!BeginsWithEat(line, "<node")) { Die; return NULL; }
   
	ret->id = DAEEatAttAndReturnIt(line, "id");
	ret->name = DAEEatAttAndReturnIt(line, "name");
	EatSpaces(line);
	if (BeginsWith(*line, "sid"))
	{
		ret->sid = DAEEatAttAndReturnIt(line, "sid");
	}
	else
	{
		ret->sid = {};
	}
	ret->type = DAEEatAttAndReturnIt(line, "type");
   
	ret->transformations = DAETransformationCreateDFArray(1);
	while (remaining->amount)
	{
		*line = ConsumeNextLineSanitizeEatSpaces(remaining);
      
		DAETransformation trans;
      
		if (BeginsWithEat(line, "<translate"))
		{
			trans.type = DAE_Translate;
			trans.sid = DAEEatAttAndReturnIt(line, "sid");
			EatToCharReturnHead(line, '>'); Eat1(line);
			trans.val = EatToCharReturnHead(line, '<');
		}
		else if (BeginsWithEat(line, "<rotate"))
		{
			trans.type = DAE_Rotate;
			trans.sid = DAEEatAttAndReturnIt(line, "sid");
			EatToCharReturnHead(line, '>'); Eat1(line);
			trans.val = EatToCharReturnHead(line, '<');
		}
		else if (BeginsWithEat(line, "<scale"))
		{
			trans.type = DAE_Scale;
			trans.sid = DAEEatAttAndReturnIt(line, "sid");
			EatToCharReturnHead(line, '>'); Eat1(line);
			trans.val = EatToCharReturnHead(line, '<');
		}
		else if (BeginsWithEat(line, "<matrix"))
		{
			trans.type = DAE_Matrix;
			trans.sid = DAEEatAttAndReturnIt(line, "sid");
			EatToCharReturnHead(line, '>'); Eat1(line);
			trans.val = EatToCharReturnHead(line, '<');
		}
		else
		{
			break;
		}
      
		ArrayAdd(&ret->transformations, trans);
	}
   
	if (BeginsWithEat(line, "<instance_controller")) // dono ignoring this for now todo
	{
		while (*line != "</instance_controller>")
		{
			*line = ConsumeNextLineSanitizeEatSpaces(remaining);
		}
		*line = ConsumeNextLineSanitizeEatSpaces(remaining);
	}
   
   
	ret->children = DAENodePtrCreateDFArray(5);
	while (BeginsWith(*line, "<node"))
	{
		ArrayAdd(&ret->children, DAEParseNode(line, remaining));
	}
   
	if (*line == "<extra>") // skip extra for now, as i do not know it it is neccessary
	{
		while (remaining->length)
		{
			*line = ConsumeNextLineSanitizeEatSpaces(remaining);
			if (*line == "</extra>")
			{
				break;
			}
		}
      
		*line = ConsumeNextLineSanitizeEatSpaces(remaining);
	}
   
	Assert(*line == "</node>");
	*line = ConsumeNextLineSanitizeEatSpaces(remaining);
   
	return ret;
}

// this assume that we just ate "<source"
static DAESource DAEParseSource(String line, String *remaining)
{
   
	DAESource ret = {};
   
	ret.id = DAEEatAttAndReturnIt(&line, "id");
   
	b32 hadArrAllready = false;
	b32 hadTechAllready = false;
   
	b32 success = true;
	while (remaining->amount)
	{
		line = ConsumeNextLineSanitizeEatSpaces(remaining);
      
		if (line == "</source>")
		{
			break;
		}
      
		if (BeginsWithEat(&line, "<float_array"))
		{
			Assert(!hadArrAllready);
			hadArrAllready = true;
         
			String id = DAEEatAttAndReturnIt(&line, "id"); // not sure, this does not seem to make a lot of sense, as they can only be one array per source?
			String countS = DAEEatAttAndReturnIt(&line, "count");
         
			u32 count = StoU(countS, &success);
			Assert(success);
         
			ret.elementCount = count;
			ret.type = DAE_Float;
         
			EatToCharReturnHead(&line, '>'); // this should be nothing?
			Eat1(&line);
         
			ret.arr = EatToCharReturnHead(&line, '<'); // we assume that arrays are in one line allways
		}
		else if (BeginsWithEat(&line, "<Name_array"))
		{
			Assert(!hadArrAllready);
			hadArrAllready = true;
         
			String id = DAEEatAttAndReturnIt(&line, "id"); // not sure, this does not seem to make a lot of sense, as they can only be one array per source?
			String countS = DAEEatAttAndReturnIt(&line, "count");
         
			u32 count = StoU(countS, &success);
			Assert(success);
         
			ret.elementCount = count;
			ret.type = DAE_Name;
         
			EatToCharReturnHead(&line, '>'); // this should be nothing?
			Eat1(&line);
         
			ret.arr = EatToCharReturnHead(&line, '<'); // we assume that arrays are in one line allways
		}
		else if (line == "<technique_common>")
		{
			Assert(!hadTechAllready);
			hadTechAllready = true;
         
			line = ConsumeNextLineSanitizeEatSpaces(remaining);
         
			DAETechnique t;
         
			if (!BeginsWithEat(&line, "<accessor")) { Die; return {}; };
			String source = DAEEatAttAndReturnIt(&line, "source");
			String countS = DAEEatAttAndReturnIt(&line, "count");
			String strideS = DAEEatAttAndReturnIt(&line, "stride");
         
			u32 count = StoU(countS, &success);
			u32 stride = StoU(strideS, &success);
         
			t.count = count;
			t.stride = stride;
         
			DAEParamDFArray params = DAEParamCreateDFArray();
			while (remaining->amount)
			{
				line = ConsumeNextLineSanitizeEatSpaces(remaining);
            
				if (line == "</accessor>")
				{
					break;
				}
            
				if (!BeginsWithEat(&line, "<param")) { Die; return {}; };
				DAEParam param;
            
				param.name = DAEEatAttAndReturnIt(&line, "name");
				param.type = DAEEatAttAndReturnIt(&line, "type");
            
				ArrayAdd(&params, param);
            
			}
			t.params = params;
         
			ret.technique = t;
         
			line = ConsumeNextLineSanitizeEatSpaces(remaining);
			Assert(line == "</technique_common>");
		}
      
	}
   
	return ret;
}

static DAEInputUnsharedDFArray DAEParseUnsharedInputs(String *line, String *remaining)
{
	DAEInputUnsharedDFArray ret = DAEInputUnsharedCreateDFArray(4);
   
	while (BeginsWithEat(line, "<input"))
	{
		DAEInputUnshared toAdd;
		toAdd.semantic = DAEEatAttAndReturnIt(line, "semantic");
		toAdd.source = DAEEatAttAndReturnIt(line, "source");
      
		ArrayAdd(&ret, toAdd);
      
		*line = ConsumeNextLineSanitizeEatSpaces(remaining);
	}
   
	return ret;
}

static DAEInputSharedDFArray DAEParseSharedInputs(String *line, String *remaining)
{
	DAEInputSharedDFArray ret = DAEInputSharedCreateDFArray(4);
	b32 success = true;
   
	while (BeginsWithEat(line, "<input"))
	{
		DAEInputShared toAdd;
		toAdd.semantic = DAEEatAttAndReturnIt(line, "semantic");
		toAdd.source = DAEEatAttAndReturnIt(line, "source");
		String offsetS = DAEEatAttAndReturnIt(line, "offset");
		toAdd.offset = StoU(offsetS, &success);
      
		ArrayAdd(&ret, toAdd);
      
		*line = ConsumeNextLineSanitizeEatSpaces(remaining);
	}
   
	Assert(success);
   
	return ret;
}

static m4x4 Eatm4x4(String *s, b32 *success)
{
	m4x4 ret;
   
	for (u32 i = 0; i < 4; i++)
	{
		for (u32 j = 0; j < 4; j++)
		{
			EatSpaces(s);
			ret.a[i][j] = Eatf32(s, success);
		}
	}
   
	return ret;
}

static void BuildBoneArray(BoneArray bones, DAENode *node, StringArray boneNames, m4x4Array inverseBindShapeMatrices, u32 parentIndex, Arena *constantArena, m4x4 bindShapeMatrix)
{
	u32 index = 0xFFFFFFFF;
   
	if (node->type == "JOINT")
	{
      
		For(boneNames)
		{
			if (node->sid == *it)
			{
				index = (u32)(it - boneNames.data);
				break;
			}
		}
      
		Assert(index != 0xFFFFFFFF);
      
		// note : we do use "index" here, we assume it is _topological_ ordered by parenting, so we can generate the matrices faster later
      
		Bone *bone = bones + index;
		bone->name = CopyString(node->name, constantArena);
		bone->parentIndex = parentIndex;
		bone->inverseBindShapeMatrix = inverseBindShapeMatrices[index] * bindShapeMatrix;
      
		b32 success = true;
		String transformation = {};
      
		For(node->transformations)
		{
			if (it->type == DAE_Matrix)
			{
				transformation = it->val;
				break;
			}
		}
      
		m4x4 mat = Eatm4x4(&transformation, &success); // these are the transforms in local space, i.e relative to its parent. still do not reall know what they do
      
		bone->interp = MatrixToInterpolationData(mat);
		Assert(success);
	}
	else
	{
		Assert(node->type == "NODE");
	}
   
	For(node->children)
	{
		BuildBoneArray(bones, *it, boneNames, inverseBindShapeMatrices, index, constantArena, bindShapeMatrix);
	}
}

static void SortByWeight(WeightDataArray arr) // todo for now bouble sort...
{
   for(u32 i = 0; i < arr.amount; i++)
   {
      for(u32 j = 0; j < arr.amount - 1; j++)
      {
         WeightData *a = arr + j;
         WeightData *b = arr + j + 1;
         
         if(a->weight < b->weight)
         {
            WeightData temp = *a;
            *a = *b;
            *b = temp;
         }
      }
   }
}
static DAEReturn ReadDAE(Arena *constantArena, char *fileName)
{
	TimedBlock;
   
	File file = LoadFile(fileName, frameArena);
	if (!file.amount) return {};
   
	b32 success = true;
   
	DAEdata gatheredData;
   
	String remaining = CreateString(file.memory, file.amount);
	String xml = ConsumeNextLineSanitizeEatSpaces(&remaining);
	String collada = ConsumeNextLineSanitizeEatSpaces(&remaining);
   
	{ // Asset info, we ignore this
		String asset = ConsumeNextLineSanitizeEatSpaces(&remaining);
		Assert(asset == "<asset>");
      
		while (remaining.length)
		{
			String head = ConsumeNextLineSanitizeEatSpaces(&remaining);
			if (head == "</asset>")
			{
				break;
			}
		}
	}
   
   
	{ // images
		String imageLib = ConsumeNextLineSanitizeEatSpaces(&remaining);
		Assert(imageLib == "<library_images>");
      
		BeginArray(frameArena, DAEImage, images);
      
		while (remaining.length)
		{
			String line = ConsumeNextLineSanitizeEatSpaces(&remaining);
			if (line == "</library_images>")
			{
				break;
			}
			String head = EatToNextSpaceReturnHead(&line);
			if (head == "<image")
			{
            
				DAEImage *image = PushStruct(frameArena, DAEImage);
				image->id = DAEEatAttAndReturnIt(&line, "id");
				image->name = DAEEatAttAndReturnIt(&line, "name");
            
				line = ConsumeNextLineSanitizeEatSpaces(&remaining);
            
				if (!BeginsWithEat(&line, "<init_from>")) { Die; return {}; };
				image->file = EatToCharReturnHead(&line, '<');
            
				line = ConsumeNextLineSanitizeEatSpaces(&remaining);
            
				Assert(line == "</image>");
            
			}
		}
      
		EndArray(frameArena, DAEImage, images);
		gatheredData.images = images;
	}
   
   
	{ // materials
		String effects_lib = ConsumeNextLineSanitizeEatSpaces(&remaining);
		Assert(effects_lib == "<library_effects>");
		// todo: this does the material, we ignore this for now
      MaterialDFArray materials = MaterialCreateDFArray(1);
      
		while (remaining.length)
		{
			String line = ConsumeNextLineSanitizeEatSpaces(&remaining);
			if (line == "</library_effects>")
			{
				break;
			}
         
         if(BeginsWith(line, "<technique"))
         {
            while (remaining.length)
            {
               line = ConsumeNextLineSanitizeEatSpaces(&remaining);
               if (line == "</technique>")
               {
                  break;
               }
               
               if(line == "<phong>")
               {
                  Material mat = {};
                  while (remaining.length)
                  {
                     line = ConsumeNextLineSanitizeEatSpaces(&remaining);
                     if (line == "</phong>")
                     {
                        break;
                     }
                     
                     if(line == "<emission>")
                     {
                        line = ConsumeNextLineSanitizeEatSpaces(&remaining);
                        Assert(BeginsWithEat(&line, "<color sid=\"emission\">"));
                        f32 r = Eatf32(&line, &success);
                        EatSpaces(&line);
                        f32 g = Eatf32(&line, &success);
                        EatSpaces(&line);
                        f32 b = Eatf32(&line, &success);
                        EatSpaces(&line);
                        f32 a = Eatf32(&line, &success);
                        
                        mat.ke = V3(r, g, b);
                        line = ConsumeNextLineSanitizeEatSpaces(&remaining);
                     }
                     else if(line == "<ambient>")
                     {
                        line = ConsumeNextLineSanitizeEatSpaces(&remaining);
                        Assert(BeginsWithEat(&line, "<color sid=\"ambient\">"));
                        f32 r = Eatf32(&line, &success);
                        EatSpaces(&line);
                        f32 g = Eatf32(&line, &success);
                        EatSpaces(&line);
                        f32 b = Eatf32(&line, &success);
                        EatSpaces(&line);
                        f32 a = Eatf32(&line, &success);
                        
                        mat.ka = V3(r, g, b);
                        line = ConsumeNextLineSanitizeEatSpaces(&remaining);
                     }
                     else if(line == "<diffuse>")
                     {
                        // todo...
                        mat.kd = V3(1, 1, 1); //not sure
                     }
                     else if(line == "<specular>")
                     {
                        line = ConsumeNextLineSanitizeEatSpaces(&remaining);
                        Assert(BeginsWithEat(&line, "<color sid=\"specular\">"));
                        f32 r = Eatf32(&line, &success);
                        EatSpaces(&line);
                        f32 g = Eatf32(&line, &success);
                        EatSpaces(&line);
                        f32 b = Eatf32(&line, &success);
                        EatSpaces(&line);
                        f32 a = Eatf32(&line, &success);
                        
                        mat.ks = V3(r, g, b);
                        line = ConsumeNextLineSanitizeEatSpaces(&remaining);
                     }
                     else if(line == "<shininess>")
                     {
                        line = ConsumeNextLineSanitizeEatSpaces(&remaining);
                        Assert(BeginsWithEat(&line, "<float sid=\"shininess\">"));
                        f32 s = Eatf32(&line, &success);
                        mat.specularExponent = s;
                     }
                     else if(line == "<index_of_refraction>")
                     {
                        line = ConsumeNextLineSanitizeEatSpaces(&remaining);
                        Assert(BeginsWithEat(&line, "<float sid=\"index_of_refraction\">"));
                        f32 s = Eatf32(&line, &success);
                        mat.indexOfRefraction = s;
                        line = ConsumeNextLineSanitizeEatSpaces(&remaining);
                     }
                  }
                  ArrayAdd(&materials, mat);
               }
            }
         }
         
         gatheredData.materials = materials;
      }
      
      String materials_lib = ConsumeNextLineSanitizeEatSpaces(&remaining);
      Assert(materials_lib == "<library_materials>");
      
      while (remaining.length)
      
      {
         String line = ConsumeNextLineSanitizeEatSpaces(&remaining);
         if (line == "</library_materials>")
         {
            break;
         }
         // todo: we do not have _real_ materials right now, so no sense in parsing this.
      }
   }
   
   
   { // meshes
      String geometries_lib = ConsumeNextLineSanitizeEatSpaces(&remaining);
      Assert(geometries_lib == "<library_geometries>");
      
      DAEMeshDFArray meshes = DAEMeshCreateDFArray(4);
      
      while (remaining.length)
      {
         String line = ConsumeNextLineSanitizeEatSpaces(&remaining);
         if (line == "</library_geometries>")
         {
            break;
         }
         
         if (BeginsWithEat(&line, "<geometry"))
         {
            DAEMesh mesh = {};
            
            {
               mesh.id = DAEEatAttAndReturnIt(&line, "id");
               mesh.name = DAEEatAttAndReturnIt(&line, "name");
            }
            
            line = ConsumeNextLineSanitizeEatSpaces(&remaining);
            Assert(line == "<mesh>");
            
            
            
            b32 hadVertsAllready = false;
            
            while (remaining.length)
            {
               line = ConsumeNextLineSanitizeEatSpaces(&remaining);
               
               if (line == "</mesh>")
               {
                  break;
               }
               
               if (BeginsWithEat(&line, "<source ")) // todo we ignore techinique_common right now
               {
                  ArrayAdd(&mesh.sources, DAEParseSource(line, &remaining));
               }
               else if (BeginsWithEat(&line, "<vertices")) // why does it do this?
               {
                  if (hadVertsAllready) { Die;  return {}; };
                  
                  mesh.verts.id = DAEEatAttAndReturnIt(&line, "id");
                  line = ConsumeNextLineSanitizeEatSpaces(&remaining);
                  
                  mesh.verts.inputs = DAEParseUnsharedInputs(&line, &remaining);
                  Assert(line == "</vertices>");
               }
               else if (BeginsWithEat(&line, "<triangles"))
               {
                  
                  mesh.triangles.material = DAEEatAttAndReturnIt(&line, "material");
                  String countS = DAEEatAttAndReturnIt(&line, "count");
                  
                  mesh.triangles.count = StoU(countS, &success);
                  Assert(success);
                  line = ConsumeNextLineSanitizeEatSpaces(&remaining);
                  mesh.triangles.inputs = DAEParseSharedInputs(&line, &remaining);
                  
                  if (!BeginsWithEat(&line, "<p>")) { Die; return {}; };
                  
                  mesh.triangles.indexArray = EatToCharReturnHead(&line, '<'); // warning we assume that the indecies are all in one line.
                  
                  line = ConsumeNextLineSanitizeEatSpaces(&remaining);
                  Assert(line == "</triangles>");
                  
               }
               else Die;
            }
            
            ArrayAdd(&meshes, mesh);
         }
         //else if
      }
      gatheredData.meshes = meshes;
   }
   
   // these are the keyframes
   {
      String animations_lib = ConsumeNextLineSanitizeEatSpaces(&remaining);
      Assert(animations_lib == "<library_animations>");
      
      DAEAnimationDFArray animations = DAEAnimationCreateDFArray(30);
      
      while (remaining.length)
      {
         String line = ConsumeNextLineSanitizeEatSpaces(&remaining);
         if (line == "</library_animations>")
         {
            break;
         }
         
         if (!BeginsWithEat(&line, "<animation")) { Die; return {}; }; // todo  maybe do not assume that they always look the same?
         
         DAEAnimation animation;
         
         animation.id = DAEEatAttAndReturnIt(&line, "id");
         animation.sources = DAESourceCreateDFArray(4);
         
         b32 hadSampler = false;
         b32 hadChannel = false;
         
         while (remaining.length)
         {
            line = ConsumeNextLineSanitizeEatSpaces(&remaining);
            if (line == "</animation>")
            {
               break;
            }
            
            if (BeginsWithEat(&line, "<source"))
            {
               ArrayAdd(&animation.sources, DAEParseSource(line, &remaining));
            }
            else if (BeginsWithEat(&line, "<sampler"))
            {
               Assert(!hadSampler); hadSampler = true;
               animation.sampler.id = DAEEatAttAndReturnIt(&line, "id");
               line = ConsumeNextLineSanitizeEatSpaces(&remaining);
               animation.sampler.inputs = DAEParseUnsharedInputs(&line, &remaining);
               Assert(line == "</sampler>");
            }
            else if (BeginsWithEat(&line, "<channel"))
            {
               Assert(!hadChannel); hadChannel = true;
               animation.channel.source = DAEEatAttAndReturnIt(&line, "source");
               animation.channel.target = DAEEatAttAndReturnIt(&line, "target");
            }
            else Die;
            
         }
         ArrayAdd(&animations, animation);
      }
      gatheredData.animations = animations;
   }
   
   
   {	// this specifies the bones/joints, and tells you how the skin is attatched to them
      String controllers_lib = ConsumeNextLineSanitizeEatSpaces(&remaining);
      Assert(controllers_lib == "<library_controllers>");
      
      DAEControllerDFArray controllers = DAEControllerCreateDFArray();
      
      while (remaining.amount)
      {
         String line = ConsumeNextLineSanitizeEatSpaces(&remaining);
         if (line == "</library_controllers>")
         {
            break;
         }
         
         if (!BeginsWithEat(&line, "<controller")) { Die; return {}; }
         
         DAEController controller = {};
         
         controller.id = DAEEatAttAndReturnIt(&line, "id");
         controller.name = DAEEatAttAndReturnIt(&line, "name");
         
         // skin
         line = ConsumeNextLineSanitizeEatSpaces(&remaining);
         if (!BeginsWithEat(&line, "<skin")) { Die; return {}; }
         
         controller.skin.skinSource = DAEEatAttAndReturnIt(&line, "source");
         
         while (remaining.length)
         {
            line = ConsumeNextLineSanitizeEatSpaces(&remaining);
            
            if (line == "</skin>")
            {
               break;
            }
            
            if (BeginsWithEat(&line, "<bind_shape_matrix>"))
            {
               controller.skin.bind_shape_matrix = EatToCharReturnHead(&line, '<');
            }
            else if (BeginsWithEat(&line, "<source"))
            {
               ArrayAdd(&controller.skin.sources, DAEParseSource(line, &remaining));
            }
            else if (line == "<joints>")
            {
               line = ConsumeNextLineSanitizeEatSpaces(&remaining);
               controller.skin.joints.inputs = DAEParseUnsharedInputs(&line, &remaining);
               Assert(line == "</joints>");
            }
            else if (BeginsWithEat(&line, "<vertex_weights"))
            {
               String countS = DAEEatAttAndReturnIt(&line, "count");
               controller.skin.vertex_weights.count = StoU(countS, &success);
               Assert(success);
               line = ConsumeNextLineSanitizeEatSpaces(&remaining);
               controller.skin.vertex_weights.inputs = DAEParseSharedInputs(&line, &remaining);
               if (!BeginsWithEat(&line, "<vcount>")) { Die; return {}; }
               controller.skin.vertex_weights.vcount = EatToCharReturnHead(&line, '<');
               line = ConsumeNextLineSanitizeEatSpaces(&remaining);
               if (!BeginsWithEat(&line, "<v>")) { Die; return {}; }
               controller.skin.vertex_weights.v = EatToCharReturnHead(&line, '<');
               
               line = ConsumeNextLineSanitizeEatSpaces(&remaining);
               Assert(line == "</vertex_weights>");
               
            }
            else Die;
         }
         line = ConsumeNextLineSanitizeEatSpaces(&remaining);
         Assert(line == "</controller>");
         
         ArrayAdd(&controllers, controller);
      }
      gatheredData.controllers = controllers;
   }
   
   {// this build the skelleton hirachy
      String visual_scenes_lib = ConsumeNextLineSanitizeEatSpaces(&remaining);
      Assert(visual_scenes_lib == "<library_visual_scenes>");
      DAEVisualSceneDFArray scenes = DAEVisualSceneCreateDFArray(2);
      
      while (remaining.amount)
      {
         
         String line = ConsumeNextLineSanitizeEatSpaces(&remaining);
         if (line == "</library_visual_scenes>")
         {
            break;
         }
         
         DAEVisualScene scene = {};
         
         if (!BeginsWithEat(&line, "<visual_scene")) { Die; return {}; }
         {
            scene.id = DAEEatAttAndReturnIt(&line, "id");
            scene.name = DAEEatAttAndReturnIt(&line, "name");
         }
         line = ConsumeNextLineSanitizeEatSpaces(&remaining);
         while (BeginsWith(line, "<node"))
         {
            ArrayAdd(&scene.nodes, DAEParseNode(&line, &remaining));
         }
         
         ArrayAdd(&scenes, scene);
      }
      gatheredData.scenes = scenes;
   }
   
   Assert(gatheredData.meshes.amount == 1);
   
   DAEMesh *daeMesh = gatheredData.meshes + 0;
   
   v3Array positions = {};
   u32 pOffset = 0;
   v3Array normals;
   u32 nOffset = 0;
   v2Array uvs;
   u32 uvOffset = 0;
   
   For(daeMesh->triangles.inputs)
   {
      if (it->semantic == "VERTEX")
      {
         DAESource *source = NULL;
         For(s, daeMesh->sources)
         {
            if (s->id == daeMesh->verts.inputs[0].source) // this for some reason goes through the vertices thing so hack todo
            {
               source = s;
               break;
            }
         }
         
         pOffset = it->offset;
         
         String vertIt = source->arr;
         
         positions = PushArray(constantArena, v3, source->technique.count);
         
         For(p, positions)
         {
            p->x = Eatf32(&vertIt, &success);
            EatSpaces(&vertIt);
            p->y = Eatf32(&vertIt, &success);
            EatSpaces(&vertIt);
            p->z = Eatf32(&vertIt, &success);
            EatSpaces(&vertIt);
         }
         
         Assert(success);
         
      }
      else if (it->semantic == "NORMAL")
      {
         DAESource *source = NULL;
         For(s, daeMesh->sources)
         {
            if (s->id == it->source)
            {
               source = s;
               break;
            }
         }
         
         nOffset = it->offset;
         
         String vertIt = source->arr;
         
         normals = PushArray(frameArena, v3, 0);
         
         for (u32 i = 0; i < source->technique.count; i++)
         {
            v3 *p = PushStruct(frameArena, v3);
            
            p->x = Eatf32(&vertIt, &success);
            EatSpaces(&vertIt);
            p->y = Eatf32(&vertIt, &success);
            EatSpaces(&vertIt);
            p->z = Eatf32(&vertIt, &success);
            EatSpaces(&vertIt);
         }
         
         normals.amount = (u32)((v3*)frameArena->current - normals.data);
         
         Assert(success);
      }
      else if (it->semantic == "TEXCOORD")
      {
         DAESource *source = NULL;
         For(s, daeMesh->sources)
         {
            if (s->id == it->source)
            {
               source = s;
               break;
            }
         }
         
         uvOffset = it->offset;
         
         String vertIt = source->arr;
         
         uvs = PushArray(frameArena, v2, 0);
         
         for (u32 i = 0; i < source->technique.count; i++)
         {
            v2 *p = PushStruct(frameArena, v2);
            
            p->u = Eatf32(&vertIt, &success);
            EatSpaces(&vertIt);
            p->v = Eatf32(&vertIt, &success);
            EatSpaces(&vertIt);
         }
         
         uvs.amount = (u32)((v2*)frameArena->current - uvs.data);
         
         Assert(success);
      }
      else Die;
   }
   
   u32 flattener = 0;
   
   UVListPtrArray flatten = PushZeroArray(frameArena, UVListPtr, positions.amount);
   DAETriangles *daeTs = &daeMesh->triangles;
   String indIt = daeTs->indexArray;
   u16Array indices = PushArray(constantArena, u16, daeTs->count * 3u);
   
   for (u32 i = 0; i < daeTs->count * 3u; i++) // todo: assuming 3 inputs : p, n, uv
   {
      u32 pI = Eatu32(&indIt, &success);
      EatSpaces(&indIt);
      u32 nI = Eatu32(&indIt, &success);
      EatSpaces(&indIt);
      u32 uvI = Eatu32(&indIt, &success);
      EatSpaces(&indIt);
      
      b32 found = false;
      u16 index = 0;
      
      for (UVList *it = flatten[pI]; it; it = it->next)
      {
         if (it->normalIndex == nI && it->uvIndex == uvI)
         {
            found = true;
            index = it->flattendIndex;
            break;
         }
      }
      
      if (!found)
      {
         UVList *append = PushStruct(frameArena, UVList);
         Assert(flattener < 0xFFFF);
         append->flattendIndex = (u16)flattener++;
         append->next = flatten[pI];
         append->normalIndex = (u16)nI;
         append->uvIndex = (u16)uvI;
         
         flatten[pI] = append;
         
         index = append->flattendIndex;
      }
      
      indices[i] = index;
   }
   
   u16Array skeletonVertexMap = PushArray(constantArena, u16, flattener);
   v3Array  flatPositions = PushArray(constantArena, v3,  flattener);
   v3Array  flatNormals   = PushArray(constantArena, v3,  flattener);
   v2Array  flatUvs       = PushArray(constantArena, v2,  flattener);
   u32Array flatColors    = PushArray(constantArena, u32, flattener);
   
   For(flatten)
   {
      u16 it_index = (u16)(it - flatten.data);
      for (UVList *c = *it; c; c = c->next)
      {
         u32 i = c->flattendIndex;
         flatColors[i]    = 0xFFFFFFFF;
         flatPositions[i] = positions[it_index];
         flatNormals[i]   = normals[c->normalIndex];
         flatUvs[i]       = uvs[c->uvIndex];
         
         skeletonVertexMap[i] = it_index;
      }
   }
   
   Assert(gatheredData.controllers.amount == 1);
   
   DAESkin *daeSkin = &gatheredData.controllers[0].skin;
   
   // todo  is there a  good way to make him shut up about these having to be initialized?
   StringArray boneNames = {};
   m4x4Array inverseBindShapeMatrixArray = {};
   
   For(i, daeSkin->joints.inputs)
   {
      if (i->semantic == "JOINT")
      {
         DAESource *source = NULL;
         For(s, daeSkin->sources)
         {
            if (i->source == s->id)
            {
               source = s;
               break;
            }
         }
         
         String nameIt = source->arr;
         boneNames = PushArray(frameArena, String, source->technique.count); // arena? // /source->technique.stride
         
         For(boneNames)
         {
            *it = EatToNextSpaceReturnHead(&nameIt);
            EatSpaces(&nameIt);
         }
         Assert(!nameIt.amount);
      }
      else if (i->semantic == "INV_BIND_MATRIX")
      {
         DAESource *source = NULL;
         For(s, daeSkin->sources)
         {
            if (i->source == s->id)
            {
               source = s;
               break;
            }
         }
         
         String nameIt = source->arr;
         //u32 amount = source->elementCount / source->technique.stride;
         
         inverseBindShapeMatrixArray = PushArray(frameArena, m4x4, source->technique.count); // arena?
         
         For(inverseBindShapeMatrixArray)
         {
            *it = Eatm4x4(&nameIt, &success);
         }
         Assert(success);
         Assert(!nameIt.amount);
      }
   }
   
   Assert(inverseBindShapeMatrixArray.amount == boneNames.amount);
   u32 amountOfBones = boneNames.amount;
   
   
   // <vcount>, <v> describes a map,  position <-> vcount  and then the corresponding vcount entries in <v> give the weight data.
   // position is not linearized
   Assert(daeSkin->vertex_weights.count == positions.amount);
   
   f32Array weights;
   StringArray jointNames;
   
   For(i, daeSkin->vertex_weights.inputs)
   {
      if (i->semantic == "WEIGHT")
      {
         DAESource *source = NULL;
         For(s, daeSkin->sources)
         {
            if (s->id == i->source)
            {
               source = s;
               break;
            }
         }
         
         String weightIt = source->arr;
         weights = PushArray(frameArena, f32, source->technique.count); //arena?
         
         For(weights)
         {
            *it = Eatf32(&weightIt, &success);
            EatSpaces(&weightIt);
         }
         Assert(success);
      }
      else if (i->semantic == "JOINT")
      {
         DAESource *source = NULL;
         For(s, daeSkin->sources)
         {
            if (s->id == i->source)
            {
               source = s;
               break;
            }
         }
         
         String jointIt = source->arr;
         jointNames = PushArray(frameArena, String, source->technique.count); //arena?
         
         For(jointNames)
         {
            *it = EatToNextSpaceReturnHead(&jointIt);
            EatSpaces(&jointIt);
         }
         Assert(success);
      }
   }
   
   // collect weights
   WeightDataArrayArray vertexToWeightsMap = PushArray(constantArena, WeightDataArray, positions.amount);
   
   String vCountIt = daeSkin->vertex_weights.vcount;
   String vIt = daeSkin->vertex_weights.v;
   
   For(weightArray, vertexToWeightsMap)
   {
      u32 amountOfWeights = Eatu32(&vCountIt, &success);
      *weightArray = PushArray(constantArena, WeightData, amountOfWeights);
      EatSpaces(&vCountIt);
      For(*weightArray)
      {
         it->boneIndex = Eatu32(&vIt, &success);
         EatSpaces(&vIt);
         u32 weightIndex = Eatu32(&vIt, &success);
         it->weight = weights[weightIndex];
         EatSpaces(&vIt);
      }
      
      SortByWeight(*weightArray);
   }
   Assert(success);
   
   //collect animation
   m4x4ArrayArray matrixArrayPerBone = PushArray(frameArena, m4x4Array, amountOfBones);
   f32ArrayArray timeArrayPerBone = PushArray(frameArena, f32Array, amountOfBones);
   
   Assert(gatheredData.animations.amount == amountOfBones);
   
   For(a, gatheredData.animations)
   {
      u32 it_index = (u32)(a - gatheredData.animations.data);
      For(i, a->sampler.inputs)
      {
         if (i->semantic == "INPUT")
         {
            DAESource *source = NULL;
            For(s, a->sources)
            {
               if (s->id == i->source)
               {
                  source = s;
                  break;
               }
            }
            
            String timeIt = source->arr;
            
            timeArrayPerBone[it_index] = PushArray(frameArena, f32, source->technique.count);
            For(timeArrayPerBone[it_index])
            {
               *it = Eatf32(&timeIt, &success);
               EatSpaces(&timeIt);
            }
            Assert(success);
            
         }
         else if (i->semantic == "OUTPUT")
         {
            DAESource *source = NULL;
            For(s, a->sources)
            {
               if (s->id == i->source)
               {
                  source = s;
                  break;
               }
            }
            
            String matIt = source->arr;
            
            matrixArrayPerBone[it_index] = PushArray(frameArena, m4x4, source->technique.count);
            For(matrixArrayPerBone[it_index])
            {
               *it = Eatm4x4(&matIt, &success);
               EatSpaces(&matIt);
            }
            Assert(success);
         }
      }
   }
   
   u32 amountOfKeyFrames = timeArrayPerBone[0].amount;
   for (u32 i = 0; i < amountOfBones; i++)
   {
      Assert(timeArrayPerBone[i].amount == amountOfKeyFrames);
      Assert(matrixArrayPerBone[i].amount == amountOfKeyFrames);
   }
   
   for (u32 j = 0; j < amountOfKeyFrames; j++)
   {
      f32 time = timeArrayPerBone[0][j];
      
      for (u32 i = 0; i < amountOfBones; i++)
      {
         Assert(time == timeArrayPerBone[i][j]);
      }
   }
   
   KeyFrameArray keyFrames = PushArray(constantArena, KeyFrame, amountOfKeyFrames);
   
   For(keyFrames)
   {
      u32 it_index = (u32)(it - keyFrames.data);
      it->boneStates = PushArray(constantArena, InterpolationData, amountOfBones);
      
      for (u32 i = 0; i < amountOfBones; i++)
      {
         it->boneStates[i] = MatrixToInterpolationData(matrixArrayPerBone[i][it_index]);
      }
      
      it->t = timeArrayPerBone[0][it_index]; // as they are all the same
   }
   
#if 0
   skeleton.boneNames = jointNames; // right now these break // todo todo todo
   skeleton.boneTransformes = boneTransformes;
#endif
   
   BoneArray bones = PushArray(constantArena, Bone, amountOfBones);
   Assert(gatheredData.scenes.amount == 1);
   
   m4x4 bindShapeMatrix = Eatm4x4(&gatheredData.controllers[0].skin.bind_shape_matrix, &success);
   Assert(success);
   
   BuildBoneArray(bones, gatheredData.scenes[0].nodes[0], boneNames, inverseBindShapeMatrixArray, 0xFFFFFFFF, constantArena, bindShapeMatrix);
   
   Skeleton skeleton;
   skeleton.vertices = positions;
   skeleton.vertexMap = skeletonVertexMap;
   skeleton.vertexToWeightsMap = vertexToWeightsMap;
   skeleton.bones = bones;
   
   TriangleMesh triangleMesh;
   triangleMesh.type      = TriangleMeshType_List;
   triangleMesh.indices   = indices;
   triangleMesh.positions = flatPositions;
   triangleMesh.uvs       = flatUvs;
   triangleMesh.normals   = flatNormals;
   triangleMesh.colors    = flatColors;
   
   triangleMesh.indexSets = PushArray(constantArena, IndexSet, 1);
   triangleMesh.indexSets[0].mat = gatheredData.materials[0];
   triangleMesh.indexSets[0].mat.texturePath = FormatString("%s.texture", gatheredData.images[0].name, constantArena);
   triangleMesh.indexSets[0].offset = 0;
   triangleMesh.indexSets[0].amount = indices.amount;
   triangleMesh.name = CopyString(gatheredData.scenes[0].nodes[1]->name, constantArena);
   triangleMesh.skeleton = skeleton;
   
   AABB aabb = InvertedInfinityAABB();
   
   For(triangleMesh.positions)
   {
      aabb.maxDim.x = Max(it->x, aabb.maxDim.x);
      aabb.maxDim.y = Max(it->y, aabb.maxDim.y);
      aabb.maxDim.z = Max(it->z, aabb.maxDim.z);
      
      aabb.minDim.x = Min(it->x, aabb.minDim.x);
      aabb.minDim.y = Min(it->y, aabb.minDim.y);
      aabb.minDim.z = Min(it->z, aabb.minDim.z);
   }
   
   triangleMesh.aabb = aabb;
   
   KeyFramedAnimation animation;
   animation.keyFrames = keyFrames;
   animation.length = keyFrames[amountOfKeyFrames - 1].t;
   String name = S(fileName);
   name = EatToCharFromBackReturnTail(&name, '/');
   name = EatToCharReturnHead(&name, '.');
   animation.animationName = CopyString(name, constantArena);
   animation.meshName = triangleMesh.name;
   
   DAEReturn ret;
   ret.animation = animation;
   ret.mesh = triangleMesh;
   ret.success = success;
   
   
   return ret;
}




static Bitmap CreateBitmap(u32* pixels, u32 width, u32 height)
{
	Bitmap ret;
	ret.pixels = pixels;
	ret.width = width;
	ret.height = height;
	return ret;
}


#pragma pack(push, 1)
struct BitmapFileHeader
{
	u16 bfType;
	u32 bfSize;
	u16 bfReserved1;
	u16 bfReserved2;
	u32 bfOffBits;
   
	//infoheader
	u32 biSize;
	i32 biWidth;
	i32 biHeight;
	u16 biPlanes;
	u16 biBitCount;
	u32 compression;
	u32 sizeImage;
	i32 biXPerlsPerMeter;
	i32 biYPerlsPerMeter;
	u32 biClrUsed;
	u32 biCLrImpartant;
   
	u32 redMask;
	u32 greenMask;
	u32 blueMask;
};
#pragma pack(pop)


static Bitmap CreateBitmap(char* fileName, Arena *arena, bool wrapping = false)
{
	Bitmap ret = {};
	//TODO: maybe check if its actually a bmp
	File tempFile;
   tempFile = LoadFile(fileName, arena);
   
   
	void *memPointer = tempFile.memory;
	if (!memPointer) return ret;
	
	BitmapFileHeader *header = (BitmapFileHeader *)memPointer;
   
	ret.width = header->biWidth;
	ret.height = header->biHeight;
   
	u8 *bitMemPointer = (u8 *)memPointer;
	bitMemPointer += header->bfOffBits;
	ret.pixels = (u32 *)bitMemPointer;
   
	//switching masks
	u32 redMask = header->redMask;
	u32 blueMask = header->blueMask;
	u32 greenMask = header->greenMask;
	u32 alphaMask = ~(redMask | blueMask | greenMask);
   
	u32 redShift = BitwiseScanForward(redMask);
	u32 blueShift = BitwiseScanForward(blueMask);
	u32 greenShift = BitwiseScanForward(greenMask);
	u32 alphaShift = BitwiseScanForward(alphaMask);
   
	u32 *tempPixels = ret.pixels;
	for (i32 x = 0; x < header->biWidth; x++)
	{
		for (i32 y = 0; y < header->biHeight; y++)
		{
			u32 c = *tempPixels;
         
			u32 R = ((c >> redShift) & 0xFF);
			u32 G = ((c >> greenShift) & 0xFF);
			u32 B = ((c >> blueShift) & 0xFF);
			u32 A = ((c >> alphaShift) & 0xFF);
			float an = (A / 255.0f);
         
			R = (u32)((float)R * an);
			G = (u32)((float)G * an);
			B = (u32)((float)B * an);
			R = (u32)((float)R * an);
         
			*tempPixels++ = ((A << 24) | (R << 16) | (G << 8) | (B << 0));
		}
	}
   
	return ret;
}

static void DoNothing(void *x) { }

#define STBTT_ifloor(x)   ((int) Floor(x))
#define STBTT_iceil(x)    ((int) Ceil(x))
#define STBTT_sqrt(x)      Sqrt(x)
#define STBTT_pow(x,y)     pow(x,y)
#define STBTT_fmod(x,y)    fmod(x,y)
#define STBTT_cos(x)       cos(x)
#define STBTT_acos(x)      acos(x)
#define STBTT_fabs(x)      fabs(x)
#define STBTT_malloc(x,u)  ((void)(u), (void *)PushData(frameArena, u8, (u32)(x)))
#define STBTT_free(x,u)    ((void)(u), DoNothing(x));
#define STBTT_assert(x)    {Assert(x)}
#define STBTT_strlen(x)    NullTerminatedStringLength(x)
#define STBTT_memcpy       memcpy
#define STBTT_memset       memset

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

static Font CreateFontFromSTB(u32 width, u32 height, u8 *pixels, u32 amountOfChars, f32 charHeight, stbtt_bakedchar *charData, Arena *arena)
{
	Font ret;
   
	u32 *output = PushData(arena, u32, width * height);
	u32 *out = output;
	u8 *inp = pixels;
   
	for (u32 h = 0; h < height; h++)
	{
		for (u32 w = 0; w < width; w++)
		{
			u8 source = *inp++;
			//u32 color = 0xFFFFFFFF;
			u32 color = source << 24 | source << 16 | source << 8 | source << 0;
			*out++ = color;
		}
	}
   
	ret.charData = PushData(arena, CharData, amountOfChars);
	v2 scale = V2(1.0f / (f32)width, 1.0f / (f32)height);
	for (u32 i = 0; i < amountOfChars; i++)
	{
		stbtt_bakedchar *cur = charData + i;
		CharData data;
      
		data.xAdvance = cur->xadvance;
		data.minUV = scale * V2(cur->x0, cur->y0);
		data.maxUV = scale * V2(cur->x1, cur->y1);
		data.width = cur->x1 - cur->x0;
		data.height = cur->y1 - cur->y0;
		data.xOff = cur->xoff;
		data.yOff = cur->yoff;
      
		ret.charData[i] = data;
	}
   
   
	ret.bitmap = CreateBitmap(output, width, height);
	ret.amountOfChars = amountOfChars;
	ret.charHeight = charHeight;
	return ret;
}

static Font LoadFont(char *fileName, Arena *arena)
{
	File file = LoadFile(fileName, frameArena);
   
	u32 width = 1024;
	u32 height = 1024;
	u8 *pixels = PushData(frameArena, u8, width * height);
	
	u32 amountOfChars = 255;
	f32 charHeight = 64.0f;
	stbtt_bakedchar *charData = PushData(frameArena, stbtt_bakedchar, amountOfChars);
	stbtt_BakeFontBitmap((char *)file.memory, 0, charHeight, pixels, width, height, 0, amountOfChars, charData);
   
	return CreateFontFromSTB(width, height, pixels, amountOfChars, charHeight, charData, arena);
}


#define PullOff(type) *(type *)at; at += sizeof(type);

#define PullOffArray(type, arr) \
(arr).amount = PullOff(u32); \
(arr).data = (type *)at; \
at += (arr).amount * sizeof(type); 

static void UnloadMesh(AssetInfo *info)
{
   Assert(info->currentlyLoaded);
   Assert(info->type == Asset_Mesh);
   
   DynamicFree(globalAlloc, info->meshInfo->fileLocation);
}

static TriangleMesh LoadMesh(AssetHandler *assetHandler, char *fileName, void **filePtr)
{
   //return {};
   // todo this should not call Register mesh, we should get rid of type
   TriangleMesh ret;
   
   File file = LoadFile(fileName, globalAlloc);
   if (!file.fileSize) return {};
   *filePtr = file.data;
   u8 *at = (u8 *)file.memory;
   
   u32 version = PullOff(u32);
   
   Assert(version == 1);
   
   PullOffArray(Char, ret.name);
   
   ret.type = PullOff(u32);
   
   PullOffArray(v3,  ret.positions);
   PullOffArray(u32, ret.colors);
   PullOffArray(v2,  ret.uvs);
   PullOffArray(v3,  ret.normals);
   
   PullOffArray(u16, ret.indices);
   PullOffArray(IndexSet, ret.indexSets);
   
   For(ret.indexSets)
   {
      it->mat.name.length = PullOff(u32);
      it->mat.name.data = (Char *)at;
      at += it->mat.name.length * sizeof(Char);
      at++; // to get it zero Terminated
      
      it->mat.texturePath.length = PullOff(u32);
      it->mat.texturePath.data = (Char *)at;
      at += it->mat.texturePath.length * sizeof(Char);
      at++; // to get it zero Terminated
      
      it->mat.bitmapID = RegisterAsset(assetHandler, Asset_Texture, (char *)it->mat.texturePath.data);
   }
   
   ret.aabb = PullOff(AABB);
   
#if 0
   
   { // vertex to weight map
      *PushStruct(frameArena, u32) = skeleton->vertexToWeightsMap.amount;
      For(skeleton->vertexToWeightsMap)
      {
         *PushStruct(frameArena, u32) = it->amount;
         WeightDataArray dest = PushArray(frameArena, WeightData, it->amount);
         memcpy(dest.data, it->data, it->amount * sizeof(WeightData));
      }
   }
   
#endif
   PullOffArray(v3, ret.skeleton.vertices);
   PullOffArray(u16, ret.skeleton.vertexMap);
   
   PullOffArray(WeightDataArray, ret.skeleton.vertexToWeightsMap);
   For(ret.skeleton.vertexToWeightsMap)
   {
      PullOffArray(WeightData, *it);
   }
   
   PullOffArray(Bone, ret.skeleton.bones);
   // todo update mesh!
   RegisterTriangleMesh(&ret);
   
   return ret;
}

#define PushOn(type, val) *PushStruct(arena, type) = val

#define PushOnArray(type, arr)								\
{															\
   *PushStruct(arena, u32) = (arr).amount;						\
   type##Array dest = PushArray(arena, type, (arr).amount);	\
   memcpy(dest.data, (arr).data, (arr).amount * sizeof(type)); \
}
// todo PushOn, PushOnArray
static void WriteTriangleMesh(TriangleMesh mesh, char *fileName) // todo : when we feel bored, we can make this a bit mor efficient
{
   File file;
   // begin of daisy chain
   file.memory = PushData(frameArena, u8, 0);
   
   Arena *arena = frameArena;
   
   PushOn(u32, 1);// version Number
   
   PushOnArray(u8, mesh.name);
   
   PushOn(u32, mesh.type);
   
   PushOnArray(v3,  mesh.positions);
   PushOnArray(u32, mesh.colors);
   PushOnArray(v2,  mesh.uvs);
   PushOnArray(v3,  mesh.normals);
   
   PushOnArray(u16, mesh.indices);
   PushOnArray(IndexSet, mesh.indexSets);
   
   For(mesh.indexSets)
   {
      Material mat = it->mat;
      
      *PushStruct(frameArena, u32) = mat.name.length;
      Char *dest1 = PushData(frameArena, Char, mat.name.length);
      memcpy(dest1, mat.name.data, mat.name.length * sizeof(Char));
      PushZeroStruct(frameArena, Char); // to get it zero Terminated
      
      *PushStruct(frameArena, u32) = mat.texturePath.length;
      Char *dest2 = PushData(frameArena, Char, mat.texturePath.length);
      memcpy(dest2, mat.texturePath.data, mat.texturePath.length * sizeof(Char));
      PushZeroStruct(frameArena, Char); // to get it zero Terminated
   }
   
   *PushStruct(frameArena, AABB) = mesh.aabb;
   
   // skeleton // untested
   Skeleton *skeleton = &mesh.skeleton;
   
   { // vertices
      PushOnArray(v3, skeleton->vertices);
   }
   
   
   { // vertex Map
      PushOnArray(u16, skeleton->vertexMap);
   }
   
   { // vertex to weight map
      PushOnArray(WeightDataArray, skeleton->vertexToWeightsMap);
      
      For(skeleton->vertexToWeightsMap)
      {
         PushOnArray(WeightData, *it);
      }
   }
   
   { // bones
      PushOnArray(Bone, skeleton->bones);
   }
   
   file.fileSize = (u32)(frameArena->current - (u8 *)file.memory);
   
   WriteEntireFile(fileName, file);
}

static bool WriteAnimation(char *fileName, KeyFramedAnimation animation) // untested
{
   File file = PushArray(frameArena, u8, 0);
   
   Arena *arena = frameArena;
   
   PushOn(u32, 0); // version Number
   PushOnArray(u8, animation.animationName);
   PushOnArray(u8, animation.meshName);
   PushOnArray(KeyFrame, animation.keyFrames);
   For(animation.keyFrames)
   {
      PushOnArray(InterpolationData, it->boneStates);
   }
   
   EndArray(frameArena, u8, file);
   return WriteEntireFile(fileName, file);
}

static KeyFramedAnimation LoadAnimation(char *fileName) // untested
{
   File file = LoadFile(fileName, globalAlloc);
   if (!file.fileSize) return {};
   
   u8 *at = file.memory;
   
   KeyFramedAnimation ret;
   u32 version = PullOff(u32);
   Assert(version == 0);
   PullOffArray(Char, ret.animationName);
   PullOffArray(Char, ret.meshName);
   PullOffArray(KeyFrame, ret.keyFrames);
   For(ret.keyFrames)
   {
      PullOffArray(InterpolationData, it->boneStates);
   }
   
   // we assume there are more then zero keyFrames...
   ret.length = ret.keyFrames[ret.keyFrames.amount - 1].t;
   
   return ret;
}

static Bitmap DownSampleTexture(Bitmap bitmap)
{
   u32 bitmapWidth = (bitmap.width >> 1);
   u32 bitmapHeight = (bitmap.height >> 1);
   
   u32* data = PushData(frameArena, u32, bitmapWidth * bitmapHeight);
   
   for (u32 h = 0; h < bitmapHeight; h++)
   {
      for (u32 w = 0; w < bitmapWidth; w++)
      {
         v4 ul = Unpack4x8(*GetPixel(bitmap, 2 * w, 2 * h));
         v4 ur = Unpack4x8(*GetPixel(bitmap, 2 * w + 1, 2 * h));
         v4 ll = Unpack4x8(*GetPixel(bitmap, 2 * w, 2 * h + 1));
         v4 lr = Unpack4x8(*GetPixel(bitmap, 2 * w + 1, 2 * h + 1));
         data[h * bitmapWidth + w] = Pack4x8(0.25f * (ul + ur + ll + lr));
      }
   }
   return CreateBitmap(data, bitmapWidth, bitmapHeight);
}

static Bitmap LoadTexture(char *filefile)
{
   File file = LoadFile(filefile, frameArena);
   if (!file.fileSize) return {};
   u8 *at = (u8 *)file.memory;
   u32 version = *(u32 *)at;
   Assert(version == 1);
   at += sizeof(u32);
   u32 width = *(u32 *)at;
   at += sizeof(u32);
   u32 height = *(u32 *)at;
   at += sizeof(u32);
   
   Bitmap bitmap = CreateBitmap((u32 *)at, width, height);
   
   return bitmap;
}


static void WriteTexture(char *fileName, Bitmap bitmap)
{
   DeferRestore(frameArena);
   
   Assert(bitmap.height == Asset_Bitmap_Size);
   Assert(bitmap.width == Asset_Bitmap_Size);
   Assert(bitmap.pixels);
   
   u8 *data = PushData(frameArena, u8, 0);
   
   *PushStruct(frameArena, u32) = 1; // version
   *PushStruct(frameArena, u32) = bitmap.width;
   *PushStruct(frameArena, u32) = bitmap.height;
   u32 *pixels = PushData(frameArena, u32, bitmap.width * bitmap.height);
   memcpy(pixels, bitmap.pixels, bitmap.width * bitmap.height * sizeof(u32));
   
   u32 fileSize = (u32)(frameArena->current - data);
   
   File file = CreateFile(data, fileSize);
   WriteEntireFile(fileName, file);
}

static bool WriteLevel(char *fileName, Level level, AssetHandler *assetHandler)
{
	u8 *mem = PushData(frameArena, u8, 0);
   
	*PushStruct(frameArena, u32) = 5; // Version number, do not remove
   
	*PushStruct(frameArena, LightSource) = level.lightSource;
   
	*PushStruct(frameArena, v3)         = level.camera.pos;
	*PushStruct(frameArena, Quaternion) = level.camera.orientation;
	*PushStruct(frameArena, f32)        = level.camera.aspectRatio;
	*PushStruct(frameArena, f32)        = level.camera.focalLength;
   
	*PushStruct(frameArena, u32) = level.entities.amount;
   
	For(level.entities)
	{
		*PushStruct(frameArena, Quaternion) = it->orientation;
		*PushStruct(frameArena, v3i) = it->physicalPos;
		*PushStruct(frameArena, v3)  = it->offset;
		*PushStruct(frameArena, v4)  = it->color;
		*PushStruct(frameArena, f32) = it->scale;
		*PushStruct(frameArena, u32) = it->type;
		*PushStruct(frameArena, u64) = it->flags;
		String s = GetAssetInfo(assetHandler, it->meshId).name;
		*PushStruct(frameArena, u32) = s.length;
		Char *dest = PushData(frameArena, Char, s.length);
		memcpy(dest, s.data, s.length * sizeof(Char));
	}
   
	u32 size = (u32)((u8*)frameArena->current - mem);
	File file = CreateFile(mem, size);
	return WriteEntireFile(fileName, file);
}

#define PullOff(type) *(type *)at; at += sizeof(type);

static Level LoadLevel(String fileName, Arena *arena, AssetHandler *assetHandler)
{
   File file = LoadFile(FormatCString("level/%s.level", fileName), frameArena);
	if (!file.fileSize) return {};
   
	u8 *at = (u8 *)file.memory;
	u32 version = PullOff(u32);
   
   Level level;
   
   level.name = CopyString(fileName, arena);
   level.lightSource = PullOff(LightSource);
   
	level.camera.pos = PullOff(v3);
   
   if(version < 5)
   {
      PullOff(v3);
      PullOff(v3);
      PullOff(v3);
      level.camera.orientation = QuaternionId();
   }
   else 
   {
      level.camera.orientation = PullOff(Quaternion);
   }
   
	level.camera.aspectRatio = PullOff(f32);
	level.camera.focalLength = PullOff(f32);
   
	u32 meshAmount = PullOff(u32);
   
	level.entities = PushArray(arena, EntityCopyData, meshAmount);
   
	For(level.entities)
	{
		it->orientation = PullOff(Quaternion);
		it->physicalPos = PullOff(v3i);
		it->offset = PullOff(v3);
		it->color = PullOff(v4);
		it->scale = PullOff(f32);
      it->type = (EntityType)PullOff(u32);
		it->flags = PullOff(u64);
      
		u32 nameLength = PullOff(u32);
		String name = CreateString((Char *)at, nameLength);
      at += nameLength * sizeof(Char);
      
      it->meshId = RegisterAsset(assetHandler, Asset_Mesh, name);
	}
   
	level.blocksNeeded = 10000;
   
	return level;
}

#undef PullOff
#undef PullOffArray

#undef PushOn
#undef PushOnArray
