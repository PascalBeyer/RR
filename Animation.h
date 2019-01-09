
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

// assumed to be from zero to one
static InterpolationData Interpolate(InterpolationData prev, InterpolationData next, f32 t) 
{
	
	InterpolationData ret; 
	ret.translation = Lerp(prev.translation, t, next.translation);
	ret.scale		= Lerp(prev.scale, t, next.scale);
	ret.orientation = NLerp(prev.orientation, t, next.orientation);
   
	return ret;
}

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
   //Assert(skeleton->name == animation->meshName);
   Assert(skeleton->bones.amount == animation->keyFrames[0].boneStates.amount);
   
	m4x4Array ret = PushArray(frameArena, m4x4, skeleton->bones.amount);
   
	if (!animation)
	{
		ret[0] = InterpolationDataToMatrix(skeleton->bones[0].interp);
      
		for (u32 i = 1; i < skeleton->bones.amount; i++)
		{
			Bone *bone = skeleton->bones + i;
			Assert(bone->parentIndex < i);
			ret[i] =  ret[bone->parentIndex] * InterpolationDataToMatrix(bone->interp)  * bone->inverseBindShapeMatrix; // todo is it faster to do this multiplication in InterpolationData ?
		}
      
      for (u32 i = 0; i < skeleton->bones.amount; i++)
      {
         ret[i] = ret[i] * skeleton->bones[i].inverseBindShapeMatrix;
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
   
   for (u32 i = 0; i < skeleton->bones.amount; i++)
   {
      ret[i] = ret[i] * skeleton->bones[i].inverseBindShapeMatrix;
   }
   
	return ret;
}
