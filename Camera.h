

//Righthanded coordinates, +Z going into the screen, +Y is "down" ? , +X is to the right
//todo just store the projection matrix instead of aspectratio and focal length?
struct Camera
{
	v3 pos;
	f32 aspectRatio;
	f32 focalLength;
   
	Quaternion orientation;
};

static void UpdateCamFocus(v3 focusPoint, Camera *cam, Input input)
{
   //
   // The general map should be Quaternion -> Basis, mapping q to the -> ROW <- vectors
   // of the associated Matrix
   //
   
   Quaternion c = cam->orientation;
   
   v2 mouseDelta = input.mouseDelta;
   
   f32 rotSpeed = 0.001f * 3.141592f;
   
   f32 mouseCXRot =  mouseDelta.y * rotSpeed;
   f32 mouseZRot  = -mouseDelta.x * rotSpeed;
   
   Quaternion rotX = AxisAngleToQuaternion(mouseCXRot, V3(1, 0, 0));
   Quaternion rotZ = AxisAngleToQuaternion(mouseZRot,  V3(0, 0, 1));
   Quaternion rot = rotX * c * rotZ;
   
   v3 delta = cam->pos - focusPoint;
   
   // todo really understand how this is the right math
   Quaternion conj = Inverse(c) * Inverse(rotX) * c * Inverse(rotZ);
   cam->pos = focusPoint +  QuaternionToMatrix3(conj) * delta;
   
   cam->orientation = rot;
}

static v2 ScreenZeroToOneToInGame(Camera cam, v2 point)
{
   
	m4x4 proj = Projection(cam.aspectRatio, cam.focalLength) * CameraTransform(cam.orientation, cam.pos);
	m4x4 inv = InvOrId(proj);
   
	v3 p1 = inv * V3(-1, -1, -1);
	v3 p2 = inv * V3(1, -1, -1);
	v3 p3 = inv * V3(-1, 1, -1);
	//v3 p4 = inv * V3(1, 1, -1);
   
	f32 posXinGame = point.x;
	f32 posYinGame =  1.0f - point.y;
   
	// this works as the screen in game is again a Rectangle
	v3 inGameP = p1 + posXinGame * (p2 - p1) + posYinGame * (p3 - p1);
   
	float ctPos = -cam.pos.z / (inGameP.z - cam.pos.z);
	return p12(ctPos * (inGameP - cam.pos) + cam.pos);
}

static v3 ScreenZeroToOneToZ(Camera cam, v2 point, i32 z)
{
   
	m4x4 proj = Projection(cam.aspectRatio, cam.focalLength) * CameraTransform(cam.orientation, cam.pos);
	m4x4 inv = InvOrId(proj);
   
	v3 p1 = inv * V3(-1, -1, -1);
	v3 p2 = inv * V3(1, -1, -1);
	v3 p3 = inv * V3(-1, 1, -1);
	//v3 p4 = inv * V3(1, 1, -1);
   
	f32 posXinGame = point.x;
	f32 posYinGame = 1.0f - point.y;
   
	// this works as the screen in game is again a Rectangle
	v3 inGameP = p1 + posXinGame * (p2 - p1) + posYinGame * (p3 - p1);
   
	float ctPos = ((f32)z - cam.pos.z) / (inGameP.z - cam.pos.z);
	return (ctPos * (inGameP - cam.pos) + cam.pos);
}

static v3 ScreenZeroToOneToScreenInGame(Camera cam, v2 point)
{
   
	m4x4 proj = Projection(cam.aspectRatio, cam.focalLength) * CameraTransform(cam.orientation, cam.pos);
	m4x4 inv = InvOrId(proj);
   
	v3 p1 = inv * V3(-1, -1, -1);
	v3 p2 = inv * V3(1, -1, -1);
	v3 p3 = inv * V3(-1, 1, -1);
	//v3 p4 = inv * V3(1, 1, -1);
   
	f32 posXinGame = point.x;
	f32 posYinGame = 1.0f - point.y;
   
	// this works as the screen in game is again a Rectangle
	v3 inGameP = p1 + posXinGame * (p2 - p1) + posYinGame * (p3 - p1);
   
	return inGameP;
}

