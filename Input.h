#ifndef RR_INPUT
#define RR_INPUT

#include "buffers.h"
#include "Vector2.h"

struct Input
{
	MouseInput *mouse;
	KeybordInput *keybord;
	v2 mousePos;
	v2 mouseDelta;
	v2 oldMousePos;
};

void UpdateInput(Input *input, MouseInput *mouseInput, KeybordInput *keybord);

char GetPressedChar(Input* input);


#endif
