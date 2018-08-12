#include "Input.h"

void UpdateInput(Input *input, MouseInput *mouseInput, KeybordInput *keybordInput)
{
	v2 mousePos = V2((float)mouseInput->x, (float)mouseInput->y);
	input->oldMousePos = input->mousePos;
	input->mouseDelta = mousePos - input->mousePos;
	input->keybord = keybordInput;
	input->mouse = mouseInput;
	input->mousePos = mousePos;
	
}

char GetPressedChar(Input *input)
{
	if (input->keybord->d.pressedThisFrame)
	{
		return 'd';
	}
	else if (input->keybord->s.pressedThisFrame)
	{
		return 's';
	}
	else if (input->keybord->u.pressedThisFrame)
	{
		return 'u';
	}
	return '\0';
	
}