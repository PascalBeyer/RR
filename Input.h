#ifndef RR_INPUT
#define RR_INPUT


struct Input
{
	MouseInput mouse;
	KeybordInput keybord;
	v2 mousePos;
	v2 mouseDelta;
	v2 oldMousePos;
	v2 mouseZeroToOne;

	f32 secondsPerFrame;
};

static void UpdateInput(Input *input, MouseInput mouseInput, KeybordInput keybordInput, int windowWidth, int windowHeight, f32 targetSecondsPerFrame)
{
	v2 mousePos = V2((float)mouseInput.x, (float)mouseInput.y);
	input->oldMousePos = input->mousePos;
	input->mouseDelta = mousePos - input->mousePos;
	input->keybord = keybordInput;
	input->mouse = mouseInput;
	input->mousePos = mousePos;
	input->mouseZeroToOne = V2((float)mouseInput.x / (float)windowWidth, 1.0f - (float)mouseInput.y / (float) windowHeight);
	input->secondsPerFrame = targetSecondsPerFrame;
}

static char GetPressedChar(Input *input)
{
	Die;
}

#endif
