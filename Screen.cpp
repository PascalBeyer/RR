#include "Screen.h"
#include "Math.h"
Screen::Screen()
{
}

Screen::~Screen()
{
}

Screen::Screen(float x, float  y, float z, u32 pixelWidth, u32 pixelHeight, float  width, float  maxX, float  maxY, float  screenScrollSpeed, float tileSize, float screenScrollAreaWidth, float focalLength)
{
	cam.pos = V3(x, y, z);
	cam.basis = TransformBasis(v3StdBasis, XRotation(0.2f));
	//Righthanded coordinates, +Z going into the screen, +Y is "down", +X is to the right	

	this->focalLength = focalLength;
	this->tileSize = tileSize;
	this->pixelHeight = pixelHeight;
	this->pixelWidth = pixelWidth;
	this->width = width;
	this->height = ((float)pixelHeight / (float)pixelWidth) * width;
	this->maxX = maxX;
	this->maxY = maxY;
	this->screenScrollSpeed = screenScrollSpeed;
	this->screenScrollAreaWidth = screenScrollAreaWidth;

}
void Screen::Update(v2 mousePos)
{
	if (mousePos.x < screenScrollAreaWidth)
	{
		cam.pos.x -= screenScrollSpeed;
	}
		
	if (mousePos.y < screenScrollAreaWidth)
	{
		cam.pos.y -= screenScrollSpeed;
	}
		
	if (mousePos.x > pixelWidth - screenScrollAreaWidth)
	{
		cam.pos.x += screenScrollSpeed;

	}
		
	if (mousePos.y > pixelHeight - screenScrollAreaWidth)
	{
		cam.pos.y += screenScrollSpeed;

	}
		
}

v2 Screen::ScreenToInGame(v2 point)
{
	v3 camRectUL = cam.pos + cam.basis.d3 * focalLength * 2.5f - 0.5f * (width * cam.basis.d1 + height * cam.basis.d2);
	float posXinGame = point.x / (float)pixelWidth * width;
	float posYinGame = point.y / (float)pixelHeight * height;

	v3 inGameP = camRectUL + posXinGame  * cam.basis.d1 + posYinGame * cam.basis.d2;
	float ctPos = -cam.pos.z / (inGameP.z - cam.pos.z);

	return p12(ctPos * (inGameP - cam.pos) + cam.pos);
}
