
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


struct VertexFormatPC
{
	v3 p;
	u32 c;
};

struct VertexFormatPCU
{
	v3 p;
	u32 c;
   v2 uv;
};
DefineArray(VertexFormatPC);

struct VertexFormatPCUN
{
	v3 p;
   u32 c;
	v2 uv;
	v3 n;
};
DefineArray(VertexFormatPCUN);

enum TriangleMeshType
{
	TriangleMeshType_List,
	TrianlgeMeshType_Strip,
};

struct Material
{
	f32 spectularExponent;		// Ns
	v3 ambientColor;			// Ka
	v3 diffuseColor;			// Kd
	v3 specularColor;			// Ks
	v3 ke;						// not in the spec and always (0,0,0)
   
	f32 indexOfReflection;		// Ni ("optical density")	0.001 - 10
	f32 dissolved;				// d						0 - 1	
   //v3 transmissionFilter;		// Tf						
	u32 illuminationModel;		// illum					0 - 10
	u32 bitmapID;
	String name;
	String texturePath;
};

DefineDynamicArray(Material);

struct IndexSet
{
	u32 amount;
	u32 offset;
   
	Material mat;
};

DefineArray(IndexSet);

DefineDynamicArray(IndexSet);

struct InterpolationData // naming?
{
	v3 translation;
	Quaternion orientation;
	v3 scale;
};
DefineArray(InterpolationData);

static m4x4 InterpolationDataToMatrix(InterpolationData a)
{
	m4x4 ret;
   
	ret = QuaternionToMatrix(a.orientation);
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

// assumed to be from zero to one
static InterpolationData Interpolate(InterpolationData prev, InterpolationData next, f32 t) 
{
	
	InterpolationData ret; 
	ret.translation = Lerp(prev.translation, t, next.translation);
	ret.scale		= Lerp(prev.scale, t, next.scale);
	ret.orientation = NLerp(prev.orientation, t, next.orientation);
   
	return ret;
}

struct WeightData
{
	f32 weight;
	u32 boneIndex;
};
DefineArray(WeightData);
DefineArray(WeightDataArray);

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
	v3Array vertices; // not flattend
	WeightDataArrayArray vertexToWeightsMap; // from not flattend
   
	BoneArray bones;
	// m4x4 bindShapeMatrix; // this should be premultiplied into the bindshape matricies above? and is now.
};

struct TriangleMesh
{
	String name;
	u32 type;
	VertexFormatPCUNArray vertices;
	u16Array indices;
	IndexSetArray indexSets;
   
	Skeleton skeleton;
   
	AABB aabb;
   
	u32 vertexVBO; // to init this call glBufferData and glBindData
	u32 indexVBO;
};

DefineArray(TriangleMesh);
DefineDynamicArray(TriangleMesh);

void RegisterTriangleMesh(TriangleMesh *mesh);

static m4x4Array InterpolateKeyFrames(InterpolationDataArray prevBoneStates, InterpolationDataArray nextBoneStates, f32 t)
{
	Assert(prevBoneStates.amount == nextBoneStates.amount);
	u32 amountOfBones = prevBoneStates.amount;
	
   
	m4x4Array ret = PushArray(frameArena, m4x4, amountOfBones);
   
	for (u32 i = 0; i < amountOfBones; i++)
	{
		InterpolationData prev = prevBoneStates[i];
		InterpolationData next = nextBoneStates[i];
      
		ret[i] = InterpolationDataToMatrix(Interpolate(prev, next, t));
	}
   
	return ret;
}

static m4x4Array CalculateBoneTransforms(Skeleton *skeleton, KeyFramedAnimation *animation, f32 t)
{
	m4x4Array ret = PushArray(frameArena, m4x4, skeleton->bones.amount);
   
	
	if (!animation)
	{
		ret[0] = InterpolationDataToMatrix(skeleton->bones[0].interp);
      
		for (u32 i = 1; i < skeleton->bones.amount; i++)
		{
			Bone *bone = skeleton->bones + i;
			Assert(bone->parentIndex < i);
			ret[i] =  ret[bone->parentIndex] * InterpolationDataToMatrix(bone->interp); // todo is it faster to do this multiplication in InterpolationData ?
		}
      
		return ret;
	}
	
	f32 at = (f32)fmod(t, animation->length);
   
	KeyFrame *nextKeyFrame = NULL;
	KeyFrame *prevKeyFrame = NULL;
	For(animation->keyFrames)
	{
		if (it->t > at)
		{
			nextKeyFrame = it;
			prevKeyFrame = it - 1;
			break;
		}
	}
   
	f32 endT = nextKeyFrame->t;
	f32 begT;
	if (prevKeyFrame < animation->keyFrames.data)
	{
		prevKeyFrame = &animation->keyFrames[animation->keyFrames.amount - 1];
		begT = prevKeyFrame->t - animation->length;
	}
	else
	{
		begT = prevKeyFrame->t;
	}
   
	Assert(endT != begT);
   
	f32 interpolationT = (at - begT) / (endT - begT);
   
	m4x4Array transformedMatrices = InterpolateKeyFrames(prevKeyFrame->boneStates, nextKeyFrame->boneStates, interpolationT);
   
	ret[0] = transformedMatrices[0];
	for (u32 i = 1; i < skeleton->bones.amount; i++)
	{
		Bone *bone = skeleton->bones + i;
		Assert(bone->parentIndex < i);
		// this way around: first transform the hand -> transform the hand according to the arm transform.
		ret[i] = ret[bone->parentIndex] * transformedMatrices[i];
	}
   
	return ret;
}
