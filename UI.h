#ifndef RR_UI
#define RR_UI

#include "Button.h"
#include "Input.h"

class UI
{
public:
	UI();
	~UI();

	void Update(Input *input);
	Button *buttons;
	u32 amountOfButtons;

private:

};
#endif