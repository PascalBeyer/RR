#include "Debug.h"



u32 GetDebugCounterIndex(char *fileName, int lineNumber, char *function, float camZ)
{
	return 0;
}

static v2 debugLastMousePos = V2();
static float mouseXRot = 0.0f;
static float mouseZRot = 0.0f;
static float camZoffSet = 0.0f;
static v2 camPlaneOffset = V2();

void DebugZeroCamera()
{
	debugLastMousePos = V2();
	mouseXRot = 0.0f;
	mouseZRot = 0.0f;
	camZoffSet = 0.0f;
	camPlaneOffset = V2();
}

float debugXRot()
{
	return mouseXRot;
}
float debugZRot()
{
	return mouseZRot;
}
float debugZOffset()
{
	return camZoffSet;
}
v2 debugPlaneOffset()
{
	return camPlaneOffset;
}

void UpdateDebugCamera(Input *input, float camZ)
{
	if (input->keybord->u.isDown)
	{
		if (input->keybord->u.pressedThisFrame)
		{
			debugLastMousePos = input->mousePos;
		}
		v2 mouseDelta = input->mousePos - debugLastMousePos;
		debugLastMousePos = input->mousePos;
		float rotSpeed = 0.001f * 3.141592f;
		mouseZRot += mouseDelta.x * rotSpeed;
		mouseXRot += mouseDelta.y * rotSpeed;
	}
	if (input->keybord->d.isDown)
	{
		if (input->keybord->d.pressedThisFrame)
		{
			debugLastMousePos = input->mousePos;
		}
		v2 mouseDelta = input->mousePos - debugLastMousePos;
		debugLastMousePos = input->mousePos;
		float zoomSpeed = 0.05f;
		camPlaneOffset += mouseDelta * zoomSpeed;
	}
	if (input->keybord->s.isDown)
	{
		if (input->keybord->s.pressedThisFrame)
		{
			debugLastMousePos = input->mousePos;
		}
		v2 mouseDelta = input->mousePos - debugLastMousePos;
		debugLastMousePos = input->mousePos;
		float zoomSpeed = 0.008f;
		camZoffSet += (Abs(camZ) + camZoffSet) * mouseDelta.y * zoomSpeed;

	}
}
