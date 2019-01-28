
enum EntityType // warning gets used in  files
{
	Entity_None,
   
	Entity_Dude,
   Entity_Wall,
	Entity_Bit,
   
   Entity_Count,
};

enum EntityFlags
{
	EntityFlag_SupportsUnit = 0x1,
	EntityFlag_BlocksUnit = 0x2,
   EntityFlag_Interactable = 0x4,
   
   EntityFlag_IsDynamic = 0x200,
   
   // dynamic flags
	EntityFlag_IsMoving = 0x20,
	EntityFlag_InTree = 0x40,
	
   EntityFlag_FrameResetFlag = EntityFlag_IsMoving,
	
};


struct Entity
{
   // general entity data header if you will
   EntityType type;
   u32 serial;
   
   // render stuff
   u32 meshId;
	f32 scale;
	Quaternion orientation;
   v4 color;
   v4 frameColor;
   v3 visualPos;
	
   // simulation stuff, also needed for render
   v3i physicalPos;
	v3i initialPos;
	u64 flags;
   
};

typedef Entity* EntityPtr;
DefineDynamicArray(Entity);
DefineDynamicArray(EntityPtr);
DefineArray(Entity);
DefineArray(EntityPtr);
DefineDFArray(EntityPtr);

struct EntityData
{
   Quaternion orientation;
   v3i physicalPos;
   v3  offset;
   v4  color;
   f32 scale;
   
   EntityType type;
   u64 flags; // this is kinda useless
   
	u32 meshId;
};

static EntityData EntityToData(Entity e)
{
	EntityData ret;
   
	ret.color         = e.color;
	ret.flags         = e.flags;
	ret.orientation   = e.orientation;
	ret.meshId        = e.meshId;
	ret.scale         = e.scale;
	ret.type          = e.type;
	ret.physicalPos   = e.physicalPos;
   
	ret.offset        = e.visualPos - V3(e.physicalPos);
	return ret;
   
}

static void ApplyEntityDataToEntity(Entity *e, EntityData *data)
{
   e->orientation = data->orientation;
   e->physicalPos = data->physicalPos;
   e->initialPos  = data->physicalPos;
   e->visualPos   = V3(data->physicalPos) + data->offset;
   e->color       = data->color;
   e->scale	   = data->scale;
   
   e->type		= data->type;
   e->flags	   = data->flags;
   
   e->meshId      = data->meshId;
}

DefineArray(EntityData);
DefineDynamicArray(EntityData);

struct LightSource
{
   v3 pos;
   Quaternion orientation;
   v3 color;
};

// todo right now I do not see a reason why this should be an asset, so where should it live?
struct Level
{
   String name;
   
	Camera camera;
   LightSource lightSource;
	EntityDataArray entities;
	u32 blocksNeeded;
   
	//u32 amountOfDudes; right now not used.
};


struct AABBi
{
	v3i minDim;
	v3i maxDim;
};

static AABBi CreateAABBi(v3i minDim, v3i maxDim)
{
	AABBi ret;
	ret.minDim = minDim;
	ret.maxDim = maxDim;
	return ret;
}

static b32 PointInAABBi(AABBi aabb, v3i point)
{
	return
      (
		aabb.minDim.x <  point.x &&
		aabb.minDim.y <  point.y &&
		aabb.minDim.z <  point.z &&
		aabb.maxDim.x >= point.x &&
		aabb.maxDim.y >= point.y &&
		aabb.maxDim.z >= point.z
      );
}


struct TextureIndex
{
   u32 index;
};
static TextureIndex CreateTextureIndex(u32 index)
{
   return {index};
}

struct Bitmap
{
	u32 *pixels;
	u32 width;
	u32 height;
};

static void UpdateWrapingTexture(TextureIndex textureIndex, u32 width, u32 height, u32 *pixels);

static u32 *GetPixel(Bitmap bitmap, u32 x, u32 y)
{
	Assert(x < bitmap.width && y < bitmap.height);
	u32 shift = y * bitmap.width + x;
	u32 *pixP = bitmap.pixels + shift;
	return pixP;
}

struct CharInfo
{
	v2 minUV;
	v2 maxUV;
	f32 xAdvance;
	u32 width;
	u32 height;
	f32 xOff;
	f32 yOff;
};
typedef CharInfo CharData;
DefineArray(CharInfo);

struct Font
{
   void *mem;
   u32 textureId;
	CharInfoArray charData;
	f32 charHeight;
};

static Font globalFont;


