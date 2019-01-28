
enum PathCreatorState
{
	PathCreator_None,
	PathCreator_CreatingPath,
	PathCreator_PlacingUnits, // or something
   PathCreator_DraggingDisplay,
   PathCreator_ScrollingDisplay,
   PathCreator_ResizingDisplay,
   
   PathCreator_Count,
};

struct PathCreator
{
	u32 hotUnit;
	PathCreatorState state;
   
   struct // UI
   {
      f32 scrollOffset;
      Rectangle2D displayRect;
   };
   
   Rectangle2D resetUnitButton;
	Rectangle2D	finishButton;
};

static PathCreator InitPathCreator()
{
	PathCreator ret;
	ret.hotUnit = 0xFFFFFFFF;
	ret.state = PathCreator_None;
	ret.finishButton = CreateRectangle2D(V2(0.8f, 0.7f), 0.2f, 0.1f);
	ret.resetUnitButton = CreateRectangle2D(V2(0.8f, 0.8f), 0.2f, 0.1f);
   
   ret.scrollOffset = 0.0f;
   ret.displayRect  = CreateRectangle2D(V2(0.7f, 0.1f), 0.2f, 0.6f);
   
   return ret;
}

// todo copy and paste of editor.h, they will differ eventrually, because this shout utilize the entity tree.
static Entity *GetHotEntity(Camera cam, EntityManager *entityManager, AssetHandler *handler,v2 mousePosZeroToOne)
{
   v3 camP = cam.pos; // todo camera or debugCamera? Maybe we should again unify them
   v3 camD = ScreenZeroToOneToDirecion(cam, mousePosZeroToOne);
   
   f32 minDist = F32MAX;
   
   Entity *ret = NULL;
   
   for(u32 i = 0; i < Entity_Count; i++)
   {
      For(entityManager->entityArrays[i])
      {
         MeshInfo *info = GetMeshInfo(handler, it->meshId);
         
         if (!info) continue; // probably not on screen, if never rendered
         
         m4x4 mat = QuaternionToMatrix4(Inverse(it->orientation)); // todo save these?
         v3 rayP = mat * (camP - it->visualPos);
         v3 rayD = mat * camD; 
         // better rayCast system, right now this loads every mesh, to find out the aabb....
         
         AABB aabb = info->aabb;
         
         aabb.maxDim *= it->scale;
         aabb.minDim *= it->scale;
         f32 curIntersectionMin = MAXF32;
         
         f32 x = rayP.x;
         f32 dx = rayD.x;
         f32 y = rayP.y;
         f32 dy = rayD.y;
         f32 z = rayP.z;
         f32 dz = rayD.z;
         
         f32 aabbMinX = aabb.minDim.x;
         f32 aabbMaxX = aabb.maxDim.x; 
         f32 aabbMinY = aabb.minDim.y;
         f32 aabbMaxY = aabb.maxDim.y;
         f32 aabbMinZ = aabb.minDim.z;
         f32 aabbMaxZ = aabb.maxDim.z;
         
         f32 t1x = (aabbMaxX - x) / dx;
         if (dx > 0 && t1x <= curIntersectionMin)
         {
            curIntersectionMin = t1x;
         }
         
         f32 t2x = (aabbMinX - x) / dx;
         if (dx < 0 && t2x <= curIntersectionMin)
         {
            curIntersectionMin = t2x;
         }
         
         f32 t1y = (aabbMaxY - y) / dy;
         if (dy > 0 && t1y <= curIntersectionMin)
         {
            curIntersectionMin = t1y;
         }
         
         f32 t2y = (aabbMinY - y) / dy;
         if (dy < 0 && t2y <= curIntersectionMin)
         {
            curIntersectionMin = t2y;
         }
         
         f32 t1z = (aabbMaxZ - z) / dz;
         if (dz > 0 && t1z <= curIntersectionMin)
         {
            curIntersectionMin = t1z;
         }
         
         f32 t2z = (aabbMinZ - z) / dz;
         if (dz < 0 && t2z <= curIntersectionMin)
         {
            curIntersectionMin = t2z;
         }
         v3 curExit = rayD * curIntersectionMin + rayP;
         
         
         if (PointInAABB(aabb, curExit))
         {
            f32 dist = Dist(curExit, rayP);
            if (dist < minDist)
            {
               minDist = dist;
               ret = it;
            }
         }
      }
   }
   return ret;
}



enum ExecuteState
{
   Execute_None,
   
