#include "UI.h"

UI::UI()
{

}

UI::~UI()
{

}

void UI::Update(Input *input)
{

	for (u32 i = 0; i < amountOfButtons; i++)
	{
		if (PointOnButton(buttons + i, input->mousePos))
		{
			buttons[i].currentColor = buttons[i].hoverColor;
			if (input->mouse->leftButtonPressedThisFrame)
			{
				buttons[i].OnClick();
				buttons[i].WhileDown();

			}
			else if (input->mouse->leftButtonDown)
			{
				for (u32 i = 0; i < amountOfButtons; i++)
				{
					if (PointOnButton(buttons + i, input->mousePos))
					{
						buttons[i].WhileDown();
					}
				}
			}

		}
		else
		{
			buttons[i].currentColor = buttons[i].defaultColor;
		}
	}
}