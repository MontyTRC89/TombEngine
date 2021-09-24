#include "framework.h"
#include "tr5_bodypart.h"
#include "effects\effects.h"
#include "Specific\trmath.h"
#include "Sound\sound.h"
#include "tr5_missile.h"
#include "control/control.h"
#include "items.h"
#include "level.h"
void ControlBodyPart(short fxNumber)
{
	ITEM_INFO* fx = &g_Level.Items[fxNumber];
	FX_INFO* fxInfo = fx->data;
	int x = fx->pos.xPos;
	int y = fx->pos.yPos;
	int z = fx->pos.zPos;

	if (fxInfo->counter <= 0)
	{
		if (fx->speed)
			fx->pos.xRot += 4 * fx->fallspeed;
		fx->fallspeed += 6;
	}
	else
	{
		int modulus = 62 - fxInfo->counter;
		int random = modulus <= 1 ? 0 : 2 * GetRandomControl() % modulus;
		if (fxNumber & 1)
		{
			fx->pos.zRot -= random;
			fx->pos.xRot += random;
		}
		else
		{
			fx->pos.zRot += random;
			fx->pos.xRot -= random;
		}
		if (--fxInfo->counter < 8)
			fx->fallspeed += 2;
	}

	fx->pos.xPos += fx->speed * phd_sin(fx->pos.yRot);
	fx->pos.yPos += fx->fallspeed;
	fx->pos.zPos += fx->speed * phd_cos(fx->pos.yRot);

	short roomNumber = fx->roomNumber;
	FLOOR_INFO* floor = GetFloor(fx->pos.xPos, fx->pos.yPos, fx->pos.zPos, &roomNumber);

	if (!fxInfo->counter)
	{
		int ceiling = GetCeiling(floor, fx->pos.xPos, fx->pos.yPos, fx->pos.zPos);
		if (fx->pos.yPos < ceiling)
		{
			fx->pos.yPos = ceiling;
			fx->fallspeed = -fx->fallspeed;
			fx->speed -= (fx->speed / 8);
		}

		int height = GetFloorHeight(floor, fx->pos.xPos, fx->pos.yPos, fx->pos.zPos);
		if (fx->pos.yPos >= height)
		{
			if (fxInfo->flag2 & 1)
			{
				fx->pos.xPos = x;
				fx->pos.yPos = y;
				fx->pos.zPos = z;

				if (fxInfo->flag2 & 0x200)
					ExplodeFX(fx, -2, 32);
				else
					ExplodeFX(fx, -1, 32);

				KillEffect(fxNumber);
				if (fxInfo->flag2 & 0x800)
					SoundEffect(SFX_TR4_ROCK_FALL_LAND, &fx->pos, 0);
				return;
			}

			if (y <= height)
			{
				if (fx->fallspeed <= 32)
					fx->fallspeed = 0;
				else
					fx->fallspeed = -fx->fallspeed / 4;
			}
			else
			{
				fx->pos.yRot += -ANGLE(180);
				fx->pos.xPos = x;
				fx->pos.zPos = z;
			}

			fx->speed -= (fx->speed / 4);
			if (abs(fx->speed) < 4)
				fx->speed = 0;
			fx->pos.yPos = y;
		}

		if (!fx->speed && ++fxInfo->flag1 > 32)
		{
			KillEffect(fxNumber);
			return;
		}

		if (fxInfo->flag2 & 2 && (GetRandomControl() & 1))
		{
			DoBloodSplat(
				(GetRandomControl() & 0x3F) + fx->pos.xPos - 32,
				(GetRandomControl() & 0x1F) + fx->pos.yPos - 16,
				(GetRandomControl() & 0x3F) + fx->pos.zPos - 32,
				1,
				2 * GetRandomControl(),
				fx->roomNumber);
		}
	}

	if (roomNumber != fx->roomNumber)
		EffectNewRoom(fxNumber, roomNumber);
}