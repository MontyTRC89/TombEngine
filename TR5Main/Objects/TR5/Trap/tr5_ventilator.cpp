#include "framework.h"
#include "tr5_ventilator.h"
#include "draw.h"
#include "level.h"
#include "control.h"
#include "switch.h"
#include "lara.h"
#include "camera.h"
#include "effect2.h"

static void VentilatorEffect(short* bounds, int intensity, short rot, int speed)
{
	int x, y, z;

	if (abs(intensity) == 1)
	{
		x = (bounds[0] + bounds[1]) >> 1;
		if (intensity >= 0)
			y = bounds[3];
		else
			y = bounds[2];
		z = (bounds[4] + bounds[5]) >> 1;
	}
	else
	{
		y = (bounds[2] + bounds[3]) >> 1;
		if (rot & 0x7FFF)
		{
			if (intensity >= 0)
				z = bounds[5];
			else
				z = bounds[4];
			x = (bounds[0] + bounds[1]) >> 1;
		}
		else
		{
			if (intensity >= 0)
				x = bounds[1];
			else
				x = bounds[0];
			z = (bounds[4] + bounds[5]) >> 1;
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
				spark->dR = spark->dG = 48 * speed >> 7;
				spark->colFadeSpeed = 4;
				spark->fadeToBlack = 8;
				spark->dB = speed * ((GetRandomControl() & 8) + 48) >> 7;
				spark->transType = COLADD;
				spark->life = spark->sLife = (GetRandomControl() & 3) + 20;

				if (abs(intensity) == 1)
				{
					int factor = 3 * (bounds[1] - bounds[0]) >> 3;
					short angle = 2 * GetRandomControl();

					spark->x = ((bounds[0] + bounds[1]) >> 1) + ((GetRandomControl() % factor) * phd_sin(angle) >> W2V_SHIFT);
					spark->z = ((bounds[4] + bounds[5]) >> 1) + ((GetRandomControl() % factor) * phd_cos(angle) >> W2V_SHIFT);

					if (intensity >= 0)
						spark->y = bounds[3];
					else
						spark->y = bounds[2];

					spark->zVel = 0;
					spark->xVel = 0;
					spark->yVel = 32 * intensity * ((GetRandomControl() & 0x1F) + 224);
				}
				else
				{
					int factor = 3 * (bounds[3] - bounds[2]) >> 3;
					short angle = 2 * GetRandomControl();

					spark->y = (bounds[2] + bounds[3]) >> 1;

					if (rot & 0x7FFF)
					{
						if (intensity >= 0)
							spark->z = bounds[5];
						else
							spark->z = bounds[4];

						spark->x = ((bounds[0] + bounds[1]) >> 1) + ((GetRandomControl() % factor) * phd_cos(angle) >> W2V_SHIFT);
						spark->y += (GetRandomControl() % factor) * phd_sin(angle) >> W2V_SHIFT;
						spark->xVel = 0;
						spark->zVel = 16 * intensity * ((GetRandomControl() & 0x1F) + 224);
					}
					else
					{
						if (intensity >= 0)
							spark->x = bounds[1];
						else
							spark->x = bounds[0];

						spark->y += (GetRandomControl() % factor) * phd_sin(angle) >> W2V_SHIFT;
						spark->z = ((bounds[4] + bounds[5]) >> 1) + ((GetRandomControl() % factor) * phd_cos(angle) >> W2V_SHIFT);
						spark->zVel = 0;
						spark->xVel = 16 * intensity * ((GetRandomControl() & 0x1F) + 224);
					}

					spark->yVel = 0;
				}

				spark->friction = 85;
				spark->xVel = speed * spark->xVel >> 7;
				spark->yVel = speed * spark->yVel >> 7;
				spark->zVel = speed * spark->zVel >> 7;
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

	item->itemFlags[0] = item->triggerFlags << WALL_SHIFT;
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
		TestTriggersAtXYZ(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, 1, 0);
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

	short* bounds = GetBoundsAccurate(item);
	short effectBounds[6];

	effectBounds[2] = item->pos.yPos + bounds[2];
	effectBounds[3] = item->pos.yPos + bounds[3];

	if (item->objectNumber != ID_PROPELLER_V) // TODO: check this ID
	{
		if (item->pos.yRot != -ANGLE(180.0f))
		{
			if (item->pos.yRot == -ANGLE(90.0f))
			{
				effectBounds[0] = item->pos.xPos - bounds[5];
				effectBounds[1] = item->pos.xPos - bounds[4];
				effectBounds[4] = item->pos.zPos + bounds[0];
				effectBounds[5] = item->pos.zPos + bounds[1];
				xChange = 0;
				zChange = 1;
			}
			else
			{
				if (item->pos.yRot != ANGLE(90.0f))
				{
					effectBounds[0] = item->pos.xPos + bounds[0];
					effectBounds[1] = item->pos.xPos + bounds[1];
					effectBounds[4] = item->pos.zPos + bounds[4];
					effectBounds[5] = item->pos.zPos + bounds[5];
					zChange = 0;
				}
				else
				{
					effectBounds[0] = item->pos.xPos + bounds[4];
					effectBounds[1] = item->pos.xPos + bounds[5];
					effectBounds[4] = item->pos.zPos - bounds[1];
					effectBounds[5] = item->pos.zPos - bounds[0];
					xChange = 0;
					zChange = 1;
				}
			}
		}
		else
		{
			effectBounds[0] = item->pos.xPos - bounds[1];
			effectBounds[1] = item->pos.xPos - bounds[0];
			effectBounds[4] = item->pos.zPos - bounds[5];
			effectBounds[5] = item->pos.zPos - bounds[4];
			zChange = 0;
		}

		VentilatorEffect(effectBounds, 2, item->pos.yRot, speed);
		VentilatorEffect(effectBounds, -2, item->pos.yRot, speed);

		if (LaraItem->pos.yPos >= effectBounds[2] && LaraItem->pos.yPos <= effectBounds[3])
		{
			if (zChange)
			{
				if (LaraItem->pos.xPos >= effectBounds[0] && LaraItem->pos.xPos <= effectBounds[1])
				{
					int z1 = abs(LaraItem->pos.zPos - effectBounds[4]);
					int z2 = abs(LaraItem->pos.zPos - effectBounds[5]);

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
				if (LaraItem->pos.zPos >= effectBounds[4] && LaraItem->pos.zPos <= effectBounds[5])
				{
					int x1 = abs(LaraItem->pos.xPos - effectBounds[0]);
					int x2 = abs(LaraItem->pos.xPos - effectBounds[0]);

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
		short tbounds[6];
		phd_RotBoundingBoxNoPersp(&item->pos, bounds, tbounds);

		effectBounds[0] = item->pos.xPos + tbounds[0];
		effectBounds[1] = item->pos.xPos + tbounds[1];
		effectBounds[4] = item->pos.zPos + tbounds[4];
		effectBounds[5] = item->pos.zPos + tbounds[5];

		VentilatorEffect(effectBounds, 1, 0, speed);
		VentilatorEffect(effectBounds, -1, 0, speed);

		if (LaraItem->pos.xPos >= effectBounds[0] && LaraItem->pos.xPos <= effectBounds[1])
		{
			if (LaraItem->pos.zPos >= effectBounds[4] && LaraItem->pos.zPos <= effectBounds[5])
			{
				int y = effectBounds[3];

				if (LaraItem->pos.yPos <= effectBounds[3])
				{
					if (effectBounds[2] - LaraItem->pos.yPos >= item->itemFlags[0])
						return;
					y = 96 * (effectBounds[3] - item->itemFlags[0]) / item->itemFlags[0];
				}
				else
				{
					if (LaraItem->pos.yPos - effectBounds[3] >= item->itemFlags[0])
						return;
					y = 96 * (item->itemFlags[0] - (LaraItem->pos.yPos - effectBounds[3])) / item->itemFlags[0];
				}
				if (item->currentAnimState == 1)
					y = speed * y / 120;
				LaraItem->pos.yPos += y;
			}
		}
	}
}