#include "Button.h"


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
