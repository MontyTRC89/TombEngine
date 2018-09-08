#include "missile.h"
#include "..\Global\global.h"
#include "..\Game\control.h"
#include "..\Game\sound.h"
#include "..\Game\items.h"
#include "..\Game\effects.h"
#include "..\Game\draw.h"
//#include "..\CustomObjects\frogman.h"
#include <stdio.h>

#define DIVER_HARPOON_DAMAGE 50

void ShootAtLara(FX_INFO *fx)
{
	__int32 x, y, z, distance;
	__int16* bounds;

	x = LaraItem->pos.xPos - fx->pos.xPos;
	y = LaraItem->pos.yPos - fx->pos.yPos;
	z = LaraItem->pos.zPos - fx->pos.zPos;

	bounds = GetBoundsAccurate(LaraItem);
	y += bounds[3] + (bounds[2] - bounds[3]) * 3 / 4;

	distance = SQRT_ASM(SQUARE(x) + SQUARE(z));
	fx->pos.xRot = -ATAN(distance, y);
	fx->pos.yRot = ATAN(z, x);

	/* Random scatter (only a little bit else it's too hard to avoid) */
	fx->pos.xRot += (GetRandomControl() - 0x4000) / 0x40;
	fx->pos.yRot += (GetRandomControl() - 0x4000) / 0x40;
}

void ControlMissile(__int16 fxNumber)
{
	FX_INFO *fx;
	FLOOR_INFO *floor;
	__int16 roomNumber;
	__int32 speed;

	fx = &Effects[fxNumber];
	printf("ControlMissile\n", fx->objectNumber);

	if (fx->objectNumber == ID_SCUBA_HARPOON && !(Rooms[fx->roomNumber].flags & 1) && fx->pos.xRot > -0x3000)
		fx->pos.xRot -= ONE_DEGREE;

	fx->pos.yPos += (fx->speed * SIN(-fx->pos.xRot) >> W2V_SHIFT);
	speed = fx->speed * COS(fx->pos.xRot) >> W2V_SHIFT;
	fx->pos.zPos += (speed * COS(fx->pos.yRot) >> W2V_SHIFT);
	fx->pos.xPos += (speed * SIN(fx->pos.yRot) >> W2V_SHIFT);
	roomNumber = fx->roomNumber;
	floor = GetFloor(fx->pos.xPos, fx->pos.yPos, fx->pos.zPos, &roomNumber);

	/* Check for hitting something */
	if (fx->pos.yPos >= TrGetHeight(floor, fx->pos.xPos, fx->pos.yPos, fx->pos.zPos) ||
		fx->pos.yPos <= GetCeiling(floor, fx->pos.xPos, fx->pos.yPos, fx->pos.zPos))
	{
		if (/*fx->objectNumber == KNIFE ||*/ fx->objectNumber == ID_SCUBA_HARPOON)
		{
			/* Change shard into ricochet */
			//			fx->speed = 0;
			//			fx->frameNumber = -GetRandomControl()/11000;
			//			fx->counter = 6;
			//			fx->objectNumber = RICOCHET1;
			SoundEffect((fx->objectNumber == ID_SCUBA_HARPOON) ? 10 : 258, &fx->pos, 0);
		}
		/*else if (fx->objectNumber == DRAGON_FIRE)
		{
			AddDynamicLight(fx->pos.xPos, fx->pos.yPos, fx->pos.zPos, 14, 11);
			KillEffect(fx_number);
		}*/
		return;
	}

	if (roomNumber != fx->roomNumber)
		EffectNewRoom(fxNumber, roomNumber);

	/* Check for hitting Lara */
	/*if (fx->objectNumber == DRAGON_FIRE)
	{
		if (ItemNearLara(&fx->pos, 350))
		{
			LaraItem->hitPoints -= 3;
			LaraItem->hitStatus = 1;
			LaraBurn();
			return;
		}
	}*/
	else if (ItemNearLara(&fx->pos, 200))
	{
		/*if (fx->objectNumber == KNIFE)
		{
			LaraItem->hitPoints -= KNIFE_DAMAGE;
			SoundEffect(317, &fx->pos, 0);
			KillEffect(fx_number);
		}
		else*/ if (fx->objectNumber == ID_SCUBA_HARPOON)
		{
			LaraItem->hitPoints -= DIVER_HARPOON_DAMAGE;
			SoundEffect(317, &fx->pos, 0);
			KillEffect(fxNumber);
		}
	LaraItem->hitStatus = 1;

		fx->pos.yRot = LaraItem->pos.yRot;
		fx->speed = LaraItem->speed;
		fx->frameNumber = fx->counter = 0;
	}

	printf("Bubbles\n");
	printf("Num: %d\n", fx->objectNumber);

	/* Create bubbles in wake of harpoon bolt */
	if (fx->objectNumber == ID_SCUBA_HARPOON && Rooms[fx->roomNumber].flags & 1)
		CreateBubble(&fx->pos, fx->roomNumber);
	/*else if (fx->objectNumber == DRAGON_FIRE && !fx->counter--)
	{
		AddDynamicLight(fx->pos.xPos, fx->pos.yPos, fx->pos.zPos, 14, 11);
		SoundEffect(305, &fx->pos, 0);
		KillEffect(fx_number);
	}
	else if (fx->objectNumber == KNIFE)
		fx->pos.zRot += 30 * ONE_DEGREE;*/
}