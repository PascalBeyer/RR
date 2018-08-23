#ifndef RR_BUTTON
#define RR_BUTTON

#include "ButtonFunction.h"
#include "String.h"
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


#endif // !RR_BUTTON

