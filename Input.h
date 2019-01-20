#ifndef RR_INPUT
#define RR_INPUT

enum KeyStateFlag
{
	KeyState_Up = 0x1,
	KeyState_Down = 0x2,
	KeyState_ReleasedThisFrame = 0x4,
	KeyState_PressedThisFrame = 0x8,
	KeyState_Repeaded = 0x10,
	KeyState_ShiftDown = 0x20,
	KeyState_ControlDown = 0x40
};


enum KeyEnum
{
	Key_invalid = 0x0,
	Key_leftMouse = 0x1,
	Key_rightMouse,
   
	Key_middleMouse = 0x04,
   
	Key_backSpace = 0x8,
	Key_tab = 0x9,
	Key_enter = 0xD,
	Key_shift = 0x10,
	Key_control = 0x11,
   
	Key_escape = 0x1B,
   
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
   
	Key_num0 = 0x60,
	Key_num1,
	Key_num2,
	Key_num3,
	Key_num4,
	Key_num5,
	Key_num6,
	Key_num7,
	Key_num8,
	Key_num9,
	Key_numMultiply,
	Key_numAdd,
	Key_numSeperator,
	Key_numSubtract,
	Key_numDot,
	key_numDivide,
   
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
   
	Key_plus = 0xBB,
	Key_comma = 0xBC,
	Key_minus = 0xBD,
	Key_dot = 0xBE,
	
	
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
DefineArray(KeyStateMessage);

struct MouseInput
{
	long x;
	long y;
};

struct Input
{
	MouseInput mouse;
   KeyStateMessageArray keyMessages;
	v2 mousePos;
	v2 mouseDelta;
	v2 oldMousePos;
	v2 mouseZeroToOne;
	v2 mouseZeroToOneDelta;
   
	f32 dt;
};

static void UpdateInput(Input *input,  MouseInput mouseInput, int windowWidth, int windowHeight, f32 secondsPerFrame, KeyStateMessageArray messages)
{
	v2 mousePos = V2((float)mouseInput.x, (float)mouseInput.y);
	input->oldMousePos = input->mousePos;
	input->mouseDelta = mousePos - input->mousePos;
	input->mouse = mouseInput;
	input->mousePos = mousePos;
   
	v2 mouseZeroToOne = V2((float)mouseInput.x / (float)windowWidth, (float)mouseInput.y / (float)windowHeight);
	input->mouseZeroToOneDelta = mouseZeroToOne - input->mouseZeroToOne;
	input->mouseZeroToOne = mouseZeroToOne;
	input->dt = secondsPerFrame;
	input->keyMessages = messages;
}

static Char KeyToChar(u32 keyCode, bool shiftDown)
{
	if (shiftDown)
	{
		switch (keyCode)
		{
         case Key_0:
         {
            return '=';
         }break;
         case Key_1:
         {
            return '!';
         }break;
         case Key_2:
         {
            return '"';
         }break;
         case Key_3:
         {
            Die; // apperantly that value is not utf8...
            return ' ';
         }break;
         case Key_4:
         {
            return '$';
         }break;
         case Key_5:
         {
            return '%';
         }break;
         case Key_6:
         {
            return '&';
         }break;
         case Key_7:
         {
            return '/';
         }break;
         case Key_8:
         {
            return '(';
         }break;
         case Key_9:
         {
            return ')';
         }break;
         case Key_dot:
         {
            return ':';
         }break;
         case Key_comma:
         {
            return ';';
         }break;
         case Key_minus:
         {
            return '_';
         }break;
		}
      
	}
	else
	{
		switch (keyCode)
		{
         case Key_dot:
         {
            return '.';
         }break;
         case Key_comma:
         {
            return ',';
         }break;
         case Key_minus:
         {
            return '-';
         }break;
		}
	}
   
	if (Key_a <= keyCode && keyCode <= Key_z)
	{
      Char shiftedKeyCode = (Char)(keyCode - Key_a);
		if (shiftDown)
		{
			return (shiftedKeyCode + 'A');
		}
		else
		{
			return (shiftedKeyCode + 'a');
		}
	}
	else if (Key_0 <= keyCode &&  keyCode <= Key_9)
	{
		u32 shiftedKeyCode = keyCode - Key_0;
		return (Char)(shiftedKeyCode + 0x30);
	}
	else if (keyCode == Key_space)
	{
		return ' ';
	}
	return 0;
}

#endif
