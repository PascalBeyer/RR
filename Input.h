#ifndef RR_INPUT
#define RR_INPUT

enum KeyStateFlag
{
	KeyState_Up = 0x0,
	KeyState_Down = 0x1,
	KeyState_ReleasedThisFrame = 0x2,
	KeyState_PressedThisFrame = 0x4,
	KeyState_Repeaded = 0x8,
	KeyState_ShiftDown = 0x10,
	KeyState_ControlDown = 0x20
};


enum KeyEnum
{
	Key_invalid = 0x0,
	Key_leftMouse = 0x1,
	Key_rightMouse,

	Key_backSpace = 0x8,
	Key_tab = 0x9,
	Key_enter = 0xD,
	Key_shift = 0x10,
	Key_control = 0x11,
	Key_space = 0x20,
	Key_end = 0x23,
	Key_pos1 = 0x24,
	Key_left = 0x25,
	Key_up,
	Key_right,
	Key_down,

	Key_0 = 0x30,
	Key_1,
	Key_2,
	Key_3,
	Key_4,
	Key_5,
	Key_6,
	Key_7,
	Key_8,
	Key_9,

	Key_a = 0x41,
	Key_b,
	Key_c,
	Key_d,
	Key_e,
	Key_f,
	Key_g,
	Key_h,
	Key_i,
	Key_j,
	Key_k,
	Key_l,
	Key_m,
	Key_n,
	Key_o,
	Key_p,
	Key_q,
	Key_r,
	Key_s,
	Key_t,
	Key_u,
	Key_v,
	Key_w,
	Key_x,
	Key_y,
	Key_z,

	Key_plus = 0xBB,
	Key_comma = 0xBC,
	Key_minus = 0xBD,
	Key_dot = 0xBE,


	Key_F1 = 0x70,
	Key_F2,
	Key_F3,
	Key_F4,
	Key_F5,
	Key_F6,
	Key_F7,
	Key_F8,
	Key_F9,
	Key_F10,
	Key_F11,
	Key_F12,

	//non keyBord Key events
	 Key_mouseWheelForward = 0xFF,
	 Key_mouseWheelBack,

	 Key_count
};

struct KeyStateMessage
{
	KeyEnum key;
	u32 flag;
};

struct KeyMessageBuffer
{
	KeyStateMessage *messages;
	u32 amountOfMessages;
	u32 maxAmountOfMessages;

};

struct MouseInput
{
	long x;
	long y;
};

struct DEBUGKeyTracker 
{
	b32 wDown = false;
	b32 sDown = false;
	b32 dDown = false;
	b32 aDown = false;
	b32 spaceDown = false;
};

struct Input
{
	MouseInput mouse;
	KeyMessageBuffer buffer;
	v2 mousePos;
	v2 mouseDelta;
	v2 oldMousePos;
	v2 mouseZeroToOne;
	v2 mouseZeroToOneDelta;

	f32 secondsPerFrame;
};

static void UpdateInput(Input *input, MouseInput mouseInput, int windowWidth, int windowHeight, f32 targetSecondsPerFrame, KeyMessageBuffer messages)
{
	v2 mousePos = V2((float)mouseInput.x, (float)mouseInput.y);
	input->oldMousePos = input->mousePos;
	input->mouseDelta = mousePos - input->mousePos;
	input->mouse = mouseInput;
	input->mousePos = mousePos;

	v2 mouseZeroToOne = V2((float)mouseInput.x / (float)windowWidth, (float)mouseInput.y / (float)windowHeight);
	input->mouseZeroToOneDelta = mouseZeroToOne - input->mouseZeroToOne;
	input->mouseZeroToOne = mouseZeroToOne;
	input->secondsPerFrame = targetSecondsPerFrame;
	input->buffer = messages;
}

static char GetPressedChar(Input *input)
{
	Die;
}

#endif