static v3 ScreenZeroToOneToDirecion(Camera cam, v2 point)
{
	m4x4 proj = Projection(cam.aspectRatio, cam.focalLength) * CameraTransform(cam.orientation, cam.pos);
	m4x4 inv = InvOrId(proj);
   
	v3 p1 = inv * V3(-1, -1, -1);
	v3 p2 = inv * V3(1, -1, -1);
	v3 p3 = inv * V3(-1, 1, -1);
	v3 p4 = inv * V3(1, 1, -1);
   
	f32 posXinGame = point.x;
	f32 posYinGame = 1.0f - point.y;
   
	v3 inGameP = p1 + posXinGame * (p2 - p1) + posYinGame * (p3 - p1);
   
	return (inGameP - cam.pos);
}


struct Triangle
{
   v3 p1;
   u32 c1;
   v3 p2;
   u32 c2;
   v3 p3;
   u32 c3;
};

DefineArray(Triangle);

static u32 GrayFromU32(u32 i)
{
	u32 c = i & 0xFF;
	u32 ret = (0xFF << 24) | (c << 16) | (c << 8) | (c << 0);
	return ret;
}

#if 0
static void UpdateCamGame(Input *input, Camera *camera)
{
	Tweekable(f32, screenScrollBorder, 0.05f);
	Tweekable(f32, screenScrollSpeed, 1.0f);
   
	v2 asd = input->mouseZeroToOne - V2(0.5f, 0.5f);
   
	f32 boxNorm = BoxNorm(asd) + screenScrollBorder;
   
	if (boxNorm > 0.5f)
	{
		camera->pos += 2.0f * boxNorm * screenScrollSpeed * i12(asd);
	}
   
}


static void UpdateCamFocus(Input *input, Camera *camera, DEBUGKeyTracker *tracker)
{
	v3 focusPoint = V3(ScreenZeroToOneToInGame(*camera, V2(0.5f, 0.5f)), 0.0f);
   
	f32 factor = 1.0f + tracker->middleBack * 0.1f - tracker->middleForward * 0.1f;
   
	tracker->middleForward = false;
	tracker->middleBack = false;
   
	camera->pos = focusPoint + factor * (camera->pos - focusPoint);
   
	if (!tracker->middleDown)
	{
		return;
		
	}
   
	v2 mouseDelta = input->mouseDelta;
	
	float rotSpeed = 0.001f * 3.141592f;
	
	f32 mouseZRot = -mouseDelta.y * rotSpeed; // this should rotate around the z axis
	f32 mouseCXRot = mouseDelta.x * rotSpeed; // this should rotate around the camera x axis
   
	m3x3 cameraT = Rows3x3(camera->b1, camera->b2, camera->b3);
	m3x3 cameraTInv = Invert(cameraT);
   
	m3x3 id = cameraT * cameraTInv;
	m3x3 rotX = XRotation3x3(mouseZRot);
	m3x3 rotZ = ZRotation3x3(mouseCXRot);
	m3x3 rot = cameraTInv * rotX * cameraT * rotZ;
   
	v3 delta = camera->pos - focusPoint;
   
	camera->pos = focusPoint + rot * delta;
   
	camera->basis = TransformBasis(camera->basis, rot);
}

static void UpdateCamGodMode(Input *input, Camera *cam, DEBUGKeyTracker tracker)
{
	Tweekable(f32, cameraMoveSpeed);
#if 0
	if (input->keybord[Key_shift].flag & KeyState_Down)
	{
		moveSpeed = 0.25f;
	}
#endif
   
	if (tracker.wDown)
	{
		cam->pos += cam->basis.d3 * cameraMoveSpeed;
	}
	if (tracker.sDown)
	{
		cam->pos -= cam->basis.d3 * cameraMoveSpeed;
	}
	if (tracker.dDown)
	{
		cam->pos += cam->basis.d1 * cameraMoveSpeed;
	}
	if (tracker.aDown)
	{
		cam->pos -= cam->basis.d1 * cameraMoveSpeed;
	}
   
	if (tracker.spaceDown)
	{
		float rotSpeed = 0.001f * 3.141592f;
		m4x4 rot = XRotation(-rotSpeed * input->mouseDelta.y) * YRotation(rotSpeed * input->mouseDelta.x);
      
		m4x4 cameraMat = CamBasisToMat(cam->basis);
		m4x4 invCamMat = InvOrId(cameraMat);
      
		cam->basis = TransformBasis(cam->basis, cameraMat * rot * invCamMat);
	}
   
	
}
#endif
