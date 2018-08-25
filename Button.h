#ifndef RR_BUTTON
#define RR_BUTTON


static void ButtonEventStub(void *data)
{

}

static void ButtomToggleBool(void* data)
{
	bool *theBool = (bool *)data;
	*theBool = !*theBool;
}

#pragma pack(push,1)
struct ButtonScrollDatum
{
	float *t;
	v2 *mousePos;
	v2 buttonPos;
	float width;
	float height;
};
#pragma pack(pop)

static void ButtonHorizontalScroll(void* data)
{
	ButtonScrollDatum *datum = (ButtonScrollDatum *)data;

	bool inX = (datum->buttonPos.x < datum->mousePos->x) && (datum->buttonPos.x + datum->width > datum->mousePos->x);
	bool inY = (datum->buttonPos.y < datum->mousePos->y) && (datum->buttonPos.y + datum->height > datum->mousePos->y);

	bool inside = inX & inY;

	if (inside)
	{
		*datum->t = ((datum->mousePos->x - datum->buttonPos.x) / datum->width);
	}
}

#pragma pack(push,1)
struct ButtonFloatAssignDatum
{
	float *t;
	float value;
};
#pragma pack(pop)


static void ButtonFloatAssign(void *data)
{
	ButtonFloatAssignDatum *datum = (ButtonFloatAssignDatum *)data;
	*datum->t = datum->value;
}

#pragma pack(push,1)
struct ButtonUint32AssignDatum
{
	u32 *toAssign;
	u32 value;
};
#pragma pack(pop)

static void ButtonUint32Assign(void *data)
{
	ButtonUint32AssignDatum *datum = (ButtonUint32AssignDatum *)data;
	*datum->toAssign = datum->value;
}

class Button
{
public:
	Button();
	Button(v2 pos, float width, float height, String text, void (*onClick)(void *data), void(*onRelease)(void *data), void(*whileDown)(void *data), void* dataToActOn);
	~Button();

	String text;

	void *dataToActOn;

	void OnClick();
	void OnRelease();
	void WhileDown();

	v4 hoverColor;
	v4 defaultColor;
	v4 currentColor;

	v2 pos;
	float width;
	float height;
	
	void (*onClick)(void *data);
	void (*onRelease)(void *data);
	void (*whileDown)(void *data);

};


inline bool PointOnButton(Button *button,  v2 point)
{
	bool inX = (button->pos.x < point.x) && (button->pos.x + button->width > point.x);
	bool inY = (button->pos.y < point.y) && (button->pos.y+ button->height > point.y);

	return inX & inY;
}

Button::Button()
{
	onClick = &ButtonEventStub;
	onRelease = &ButtonEventStub;
	whileDown = &ButtonEventStub;
	pos = v2();
	width = 0;
	height = 0;
}

Button::Button(v2 pos, float width, float  height, String text, void(*onClick)(void *data), void(*onRelease)(void *data), void(*whileDown)(void *data), void *dataToActOn)
{
	this->onClick = onClick;
	this->onRelease = onRelease;
	this->whileDown = whileDown;
	this->pos = pos;
	this->width = width;
	this->height = height;
	this->dataToActOn = dataToActOn;
	this->text = text;
}

Button::~Button()
{

}

void Button::OnClick()
{
	onClick(dataToActOn);
}
void Button::OnRelease()
{
	onRelease(dataToActOn);
}
void Button::WhileDown()
{
	whileDown(dataToActOn);
}


#endif // !RR_BUTTON

