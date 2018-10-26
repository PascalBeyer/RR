#ifndef RR_SCREEN
#define RR_SCREEN

struct Screen
{
	f32 maxX, maxY;
	f32 screenScrollSpeed;
	f32 screenScrollAreaWidth;
};


static Screen CreateScreen(float width, f32 height, float maxX, float maxY, float screenScrollSpeed, float screenScrollAreaWidth)
{
	Screen ret;
	ret.maxX = maxX;
	ret.maxY = maxY;
	ret.screenScrollSpeed = screenScrollSpeed;
	ret.screenScrollAreaWidth = screenScrollAreaWidth;
	return ret;
}

static v2 ScreenZeroToOneToInGame(Camera cam, v2 point, f32 aspectRatio, f32 focalLength)
{

	m4x4 proj = Projection(aspectRatio, focalLength) * CameraTransform(cam.basis.d1, cam.basis.d2, cam.basis.d3, cam.pos);
	m4x4 inv = InvOrId(proj);
	
	v3 p1 = inv * V3(-1, -1, -1);
	v3 p2 = inv * V3(1, -1, -1);
	v3 p3 = inv * V3(-1, 1, -1);
	v3 p4 = inv * V3(1, 1, -1);

	f32 posXinGame = point.x;
	f32 posYinGame = point.y;

	v3 inGameP = p1 + posXinGame * (p2 - p1) + posYinGame * (p3 - p1);

	float ctPos = -cam.pos.z / (inGameP.z - cam.pos.z);
	return p12(ctPos * (inGameP - cam.pos) + cam.pos);
}




#endif
