#include "framework.h"
#include "Game/missile.h"

#include "Game/animation.h"
#include "Game/collision/collide_item.h"
#include "Game/effects/effects.h"
#include "Game/Lara/lara.h"
#include "Game/items.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

#define SHARD_DAMAGE 30
#define ROCKET_DAMAGE 100
#define DIVER_HARPOON_DAMAGE 50

#define SHARD_SPEED  250
#define ROCKET_SPEED 220
#define NATLAGUN_SPEED 400

void ShootAtLara(FX_INFO *fx)
{
	int x, y, z, distance;
	BOUNDING_BOX* bounds;

	x = LaraItem->pos.xPos - fx->pos.xPos;
	y = LaraItem->pos.yPos - fx->pos.yPos;
	z = LaraItem->pos.zPos - fx->pos.zPos;

	bounds = GetBoundsAccurate(LaraItem);
	y += bounds->Y2 + (bounds->Y1 - bounds->Y2) * 3 / 4;

	distance = sqrt(SQUARE(x) + SQUARE(z));
	fx->pos.xRot = -phd_atan(distance, y);
	fx->pos.yRot = phd_atan(z, x);

	/* Random scatter (only a little bit else it's too hard to avoid) */
	fx->pos.xRot += (GetRandomControl() - 0x4000) / 0x40;
	fx->pos.yRot += (GetRandomControl() - 0x4000) / 0x40;
}

void ControlMissile(short fxNumber)
{
	FX_INFO *fx;
	FLOOR_INFO *floor;
	short roomNumber;
	int speed;

	fx = &EffectList[fxNumber];
	printf("ControlMissile\n");

	if (fx->objectNumber == ID_SCUBA_HARPOON && !(g_Level.Rooms[fx->roomNumber].flags & 1) && fx->pos.xRot > -0x3000)
		fx->pos.xRot -= ONE_DEGREE;

	fx->pos.yPos += fx->speed * phd_sin(-fx->pos.xRot);
	speed = fx->speed * phd_cos(fx->pos.xRot);
	fx->pos.zPos += speed * phd_cos(fx->pos.yRot);
	fx->pos.xPos += speed * phd_sin(fx->pos.yRot);
	roomNumber = fx->roomNumber;
	floor = GetFloor(fx->pos.xPos, fx->pos.yPos, fx->pos.zPos, &roomNumber);

	/* Check for hitting something */
	if (fx->pos.yPos >= GetFloorHeight(floor, fx->pos.xPos, fx->pos.yPos, fx->pos.zPos) ||
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
			LaraBurn(LaraItem);
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
		fx->speed = LaraItem->Velocity;
		fx->frameNumber = fx->counter = 0;
	}

	/* Create bubbles in wake of harpoon bolt */
	//if (fx->objectNumber == ID_SCUBA_HARPOON && g_Level.Rooms[fx->roomNumber].flags & 1)
	//	CreateBubble(&fx->pos, fx->roomNumber, 1, 0);
	/*else if (fx->objectNumber == DRAGON_FIRE && !fx->counter--)
	{
		AddDynamicLight(fx->pos.xPos, fx->pos.yPos, fx->pos.zPos, 14, 11);
		SoundEffect(305, &fx->pos, 0);
		KillEffect(fx_number);
	}
	else if (fx->objectNumber == KNIFE)
		fx->pos.zRot += 30 * ONE_DEGREE;*/
}

void ControlNatlaGun(short fx_number)
{
	FX_INFO* fx, *newfx;
	OBJECT_INFO* object;
	FLOOR_INFO* floor;
	short roomNumber;
	int x, y, z;

	fx = &EffectList[fx_number];
	object = &Objects[fx->objectNumber];
	fx->frameNumber--;
	if (fx->frameNumber <= Objects[fx->objectNumber].nmeshes)
		KillEffect(fx_number);

	/* If first frame, then start another explosion at next position */
	if (fx->frameNumber == -1)
	{
		z = fx->pos.zPos + fx->speed * phd_cos(fx->pos.yRot);
		x = fx->pos.xPos + fx->speed * phd_sin(fx->pos.yRot);
		y = fx->pos.yPos;
		roomNumber = fx->roomNumber;
		floor = GetFloor(x, y, z, &roomNumber);

		/* Don't create one if hit a wall */
		if (y >= GetFloorHeight(floor, x, y, z) || y <= GetCeiling(floor, x, y, z))
			return;

		fx_number = CreateNewEffect(roomNumber);
		if (fx_number != NO_ITEM)
		{
			newfx = &EffectList[fx_number];
			newfx->pos.xPos = x;
			newfx->pos.yPos = y;
			newfx->pos.zPos = z;
			newfx->pos.yRot = fx->pos.yRot;
			newfx->roomNumber = roomNumber;
			newfx->speed = fx->speed;
			newfx->frameNumber = 0;
			newfx->objectNumber = ID_PROJ_NATLA;
		}
	}
}

short ShardGun(int x, int y, int z, short speed, short yrot, short roomNumber)
{
	short fx_number;
	FX_INFO* fx;

	fx_number = CreateNewEffect(roomNumber);
	if (fx_number != NO_ITEM)
	{
		fx = &EffectList[fx_number];
		fx->pos.xPos = x;
		fx->pos.yPos = y;
		fx->pos.zPos = z;
		fx->roomNumber = roomNumber;
		fx->pos.xRot = fx->pos.zRot = 0;
		fx->pos.yRot = yrot;
		fx->speed = SHARD_SPEED;
		fx->frameNumber = 0;
		fx->objectNumber = ID_PROJ_SHARD;
		fx->shade = 14 * 256;
		ShootAtLara(fx);
	}

	return (fx_number);
}

short BombGun(int x, int y, int z, short speed, short yrot, short roomNumber)
{
	short fx_number;
	FX_INFO* fx;

	fx_number = CreateNewEffect(roomNumber);
	if (fx_number != NO_ITEM)
	{
		fx = &EffectList[fx_number];
		fx->pos.xPos = x;
		fx->pos.yPos = y;
		fx->pos.zPos = z;
		fx->roomNumber = roomNumber;
		fx->pos.xRot = fx->pos.zRot = 0;
		fx->pos.yRot = yrot;
		fx->speed = ROCKET_SPEED;
		fx->frameNumber = 0;
		fx->objectNumber = ID_PROJ_BOMB;
		fx->shade = 16 * 256;
		ShootAtLara(fx);
	}

	return (fx_number);
}

short NatlaGun(int x, int y, int z, short speed, short yrot, short roomNumber)
{
	short fx_number;
	FX_INFO* fx;

	fx_number = CreateNewEffect(roomNumber);
	if (fx_number != NO_ITEM)
	{
		fx = &EffectList[fx_number];
		fx->pos.xPos = x;
		fx->pos.yPos = y;
		fx->pos.zPos = z;
		fx->roomNumber = roomNumber;
		fx->pos.xRot = fx->pos.zRot = 0;
		fx->pos.yRot = yrot;
		fx->speed = NATLAGUN_SPEED;
		fx->frameNumber = 0;
		fx->objectNumber = ID_PROJ_NATLA;
		fx->shade = 16 * 256;
		ShootAtLara(fx);
	}

	return (fx_number);
}