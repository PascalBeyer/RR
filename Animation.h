
// assumed to be from zero to one
static InterpolationData Interpolate(InterpolationData prev, InterpolationData next, f32 t) 
{
	
	InterpolationData ret; 
	ret.translation = Lerp(prev.translation, t, next.translation);
	ret.scale	   = Lerp(prev.scale, t, next.scale);
   if(Dot(prev.orientation, next.orientation) < 0.0f)
   {
      ret.orientation = NLerp(prev.orientation, t, Negate(next.orientation));
   }
   else
   {
      ret.orientation = NLerp(prev.orientation, t, next.orientation);
   }
   // he did one more thing here.
	return ret;
}

static InterpolationDataArray GetLocalTransforms(KeyFramedAnimation *animation, f32 t)
{
   f32 at = fmodf(t, animation->length);
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
   
   Assert(interpolationT <= 1.0f);
   Assert(interpolationT >= 0.0f);
   
   InterpolationDataArray prevBoneStates = prevKeyFrame->boneStates;
   InterpolationDataArray nextBoneStates = nextKeyFrame->boneStates;
   
   Assert(prevBoneStates.amount == nextBoneStates.amount);
   u32 amountOfBones = prevBoneStates.amount;
   
   InterpolationDataArray ret = PushArray(frameArena, InterpolationData, amountOfBones);
   
   for (u32 i = 0; i < amountOfBones; i++)
   {
      InterpolationData prev = prevBoneStates[i];
      InterpolationData next = nextBoneStates[i];
      
      ret[i] = Interpolate(prev, next, interpolationT);
   }
   
   return ret;
}

static m4x4Array LocalToWorld(Skeleton *skeleton, InterpolationDataArray data)
{
   m4x4Array ret = PushArray(frameArena, m4x4, skeleton->bones.amount);
   
#if 0
   ret[0] = InterpolationDataToMatrix(skeleton->bones[0].interp) * InterpolationDataToMatrix(data[0]);
   for (u32 i = 1; i < skeleton->bones.amount; i++)
   {
      Bone *bone = skeleton->bones + i;
      Assert(bone->parentIndex < i);
      // this way around: first transform the hand -> transform the hand according to the arm transform.
      ret[i] = ret[bone->parentIndex] * InterpolationDataToMatrix(bone->interp) * InterpolationDataToMatrix(data[i]);
   }
#endif
   ret[0] = InterpolationDataToMatrix(data[0]);
   for (u32 i = 1; i < skeleton->bones.amount; i++)
   {
      Bone *bone = skeleton->bones + i;
      Assert(bone->parentIndex < i);
      // this way around: first transform the hand -> transform the hand according to the arm transform.
      ret[i] = ret[bone->parentIndex] * InterpolationDataToMatrix(data[i]);
   }
   
   return ret;
}

static void ApplyIK(BoneArray bones, InterpolationDataArray local, u32 index, v3 pos, u32 desiredDepth, char axis, u32 iterations)
{
   v3 dir;
   switch (axis)
   {
      case 'x':
      {
         dir = V3(1, 0, 0);
      }break;
      case 'y':
      {
         dir = V3(0, 1, 0);
      }break;
      case 'z':
      {
         dir = V3(0, 0, 1);
      }break;
      default:
      {
         Die;
         dir = V3();
      }
   }
   
   InterpolationDataArray relevantData = PushArray(frameArena, InterpolationData, 0);
   for(u32 it = index; it != 0xFFFFFFFF; it = bones[it].parentIndex)
   {
      *PushStruct(frameArena, InterpolationData) = local[it];
   }
   EndArray(frameArena, InterpolationData, relevantData);
   
   for(i32 it = relevantData.amount - 2; it >= 0; it--)
   {
      relevantData[it] = relevantData[it] * relevantData[it + 1];
   }
   
   Quaternion cur = QuaternionId();
   u32 depth = 0;
   u32 dataIndex = relevantData.amount - 1;
   for(u32 it = index; it != 0xFFFFFFFF; it = bones[it].parentIndex)
   {
      InterpolationData spaceData = relevantData[dataIndex--];
      spaceData.orientation *= cur;
      m3x3 mat = QuaternionToMatrix3(spaceData.orientation);
      v3 view = mat * dir;
      v3 boneP = spaceData.translation;
      
      local[it].orientation = LookAt(Normalize(view - boneP), Normalize(view - pos)) * local[it].orientation;
      
      cur = local[it].orientation * cur;
      *PushStruct(frameArena, u32) = it;
      
      depth++;
      if(depth >= desiredDepth) break;
   }
   
}

struct AnimationInput // unused if animationId = 0xFFFFFFFF
{
   u32 animationId;
   f32 timeScale;
   f32 t;
   
   InterpolationData objectTransform;
};

struct IKInput // unused if boneIndex = 0xFFFFFFFF
{
   v3 focusP;
   Char axis;
   u32 depth;
   u32 iterations;
   u32 boneIndex;
};

struct AnimationState
{
   u32 meshId;
   u32 serial;
   //f32 animationLerpT;
   AnimationInput animations[2];
   IKInput iks[5]; // we should make this like an enum IK_Head, IK_LArm, ... orsth
   m4x4Array boneStates;
};
DefineDynamicArray(AnimationState);

static void AddAnimation(AnimationState *state, u32 animationId, f32 timeScale, f32 startT, InterpolationData mat)
{
   AnimationInput toAdd;
   toAdd.animationId = animationId;
   toAdd.timeScale   = timeScale;
   toAdd.t           = startT;
   toAdd.objectTransform = mat;
   
   if(state->animations[0].animationId == 0xFFFFFFFF)
   {
      state->animations[0] = toAdd;
   }
   else if(state->animations[1].animationId == 0xFFFFFFFF)
   {
      state->animations[1] = toAdd;
   }
   else
   {
      u32 place = (state->animations[0].t > state->animations[1].t); // not sure.
      state->animations[place] = toAdd;
   }
}

static void AddIK(AnimationState *state, u32 boneIndex, v3 focusP, Char axis, u32 iterations, u32 depth)
{
   for(u32 i = 0; i < ArrayCount(state->iks); i++)
   {
      if(state->iks[i].boneIndex == 0xFFFFFFFF)
      {
         IKInput toAdd;
         toAdd.boneIndex = boneIndex;
         toAdd.focusP = focusP;
         toAdd.axis = axis;
         toAdd.iterations = iterations;
         toAdd.depth = depth;
         
         state->iks[i] = toAdd;
         return;
      }
   }
}

static m4x4Array DumbBindShapeThing(AssetHandler *assetHandler, TriangleMesh *mesh)
{
   Skeleton *skeleton = &mesh->skeleton;
   m4x4Array ret = PushArray(frameArena, m4x4, skeleton->bones.amount);
   ret[0] = InterpolationDataToMatrix(skeleton->bones[0].interp);
   for(u32 i = 1; i < skeleton->bones.amount; i++)
   {
      ret[i] = ret[skeleton->bones[i].parentIndex] * InterpolationDataToMatrix(skeleton->bones[i].interp);
   }
   
   return ret;
}