   Execute_LevelBegin,
   Execute_PlacingUnits,
   Execute_PathCreator,
   Execute_Simulation,
   Execute_Victory,
   
   Execute_Count,
};

struct SimData
{
   u32 blocksNeeded;
   u32 blocksCollected;
   f32 timeScale;
};

struct ExecuteData
{
   u32 state;
   
   b32 debug;
   b32 middleMouseDown;
   Camera debugCamera;
   
   Camera camera;
   LightSource lightSource;
   
   u32 at; // are these used?
   f32 t;
   
   PathCreator pathCreator;
   SimData simData;
};

static void ChangeExecuteState(EntityManager *entityManager, ExecuteData *exe, ExecuteState state)
{
   exe->state = state;
   switch (state)
   {
      case Execute_LevelBegin:
      {
         
      }break;
      case Execute_PlacingUnits:
      {
         
      }break;
      case Execute_PathCreator:
      {
         exe->simData.timeScale = 1.0f;
         exe->at = 0;
         exe->t = 0.0f;
         
         exe->pathCreator = InitPathCreator();
         ResetEntityManager(entityManager);
      }break;
      case Execute_Simulation:
      {
         exe->simData.timeScale = 1.0f;
         exe->at = 0;
         exe->t = 0.0f;
         
         exe->simData.blocksCollected = 0;
         exe->simData.blocksNeeded = 1000;
         ResetEntityManager(entityManager);
      }break;
      case Execute_Victory:
      {
         
      }break;
      
      InvalidDefaultCase;
   }
}

static b32 ShouldPhysicallyMove(Entity *e)
{
   return (RoundToTileMap(e->visualPos) != e->physicalPos);
}

static b32 MaybePhysicallyMove(Entity *e, EntityManager *entityManager)
{
   v3i intendedPos = RoundToTileMap(e->visualPos);
   
   if(intendedPos == e->physicalPos) {return false;}
   
   EntityPtrArray blockingEntities = GetEntities(entityManager, intendedPos);
   
   if(blockingEntities.amount)
   {
      return false;
   }
   RemoveEntityFromTree(entityManager, e);
   e->physicalPos = intendedPos;
   InsertEntity(entityManager, e);
   
   e->flags |= EntityFlag_IsMoving;
   return true;
}

