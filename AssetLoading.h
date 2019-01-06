
struct UVList
{
	u16 uvIndex;
	u16 flattendIndex;
	u16 normalIndex;
	UVList *next;
};
typedef UVList * UVListPtr;
DefineArray(UVListPtr);


// look at 1:50 in vertex vbo
static MaterialDynamicArray LoadMTL(String path, String fileName, Arena *arena)
{
	MaterialDynamicArray ret = MaterialCreateDynamicArray();
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
			cur.spectularExponent = StoF(line, &success);
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
         
			cur.ambientColor = V3(a1, a2, a3);
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
         
			cur.diffuseColor = V3(a1, a2, a3);
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
         
			cur.specularColor = V3(a1, a2, a3);
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
         
			cur.indexOfReflection = a1;
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

static TriangleMesh ReadObj(char *fileName, Arena *arena, Arena *workingArena)
{
   
	String filename = CreateString(fileName);
	String path = GetDirectioryFromFilePath(filename);
	TriangleMesh ret = {};
#if 0 // todo nuked for now, not debugging everything at once, and this needs work anyways.
   {
      String name = filename;
      name = EatToCharFromBackReturnTail(&name, '/');
      name = EatToCharReturnHead(&name, '.');
      ret.name = name;
   }
   
	//ret.skeleton = {}; not neccesary, as ret has {}
	u8 *frameArenaReset = frameArena->current;
	defer(frameArena->current = frameArenaReset);
   
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
	u32 amountOfVertsBefore = 1;
	u32 amountOfNormalsBefore = 1;
	u32 amountOfUVsBefore = 1;
	u32 amountOfIndeciesBefore = 0;
	String line = ConsumeNextLineSanitize(&string);
   
	u16PtrDynamicArray indexPointerArray = u16PtrCreateDynamicArray();
	IndexSetDynamicArray indexSets = IndexSetCreateDynamicArray();
   
	// begin on vertex daisy chain
	ret.vertices.data = (VertexFormatPCUN *)arena->current;
   
	while (string.length)
	{
		IndexSet cur;
      
		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }
		if (!string.length) break;
      
		// o
		{
			String head = EatToNextSpaceReturnHead(&line);
			Assert(head == "o");
			EatSpaces(&line);
			String o = line; // ignored for now
		}
      
		line = ConsumeNextLineSanitize(&string);
		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }
		if (!string.length) break;
      
		// todo  speed, here we first load and then copy later when we read in the triangles, not sure if we could make this better...
      
		BeginArray(frameArena, v3, tempVertexArray); // could silent fill and the push, which is not as "save".
		while (line[0] == 'v' && line[1] == ' ')
		{
			Eat1(&line);
			EatSpaces(&line);
			String f1 = EatToNextSpaceReturnHead(&line);
			EatSpaces(&line);
			String f2 = EatToNextSpaceReturnHead(&line);
			EatSpaces(&line);
			String f3 = EatToNextSpaceReturnHead(&line);
         
			f32 s1 = StoF(f1, &success);
			f32 s2 = StoF(f2, &success);
			f32 s3 = StoF(f3, &success);
			v3 *val = PushStruct(frameArena, v3);
			*val = V3(s1, s2, s3);
         
			line = ConsumeNextLineSanitize(&string);
		}
		EndArray(frameArena, v3, tempVertexArray);
      
		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }
		if (!string.length) break;
      
		BeginArray(frameArena, v2, textrueCoordinates);
		while (line[0] == 'v' && line[1] == 't')
		{
			Eat1(&line);
			Eat1(&line);
			EatSpaces(&line);
			String f1 = EatToNextSpaceReturnHead(&line);
			EatSpaces(&line);
			String f2 = EatToNextSpaceReturnHead(&line);
         
			f32 s1 = StoF(f1, &success);
			f32 s2 = StoF(f2, &success);
			v2 *val = PushStruct(frameArena, v2);
			*val = V2(s1, s2);
         
			line = ConsumeNextLineSanitize(&string);
		}
		EndArray(frameArena, v2, textrueCoordinates);
      
		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }
		if (!string.length) break;
		BeginArray(frameArena, v3, normals);
		while (line[0] == 'v' && line[1] == 'n')
		{
			Eat1(&line);
			Eat1(&line);
			EatSpaces(&line);
			String f1 = EatToNextSpaceReturnHead(&line);
			EatSpaces(&line);
			String f2 = EatToNextSpaceReturnHead(&line);
			EatSpaces(&line);
			String f3 = EatToNextSpaceReturnHead(&line);
         
			f32 s1 = StoF(f1, &success);
			f32 s2 = StoF(f2, &success);
			f32 s3 = StoF(f3, &success);
			v3 *val = PushStruct(frameArena, v3);
			*val = V3(s1, s2, s3);
         
			line = ConsumeNextLineSanitize(&string);
		}
		EndArray(frameArena, v3, normals);
      
		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }
		if (!string.length) break;
      
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
		if (!string.length) break;
      
		// s -smoothening, what ever that means
		{
			String head = EatToNextSpaceReturnHead(&line);
			Assert(head == "s");
			EatSpaces(&line);
         
			//u32 s = StoU(line, &success); // ignored for now
		}
      
		line = ConsumeNextLineSanitize(&string);
		while (string.length && line.length == 0 || line[0] == '#') { line = ConsumeNextLineSanitize(&string); }
		if (!string.length) break;
      
      
		Clear(workingArena);
		UVListPtrArray flatten = PushZeroArray(workingArena, UVListPtr, tempVertexArray.amount);
		u32 vertexArrayLength = 0;
      
		cur.offset = amountOfIndeciesBefore;
      
		BeginArray(frameArena, u16, indecies);
		ArrayAdd(&indexPointerArray, indecies.data);
		while (line[0] == 'f')
		{
			Eat1(&line);
         
			// we assume they are all here
         
			for (u32 parseIndex = 0; parseIndex < 3; parseIndex++)
			{
				EatSpaces(&line);
				String s1 = EatToCharReturnHead(&line, '/', ' ');
				Eat1(&line);
				String s2 = EatToCharReturnHead(&line, '/', ' ');
				Eat1(&line);
				String s3 = EatToCharReturnHead(&line, ' ');
            
            
				u32 u1 = StoU(s1, &success) - amountOfVertsBefore;		// this has to be relative to be an index, but also saved absolute wrt the flattend array
				u32 u2 = StoU(s2, &success) - amountOfUVsBefore;		// this is relative as we just need it to build our array
				u32 u3 = StoU(s3, &success) - amountOfNormalsBefore;	// this is also relative 
            
            //todo no handling for meshes, that have more then 0xFFFE verticies
            
            // we are flattening trough an array of vertex positions
				u16 flattenedIndex = 0xFFFF;
				for (UVListPtr it = flatten[u1]; it; it = it->next)
				{
					if (it->uvIndex == u2 && it->normalIndex == u3)
					{
						flattenedIndex = it->flattendIndex;
						break;
					}
				}
            
				if (flattenedIndex == 0xFFFF)
				{
					UVList *append = PushStruct(workingArena, UVList);
					append->flattendIndex = (u16)vertexArrayLength++;
					append->next = flatten[u1];
					append->uvIndex = (u16)u2;
					append->normalIndex = (u16)u3;
					flatten[u1] = append;
               
					flattenedIndex = append->flattendIndex;
				}
            
            u32 indexToWrite = flattenedIndex + ret.vertices.amount;
            Assert(indexToWrite < 0xFFFF);
            
				*PushStruct(frameArena, u16) = (u16)(indexToWrite);
			}
         
			line = ConsumeNextLineSanitize(&string);
		}
		EndArray(frameArena, u16, indecies);
		cur.amount = indecies.amount;
      
		ArrayAdd(&indexSets, cur);
      
		amountOfIndeciesBefore += indecies.amount;
		amountOfVertsBefore += tempVertexArray.amount;
		amountOfNormalsBefore += normals.amount;
		amountOfUVsBefore += textrueCoordinates.amount;
		ret.vertices.amount += vertexArrayLength;
      
		// vertexArrayLength allready incrented by flattening
		VertexFormatPCUNArray save = PushArray(arena, VertexFormatPCUN, vertexArrayLength);
		//
		// WARNING, we are daisy chaining here into constantArena and into frameArena
		//
		For(flatten)
		{
			u32 it_index = (u32)(it - &flatten[0]);
			for (UVListPtr i = *it; i; i = i->next)
			{
				save[i->flattendIndex].p = tempVertexArray[it_index];
				save[i->flattendIndex].uv = textrueCoordinates[i->uvIndex];
				save[i->flattendIndex].n = normals[i->normalIndex];
				save[i->flattendIndex].c = 0xFFFFFFFF;
			}
		}
	}
   
	ret.indices.data = (u16 *)arena->current;
	Assert(indexPointerArray.amount == indexSets.amount);
	for (u32 i = 0; i < indexPointerArray.amount; i++)
	{
		IndexSet set = indexSets[i];
		u16 *source = indexPointerArray[i];
		u16 *dest = PushData(arena, u16, set.amount);
		memcpy(dest, source, set.amount * sizeof(u16));
	}
	ret.indices.amount = (u32)((u16 *)arena->current - ret.indices.data);
   
	ret.indexSets = PushArray(arena, IndexSet, indexSets.amount);
	memcpy(ret.indexSets.data, indexSets.data, indexSets.amount * sizeof(IndexSet));
   
	ret.type = TriangleMeshType_List;
   
	AABB aabb = InvertedInfinityAABB();
   
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
   
	Assert(success);
