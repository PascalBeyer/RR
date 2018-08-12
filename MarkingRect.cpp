#include "MarkingRect.h"

using namespace Shape;

MarkingRect::MarkingRect() : Shape::Rectangle(V2(), 0, 0, V4(0.4f, 0, 0.0f, 1.0f))
{

}
MarkingRect::~MarkingRect()
{
}

void MarkingRect::Update(EntitySelection *entitySelection, Input *input, Screen * screen, EntitySelection *entitys)
{
	v2 mousePos = input->mousePos;
	if (!mouseCreatesBox)
	{
		if (input->mouse->leftButtonDown)
		{
			mouseCreatesBox = true;
			mouseBoxPos = mousePos;
		}
	}
	else
	{
		Rectangle rect = Rectangle((mouseBoxPos), (mousePos));
		this->pos = rect.pos;
		this->dim.x = rect.dim.x;
		this->dim.y = rect.dim.y;

		if (!input->mouse->leftButtonDown)
		{
			mouseCreatesBox = false;
			entitySelection->Clear();

			v3 camPos = screen->cam.pos;
			float screenWidth = screen->width;
			float screenHeight = screen->height;

			v2 adjustedPos = pos - dim * 0.5f;

			float posXinGame = adjustedPos.x / (float)screen->pixelWidth * screenWidth;
			float posYinGame = adjustedPos.y / (float)screen->pixelHeight * screenHeight;

			float width = dim.x / (float)screen->pixelWidth * screenWidth;
			float height = dim.y / (float)screen->pixelHeight * screenHeight;

			v3 camRectUL = screen->cam.pos + screen->cam.basis.d3 * screen->focalLength * 2.5f - 0.5f * (screenWidth * screen->cam.basis.d1 + screenHeight * screen->cam.basis.d2);

			v3 inGameRectUL = camRectUL + posXinGame * screen->cam.basis.d1 + posYinGame * screen->cam.basis.d2;
			v3 inGameRectUR = inGameRectUL + screen->cam.basis.d1 * width;
			v3 inGameRectBL = inGameRectUL + screen->cam.basis.d2 * height;
			v3 inGameRectBR = inGameRectUL + screen->cam.basis.d1 * width + screen->cam.basis.d2 * height;

			v3 inGamePos = inGameRectUL + 0.5f * (screen->cam.basis.d1 * width + screen->cam.basis.d2 * height);

			float ctUL = -camPos.z / (inGameRectUL.z - camPos.z);
			float ctUR = -camPos.z / (inGameRectUR.z - camPos.z);
			float ctBL = -camPos.z / (inGameRectBL.z - camPos.z);
			float ctBR = -camPos.z / (inGameRectBR.z - camPos.z);
			float ctPos = -camPos.z / (inGamePos.z - camPos.z);

			v2 projRectUL = p12(ctUL * (inGameRectUL - camPos) + camPos);
			v2 projRectUR = p12(ctUR * (inGameRectUR - camPos) + camPos);
			v2 projRectBL = p12(ctBL * (inGameRectBL - camPos) + camPos);
			v2 projRectBR = p12(ctBR * (inGameRectBR - camPos) + camPos);
			v2 projRectPos = p12(ctPos * (inGamePos - camPos) + camPos);

			v2 ULUR = projRectUL - projRectUR;
			v2 PULUR = PerpendicularVector(ULUR);

			v2 ULBL = projRectUL - projRectBL;
			v2 PULBL = PerpendicularVector(ULBL);

			v2 BRUR = projRectBR - projRectUR;
			v2 PBRUR = PerpendicularVector(BRUR);

			v2 BRBL = projRectBR - projRectBL;
			v2 PBRBL = PerpendicularVector(BRBL);

			for (int i=0; i< entitys->amountSelected; i++)
			{
				Entity *entity = entitys->Get(i);

				v2 unitPos = entity->GetPos();

				float up = Dot(PULUR, unitPos - projRectUR);
				float down = Dot(PBRBL, unitPos - projRectBL);
				float left = Dot(PULBL, unitPos - projRectBL);
				float right = Dot(PBRUR, unitPos - projRectUR);

				if (up < 0.0f && down < 0.0f && left > 0.0f && right > 0.0f)
				{
					entitySelection->PushBack(entity);			
				}
			}
			this->pos = v2();
			this->dim.x = 0;
			this->dim.y = 0;

		}
	}
}