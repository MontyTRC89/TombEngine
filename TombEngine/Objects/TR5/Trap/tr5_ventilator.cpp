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

	if (abs(Camera.pos.x - x) <= SECTOR(7))
	{
		if (abs(Camera.pos.y - y) <= SECTOR(7))
		{
			if (abs(Camera.pos.z - z) <= SECTOR(7))
			{
				auto* spark = GetFreeParticle();

				spark->on = 1;
				spark->sR = 0;
				spark->sG = 0;
				spark->sB = 0;
				spark->dR = spark->dG = (48 * speed) / 128;
				spark->colFadeSpeed = 4;
				spark->fadeToBlack = 8;
				spark->dB = (speed * ((GetRandomControl() & 8) + 48)) / 128;
				spark->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
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
	auto* item = &g_Level.Items[itemNumber];

	item->ItemFlags[0] = item->TriggerFlags * SECTOR(1);
	if (item->ItemFlags[0] < 2048)
		item->ItemFlags[0] = 3072;
}

void VentilatorControl(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	AnimateItem(item);

	int xChange = 0;
	int zChange = 0;

	if (TriggerActive(item))
		xChange = 1;
	else
	{
		xChange = 1;
		TestTriggers(item, true);
		if (item->Animation.ActiveState == 1)
		{
			//result = 5 * item->animNumber;
			if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameEnd)
				return;
		}
		else
			item->Animation.TargetState = 1;
	}

	int speed = 0;
	if (item->Animation.ActiveState == 1)
		speed = g_Level.Anims[item->Animation.AnimNumber].frameEnd - item->Animation.FrameNumber;
	else
		speed = 128;

	auto* bounds = GetBoundsAccurate(item);
	BOUNDING_BOX effectBounds;

	effectBounds.Y1 = item->Pose.Position.y + bounds->Y1;
	effectBounds.Y2 = item->Pose.Position.y + bounds->Y2;

	if (item->ObjectNumber != ID_PROPELLER_V) // TODO: check this ID
	{
		if (item->Pose.Orientation.y != -ANGLE(180.0f))
		{
			if (item->Pose.Orientation.y == -ANGLE(90.0f))
			{
				effectBounds.X1 = item->Pose.Position.x - bounds->Z2;
				effectBounds.X2 = item->Pose.Position.x - bounds->Z1;
				effectBounds.Z1 = item->Pose.Position.z + bounds->X1;
				effectBounds.Z2 = item->Pose.Position.z + bounds->X2;
				xChange = 0;
				zChange = 1;
			}
			else
			{
				if (item->Pose.Orientation.y != ANGLE(90.0f))
				{
					effectBounds.X1 = item->Pose.Position.x + bounds->X1;
					effectBounds.X2 = item->Pose.Position.x + bounds->X2;
					effectBounds.Z1 = item->Pose.Position.z + bounds->Z1;
					effectBounds.Z2 = item->Pose.Position.z + bounds->Z2;
					zChange = 0;
				}
				else
				{
					effectBounds.X1 = item->Pose.Position.x + bounds->Z1;
					effectBounds.X2 = item->Pose.Position.x + bounds->Z2;
					effectBounds.Z1 = item->Pose.Position.z - bounds->X2;
					effectBounds.Z2 = item->Pose.Position.z - bounds->X1;
					xChange = 0;
					zChange = 1;
				}
			}
		}
		else
		{
			effectBounds.X1 = item->Pose.Position.x - bounds->X2;
			effectBounds.X2 = item->Pose.Position.x - bounds->X1;
			effectBounds.Z1 = item->Pose.Position.z - bounds->Z2;
			effectBounds.Z2 = item->Pose.Position.z - bounds->Z1;
			zChange = 0;
		}

		VentilatorEffect(&effectBounds, 2, item->Pose.Orientation.y, speed);
		VentilatorEffect(&effectBounds, -2, item->Pose.Orientation.y, speed);

		if (LaraItem->Pose.Position.y >= effectBounds.Y1 && LaraItem->Pose.Position.y <= effectBounds.Y2)
		{
			if (zChange)
			{
				if (LaraItem->Pose.Position.x >= effectBounds.X1 && LaraItem->Pose.Position.x <= effectBounds.X2)
				{
					int z1 = abs(LaraItem->Pose.Position.z - effectBounds.Z1);
					int z2 = abs(LaraItem->Pose.Position.z - effectBounds.Z2);

					if (z2 >= z1)
						zChange = -zChange;
					else
						z1 = z2;

					if (z1 < item->ItemFlags[0])
					{
						int dz = 96 * zChange * (item->ItemFlags[0] - z1) / item->ItemFlags[0];

						if (item->Animation.ActiveState == 1)
							dz = speed * dz / 120;

						LaraItem->Pose.Position.z += dz;
					}
				}
			}
			else
			{
				if (LaraItem->Pose.Position.z >= effectBounds.Z1 && LaraItem->Pose.Position.z <= effectBounds.Z2)
				{
					int x1 = abs(LaraItem->Pose.Position.x - effectBounds.X1);
					int x2 = abs(LaraItem->Pose.Position.x - effectBounds.X2);

					if (x2 >= x1)
						xChange = -xChange;
					else
						x1 = x2;

					if (x1 < item->ItemFlags[0])
					{
						int dx = 96 * xChange * (item->ItemFlags[0] - x1) / item->ItemFlags[0];

						if (item->Animation.ActiveState == 1)
							dx = speed * dx / 120;

						LaraItem->Pose.Position.x += dx;
					}
				}
			}
		}
	}
	else
	{
		BOUNDING_BOX tbounds;
		phd_RotBoundingBoxNoPersp(&item->Pose, bounds, &tbounds);

		effectBounds.X1 = item->Pose.Position.x + tbounds.X1;
		effectBounds.X2 = item->Pose.Position.x + tbounds.X2;
		effectBounds.Z1 = item->Pose.Position.z + tbounds.Z1;
		effectBounds.Z2 = item->Pose.Position.z + tbounds.Z2;

		VentilatorEffect(&effectBounds, 1, 0, speed);
		VentilatorEffect(&effectBounds, -1, 0, speed);

		if (LaraItem->Pose.Position.x >= effectBounds.X1 &&
			LaraItem->Pose.Position.x <= effectBounds.X2)
		{
			if (LaraItem->Pose.Position.z >= effectBounds.Z1 &&
				LaraItem->Pose.Position.z <= effectBounds.Z2)
			{
				int y = effectBounds.Y2;

				if (LaraItem->Pose.Position.y <= effectBounds.Y2)
				{
					if (effectBounds.Y1 - LaraItem->Pose.Position.y >= item->ItemFlags[0])
						return;

					y = 96 * (effectBounds.Y2 - item->ItemFlags[0]) / item->ItemFlags[0];
				}
				else
				{
					if (LaraItem->Pose.Position.y - effectBounds.Y2 >= item->ItemFlags[0])
						return;

					y = 96 * (item->ItemFlags[0] - (LaraItem->Pose.Position.y - effectBounds.Y2)) / item->ItemFlags[0];
				}

				if (item->Animation.ActiveState == 1)
					y = speed * y / 120;

				LaraItem->Pose.Position.y += y;
			}
		}
	}
}
