#include "framework.h"
#include "tr5_ventilator.h"
#include "Game/animation.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Game/Lara/lara.h"
#include "Game/camera.h"
#include "Game/effects/effects.h"
#include "Game/items.h"

static void VentilatorEffect(BOUNDING_BOX* bounds, int intensity, short rot, int speed)
{
	int x, y, z;

	if (abs(intensity) == 1)
	{
		x = (bounds->X1 + bounds->X2) / 2;
		if (intensity >= 0)
			y = bounds->Y2;
		else
			y = bounds->Y1;
		z = (bounds->Z1 + bounds->Z2) / 2;
	}
	else
	{
		y = (bounds->Y1 + bounds->Y2) / 2;
		if (rot & 0x7FFF)
		{
			if (intensity >= 0)
				z = bounds->Z2;
			else
				z = bounds->Z1;
			x = (bounds->X1 + bounds->X2) / 2;
		}
		else
		{
			if (intensity >= 0)
				x = bounds->X2;
			else
				x = bounds->X1;
			z = (bounds->Z1 + bounds->Z2) / 2;
		}
	}

	if (abs(Camera.pos.x - x) <= 7168)
	{
		if (abs(Camera.pos.y - y) <= 7168)
		{
			if (abs(Camera.pos.z - z) <= 7168)
			{
				SPARKS* spark = &Sparks[GetFreeSpark()];

				spark->on = 1;
				spark->sR = 0;
				spark->sG = 0;
				spark->sB = 0;
				spark->dR = spark->dG = (48 * speed) / 128;
				spark->colFadeSpeed = 4;
				spark->fadeToBlack = 8;
				spark->dB = (speed * ((GetRandomControl() & 8) + 48)) / 128;
				spark->transType = TransTypeEnum::COLADD;
				spark->life = spark->sLife = (GetRandomControl() & 3) + 20;

				if (abs(intensity) == 1)
				{
					int factor = 3 * (bounds->X2 - bounds->X1) / 8;
					short angle = 2 * GetRandomControl();

					spark->x = ((bounds->X1 + bounds->X2) / 2) + (GetRandomControl() % factor) * phd_sin(angle);
					spark->z = ((bounds->Z1 + bounds->Z2) / 2) + (GetRandomControl() % factor) * phd_cos(angle);

					if (intensity >= 0)
						spark->y = bounds->Y2;
					else
						spark->y = bounds->Y1;

					spark->zVel = 0;
					spark->xVel = 0;
					spark->yVel = 32 * intensity * ((GetRandomControl() & 0x1F) + 224);
				}
				else
				{
					int factor = 3 * (bounds->Y2 - bounds->Y1) / 8;
					short angle = 2 * GetRandomControl();

					spark->y = (bounds->Y1 + bounds->Y2) / 2;

					if (rot & 0x7FFF)
					{
						if (intensity >= 0)
							spark->z = bounds->Z2;
						else
							spark->z = bounds->Z1;

						spark->x = ((bounds->X1 + bounds->X2) / 2) + (GetRandomControl() % factor) * phd_cos(angle);
						spark->y += (GetRandomControl() % factor) * phd_sin(angle);
						spark->xVel = 0;
						spark->zVel = 16 * intensity * ((GetRandomControl() & 0x1F) + 224);
					}
					else
					{
						if (intensity >= 0)
							spark->x = bounds->X2;
						else
							spark->x = bounds->X1;

						spark->y += (GetRandomControl() % factor) * phd_sin(angle);
						spark->z = ((bounds->Z1 + bounds->Z2) / 2) + (GetRandomControl() % factor) * phd_cos(angle);
						spark->zVel = 0;
						spark->xVel = 16 * intensity * ((GetRandomControl() & 0x1F) + 224);
					}

					spark->yVel = 0;
				}

				spark->friction = 85;
				spark->xVel = (speed * spark->xVel) / 128;
				spark->yVel = (speed * spark->yVel) / 128;
				spark->zVel = (speed * spark->zVel) / 128;
				spark->maxYvel = 0;
				spark->gravity = 0;
				spark->flags = SP_NONE;
			}
		}
	}
}

void InitialiseVentilator(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	item->itemFlags[0] = item->triggerFlags * SECTOR(1);
	if (item->itemFlags[0] < 2048)
		item->itemFlags[0] = 3072;
}

