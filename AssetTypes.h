
struct Bitmap
{
	u32 *pixels;
	u32 width;
	u32 height;
	u32 textureHandle;
	//BitmapFileHeader *header;
};

static u32 RegisterWrapingTexture(u32 width, u32 height, u32 *pixels);
static void UpdateWrapingTexture(Bitmap bitmap);

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

struct Font
{
	Bitmap bitmap;
	CharInfo *charData;
	u32 amountOfChars = 0;
	f32 charHeight;
};

static Font globalFont;


static f32 GetActualStringLength(String string, f32 fontSize, Font font)
{
	f32 ret = 0.0f;
	float fScale = fontSize / (f32)font.charHeight;
   
	for (u32 i = 0; i < string.length; i++)
	{
		if (string[i] < font.amountOfChars)
		{
			CharData data = font.charData[string[i]];
         
			f32 scaledWidth = fScale * (f32)data.width;
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


struct InterpolationData // naming?
{
	v3 translation;
	Quaternion orientation;
	v3 scale;
};
DefineArray(InterpolationData);

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
	InterpolationData interp; // do I need this?
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
   
   VertexFormatType vertexType;
	u32 vertexVBO; // to init this call glBufferData and glBindData
	u32 indexVBO;
};

DefineArray(TriangleMesh);
DefineDynamicArray(TriangleMesh);

static void RegisterTriangleMesh(TriangleMesh *mesh);
