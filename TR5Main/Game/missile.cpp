#include "framework.h"
#include "Game/missile.h"

#include "Game/animation.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/effects/effects.h"
#include "Game/Lara/lara.h"
#include "Game/items.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

#define SHARD_DAMAGE 30
#define ROCKET_DAMAGE 100
#define DIVER_HARPOON_DAMAGE 50

#define SHARD_VELOCITY  250
#define ROCKET_VELOCITY 220
#define NATLAGUN_VELOCITY 400

void ShootAtLara(FX_INFO *fx)
{
	int x = LaraItem->Position.xPos - fx->pos.xPos;
	int y = LaraItem->Position.yPos - fx->pos.yPos;
	int z = LaraItem->Position.zPos - fx->pos.zPos;

	auto* bounds = GetBoundsAccurate(LaraItem);
	y += bounds->Y2 + (bounds->Y1 - bounds->Y2) * 3 / 4;

	int distance = sqrt(pow(x, 2) + pow(z, 2));
	fx->pos.xRot = -phd_atan(distance, y);
	fx->pos.yRot = phd_atan(z, x);

	// Random scatter (only a little bit else it's too hard to avoid).
	fx->pos.xRot += (GetRandomControl() - 0x4000) / 0x40;
	fx->pos.yRot += (GetRandomControl() - 0x4000) / 0x40;
}

void ControlMissile(short fxNumber)
{
	auto* fx = &EffectList[fxNumber];
	printf("ControlMissile\n");

	if (fx->objectNumber == ID_SCUBA_HARPOON && !(g_Level.Rooms[fx->roomNumber].flags & 1) && fx->pos.xRot > -0x3000)
		fx->pos.xRot -= ANGLE(1.0f);

	fx->pos.yPos += fx->speed * phd_sin(-fx->pos.xRot);
	int speed = fx->speed * phd_cos(fx->pos.xRot);
	fx->pos.zPos += speed * phd_cos(fx->pos.yRot);
	fx->pos.xPos += speed * phd_sin(fx->pos.yRot);

	auto probe = GetCollisionResult(fx->pos.xPos, fx->pos.yPos, fx->pos.zPos, fx->roomNumber);

	// Check for hitting something.
	if (fx->pos.yPos >= probe.Position.Floor ||
		fx->pos.yPos <= probe.Position.Ceiling)
	{
		if (/*fx->objectNumber == KNIFE ||*/ fx->objectNumber == ID_SCUBA_HARPOON)
		{
			// Change shard into ricochet.
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

	if (probe.RoomNumber != fx->roomNumber)
		EffectNewRoom(fxNumber, probe.RoomNumber);

	// Check for hitting Lara.
	/*if (fx->objectNumber == DRAGON_FIRE)
	{
		if (ItemNearLara(&fx->pos, 350))
		{
			LaraItem->HitPoints -= 3;
			LaraItem->hitStatus = 1;
			LaraBurn(LaraItem);
			return;
		}
	}*/
	else if (ItemNearLara(&fx->pos, 200))
	{
		/*if (fx->objectNumber == KNIFE)
		{
			LaraItem->HitPoints -= KNIFE_DAMAGE;
			SoundEffect(317, &fx->pos, 0);
			KillEffect(fx_number);
		}
		else*/ if (fx->objectNumber == ID_SCUBA_HARPOON)
		{
			LaraItem->HitPoints -= DIVER_HARPOON_DAMAGE;
			SoundEffect(317, &fx->pos, 0);
			KillEffect(fxNumber);
		}

		LaraItem->HitStatus = 1;

		fx->pos.yRot = LaraItem->Position.yRot;
		fx->speed = LaraItem->Velocity;
		fx->frameNumber = fx->counter = 0;
	}

	// Create bubbles in wake of harpoon bolt.
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

void ControlNatlaGun(short fxNumber)
{
	auto* fx = &EffectList[fxNumber];
	auto* object = &Objects[fx->objectNumber];

	fx->frameNumber--;
	if (fx->frameNumber <= Objects[fx->objectNumber].nmeshes)
		KillEffect(fxNumber);

	/* If first frame, then start another explosion at next position */
	if (fx->frameNumber == -1)
	{
		int z = fx->pos.zPos + fx->speed * phd_cos(fx->pos.yRot);
		int x = fx->pos.xPos + fx->speed * phd_sin(fx->pos.yRot);
		int y = fx->pos.yPos;

		auto probe = GetCollisionResult(x, y, z, fx->roomNumber);

		// Don't create one if hit a wall.
		if (y >= probe.Position.Floor ||
			y <= probe.Position.Ceiling)
		{
			return;
		}

		fxNumber = CreateNewEffect(probe.RoomNumber);
		if (fxNumber != NO_ITEM)
		{
			auto* newfx = &EffectList[fxNumber];

			newfx->pos.xPos = x;
			newfx->pos.yPos = y;
			newfx->pos.zPos = z;
			newfx->pos.yRot = fx->pos.yRot;
			newfx->roomNumber = probe.RoomNumber;
			newfx->speed = fx->speed;
			newfx->frameNumber = 0;
			newfx->objectNumber = ID_PROJ_NATLA;
		}
	}
}

short ShardGun(int x, int y, int z, short velocity, short yRot, short roomNumber)
{
	short fxNumber = CreateNewEffect(roomNumber);
	if (fxNumber != NO_ITEM)
	{
		auto* fx = &EffectList[fxNumber];

		fx->pos.xPos = x;
		fx->pos.yPos = y;
		fx->pos.zPos = z;
		fx->roomNumber = roomNumber;
		fx->pos.xRot = fx->pos.zRot = 0;
		fx->pos.yRot = yRot;
		fx->speed = SHARD_VELOCITY;
		fx->frameNumber = 0;
		fx->objectNumber = ID_PROJ_SHARD;
		fx->shade = 14 * 256;
		ShootAtLara(fx);
	}

	return fxNumber;
}

short BombGun(int x, int y, int z, short velocity, short yRot, short roomNumber)
{
	short fxNumber = CreateNewEffect(roomNumber);
	if (fxNumber != NO_ITEM)
	{
		auto* fx = &EffectList[fxNumber];

		fx->pos.xPos = x;
		fx->pos.yPos = y;
		fx->pos.zPos = z;
		fx->roomNumber = roomNumber;
		fx->pos.xRot = fx->pos.zRot = 0;
		fx->pos.yRot = yRot;
		fx->speed = ROCKET_VELOCITY;
		fx->frameNumber = 0;
		fx->objectNumber = ID_PROJ_BOMB;
		fx->shade = 16 * 256;
		ShootAtLara(fx);
	}

	return fxNumber;
}

short NatlaGun(int x, int y, int z, short velocity, short yRot, short roomNumber)
{
	short fxNumber = CreateNewEffect(roomNumber);
	if (fxNumber != NO_ITEM)
	{
		auto* fx = &EffectList[fxNumber];

		fx->pos.xPos = x;
		fx->pos.yPos = y;
		fx->pos.zPos = z;
		fx->roomNumber = roomNumber;
		fx->pos.xRot = fx->pos.zRot = 0;
		fx->pos.yRot = yRot;
		fx->speed = NATLAGUN_VELOCITY;
		fx->frameNumber = 0;
		fx->objectNumber = ID_PROJ_NATLA;
		fx->shade = 16 * 256;
		ShootAtLara(fx);
	}

	return fxNumber;
}
