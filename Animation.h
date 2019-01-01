
struct WeightData
{
	f32 weight;
	u32 jointIndex;
};
DefineArray(WeightData);
DefineArray(WeightDataArray);

struct KeyFrame
{
	f32 t;
	m4x4Array boneStates; // something
};
DefineArray(KeyFrame);

struct KeyFramedAnimation
{
	KeyFrameArray keyFrames;
	f32 length;
};

struct Skeleton
{
	u16Array vertexMap; // from flattend to not flattend

	v3Array vertices; // not flattend
	WeightDataArrayArray vertexToWeightsMap; // from not flattend
	StringArray boneNames;
};


struct DAEReturn
{
	TriangleMesh mesh;
	KeyFramedAnimation animation;
	Skeleton skeleton;
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

	if(!BeginsWithEat(line, "=\"")) { Die; return {}; };

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
	String bind_shape_matrix; // optional
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
	DAENodePtrDFArray rootBones;
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
			Eat(2, line);
			trans.val = EatToCharReturnHead(line, '<');
		}
		else if (BeginsWithEat(line, "<rotate"))
		{
			trans.type = DAE_Rotate;
			trans.sid = DAEEatAttAndReturnIt(line, "sid");
			Eat(2, line);
			trans.val = EatToCharReturnHead(line, '<');
		}
		else if(BeginsWithEat(line, "<scale"))
		{
			trans.type = DAE_Scale;
			trans.sid = DAEEatAttAndReturnIt(line, "sid");
			Eat(2, line);
			trans.val = EatToCharReturnHead(line, '<');
		}
		else if(BeginsWithEat(line, "<matrix"))
		{
			trans.type = DAE_Matrix;
			trans.sid = DAEEatAttAndReturnIt(line, "sid");
			Eat(2, line);
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

			String id     = DAEEatAttAndReturnIt(&line, "id"); // not sure, this does not seem to make a lot of sense, as they can only be one array per source?
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

			if(!BeginsWithEat(&line, "<accessor")) { Die; return {}; };
			String source = DAEEatAttAndReturnIt(&line, "source");
			String countS = DAEEatAttAndReturnIt(&line, "count");
			String strideS = DAEEatAttAndReturnIt(&line, "stride");

			u32 count =  StoU(countS,  &success);
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
				String line = ConsumeNextLineSanitizeEatSpaces(&remaining);
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

	TriangleMesh mesh;
	mesh.indexSets;
	mesh.vertices;
	mesh.indices;

	DAEMesh *daeMesh = gatheredData.meshes + 0;

	v3Array positions;
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
			b32 success = true;
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
			b32 success = true;
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
			b32 success = true;
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
			append->flattendIndex = flattener++;
			append->next = flatten[pI];
			append->normalIndex = nI;
			append->uvIndex = uvI;

			flatten[pI] = append;

			index = append->flattendIndex;
		}

		indices[i] = index;
	}

	u16Array skeletonVertexMap = PushArray(constantArena, u16, flattener);
	VertexFormatArray vertices = PushArray(constantArena, VertexFormat, flattener);
	
	For(flatten)
	{
		u32 it_index = (u32)(it - flatten.data);
		for (UVList *c = *it; c; c = c->next)
		{
			VertexFormat* v = &vertices[c->flattendIndex];
			v->c = 0xFFFFFFFF;
			v->p = positions[it_index];
			v->n = normals[c->normalIndex];
			v->uv = uvs[c->uvIndex];

			skeletonVertexMap[c->flattendIndex] = it_index;
		}
	}

	Assert(gatheredData.controllers.amount == 1);

	DAESkin *daeSkin = &gatheredData.controllers[0].skin;

	StringArray boneNames;
	m4x4Array boneTransformes;

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

			boneTransformes = PushArray(frameArena, m4x4, source->technique.count); // arena?

			For(boneTransformes)
			{
				*it = Eatm4x4(&nameIt, &success);
			}
			Assert(success);
			Assert(!nameIt.amount);
		}
	}

	Assert(boneTransformes.amount == boneNames.amount);
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
			it->jointIndex = Eatu32(&vIt, &success);
			EatSpaces(&vIt);
			u32 weightIndex = Eatu32(&vIt, &success);
			it->weight = weights[weightIndex];
			EatSpaces(&vIt);
		}
	}
	Assert(success);


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
		it->boneStates = PushArray(constantArena, m4x4, amountOfBones);

		for (u32 i = 0; i < amountOfBones; i++)
		{
			it->boneStates[i] = matrixArrayPerBone[i][it_index];
		}

		it->t = timeArrayPerBone[0][it_index];
	}

	TriangleMesh triangleMesh;
	triangleMesh.type = TriangleMeshType_List;
	triangleMesh.indices = indices;
	triangleMesh.vertices = vertices;
	triangleMesh.indexSets = PushArray(constantArena, IndexSet, 1);
	triangleMesh.indexSets[0].mat = {};
	triangleMesh.indexSets[0].offset = 0;
	triangleMesh.indexSets[0].amount = indices.amount;

	RegisterTriangleMesh(&triangleMesh);

	KeyFramedAnimation animation;
	animation.keyFrames = keyFrames;
	animation.length = keyFrames[amountOfKeyFrames - 1].t;

	Skeleton skeleton;
	skeleton.boneNames = jointNames; // right now these break // todo todo todo
	skeleton.vertices = positions;
	skeleton.vertexMap = skeletonVertexMap;
	skeleton.vertexToWeightsMap = vertexToWeightsMap;

	DAEReturn ret;
	ret.animation = animation;
	ret.skeleton = skeleton;
	ret.mesh = triangleMesh;

	return ret;
}