#endif
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
   
	File file = LoadFile(fileName);
	if (!file.amount) return {};
	defer(FreeFile(file));
   
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
      
      
		while (remaining.length)
		{
			String line = ConsumeNextLineSanitizeEatSpaces(&remaining);
			if (line == "</library_effects>")
			{
				break;
			}
         
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
	triangleMesh.indexSets[0].mat = {};
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


#define PullOff(type) *(type *)at; at += sizeof(type);

#define PullOffArray(type, arr) \
(arr).amount = PullOff(u32); \
(arr).data = (type *)at; \
at += (arr).amount * sizeof(type); 

static void UnloadMesh(AssetInfo *info)
{
	Assert(info->currentlyLoaded);
	Assert(info->type == Asset_Mesh);
   
	DynamicFree(info->meshInfo->fileLocation);
}

static TriangleMesh LoadMesh(AssetHandler *assetHandler, char *fileName, void **filePtr)
{
	//return {};
	TriangleMesh ret;
   
	File file = LoadFile(fileName);
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
	File file = LoadFile(fileName);
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
static bool LoadBitmapIntoBitmap(char *filefile, Bitmap *texture)
{
	File file = LoadFile(filefile);
	defer(FreeFile(file));
	if (!file.fileSize) return false;
	u8 *at = (u8 *)file.memory;
	u32 version = *(u32 *)at;
	Assert(version == 1);
	at += sizeof(u32);
	u32 width = *(u32 *)at;
	at += sizeof(u32);
	u32 height = *(u32 *)at;
	at += sizeof(u32);
   
	memcpy(texture->pixels, at, width * height * sizeof(u32));
   
	texture->height = height;
	texture->width = width;
	return true;
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

#undef PullOff
#undef PullOffArray

#undef PushOn
#undef PushOnArray