static f32 GetActualStringLength(String string, f32 fontSize, Font font)
{
	f32 ret = 0.0f;
	float fScale = fontSize / (f32)font.charHeight;
   
	For(string)
	{
		if (*it < font.charData.amount)
		{
			CharData data = font.charData[*it];
         
			//f32 scaledWidth = fScale * (f32)data.width;
			float actualFloatWidth = data.xAdvance * fScale;
			ret += actualFloatWidth;
         
		}
		else
		{
			Assert(!"Not handled font symbol");
		}
	}
   
	return ret;
}


struct Material
{
	f32 specularExponent;		// Ns
	v3 ka;		            	// ambientColor
	v3 kd;	            		// diffuseColor
	v3 ks;	            		// specularColor
	v3 ke;						// emmissionColor
   
	f32 indexOfRefraction;		// Ni ("optical density")	0.001 - 10
	f32 dissolved;				// d						0 - 1	
   //v3 transmissionFilter;	  // Tf						
	u32 illuminationModel;		// illum					0 - 10
	u32 bitmapID;
	String name;
	String texturePath;
};
DefineDFArray(Material);
DefineDynamicArray(Material);

struct IndexSet
{
	u32 amount;
	u32 offset;
   
	Material mat;
};
DefineArray(IndexSet);
DefineDynamicArray(IndexSet);

struct WeightData
{
	f32 weight;
	u32 boneIndex;
};
DefineArray(WeightData);
DefineArray(WeightDataArray);

// todo where should these sort of mathy types live?
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


struct InterpolationData // naming?
{
	v3 translation;
	Quaternion orientation;
	v3 scale;
};
DefineArray(InterpolationData);

static InterpolationData operator*(InterpolationData a, InterpolationData b)
{
   m3x3 mat = QuaternionToMatrix3(a.orientation);
   InterpolationData ret;
   
   v3 gScale = a.scale * (mat* b.scale);
   ret.scale = V3(Abs(gScale.x), Abs(gScale.y), Abs(gScale.z));
   
   ret.translation = a.translation + a.scale * (mat * b.translation);
   ret.orientation = a.orientation * b.orientation;
   
   return ret;
}

// inverse almost as fast
static m4x4 InterpolationDataToMatrix(v3 translation, Quaternion orientation, f32 scale)
{
	m4x4 ret;
   
	ret = QuaternionToMatrix4(orientation);
	ret = Translate(ret, translation);
	ret.a[0][0] *= scale;
	ret.a[0][1] *= scale;
	ret.a[0][2] *= scale;
   
	ret.a[1][0] *= scale;
	ret.a[1][1] *= scale;
	ret.a[1][2] *= scale;
   
	ret.a[2][0] *= scale;
	ret.a[2][1] *= scale;
	ret.a[2][2] *= scale;
	
	return ret;
}

static InterpolationData MatrixToInterpolationData(m4x4 mat)
{
	// copied from (XForm states : GetFromMatrix in jBlow: Animation Playback part 2 37 min.), slightly altered
	InterpolationData ret;
   
	v3 mx = GetRow(mat, 0);
	v3 my = GetRow(mat, 1);
	v3 mz = GetRow(mat, 2);
   
	f32 normx = Norm(mx);
	f32 normy = Norm(my);
	f32 normz = Norm(mz);
   
	ret.scale = V3(normx, normy, normz);
	ret.translation = GetColumn(mat, 3);
   
	mx /= normx;
	my /= normy;
	mz /= normz;
   
	f32 coef[3][3];
	coef[0][0] = mx.x;
	coef[0][1] = mx.y;
	coef[0][2] = mx.z;
   
	coef[1][0] = my.x;
	coef[1][1] = my.y;
	coef[1][2] = my.z;
   
	coef[2][0] = mz.x;
	coef[2][1] = mz.y;
	coef[2][2] = mz.z;
   
	f32 trace = coef[0][0] + coef[1][1] + coef[2][2];
   
	Quaternion q;
   
	if (trace > 0.0f)
	{
		// |w| > 1/2
		f32 s = Sqrt(trace + 1.0f); // 2w
      
		q.w = s * 0.5f;
      
		s = 0.5f / s; // 1/(4f)
      
		q.x = (coef[2][1] - coef[1][2]) * s;
		q.y = (coef[0][2] - coef[2][0]) * s;
		q.z = (coef[1][0] - coef[0][1]) * s;
      
	}
	else
	{
		// |w| <= 1/2
		i32 i = 0;
		if (coef[1][1] > coef[0][0]) i = 1;
		if (coef[2][2] > coef[i][i]) i = 2;
      
		i32 j = (1 << i) & 3; // i+1 mod 3.
		i32 k = (1 << j) & 3; // j+1 mod 3.
      
		f32 s = Sqrt(coef[i][i] - coef[j][j] - coef[k][k] + 1.0f);
      
		q.component[i] = s * 0.5f;
		
		s = 0.5f / s;
		q.component[j] = (coef[i][j] + coef[j][i]) * s;
		q.component[k] = (coef[k][i] + coef[i][k]) * s;
		q.w = (coef[k][j] - coef[j][k]) * s;
	}
   
	ret.orientation = q;
   
	f32 norm = Norm(q);
	Assert(0.99f < norm && norm < 1.01f);
   
	return ret;
}

