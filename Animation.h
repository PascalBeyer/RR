
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

static m4x4Array LocalToSpace(Skeleton *skeleton, InterpolationDataArray data)
{
   m4x4Array ret = PushArray(frameArena, m4x4, skeleton->bones.amount);
   
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

static void ApplyIK(BoneArray bones, InterpolationDataArray local, m4x4Array spaceT, u32 index, v3 pos)
{
   Quaternion cur = QuaternionId();
   
   for(u32 it = index; it != 0xFFFFFFFF; it = bones[it].parentIndex)
   {
      if(bones[it].parentIndex == 0xFFFFFFFF) break;
      InterpolationData spaceData = MatrixToInterpolationData(spaceT[it]);
      spaceData.orientation *= cur;
      m3x3 mat = QuaternionToMatrix3(spaceData.orientation);
      v3 view = mat * V3(1, 0, 0);
      v3 boneP = spaceData.translation;
      
      local[it].orientation = LookAt(Normalize(view - boneP), Normalize(view - pos)) * local[it].orientation;
      
      cur = local[it].orientation * cur;
      *PushStruct(frameArena, u32) = it;
   }
   
   // recalc all matrices
   spaceT[0] = InterpolationDataToMatrix(local[0]);
   for (u32 i = 1; i < bones.amount; i++)
   {
      Bone *bone = bones + i;
      Assert(bone->parentIndex < i);
      // this way around: first transform the hand -> transform the hand according to the arm transform.
      spaceT[i] = spaceT[bone->parentIndex] * InterpolationDataToMatrix(local[i]);
   }
   
}



struct AnimationPlayer
{
   
};

static void AnimateUnits(EntityManager *entityManager)
{
   u32 animationID = RegisterAsset(assetHandler, Asset_Animation, "dudeTry1Run.animation");
   KeyFramedAnimation *animation = GetAnimation(assetHandler, animationID);
   
   For(entityManager->unitData)
   {
      Entity *e = GetEntity(entityManager, it->serial);
      if(it->isWalking)
      {
         TriangleMesh *mesh = GetMesh(assetHandler, e->meshId);
         InterpolationDataArray local = GetLocalTransforms(animation, it->t);
         m4x4Array bones = LocalToSpace(&mesh->skeleton, local);
         
         for (u32 i = 0; i < mesh->skeleton.bones.amount; i++)
         {
            bones[i] = bones[i] * mesh->skeleton.bones[i].inverseBindShapeMatrix;
         }
         
         
         return;
      }
   }
}