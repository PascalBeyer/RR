#ifndef RR_SCREEN
#define RR_SCREEN

#include "2DGeometry.h"
#include "buffers.h"
#include "Vector2.h"
#include "Vector3.h"
#include "World.h"


class Screen
{
public:
	Screen();
	Screen(float x, float y, float z, u32 pixelWidth, u32 pixelHeight, float width, float maxX, float maxY, float scrollSpeed, float tileSize, float screenScrollAreaWidth, float focalLength);
	~Screen();
	Camera cam;
	//Righthanded coordinates, +Z going into the screen, +Y is "down", +X is to the right

	v2 ScreenToInGame(v2 point);

	
	const float distFromScreen = 1.0f;

	float focalLength;
	float width, height; //in tiles ?

	u32 pixelWidth, pixelHeight; //in Pixels
	float tileSize;
	void Update(v2 mousePos);

private:
	float maxX, maxY;	
	float  screenScrollSpeed;
	float  screenScrollAreaWidth;
	
};



#endif