void VentilatorControl(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	AnimateItem(item);

	int xChange = 0;
	int zChange = 0;

	if (TriggerActive(item))
	{
		xChange = 1;
	}
	else
	{
		xChange = 1;
		TestTriggers(item, true);
		if (item->currentAnimState == 1)
		{
			//result = 5 * item->animNumber;
			if (item->frameNumber == g_Level.Anims[item->animNumber].frameEnd)
				return;
		}
		else
		{
			item->goalAnimState = 1;
		}
	}

	int speed = 0;
	if (item->currentAnimState == 1)
	{
		speed = g_Level.Anims[item->animNumber].frameEnd - item->frameNumber;
	}
	else
	{
		speed = 128;
	}

	BOUNDING_BOX* bounds = GetBoundsAccurate(item);
	BOUNDING_BOX effectBounds;

	effectBounds.Y1 = item->pos.yPos + bounds->Y1;
	effectBounds.Y2 = item->pos.yPos + bounds->Y2;

	if (item->objectNumber != ID_PROPELLER_V) // TODO: check this ID
	{
		if (item->pos.yRot != -ANGLE(180.0f))
		{
			if (item->pos.yRot == -ANGLE(90.0f))
			{
				effectBounds.X1 = item->pos.xPos - bounds->Z2;
				effectBounds.X2 = item->pos.xPos - bounds->Z1;
				effectBounds.Z1 = item->pos.zPos + bounds->X1;
				effectBounds.Z2 = item->pos.zPos + bounds->X2;
				xChange = 0;
				zChange = 1;
			}
			else
			{
				if (item->pos.yRot != ANGLE(90.0f))
				{
					effectBounds.X1 = item->pos.xPos + bounds->X1;
					effectBounds.X2 = item->pos.xPos + bounds->X2;
					effectBounds.Z1 = item->pos.zPos + bounds->Z1;
					effectBounds.Z2 = item->pos.zPos + bounds->Z2;
					zChange = 0;
				}
				else
				{
					effectBounds.X1 = item->pos.xPos + bounds->Z1;
					effectBounds.X2 = item->pos.xPos + bounds->Z2;
					effectBounds.Z1 = item->pos.zPos - bounds->X2;
					effectBounds.Z2 = item->pos.zPos - bounds->X1;
					xChange = 0;
					zChange = 1;
				}
			}
		}
		else
		{
			effectBounds.X1 = item->pos.xPos - bounds->X2;
			effectBounds.X2 = item->pos.xPos - bounds->X1;
			effectBounds.Z1 = item->pos.zPos - bounds->Z2;
			effectBounds.Z2 = item->pos.zPos - bounds->Z1;
			zChange = 0;
		}

		VentilatorEffect(&effectBounds, 2, item->pos.yRot, speed);
		VentilatorEffect(&effectBounds, -2, item->pos.yRot, speed);

		if (LaraItem->pos.yPos >= effectBounds.Y1 && LaraItem->pos.yPos <= effectBounds.Y2)
		{
			if (zChange)
			{
				if (LaraItem->pos.xPos >= effectBounds.X1 && LaraItem->pos.xPos <= effectBounds.X2)
				{
					int z1 = abs(LaraItem->pos.zPos - effectBounds.Z1);
					int z2 = abs(LaraItem->pos.zPos - effectBounds.Z2);

					if (z2 >= z1)
						zChange = -zChange;
					else
						z1 = z2;

					if (z1 < item->itemFlags[0])
					{
						int dz = 96 * zChange * (item->itemFlags[0] - z1) / item->itemFlags[0];
						if (item->currentAnimState == 1)
							dz = speed * dz / 120;
						LaraItem->pos.zPos += dz;
					}
				}
			}
			else
			{
				if (LaraItem->pos.zPos >= effectBounds.Z1 && LaraItem->pos.zPos <= effectBounds.Z2)
				{
					int x1 = abs(LaraItem->pos.xPos - effectBounds.X1);
					int x2 = abs(LaraItem->pos.xPos - effectBounds.X2);

					if (x2 >= x1)
						xChange = -xChange;
					else
						x1 = x2;

					if (x1 < item->itemFlags[0])
					{
						int dx = 96 * xChange * (item->itemFlags[0] - x1) / item->itemFlags[0];
						if (item->currentAnimState == 1)
							dx = speed * dx / 120;
						LaraItem->pos.xPos += dx;
					}
				}
			}
		}
	}
	else
	{
		BOUNDING_BOX tbounds;
		phd_RotBoundingBoxNoPersp(&item->pos, bounds,&tbounds);

		effectBounds.X1 = item->pos.xPos + tbounds.X1;
		effectBounds.X2 = item->pos.xPos + tbounds.X2;
		effectBounds.Z1 = item->pos.zPos + tbounds.Z1;
		effectBounds.Z2 = item->pos.zPos + tbounds.Z2;

		VentilatorEffect(&effectBounds, 1, 0, speed);
		VentilatorEffect(&effectBounds, -1, 0, speed);

		if (LaraItem->pos.xPos >= effectBounds.X1 && LaraItem->pos.xPos <= effectBounds.X2)
		{
			if (LaraItem->pos.zPos >= effectBounds.Z1 && LaraItem->pos.zPos <= effectBounds.Z2)
			{
				int y = effectBounds.Y2;

				if (LaraItem->pos.yPos <= effectBounds.Y2)
				{
					if (effectBounds.Y1 - LaraItem->pos.yPos >= item->itemFlags[0])
						return;
					y = 96 * (effectBounds.Y2 - item->itemFlags[0]) / item->itemFlags[0];
				}
				else
				{
					if (LaraItem->pos.yPos - effectBounds.Y2 >= item->itemFlags[0])
						return;
					y = 96 * (item->itemFlags[0] - (LaraItem->pos.yPos - effectBounds.Y2)) / item->itemFlags[0];
				}
				if (item->currentAnimState == 1)
					y = speed * y / 120;
				LaraItem->pos.yPos += y;
			}
		}
	}
}