static m4x4 InterpolationDataToMatrix(InterpolationData a)
{
	m4x4 ret;
   
	ret = QuaternionToMatrix4(a.orientation);
	ret = Translate(ret, a.translation);
	ret.a[0][0] *= a.scale.x;
	ret.a[0][1] *= a.scale.x;
	ret.a[0][2] *= a.scale.x;
   
	ret.a[1][0] *= a.scale.y;
	ret.a[1][1] *= a.scale.y;
	ret.a[1][2] *= a.scale.y;
   
	ret.a[2][0] *= a.scale.z;
	ret.a[2][1] *= a.scale.z;
	ret.a[2][2] *= a.scale.z;
	
	return ret;
}


struct KeyFrame
{
	f32 t;
	InterpolationDataArray boneStates; // something
};
DefineArray(KeyFrame);

// mesh name right now is on the 2nd node of the visual scene, which we otherwise do not use.
struct KeyFramedAnimation  
{
	String meshName;
	String animationName;
	KeyFrameArray keyFrames;
	f32 length;
};

DefineDynamicArray(KeyFramedAnimation);


struct Bone
{
	String name;
	u32 parentIndex;
	m4x4 inverseBindShapeMatrix;
	InterpolationData interp; // do I need this? yes for IK
};
DefineArray(Bone);

struct Skeleton
{
	u16Array vertexMap; // from flattend to not flattend
	v3Array  vertices;  // not flattend
	WeightDataArrayArray vertexToWeightsMap; // from not flattend
   
	BoneArray bones;
	// m4x4 bindShapeMatrix; // this should be premultiplied into the bindshape matricies above? and is now.
};

enum TriangleMeshType
{
	TriangleMeshType_List,
	TrianlgeMeshType_Strip,
};

enum VertexFormatType
{
   VertexFormat_PC,
   VertexFormat_PCU,
   VertexFormat_PCUN,
   VertexFormat_PCUNBD,
};

// NOTE  WARNING The identifiers have to be unique, st this dumb hack works
// this is used to simplify the OffsetOf situation for setting up Attrib arrays
struct VertexFormatTemplate
{
   v3  p;
   u32 c;
   v2  uv;
   union 
   {
      v3  n;
      u16 textureIndex;
   };
   
   v4i bi;
   v4  bw;
};


struct VertexFormatPC
{
	v3  p;
	u32 c;
};

struct VertexFormatPCU
{
	v3  p;
	u32 c;
   v2  uv;
};
DefineArray(VertexFormatPC);


struct VertexFormatPCUI
{
	v3  p;
	u32 c;
   v2  uv;
   u16 textureIndex;
};

struct VertexFormatPCUN
{
	v3  p;
   u32 c;
	v2  uv;
	v3  n;
};
DefineArray(VertexFormatPCUN);

struct VertexFormatPCUNBD
{
   v3  p;
   u32 c;
   v2  uv;
   v3  n;
   v4i bi;
   v4  bw;
};


struct TriangleMesh
{
	String name;
	u32 type;
   
   // these are allready linearized, but we pack em in RegisterTriangleMesh
   v3Array positions;
   u32Array colors;
   v2Array uvs;
   v3Array normals;
   
	u16Array indices;
	IndexSetArray indexSets;
   
	Skeleton skeleton;
   
	AABB aabb;
   
   // OpenGL draw stuff
   u32 vertexFormatSize;
   u32 vertexVBO; // to init this call glBufferData and glBindData
	u32 indexVBO;
};

DefineArray(TriangleMesh);
DefineDynamicArray(TriangleMesh);

static void RegisterTriangleMesh(TriangleMesh *mesh);