static void GameExecuteUpdate(EntityManager *entityManager, ExecuteData *exe, AssetHandler *assetHandler, Input input)
{
   
   TimedBlock;
   f32 timePassed = input.dt * exe->simData.timeScale;
   exe->t += timePassed;
   
   timePassed = 0.33f * exe->simData.timeScale;
   
   if(exe->debug && exe->middleMouseDown)
   {
      UnitData *data = entityManager->unitData.data;
      if(data)
      {
         Entity *entity = GetEntity(entityManager, data->serial);
         v3 proj = ScreenZeroToOneToZ(exe->debugCamera, V2(0.5f, 0.5f), entity->physicalPos.z);
         
         v3 newFocus = entity->visualPos;
         v3 delta =  newFocus - proj;
         exe->debugCamera.pos += delta;
         
         UpdateCamFocus(newFocus, &exe->debugCamera, input);
      }
   }
   
   RandomSeries series = GetRandomSeries();
   
   For(entityManager->unitData)
   {
      if(!it->instructions) continue;
      UnitInstruction currentInstruction = it->instructions[it->at % it->instructions.amount];
      
      switch(currentInstruction)
      {
         case Unit_Wait:
         {
            it->t += timePassed;
            if(it->t > 1.0f)
            {
               it->at++;
               it->t = 0.0f;
               //UnitInstruction nextInstruction = it->instructions[it->at % it->instructions.amount];
            }
         }break;
         case Unit_MoveUp:
         case Unit_MoveDown:
         case Unit_MoveLeft:
         case Unit_MoveRight:
         {
            Entity *e = GetEntity(entityManager, it->serial);
            
            if(ShouldPhysicallyMove(e))
            {
               v3i dir = GetAdvanceForOneStep(currentInstruction);
               Assert(e->physicalPos + dir == RoundToTileMap(e->visualPos));
               
               bool moved = MaybePhysicallyMove(e, entityManager);
               if(moved)
               {
                  Assert(!ShouldPhysicallyMove(e)); 
                  
                  it->at++;
                  //UnitInstruction nextInstruction = it->instructions[it->at % it->instructions.amount];
                  
                  e->visualPos = V3(e->physicalPos);
                  
                  { // supply animations
                     
                     // todo where to store these locally?
                     u32 animationID = RegisterAsset(assetHandler, Asset_Animation, "Run.animation");
                     AnimationState *anim = entityManager->animationStates + (u32)(it - entityManager->unitData.data);
                     InterpolationData data;
                     data.orientation = e->orientation;
                     data.scale       = V3(e->scale, e->scale, e->scale);
                     data.translation = e->visualPos;
                     AddAnimation(anim, animationID, 1.0f, 0.0f, data);
                     
#if 0
                     v3 delta;
                     if(RandomU32(&series) % 2)
                     {
                        delta = V3(100, 100, 100);
                        For(other, entityManager->unitArray)
                        {
                           v3 thisDelta = (other->visualPos - e->visualPos);
                           if(QuadNorm(thisDelta) < QuadNorm(delta))
                           {
                              delta = thisDelta;
                           }
                        }
                     }
                     else
                     {
                        delta = (exe->camera.pos - e->visualPos);
                     }
                     
                     AddIK(anim, 1, delta, 'x', 1, 1);
#endif
                  }
               }
               
            }
            else
            {   //should not physically moves
               v3i dir = GetAdvanceForOneStep(currentInstruction);
               e->visualPos = e->visualPos + timePassed * V3(dir);
               Quaternion intendedOrientation = AxisAngleToQuaternion((f32)atan2f((f32)dir.y, (f32)dir.x), V3(0, 0, 1));
               
               e->orientation = NLerp(e->orientation, timePassed, intendedOrientation);
            }
            
         }break;
         case Unit_FlipBit:
         {
            it->t += timePassed;
            if(it->t > 1.0f)
            {
               Entity *e = GetEntity(entityManager, it->serial);
               it->at++;
               it->t = 0.0f;
               EntityPtrArray maybeBits = GetEntities(entityManager, e->physicalPos);
               For(bit, maybeBits)
               {
                  if((*bit)->type != Entity_Bit) continue;
                  
                  Tweekable(v4, bitColorOne, V4(1.0f, 0.5f, 0.8f, 0.2f));
                  Tweekable(v4, bitColorTwo, V4(1.0f, 0.5f, 0.3f, 0.8f));
                  
                  BitData *bitData = GetBitData(entityManager, *bit);
                  bitData->value = !bitData->value;
                  
                  (*bit)->color = bitData->value ? bitColorOne : bitColorTwo;
               }
            }
         }break;
         
         
         InvalidDefaultCase;
      }
   }
   
   For(s, entityManager->animationStates)
   {
      ForC(s->animations)
      {
         it->t += it->timeScale * timePassed;
      }
      //s->animationLerpT -= dt;
   }
   
   For(state, entityManager->animationStates)
   {
      AnimationInput *first  = &state->animations[0];
      AnimationInput *second = &state->animations[1];
      TriangleMesh *mesh = GetMesh(assetHandler, state->meshId);
      
      if(first->animationId == 0xFFFFFFFF)
      {
         u32 animationId = RegisterAsset(assetHandler, Asset_Animation, "dudeTry1Run.animation");
         
         KeyFramedAnimation *animation = GetAnimation(assetHandler, animationId);
         InterpolationDataArray local = GetLocalTransforms(animation, first->t);
         
         
         // todo at  IK stuff here
         m4x4Array bones = LocalToWorld(&mesh->skeleton, local);
         for (u32 i = 0; i < mesh->skeleton.bones.amount; i++)
         {
            bones[i] = bones[i] * mesh->skeleton.bones[i].inverseBindShapeMatrix;
         }
         state->boneStates = bones;
         continue;
      }
      
      KeyFramedAnimation *animation = GetAnimation(assetHandler, first->animationId);
      
      InterpolationDataArray local = GetLocalTransforms(animation, first->t);
      
      ForC(state->iks)
      {
         if(it->boneIndex == 0xFFFFFFFF) continue;
         
         ApplyIK(mesh->skeleton.bones, local, it->boneIndex, it->focusP, it->depth, it->axis, it->iterations);
      }
      
      m4x4Array bones = LocalToWorld(&mesh->skeleton, local);
      
      for (u32 i = 0; i < mesh->skeleton.bones.amount; i++)
      {
         bones[i] = bones[i] * mesh->skeleton.bones[i].inverseBindShapeMatrix;
      }
      state->boneStates = bones;
   }
}
