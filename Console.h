#ifndef RR_CONSOLE
#define RR_CONSOLE

#if 0
enum ConsoleState
{
	Console_Invalid		= 0x0,
	Console_Open		= 0x1,
	Console_Closed		= 0x2,
	Console_WideOpen	= 0x4
};
#endif

struct Console
{
	v2 minDim = V2(0.0f, 0.0f);
	v2 maxDim = V2(1.0f, 1.0f);
	f32 fontSize = 0.05f;
	f32 openness = 0.0f;
	f32 dt = 1.5f;
	f32 intededOpenness = 1.0f;

	String* history;
	u32 historyMaxLength;
	u32 historyLength = 0;

	f32 openWide = 1.0f;
	f32 open = 0.3f;
};

void UpdateConsole(Console *console, Input *input)
{
	if (input->keybord.c.pressedThisFrame)
	{
		f32 newIntededOpenness = input->keybord.shift.isDown ? console->openWide : console->open;

		if (console->intededOpenness == newIntededOpenness)
		{
			console->intededOpenness = 0.0f;
		}
		else
		{
			console->intededOpenness = newIntededOpenness;
		}
	}


	if (console->intededOpenness > console->openness)
	{
		console->openness += console->dt * input->secondsPerFrame;
		if(console->openness > console->intededOpenness)
		{ 
			console->openness = console->intededOpenness;
		}
	}
	else
	{
		console->openness -= console->dt * input->secondsPerFrame;
		if (console->openness < console->intededOpenness)
		{
			console->openness = console->intededOpenness;
		}
	}

	
}

// 0 to 1 screen space
static void DrawConsole(RenderGroup *rg, Console console) 
{
	v2 p1 = console.minDim;
	v2 p2 = V2(console.maxDim.x, console.minDim.y);
	v2 p3 = V2(console.minDim.x, console.openness * console.maxDim.y);

	v2 p4 = V2(console.maxDim.x, console.openness * console.maxDim.y);


	//main
	PushRectangle(rg, console.minDim, p4 - V2(0, console.fontSize), V4(1.0f, 0.6f, 0.7f, 0.95f));
	//typeField
	PushRectangle(rg, p3 - V2(0, console.fontSize), p4, V4(1.0f, 0.7f, 0.8f, 0.95f));



}


#endif // !RR_CONSOLE

